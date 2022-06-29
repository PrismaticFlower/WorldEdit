#pragma once

#include "asset_ref.hpp"
#include "asset_state.hpp"
#include "asset_traits.hpp"
#include "async/thread_pool.hpp"
#include "lowercase_string.hpp"
#include "msh/flat_model.hpp"
#include "odf/definition.hpp"
#include "output_stream.hpp"
#include "utility/stopwatch.hpp"
#include "utility/string_ops.hpp"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stop_token>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace we::utility {
class file_watcher;
}

namespace we::assets {

class libraries_manager;

template<typename T>
class library {
public:
   explicit library(output_stream& stream, std::shared_ptr<async::thread_pool> thread_pool)
      : _output_stream{stream}, _thread_pool{thread_pool}
   {
   }

   /// @brief Adds an asset to the library.
   /// @param asset_path The path to the asset.
   void add(std::filesystem::path asset_path) noexcept
   {
      asset_path.make_preferred(); // makes for prettier output messages

      const lowercase_string name{asset_path.stem().string()};

      auto new_state = make_asset_state(name, asset_path);

      auto [state_pair, inserted] = [&] {
         std::scoped_lock lock{_assets_mutex};

         return _assets.emplace(name, new_state);
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

   /// @brief Gets or creates a reference to an asset. The asset need not yet exist on disk.
   /// @param name The name of the asset.
   /// @return An asset_ref referencing to the asset.
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

   /// @brief Listens for load updates on the assets.
   /// @param callback Function to call whenever an asset is loaded or is reloaded.
   /// @return The event_listener for the callback.
   auto listen_for_loads(
      std::function<void(const lowercase_string& name, asset_ref<T> asset, asset_data<T> data)> callback) noexcept
      -> event_listener<void(const lowercase_string&, asset_ref<T>, asset_data<T>)>
   {
      return _load_event.listen(std::move(callback));
   }

   /// @brief Handles broadcasting notifications of any loaded or updated assets.
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

         goto restart_loop; // Hehehe, couldn't resist (sorry)
      }
   }

   /// @brief Clears the asset library.
   void clear() noexcept
   {
      std::scoped_lock lock{_assets_mutex, _load_tasks_mutex};

      _load_tasks.clear();
      _assets.clear();
   }

   void enumerate_known(const auto callback) noexcept
   {
      std::shared_lock lock{_assets_mutex};

      using namespace ranges::views;

      callback(_assets | filter([](const auto& name_asset) {
                  asset_state<T>& state = *name_asset.second;

                  std::shared_lock lock{state.mutex};

                  return state.exists;
               }) |
               transform([](const auto& name_asset) { return name_asset.first; }));
   }

private:
   auto make_asset_state(const lowercase_string& name, std::filesystem::path asset_path)
      -> std::shared_ptr<asset_state<T>>
   {
      return std::make_shared<asset_state<T>>(std::weak_ptr<T>{},
                                              not asset_path.empty(),
                                              asset_path, [this, name = name] {
                                                 enqueue_create_asset(name, false);
                                              });
   }

   auto make_placeholder_asset_state() -> std::shared_ptr<asset_state<T>>
   {
      return std::make_shared<asset_state<T>>(std::weak_ptr<T>{}, false,
                                              std::filesystem::path{}, [] {});
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
               utility::stopwatch load_timer;

               auto asset_data =
                  std::make_shared<const T>(asset_traits<T>::load(asset_path));

               _output_stream.write("Loaded asset '{}'\n   Time Taken: {:f}ms\n"sv,
                                    asset_path.string(),
                                    load_timer
                                       .elapsed<std::chrono::duration<double, std::milli>>()
                                       .count());

               return asset_data;
            }
            catch (std::exception& e) {
               _output_stream.write("Error while loading asset:\n   File: {}\n   Message: \n{}\n"sv,
                                    asset_path.string(),
                                    utility::string::indent(2, e.what()));

               return nullptr;
            }
         });
   }

   we::output_stream& _output_stream;

   std::shared_mutex _assets_mutex;
   absl::flat_hash_map<lowercase_string, std::shared_ptr<asset_state<T>>> _assets; // guarded by _assets_mutex

   std::shared_mutex _load_tasks_mutex;
   absl::flat_hash_map<lowercase_string, async::task<asset_data<T>>> _load_tasks; // guarded by _load_tasks_mutex

   std::shared_ptr<async::thread_pool> _thread_pool;

   const std::shared_ptr<asset_state<T>> _null_asset = make_placeholder_asset_state();

   utility::event<void(const lowercase_string&, asset_ref<T>, asset_data<T>)> _load_event;
};

class libraries_manager {
public:
   explicit libraries_manager(output_stream& stream,
                              std::shared_ptr<async::thread_pool> thread_pool) noexcept;

   ~libraries_manager();

   /// @brief Sets the source directory for assets.
   /// @param path The directory to search through for assets.
   void source_directory(const std::filesystem::path& path) noexcept;

   /// @brief Handles broadcasting notifications of any loaded or updated assets.
   void update_loaded() noexcept;

   library<odf::definition> odfs;
   library<msh::flat_model> models;
   library<texture::texture> textures;

private:
   void clear() noexcept;

   void register_asset(const std::filesystem::path& path) noexcept;

   std::unique_ptr<utility::file_watcher> _file_watcher;
   event_listener<void(const std::filesystem::path& path)> _file_changed_event;
   event_listener<void()> _unknown_files_changed;
};

}
