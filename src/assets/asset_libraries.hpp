#pragma once

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
#include <future>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>
#include <tbb/task_group.h>

namespace sk::utility {
class file_watcher;
}

namespace sk::assets {

class libraries_manager;

template<typename T>
class library {
public:
   explicit library(output_stream& stream) : _output_stream{stream} {}

   void add(std::filesystem::path asset_path) noexcept
   {
      std::lock_guard lock{_mutex};

      asset_path.make_preferred(); // makes for prettier output messages

      const lowercase_string name{asset_path.stem().string()};

      _known_assets.emplace(name, std::move(asset_path));
      _pending_assets.erase(name);

      if (auto existing = _cached_assets.find(name); existing != _cached_assets.end()) {
         if (auto asset = existing->second.lock(); asset) {
            enqueue_create_asset(name);
         }
      }
   }

   auto aquire_if(const lowercase_string& name) noexcept -> std::shared_ptr<T>
   {
      if (name.empty()) return nullptr;
      if (auto existing = aquire_cached_if(name); existing) return existing;

      std::lock_guard lock{_mutex};

      // make sure another thread hasn't already loaded the asset inbetween us checking for it and taking the write lock
      if (auto existing = _cached_assets.find(name); existing != _cached_assets.end()) {
         if (auto asset = existing->second.lock(); asset) {
            return asset;
         }
      }

      if (auto pending = _pending_assets.find(name); pending != _pending_assets.end()) {
         auto& [_, future] = *pending;

         if (const auto status = future.wait_for(std::chrono::seconds{0});
             status == std::future_status::ready) {

            if (auto asset = future.get(); asset) {
               _cached_assets[name] = asset;

               _pending_assets.erase(pending);

               return asset;
            }
         }

         return nullptr;
      }

      if (_known_assets.contains(name)) {
         enqueue_create_asset(name);
      }
      else {
         _output_stream.write(fmt::format(
            "Error! Unable to find referenced {} asset named '{}'!\n",
            asset_traits<T>::error_type_name, name));
      }

      return nullptr;
   }

   auto aquire_cached_if(const lowercase_string& name) noexcept -> std::shared_ptr<T>
   {
      std::shared_lock read_lock{_mutex};

      if (auto existing = _cached_assets.find(name); existing != _cached_assets.end()) {
         return existing->second.lock();
      }

      return nullptr;
   }

   auto loaded_assets() -> std::vector<std::pair<std::string, std::shared_ptr<T>>>
   {
      std::lock_guard lock{_mutex};

      std::vector<std::pair<std::string, std::shared_ptr<T>>> loaded;
      loaded.reserve(_pending_assets.size());

      for (auto& [name, future] : _pending_assets) {
         if (const auto status = future.wait_for(std::chrono::seconds{0});
             status == std::future_status::ready) {
            auto asset = future.get();

            if (not asset) continue;

            _cached_assets.emplace(name, asset);
            loaded.emplace_back(std::move(name), std::move(asset));
         }
      }

      std::erase_if(_pending_assets, [](const auto& pending) {
         return not pending.second.valid();
      });

      return loaded;
   }

private:
   void enqueue_create_asset(lowercase_string name) noexcept
   {
      using namespace std::literals;

      auto load_asset_task = std::make_unique<std::packaged_task<std::shared_ptr<T>()>>(
         [this, asset_path = _known_assets.at(name),
          name = name]() -> std::shared_ptr<T> {
            try {
               auto asset = std::make_shared<T>(asset_traits<T>::load(asset_path));

               _output_stream.write(
                  fmt::format("Loaded asset '{}'\n"sv, asset_path.string()));

               return asset;
            }
            catch (std::exception& e) {
               _output_stream.write(
                  fmt::format("Error while loading asset:\n   File: {}\n   Message: \n{}\n"sv,
                              asset_path.string(),
                              utility::string::indent(2, e.what())));

               return nullptr;
            }
         });

      _pending_assets.emplace(name, load_asset_task->get_future());

      _tasks.run([load_asset_task = std::move(load_asset_task)]() {
         (*load_asset_task)();
      });
   }

   sk::output_stream& _output_stream;

   std::shared_mutex _mutex;

   std::unordered_map<lowercase_string, std::filesystem::path> _known_assets;
   std::unordered_map<lowercase_string, std::future<std::shared_ptr<T>>> _pending_assets;
   std::unordered_map<lowercase_string, std::weak_ptr<T>> _cached_assets;

   tbb::task_group _tasks;
};

class libraries_manager {
public:
   explicit libraries_manager(output_stream& stream) noexcept;

   ~libraries_manager();

   void source_directory(const std::filesystem::path& path) noexcept;

   void update_changed() noexcept;

   library<odf::definition> odfs;
   library<msh::flat_model> models;
   library<texture::texture> textures;

private:
   void register_asset(const std::filesystem::path& path) noexcept;

   std::unique_ptr<utility::file_watcher> _file_watcher;
};

}
