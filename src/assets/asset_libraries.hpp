#pragma once

#include "asset_stable_string.hpp"
#include "lowercase_string.hpp"
#include "utility/event_listener.hpp"
#include "utility/implementation_storage.hpp"

#include <functional>
#include <memory>
#include <span>

#include "asset_ref.hpp"

namespace std::filesystem {
class path;
}

namespace we {
class output_stream;
}

namespace we::async {
class thread_pool;
}

namespace we::utility {
class file_watcher;
}

namespace we::assets {

namespace odf {
struct definition;
}

namespace msh {
struct flat_model;
}

namespace texture {
struct texture;
}

namespace sky {
struct config;
}

template<typename T>
struct library {
   explicit library(output_stream& stream,
                    std::shared_ptr<async::thread_pool> thread_pool);

   /// @brief Adds an asset to the library.
   /// @param asset_path The path to the asset.
   void add(const std::filesystem::path& asset_path) noexcept;

   /// @brief Gets or creates a reference to an asset. The asset need not yet exist on disk.
   /// @param name The name of the asset.
   /// @return An asset_ref referencing to the asset.
   auto operator[](const lowercase_string& name) noexcept -> asset_ref<T>;

   /// @brief Listens for load updates on the assets.
   /// @param callback Function to call whenever an asset is loaded or is reloaded.
   /// @return The event_listener for the callback.
   auto listen_for_loads(
      std::function<void(const lowercase_string& name, asset_ref<T> asset, asset_data<T> data)> callback) noexcept
      -> event_listener<void(const lowercase_string&, asset_ref<T>, asset_data<T>)>;

   /// @brief Handles broadcasting notifications of any loaded or updated assets.
   void update_loaded() noexcept;

   /// @brief Clears the asset library.
   void clear() noexcept;

   /// @brief Allows you to view a span of existing (on disk) assets. A shared lock is taken on the underlying data as such `add` or `clear` must not be called from within the callback.
   /// @param callback The function to call with a span of existing assets. The strings will be valid until clear is called.
   void view_existing(
      const std::function<void(const std::span<const stable_string> assets)> callback) noexcept;

   /// @brief Query the file path of an asset.
   /// @param name The name of the asset.
   /// @return The file path to the asset. Can be empty if the asset does not exist.
   auto query_path(const lowercase_string& name) noexcept -> std::filesystem::path;

private:
   struct impl;

   implementation_storage<impl, 200> self;
};

struct libraries_manager {
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
   library<sky::config> skies;

private:
   void clear() noexcept;

   void register_asset(const std::filesystem::path& path) noexcept;

   std::unique_ptr<utility::file_watcher> _file_watcher;
   event_listener<void(const std::filesystem::path& path)> _file_changed_event;
   event_listener<void()> _unknown_files_changed;
};

}
