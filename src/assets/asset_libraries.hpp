#pragma once

#include "asset_ref.hpp"
#include "asset_state.hpp"
#include "asset_traits.hpp"
#include "exceptions.hpp"
#include "lowercase_string.hpp"
#include "msh/flat_model.hpp"
#include "odf/definition.hpp"
#include "output_stream.hpp"
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
#include <fmt/format.h>
#include <tbb/task_group.h>

namespace we::utility {
class file_watcher;
}

namespace we::assets {

class libraries_manager;

template<typename T>
class library {
public:
   explicit library(output_stream& stream) : _output_stream{stream} {}

   /// @brief Adds an asset to the library.
   /// @param asset_path The path to the asset.
   void add(std::filesystem::path asset_path) noexcept
   {
      asset_path.make_preferred(); // makes for prettier output messages

      const lowercase_string name{asset_path.stem().string()};

      auto new_state = make_asset_state(name, asset_path);

      std::scoped_lock lock{_mutex};

      auto [state_pair, inserted] = _assets.emplace(name, new_state);

      auto state = state_pair->second;

      if (not inserted) {
         std::scoped_lock state_lock{state->mutex};

         state->exists = true;
         state->path = asset_path;
         state->start_load = [this, name] {
            std::scoped_lock lock{_mutex};

            enqueue_create_asset(name, false);
         };
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
         std::shared_lock lock{_mutex};

         if (auto asset = _assets.find(name); asset != _assets.end()) {
            return asset_ref{asset->second};
         }
      }

      auto placeholder_state = make_placeholder_asset_state();

      std::lock_guard lock{_mutex};

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

   /// @brief Clears the asset library.
   void clear() noexcept
   {
      {
         std::scoped_lock lock{_mutex};

         for (auto& [name, stop_source] : _loading_assets) {
            stop_source.request_stop();
         }

         _assets.clear();
         _loading_assets.clear();
      }
   }

private:
   auto make_asset_state(const lowercase_string& name, std::filesystem::path asset_path)
      -> std::shared_ptr<asset_state<T>>
   {
      return std::make_shared<asset_state<T>>(std::weak_ptr<T>{},
                                              not asset_path.empty(),
                                              asset_path, [this, name = name] {
                                                 std::scoped_lock lock{_mutex};

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

      auto asset = _assets.at(name);

      if (preempt_current_load) {
         if (auto stop_load = _loading_assets.find(name);
             stop_load != _loading_assets.end()) {
            stop_load->second.request_stop();
            _loading_assets.erase(stop_load);
         }
      }
      else if (_loading_assets.contains(name)) {
         return;
      }

      std::shared_lock lock{asset->mutex};

      const bool block = (name == lowercase_string{"metalroughspheres"sv});
      auto path = _assets.at(name)->path;

      _tasks.run([this, asset_path = _assets.at(name)->path, asset, name,
                  stop_token = _loading_assets[name].get_token()]() {
         try {
            if (stop_token.stop_requested()) return;

            auto asset_data =
               std::make_shared<const T>(asset_traits<T>::load(asset_path));

            _output_stream.write(
               fmt::format("Loaded asset '{}'\n"sv, asset_path.string()));

            if (stop_token.stop_requested()) return;

            // update the asset state's data ref
            {
               std::scoped_lock lock{asset->mutex};

               asset->data = asset_data;
            }

            // erase the loading marker/stop_source
            {
               std::scoped_lock lock{_mutex};

               _loading_assets.erase(name);
            }

            _load_event.broadcast(name, asset, asset_data);
         }
         catch (std::exception& e) {
            _output_stream.write(
               fmt::format("Error while loading asset:\n   File: {}\n   Message: \n{}\n"sv,
                           asset_path.string(), utility::string::indent(2, e.what())));
         }
      });
   }

   we::output_stream& _output_stream;

   std::shared_mutex _mutex;

   absl::flat_hash_map<lowercase_string, std::shared_ptr<asset_state<T>>> _assets;
   absl::flat_hash_map<lowercase_string, std::stop_source> _loading_assets;

   tbb::task_group _tasks;

   const std::shared_ptr<asset_state<T>> _null_asset = make_placeholder_asset_state();

   utility::event<void(const lowercase_string&, asset_ref<T>, asset_data<T>)> _load_event;
};

class libraries_manager {
public:
   explicit libraries_manager(output_stream& stream) noexcept;

   ~libraries_manager();

   /// @brief Sets the source directory for assets.
   /// @param path The directory to search through for assets.
   void source_directory(const std::filesystem::path& path) noexcept;

   /// @brief Updates assets that have been modified on disk.
   void update_modified() noexcept;

   library<odf::definition> odfs;
   library<msh::flat_model> models;
   library<texture::texture> textures;

private:
   void clear() noexcept;

   void register_asset(const std::filesystem::path& path) noexcept;

   std::unique_ptr<utility::file_watcher> _file_watcher;
};

}
