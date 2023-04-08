
#include "asset_libraries.hpp"
#include "asset_state.hpp"
#include "asset_traits.hpp"
#include "async/thread_pool.hpp"
#include "msh/flat_model.hpp"
#include "odf/definition.hpp"
#include "output_stream.hpp"
#include "utility/file_watcher.hpp"
#include "utility/stopwatch.hpp"
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

namespace {

const absl::flat_hash_set<std::wstring_view> ignored_folders =
   {L"_BUILD"sv, L"_LVL_PC"sv, L"_LVL_PS2"sv, L"_LVL_PSP"sv, L"_LVL_XBOX"sv,

    L".git"sv,   L".svn"sv,    L".vscode"sv};

}
template<typename T>
struct library<T>::members {
   members(output_stream& stream, std::shared_ptr<async::thread_pool> thread_pool,
           std::shared_ptr<asset_state<T>> null_asset)
      : _output_stream{stream}, _thread_pool{std::move(thread_pool)}, _null_asset{std::move(null_asset)}
   {
   }

   we::output_stream& _output_stream;

   std::shared_mutex _assets_mutex;
   absl::flat_hash_map<lowercase_string, std::shared_ptr<asset_state<T>>> _assets; // guarded by _assets_mutex

   std::shared_mutex _load_tasks_mutex;
   absl::flat_hash_map<lowercase_string, async::task<asset_data<T>>> _load_tasks; // guarded by _load_tasks_mutex

   std::shared_ptr<async::thread_pool> _thread_pool;

   const std::shared_ptr<asset_state<T>> _null_asset;

   utility::event<void(const lowercase_string&, asset_ref<T>, asset_data<T>)> _load_event;
};

template<typename T>
library<T>::library(output_stream& stream, std::shared_ptr<async::thread_pool> thread_pool)
   : self{stream, std::move(thread_pool), make_placeholder_asset_state()}
{
}

template<typename T>
void library<T>::add(const std::filesystem::path& unpreferred_asset_path) noexcept
{
   std::filesystem::path asset_path =
      unpreferred_asset_path; // makes for prettier output messages

   asset_path.make_preferred();

   const lowercase_string name{asset_path.stem().string()};

   auto new_state = make_asset_state(name, asset_path);

   auto [state_pair, inserted] = [&] {
      std::scoped_lock lock{self->_assets_mutex};

      return self->_assets.emplace(name, new_state);
   }();

   auto state = state_pair->second;

   if (not inserted) {
      std::scoped_lock state_lock{state->mutex};

      state->exists = true;
      state->load_failure = false;
      state->path = asset_path;
      state->start_load = [this, name] { enqueue_create_asset(name, false); };
   }

   if (state->ref_count.load(std::memory_order_relaxed) > 0) {
      enqueue_create_asset(name, true);
   }
}

template<typename T>
auto library<T>::operator[](const lowercase_string& name) noexcept -> asset_ref<T>
{
   if (name.empty()) return asset_ref{self->_null_asset};

   // Take a shared_lock to try and get an already existing asset state.
   {
      std::shared_lock lock{self->_assets_mutex};

      if (auto asset = self->_assets.find(name); asset != self->_assets.end()) {
         return asset_ref{asset->second};
      }
   }

   auto placeholder_state = make_placeholder_asset_state();

   std::lock_guard lock{self->_assets_mutex};

   auto [state_pair, inserted] = self->_assets.emplace(name, placeholder_state);

   return state_pair->second;
}

template<typename T>
auto library<T>::listen_for_loads(
   std::function<void(const lowercase_string& name, asset_ref<T> asset, asset_data<T> data)> callback) noexcept
   -> event_listener<void(const lowercase_string&, asset_ref<T>, asset_data<T>)>
{
   return self->_load_event.listen(std::move(callback));
}

template<typename T>
void library<T>::update_loaded() noexcept
{
restart_loop:
   std::unique_lock lock{self->_load_tasks_mutex};

   for (auto& [task_name, task] : self->_load_tasks) {
      if (not task.ready()) continue;

      asset_data<T> asset_data = task.get();
      lowercase_string name = task_name;

      self->_load_tasks.erase(name);
      lock.unlock(); // After thise line accessing _load_tasks is no longer safe.

      asset_ref<T> asset;
      std::shared_ptr<asset_state<T>> asset_state;

      // Get the asset_state.
      {
         std::scoped_lock assets_lock{self->_assets_mutex};

         asset_state = self->_assets[name];
      }

      // Update the asset data.
      {
         std::scoped_lock asset_lock{asset_state->mutex};

         asset_state->data = asset_data;
      }

      asset = asset_ref<T>{asset_state};

      if (asset_data != nullptr) {
         self->_load_event.broadcast(name, asset, asset_data);
      }
      else {
         asset_state->load_failure = true;
      }

      goto restart_loop;
   }
}

