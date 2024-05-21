
#include "asset_libraries.hpp"
#include "asset_state.hpp"
#include "asset_traits.hpp"
#include "async/thread_pool.hpp"
#include "io/error.hpp"
#include "msh/flat_model.hpp"
#include "odf/definition.hpp"
#include "output_stream.hpp"
#include "utility/file_watcher.hpp"
#include "utility/stopwatch.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <stop_token>
#include <string_view>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

using namespace std::literals;

namespace we::assets {

/// @brief How many times to try reloading an asset that failed loading because of a sharing violation.
constexpr int max_asset_load_attempts = 20;
constexpr std::chrono::milliseconds asset_retry_load_delay = 50ms;

namespace {

const absl::flat_hash_set<std::wstring_view> ignored_folders =
   {L"_BUILD"sv, L"_LVL_PC"sv, L"_LVL_PS2"sv, L"_LVL_PSP"sv,  L"_LVL_XBOX"sv,

    L".git"sv,   L".svn"sv,    L".vscode"sv,  L".WorldEdit"sv};

auto try_get_last_write_time(const std::filesystem::path& asset_path) noexcept -> uint64
{
   try {
      return std::filesystem::last_write_time(asset_path).time_since_epoch().count();
   }
   catch (std::filesystem::filesystem_error&) {
      return 0;
   }
}

bool is_parent_path_ignored(const std::filesystem::path& asset_path) noexcept
{
   for (const auto& part : asset_path) {
      if (ignored_folders.contains(part.native())) return true;
   }

   return false;
}

}

template<typename T>
struct library<T>::impl {
   impl(output_stream& stream, std::shared_ptr<async::thread_pool> thread_pool)
      : _output_stream{stream}, _thread_pool{std::move(thread_pool)}
   {
   }

   void add(const std::filesystem::path& unpreferred_asset_path,
            uint64 last_write_time) noexcept
   {
      std::filesystem::path asset_path =
         unpreferred_asset_path; // makes for prettier output messages

      asset_path.make_preferred();

      const lowercase_string name{asset_path.stem().string()};

      auto new_state = make_asset_state(name, asset_path, last_write_time);

      auto [state_pair, inserted] = [&] {
         std::scoped_lock lock{_assets_mutex};

         return _assets.emplace(name, new_state);
      }();

      auto state = state_pair->second;

      if (not inserted) {
         std::scoped_lock state_lock{state->mutex, _existing_assets_mutex};

         if (not state->exists) {
            _existing_assets.emplace_back(name);
            _existing_assets_sorted = false;
         }

         state->exists = true;
         state->load_failure = false;
         state->path = asset_path;
         state->start_load = [this, name] { enqueue_create_asset(name, false); };
         state->last_write_time.store(last_write_time, std::memory_order_relaxed);
      }
      else {
         std::scoped_lock lock{_existing_assets_mutex};

         _existing_assets.emplace_back(name);
         _existing_assets_sorted = false;
      }

      if (state->ref_count.load(std::memory_order_relaxed) > 0) {
         enqueue_create_asset(name, true);
      }

      _change_event.broadcast(name);
   }

   void remove(const std::filesystem::path& unpreferred_asset_path) noexcept
   {
      std::filesystem::path asset_path =
         unpreferred_asset_path; // makes for prettier output messages

      asset_path.make_preferred();

      const lowercase_string name{asset_path.stem().string()};

      const std::shared_ptr<asset_state<T>> asset_state = [&] {
         std::scoped_lock lock{_assets_mutex};

         auto it = _assets.find(name);

         return it != _assets.end() ? it->second : nullptr;
      }();

      if (not asset_state) return;

      // Remove Asset
      {
         std::scoped_lock lock{_assets_mutex, _load_tasks_mutex,
                               _existing_assets_mutex, asset_state->mutex};

         if (asset_state->path != asset_path) return;

         _assets.erase(name);

         _load_tasks.erase(name);

         std::erase_if(_existing_assets,
                       [&](const stable_string& asset) { return asset == name; });
      }

      _change_event.broadcast(name);
   }

