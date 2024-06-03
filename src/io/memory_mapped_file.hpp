#pragma once

#include <cstddef>

namespace we::io {

/// @brief The map mode for the file.
enum class map_mode {
   /// @brief Maps the file for reading.
   read,
   /// @brief Maps the file for writing.
   read_write
};

struct memory_mapped_file_params {
   /// @brief Null-terminated path to the file.
   const wchar_t* path;
   /// @brief Map mode/page protection for the file.
   const map_mode map_mode = map_mode::read_write;
   /// @brief Size to map in, if the file is smaller than this it will be extended.
   const std::size_t size = 0;
   /// @brief Truncate the file to size if it is bigger than size.
   bool truncate_to_size = false;
};

/// @brief A simple struct memory mapping a file.
struct memory_mapped_file {
   /// @brief Construct an empty memory_mapped_file.
   memory_mapped_file() = default;

   /// @brief Creates an output_file.
   /// @param params The parameters to use to open the file.
   memory_mapped_file(const memory_mapped_file_params& params);

   memory_mapped_file(const memory_mapped_file&) = delete;
   auto operator=(const memory_mapped_file&) -> memory_mapped_file& = delete;

   memory_mapped_file(memory_mapped_file&& other) noexcept;

   auto operator=(memory_mapped_file&& other) noexcept -> memory_mapped_file&;

   ~memory_mapped_file();

   /// @brief Get a pointer to the mapped file.
   /// @return The pointer to mapped file.
   [[nodiscard]] auto data() noexcept -> std::byte*;

   /// @brief Get a pointer to the mapped file.
   /// @return The pointer to mapped file.
   [[nodiscard]] auto data() const noexcept -> const std::byte*;

   /// @brief Get the size of the mapped file.
   /// @return The size of the mapped file.
   [[nodiscard]] auto size() const noexcept -> std::size_t;

   /// @brief Checks if the memory_mapped_file has been used to open a file.
   /// @return True if a file is open, false if the memory_mapped_file is empty.
   [[nodiscard]] bool is_open() const noexcept;

   /// @brief Reset the memory_mapped_file to be empty.
   void reset() noexcept;

   /// @brief Swap this memory_mapped_file handle's and pointers with another.
   /// @param other The memory_mapped_file to swap with.
   void swap(memory_mapped_file& other) noexcept;

private:
   std::byte* _bytes = nullptr;
   std::size_t _size = 0;
   void* _mapping_handle = nullptr;
   void* _file_handle = nullptr;
   map_mode _map_mode = map_mode::read;
};

}