template<typename T>
void library<T>::clear() noexcept
{
   std::scoped_lock lock{self->_assets_mutex, self->_load_tasks_mutex};

   self->_load_tasks.clear();
   self->_assets.clear();
}

template<typename T>
void library<T>::enumerate_known(
   const std::function<void(const lowercase_string& name)> callback) noexcept
{
   std::shared_lock lock{self->_assets_mutex};

   for (const auto& [name, state] : self->_assets) {
      {
         std::shared_lock state_lock{state->mutex};

         if (not state->exists) continue;
      }

      callback(name);
   }
}

template<typename T>
auto library<T>::make_asset_state(const lowercase_string& name,
                                  const std::filesystem::path& asset_path)
   -> std::shared_ptr<asset_state<T>>
{
   return std::make_shared<asset_state<T>>(std::weak_ptr<T>{}, not asset_path.empty(),
                                           asset_path, [this, name = name] {
                                              enqueue_create_asset(name, false);
                                           });
}

template<typename T>
auto library<T>::make_placeholder_asset_state() -> std::shared_ptr<asset_state<T>>
{
   return std::make_shared<asset_state<T>>(std::weak_ptr<T>{}, false,
                                           std::filesystem::path{}, [] {});
}

template<typename T>
void library<T>::enqueue_create_asset(lowercase_string name,
                                      bool preempt_current_load) noexcept
{
   using namespace std::literals;

   auto asset = [&] {
      std::scoped_lock lock{self->_assets_mutex};

      return self->_assets.at(name);
   }();

   // Do not try reload assets that previously failed loading.
   if (asset->load_failure) return;

   std::filesystem::path asset_path = [&] {
      std::shared_lock asset_lock{asset->mutex};

      return asset->path;
   }();

   std::scoped_lock tasks_lock{self->_load_tasks_mutex};

   if (preempt_current_load) {
      if (auto inprogress_load = self->_load_tasks.find(name);
          inprogress_load != self->_load_tasks.end()) {
         auto& [_, load_task] = *inprogress_load;

         load_task.cancel();

         self->_load_tasks.erase(inprogress_load);
      }
   }
   else if (self->_load_tasks.contains(name)) {
      return;
   }

   self->_load_tasks[name] = self->_thread_pool->exec(
      async::task_priority::low,
      [this, asset_path = std::move(asset_path), asset, name]() -> asset_data<T> {
         try {
            utility::stopwatch load_timer;

            auto asset_data =
               std::make_shared<const T>(asset_traits<T>::load(asset_path));

            self->_output_stream.write(
               "Loaded asset '{}'\n   Time Taken: {:f}ms\n"sv, asset_path.string(),
               load_timer.elapsed<std::chrono::duration<double, std::milli>>().count());

            return asset_data;
         }
         catch (std::exception& e) {
            self->_output_stream.write("Error while loading asset:\n   File: {}\n   Message: \n{}\n"sv,
                                       asset_path.string(),
                                       string::indent(2, e.what()));

            return nullptr;
         }
      });
}

template struct library<odf::definition>;
template struct library<msh::flat_model>;
template struct library<texture::texture>;

libraries_manager::libraries_manager(output_stream& stream,
                                     std::shared_ptr<async::thread_pool> thread_pool) noexcept
   : odfs{stream, thread_pool}, models{stream, thread_pool}, textures{stream, thread_pool}
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

      register_asset(path);
   }

   _file_watcher = std::make_unique<utility::file_watcher>(source_directory);
   _file_changed_event =
      _file_watcher->listen_file_changed([this](const std::filesystem::path& path) {
         // TODO: Skip path if parent path is ignored.

         register_asset(path);
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
}

void libraries_manager::clear() noexcept
{
   odfs.clear();
   models.clear();
   textures.clear();
}

void libraries_manager::register_asset(const std::filesystem::path& path) noexcept
{
   if (auto extension = path.extension(); extension == L".odf"sv) {
      odfs.add(path);
   }
   else if (extension == L".msh"sv) {
      models.add(path);
   }
   else if (extension == L".tga"sv) {
      textures.add(path);
   }
}

}