   auto operator[](const lowercase_string& name) noexcept -> asset_ref<T>
   {
      if (name.empty()) return asset_ref{_null_asset};

      // Take a shared_lock to try and get an already existing asset state.
      {
         std::shared_lock lock{_assets_mutex};

         if (auto asset = _assets.find(name); asset != _assets.end()) {
            return asset_ref{asset->second};
         }
      }

      auto placeholder_state = make_placeholder_asset_state();

      std::lock_guard lock{_assets_mutex};

      auto [state_pair, inserted] = _assets.emplace(name, placeholder_state);

      return state_pair->second;
   }

   auto listen_for_loads(
      std::move_only_function<void(const lowercase_string& name,
                                   asset_ref<T> asset, asset_data<T> data) const>
         callback) noexcept
      -> event_listener<void(const lowercase_string&, asset_ref<T>, asset_data<T>)>
   {
      return _load_event.listen(std::move(callback));
   }

   auto listen_for_load_failures(
      std::move_only_function<void(const lowercase_string& name, asset_ref<T> asset) const> callback) noexcept
      -> event_listener<void(const lowercase_string&, asset_ref<T>)>
   {
      return _load_failed_event.listen(std::move(callback));
   }

   auto listen_for_changes(
      std::move_only_function<void(const lowercase_string& name) const> callback) noexcept
      -> event_listener<void(const lowercase_string&)>
   {
      return _change_event.listen(std::move(callback));
   }

   void update_loaded() noexcept
   {
   restart_loop:
      std::unique_lock lock{_load_tasks_mutex};

      for (auto& [task_name, task] : _load_tasks) {
         if (not task.ready()) continue;

         asset_data<T> asset_data = task.get();
         lowercase_string name = task_name;

         _load_tasks.erase(name);
         lock.unlock(); // After thise line accessing _load_tasks is no longer safe.

         asset_ref<T> asset;
         std::shared_ptr<asset_state<T>> asset_state;

         // Get the asset_state.
         {
            std::scoped_lock assets_lock{_assets_mutex};

            asset_state = _assets[name];
         }

         // Update the asset data.
         {
            std::scoped_lock asset_lock{asset_state->mutex};

            asset_state->data = asset_data;
         }

         asset = asset_ref<T>{asset_state};

         if (asset_data != nullptr) {
            _load_event.broadcast(name, asset, asset_data);
         }
         else {
            asset_state->load_failure = true;
         }

         goto restart_loop;
      }
   }

   void clear() noexcept
   {
      std::scoped_lock lock{_assets_mutex, _load_tasks_mutex, _existing_assets_mutex};

      _load_tasks.clear();
      _assets.clear();
      _existing_assets.clear();
      _existing_assets_sorted = true;
   }

   void view_existing(
      function_ptr<void(const std::span<const stable_string> assets) noexcept> callback) noexcept
   {
      if (not _existing_assets_sorted.exchange(true)) {
         std::scoped_lock lock{_existing_assets_mutex};

         std::sort(_existing_assets.begin(), _existing_assets.end());
      }

      std::shared_lock lock{_existing_assets_mutex};

      callback(_existing_assets);
   }

   auto query_path(const lowercase_string& name) noexcept -> std::filesystem::path
   {
      std::shared_lock lock{_assets_mutex};

      auto it = _assets.find(name);

      if (it == _assets.end()) return {};

      asset_state<T>& state = *it->second;

      std::shared_lock state_lock{state.mutex};

      return state.path;
   }

   auto query_last_write_time(const lowercase_string& name) noexcept -> uint64
   {
      std::shared_lock lock{_assets_mutex};

      auto it = _assets.find(name);

      if (it == _assets.end()) return 0;

      asset_state<T>& state = *it->second;

      return state.last_write_time.load(std::memory_order_relaxed);
   }

private:
   auto make_asset_state(const lowercase_string& name,
                         const std::filesystem::path& asset_path,
                         uint64 last_write_time) -> std::shared_ptr<asset_state<T>>
   {
      return std::make_shared<asset_state<T>>(
         std::weak_ptr<T>{}, not asset_path.empty(), asset_path,
         [this, name = name] { enqueue_create_asset(name, false); }, last_write_time);
   }

   auto make_placeholder_asset_state() -> std::shared_ptr<asset_state<T>>
   {
      return std::make_shared<asset_state<T>>(
         std::weak_ptr<T>{}, false, std::filesystem::path{}, [] {}, 0);
   }

