#pragma once

#include "types.hpp"

#include <array>
#include <string_view>

namespace we::io {

struct path {
   path() noexcept;

   path(const path& other) noexcept;

   path(path&& other) noexcept;

   path(std::string_view str) noexcept;

   path(const char* c_string) noexcept;

   auto operator=(const path& other) noexcept -> path&;

   auto operator=(path&& other) noexcept -> path&;

   ~path();

   /// @brief Swap this path with another path.
   /// @param other The path to swap with.
   void swap(path& other) noexcept;

   /// @brief Get the parent path of a file or directory.
   /// @return The parent path as a string view.
   [[nodiscard]] auto parent_path() const noexcept -> std::string_view;

   /// @brief Get the file name from a path.
   /// @return A string view of the file name.
   [[nodiscard]] auto filename() const noexcept -> std::string_view;

   /// @brief Get the stem of the path (either the directory name or the file name without the extension).
   /// @return The stem of the path as a string view.
   [[nodiscard]] auto stem() const noexcept -> std::string_view;

   /// @brief Get the extension (including the dot) of the file the path points to.
   /// @return The extension or "" if the path has no extension.
   [[nodiscard]] auto extension() const noexcept -> std::string_view;

   /// @brief Get the null terminated C string of the path.
   /// @return The C string of the path.
   [[nodiscard]] auto c_str() const noexcept -> const char*;

   /// @brief Test if the path is empty.
   /// @return True if the path is empty, false otherwise.
   [[nodiscard]] bool empty() const noexcept;

   /// @brief Get a string_view of the path.
   /// @return The string view of the path.
   [[nodiscard]] auto string_view() const noexcept -> std::string_view;

   /// @brief Concatenate a string onto the path.
   /// @param right The string to add to the path.
   /// @return The (now modified) path.
   auto operator+=(std::string_view right) noexcept -> path&;

private:
   char* _data = &_small_storage[0];
   uint32 _length = 0;
   uint32 _is_small : 1 = true;
   uint32 _capacity : 31 = sizeof(_small_storage) - 1;
   std::array<char, 48> _small_storage = {};
};

/// @brief Compare to paths for equality.
/// @param left The left path.
/// @param right The right path.
/// @return The result.
bool operator==(const path& left, const path& right) noexcept;

/// @brief Make a path with the format of "{parent_path}/{stem}".
/// @param parent_path The parent path for the new path.
/// @param stem The stem for the new path.
/// @return The new path.
auto compose_path(const path& parent_path, std::string_view stem) noexcept -> path;

/// @brief Make a path with the format of "{parent_path}/{stem}{extension}".
/// @param parent_path The parent path for the new path.
/// @param stem The stem for the new path.
/// @param extension The file extension for the new path.
/// @return The new path.
auto compose_path(const path& parent_path, std::string_view stem,
                  std::string_view extension) noexcept -> path;

/// @brief Copy a path and replace it's extension.
/// @param old_path The path to copy.
/// @param new_extension The new file extension for the path.
/// @return The new path.
auto make_path_with_new_extension(const path& old_path,
                                  std::string_view new_extension) noexcept -> path;

/// @brief Make a path from a wchar_t C string.
/// @param old_path The wchar_t string.
/// @return The new path.
auto make_path_from_wide_cstring(const wchar_t* str) noexcept -> io::path;

/// @brief A wide path. For converting a path to a wchar_t string that can be passed to Win32 APIs.
struct wide_path {
   explicit wide_path(const path& path);

   wide_path() = delete;

   wide_path(const wide_path&) = delete;

   wide_path(wide_path&&) = delete;

   ~wide_path();

   [[nodiscard]] auto c_str() const noexcept -> const wchar_t*;

private:
   wchar_t* _data = &_small_storage[0];
   bool _is_small = true;
   std::array<wchar_t, 123> _small_storage = {};
};

/// @brief Check if a path exists on the filesystem.
/// @param path The path.
/// @return True if the path exists or false if it doesn't.
[[nodiscard]] bool exists(const path& path) noexcept;

/// @brief Delete a file or an empty directory from the filesystem.
/// @param path A path to a file or directory.
/// @return True if the path was deleted or false if it wasn't.
[[nodiscard]] bool remove(const path& path) noexcept;

/// @brief Updates a file's last write time to the current system time.
void update_last_write_time(const path& path) noexcept;

/// @brief Gets a file's last write time.
/// @return The file's last write time or 0 on failure.
auto get_last_write_time(const path& path) noexcept -> uint64;

/// @brief Create a directory.
/// @param path The directory to create, the parent path must already exist.
/// @return True if successful, false otherwise.
[[nodiscard]] bool create_directory(const path& path) noexcept;

/// @brief Create a directory and any parent directories needed.
/// @param path The directory to create.
/// @return True if successful (or if the directory already exists), false otherwise.
[[nodiscard]] bool create_directories(const path& path) noexcept;

/// @brief Copy a file.
/// @param src The source file path.
/// @param dest The destination file path. If destination already exists it will be overwritten.
/// @return True if successful, false otherwise.
[[nodiscard]] bool copy_file(const path& src, const path& dest) noexcept;

struct directory_entry {
   bool is_directory = false;
   bool is_file = false;

   io::path path;
   uint64 last_write_time = 0;
};

/// @brief Simple directory iterator using wrapiing FindFirstFileEx/FindNextFile.
//  Recurses into and walks child directories by default.
struct directory_iterator {
   directory_iterator() noexcept;

   directory_iterator(const path& directory, const bool recurse = true) noexcept;

   directory_iterator(const directory_iterator& other) noexcept;

   directory_iterator(directory_iterator&& other) noexcept;

   auto operator=(const directory_iterator& other) noexcept -> directory_iterator&;

   auto operator=(directory_iterator&& other) noexcept -> directory_iterator&;

   ~directory_iterator();

   [[nodiscard]] auto operator*() noexcept -> const directory_entry&;

   [[nodiscard]] auto operator->() noexcept -> const directory_entry*;

   auto operator++() noexcept -> directory_iterator&;

   /// @brief Skip iterating through the current directory.
   void skip_directory() noexcept;

   auto begin() noexcept -> directory_iterator;

   auto end() noexcept -> directory_iterator;

   bool operator==(const directory_iterator&) const noexcept;

private:
   struct impl;

   void swap(directory_iterator& other) noexcept;

   impl* _impl = nullptr;
};

}
