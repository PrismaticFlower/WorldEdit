#pragma once

#include "asset_stable_string.hpp"
#include "lowercase_string.hpp"
#include "utility/event_listener.hpp"
#include "utility/function_ptr.hpp"
#include "utility/implementation_storage.hpp"

#include <functional>
#include <memory>
#include <span>
#include <vector>

#include "asset_ref.hpp"

namespace we {
class output_stream;
}

namespace we::async {
class thread_pool;
}

namespace we::utility {
class file_watcher;
}

namespace we::io {
struct path;
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

/// @brief A directory inside the asset tree.
struct library_tree_branch {
   /// @brief Name of this directory.
   std::string name;

   /// @brief Child directories containing assets of this directory.
   std::vector<library_tree_branch> directories;

   /// @brief Assets inside this directory.
   std::vector<lowercase_string> assets;
};

/// @brief The root of the asset tree.
struct library_tree {
   /// @brief Directories inside the root containing assets.
   std::vector<library_tree_branch> directories;

   /// @brief Assets inside this directory.
   std::vector<lowercase_string> assets;

   void add(const io::path& asset_path) noexcept;

   void remove(const io::path& asset_path) noexcept;
};

/// @brief Tracks in the project folder and loads assets in the background using the thread pool when needed.
/// @tparam T The type of the asset.
template<typename T>
struct library {
   explicit library(output_stream& stream,
                    std::shared_ptr<async::thread_pool> thread_pool);

   /// @brief Adds an asset to the library.
   /// @param asset_path The path to the asset.
   void add(const io::path& asset_path, uint64 last_write_time) noexcept;

   /// @brief Removes an asset from the library.
   /// @param asset_path The path to the asset.
   void remove(const io::path& asset_path) noexcept;

   /// @brief Gets or creates a reference to an asset. The asset need not yet exist on disk.
   /// @param name The name of the asset.
   /// @return An asset_ref referencing to the asset.
   auto operator[](const lowercase_string& name) noexcept -> asset_ref<T>;

   /// @brief Listens for load updates on the assets.
   /// @param callback Function to call whenever an asset is loaded or is reloaded.
   /// @return The event_listener for the callback.
   auto listen_for_loads(
      std::move_only_function<void(const lowercase_string& name,
                                   asset_ref<T> asset, asset_data<T> data) const>
         callback) noexcept
      -> event_listener<void(const lowercase_string&, asset_ref<T>, asset_data<T>)>;

   /// @brief Listens for load failures on the assets.
   /// @param callback Function to call whenever an asset fails to load.
   /// @return The event_listener for the callback.
   auto listen_for_load_failures(
      std::move_only_function<void(const lowercase_string& name, asset_ref<T> asset) const> callback) noexcept
      -> event_listener<void(const lowercase_string&, asset_ref<T>)>;

   /// @brief Listens for asset changes.
   /// @param callback Function to call whenever an asset has changed on disk.
   /// @return The event_listener for the callback.
   auto listen_for_changes(
      std::move_only_function<void(const lowercase_string& name) const> callback) noexcept
      -> event_listener<void(const lowercase_string&)>;

   /// @brief Handles broadcasting notifications of any loaded or updated assets.
   void update_loaded() noexcept;

   /// @brief Clears the asset library.
   void clear() noexcept;

   /// @brief Allows you to view a span of existing (on disk) assets. A shared lock is taken on the underlying data as such `add` or `clear` must not be called from within the callback.
   /// @param callback The function to call with a span of existing assets. The strings will be valid until clear is called.
   void view_existing(
      function_ptr<void(const std::span<const stable_string> assets) noexcept> callback) noexcept;

   /// @brief Allows you to view a tree of existing (on disk) assets. A shared lock is taken on the underlying data as such `add` or `clear` must not be called from within the callback.
   /// @param callback The function to call with a const reference to the tree of existing assets.
   void view_tree(function_ptr<void(const library_tree& tree) noexcept> callback) noexcept;

   /// @brief Query the file path of an asset.
   /// @param name The name of the asset.
   /// @return The file path to the asset. Can be empty if the asset does not exist.
   auto query_path(const lowercase_string& name) noexcept -> io::path;

   /// @brief Query the last write time of an asset.
   /// @param name The name of the asset.
   /// @return The last write time of the asset. Can be 0 if the asset does not exist.
   auto query_last_write_time(const lowercase_string& name) noexcept -> uint64;

private:
   struct impl;

   implementation_storage<impl, 296> self;
};

/// @brief Tracks assets like library but does no loading or lifetime management.
struct directory {
   directory() noexcept;

   /// @brief Adds an asset to the directory.
   /// @param asset_path The path to the asset.
   void add(const io::path& asset_path) noexcept;

   /// @brief Removes an asset from the directory.
   /// @param asset_path The path to the asset.
   void remove(const io::path& asset_path) noexcept;

   /// @brief Clears the asset directory.
   void clear() noexcept;

   /// @brief Allows you to view a span of existing (on disk) assets. A lock is taken on the underlying data as such other directory member functions must not be from the callback.
   /// @param callback The function to call with a span of existing assets. The strings will be valid until clear is called.
   void view_existing(
      function_ptr<void(const std::span<const stable_string> assets) noexcept> callback) noexcept;

   /// @brief Query the file path of an asset.
   /// @param name The name of the asset.
   /// @return The file path to the asset. Can be empty if the asset does not exist.
   auto query_path(const lowercase_string& name) noexcept -> io::path;

private:
   struct impl;

   implementation_storage<impl, 80> self;
};

struct libraries_manager {
   explicit libraries_manager(output_stream& stream,
                              std::shared_ptr<async::thread_pool> thread_pool) noexcept;

   ~libraries_manager();

   /// @brief Sets the source directory for assets.
   /// @param path The directory to search through for assets.
   void source_directory(const io::path& path) noexcept;

   /// @brief Handles broadcasting notifications of any loaded or updated assets.
   void update_loaded() noexcept;

   library<odf::definition> odfs;
   library<msh::flat_model> models;
   library<texture::texture> textures;
   library<sky::config> skies;

   directory entity_groups;

private:
   void clear() noexcept;

   void register_asset(const io::path& path, uint64 last_write_time) noexcept;

   void forget_asset(const io::path& path) noexcept;

   std::unique_ptr<utility::file_watcher> _file_watcher;
   event_listener<void(const io::path& path)> _file_changed_event;
   event_listener<void(const io::path& path)> _file_removed_event;
   event_listener<void()> _unknown_files_changed;
};

}