   void enqueue_create_asset(lowercase_string name, bool preempt_current_load) noexcept
   {
      using namespace std::literals;

      auto asset = [&] {
         std::scoped_lock lock{_assets_mutex};

         return _assets.at(name);
      }();

      // Do not try reload assets that previously failed loading.
      if (asset->load_failure) return;

      std::filesystem::path asset_path = [&] {
         std::shared_lock asset_lock{asset->mutex};

         return asset->path;
      }();

      std::scoped_lock tasks_lock{_load_tasks_mutex};

      if (preempt_current_load) {
         if (auto inprogress_load = _load_tasks.find(name);
             inprogress_load != _load_tasks.end()) {
            auto& [_, load_task] = *inprogress_load;

            load_task.cancel();

            _load_tasks.erase(inprogress_load);
         }
      }
      else if (_load_tasks.contains(name)) {
         return;
      }

      _load_tasks[name] = _thread_pool->exec(
         async::task_priority::low,
         [this, asset_path = std::move(asset_path), asset, name]() -> asset_data<T> {
            try {
               for (int load_attempt = 0;; ++load_attempt) {
                  try {
                     utility::stopwatch load_timer;

                     auto asset_data =
                        std::make_shared<const T>(asset_traits<T>::load(asset_path));

                     _output_stream.write("Loaded asset '{}'\n   Time Taken: {:f}ms\n"sv,
                                          asset_path.string(),
                                          load_timer.elapsed_ms());

                     return asset_data;
                  }
                  catch (io::open_error& e) {
                     if (e.code() == io::open_error_code::sharing_violation) {
                        if (load_attempt >= max_asset_load_attempts) throw;

                        std::this_thread::sleep_for(asset_retry_load_delay);
                     }
                     else {
                        throw;
                     }
                  }
               }
            }
            catch (std::exception& e) {
               _output_stream.write("Error while loading asset:\n   File: {}\n   Message: \n{}\n"sv,
                                    asset_path.string(),
                                    string::indent(2, e.what()));

               _load_failed_event.broadcast(name, asset);

               return nullptr;
            }
         });
   }

   we::output_stream& _output_stream;

   std::shared_mutex _assets_mutex;
   absl::flat_hash_map<lowercase_string, std::shared_ptr<asset_state<T>>> _assets; // guarded by _assets_mutex

   std::shared_mutex _load_tasks_mutex;
   absl::flat_hash_map<lowercase_string, async::task<asset_data<T>>> _load_tasks; // guarded by _load_tasks_mutex

   std::shared_mutex _existing_assets_mutex;
   std::vector<stable_string> _existing_assets;
   std::atomic_bool _existing_assets_sorted = true;

   std::shared_ptr<async::thread_pool> _thread_pool;

   const std::shared_ptr<asset_state<T>> _null_asset = make_placeholder_asset_state();

   utility::event<void(const lowercase_string&, asset_ref<T>, asset_data<T>)> _load_event;
   utility::event<void(const lowercase_string&, asset_ref<T>)> _load_failed_event;
   utility::event<void(const lowercase_string&)> _change_event;
};

template<typename T>
library<T>::library(output_stream& stream, std::shared_ptr<async::thread_pool> thread_pool)
   : self{stream, std::move(thread_pool)}
{
}

template<typename T>
void library<T>::add(const std::filesystem::path& asset_path, uint64 last_write_time) noexcept
{
   self->add(asset_path, last_write_time);
}

template<typename T>
void library<T>::remove(const std::filesystem::path& asset_path) noexcept
{
   self->remove(asset_path);
}

template<typename T>
auto library<T>::operator[](const lowercase_string& name) noexcept -> asset_ref<T>
{
   return self.get()[name];
}

template<typename T>
auto library<T>::listen_for_loads(
   std::move_only_function<void(const lowercase_string& name, asset_ref<T> asset, asset_data<T> data) const>
      callback) noexcept
   -> event_listener<void(const lowercase_string&, asset_ref<T>, asset_data<T>)>
{
   return self->listen_for_loads(std::move(callback));
}

template<typename T>
auto library<T>::listen_for_load_failures(
   std::move_only_function<void(const lowercase_string& name, asset_ref<T> asset) const> callback) noexcept
   -> event_listener<void(const lowercase_string&, asset_ref<T>)>
{
   return self->listen_for_load_failures(std::move(callback));
}
template<typename T>
auto library<T>::listen_for_changes(
   std::move_only_function<void(const lowercase_string& name) const> callback) noexcept
   -> event_listener<void(const lowercase_string&)>
{
   return self->listen_for_changes(std::move(callback));
}

template<typename T>
void library<T>::update_loaded() noexcept
{
   self->update_loaded();
}

template<typename T>
void library<T>::clear() noexcept
{
   self->clear();
}

template<typename T>
void library<T>::view_existing(
   function_ptr<void(const std::span<const stable_string> assets) noexcept> callback) noexcept
{
   self->view_existing(callback);
}

template<typename T>
auto library<T>::query_path(const lowercase_string& name) noexcept
   -> std::filesystem::path
{
   return self->query_path(name);
}

template<typename T>
auto library<T>::query_last_write_time(const lowercase_string& name) noexcept -> uint64
{
   return self->query_last_write_time(name);
}

template struct library<odf::definition>;
template struct library<msh::flat_model>;
template struct library<texture::texture>;
template struct library<sky::config>;

libraries_manager::libraries_manager(output_stream& stream,
                                     std::shared_ptr<async::thread_pool> thread_pool) noexcept
   : odfs{stream, thread_pool},
     models{stream, thread_pool},
     textures{stream, thread_pool},
     skies{stream, thread_pool}
{
}

libraries_manager::~libraries_manager() = default;

void libraries_manager::source_directory(const std::filesystem::path& source_directory) noexcept
{
   clear();

   for (auto entry =
           std::filesystem::recursive_directory_iterator{
              source_directory,
              std::filesystem::directory_options::follow_directory_symlink |
                 std::filesystem::directory_options::skip_permission_denied};
        entry != std::filesystem::end(entry); ++entry) {
      const auto& path = entry->path();

      if (ignored_folders.contains((--path.end())->native())) {
         entry.disable_recursion_pending();

         continue;
      }

      register_asset(path, entry->last_write_time().time_since_epoch().count());
   }

   _file_watcher = std::make_unique<utility::file_watcher>(source_directory);
   _file_changed_event =
      _file_watcher->listen_file_changed([this](const std::filesystem::path& path) {
         if (is_parent_path_ignored(path)) return;

         register_asset(path, try_get_last_write_time(path));
      });
   _file_removed_event =
      _file_watcher->listen_file_removed([this](const std::filesystem::path& path) {
         if (is_parent_path_ignored(path)) return;

         forget_asset(path);
      });
   _unknown_files_changed = _file_watcher->listen_unknown_files_changed([this]() {
      // TODO: manual scan here.
   });
}

void libraries_manager::update_loaded() noexcept
{
   odfs.update_loaded();
   models.update_loaded();
   textures.update_loaded();
   skies.update_loaded();
}

void libraries_manager::clear() noexcept
{
   odfs.clear();
   models.clear();
   textures.clear();
   skies.clear();
}

void libraries_manager::register_asset(const std::filesystem::path& path,
                                       uint64 last_write_time) noexcept
{
   if (const auto extension = path.extension();
       string::iequals(extension.native(), L".odf"sv)) {
      odfs.add(path, last_write_time);
   }
   else if (string::iequals(extension.native(), L".msh"sv)) {
      models.add(path, last_write_time);
   }
   else if (string::iequals(extension.native(), L".tga"sv)) {
      textures.add(path, last_write_time);
   }
   else if (string::iequals(extension.native(), L".sky"sv)) {
      skies.add(path, last_write_time);
   }
}

void libraries_manager::forget_asset(const std::filesystem::path& path) noexcept
{
   if (const auto extension = path.extension();
       string::iequals(extension.native(), L".odf"sv)) {
      odfs.remove(path);
   }
   else if (string::iequals(extension.native(), L".msh"sv)) {
      models.remove(path);
   }
   else if (string::iequals(extension.native(), L".tga"sv)) {
      textures.remove(path);
   }
   else if (string::iequals(extension.native(), L".sky"sv)) {
      skies.remove(path);
   }
}

}
