#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string_view>
#include <type_traits>

#include <fmt/core.h>

namespace we::io {

/// @brief The output mode
enum class output_open_mode {
   /// @brief Create the file, replacing any already existing one.
   create,
   /// @brief Open an existing file and append to it, or create a new one if it doesn't exist.
   append
};

/// @brief A simple class for writing out to a file using a nice API.
class output_file {
public:
   /// @brief Creates an output_file.
   /// @param path The path to the file to open/create.
   /// @param output_mode The open mode for the file.
   output_file(const std::filesystem::path& path,
               output_open_mode output_mode = output_open_mode::create);

   output_file(const output_file&) = delete;
   auto operator=(const output_file&) -> output_file& = delete;

   output_file(output_file&&) = delete;
   auto operator=(output_file&&) -> output_file& = delete;

   ~output_file();

   /// @brief Write a formmated string to the file and then append a new line.
   /// @param str The format string.
   /// @param ...args The format args.
   template<typename... Args>
   void write_ln(const fmt::format_string<Args...> str, const Args&... args) noexcept
   {
      vwrite_ln(str, fmt::make_format_args(args...));
   }

   /// @brief Write a string to the file and then append a new line.
   /// @param str The string to write.
   void write_ln(const std::string_view str) noexcept;

   /// @brief Write a formmated string to the file.
   /// @param str The format string.
   /// @param ...args The format args.
   template<typename... Args>
   void write(const fmt::format_string<Args...> str, const Args&... args) noexcept
   {
      vwrite(str, fmt::make_format_args(args...));
   }

   /// @brief Write a string to the file.
   void write(const std::string_view str) noexcept;

   /// @brief Write raw bytes to the file.
   /// @param bytes The bytes to write.
   void write(const std::span<const std::byte> bytes) noexcept;

   /// @brief Writes a trivially copyable object to the file as raw bytes. Take
   ///        care not to accidentally write out structures with pointers in.
   /// @tparam T The type of the object.
   /// @param object The object to write.
   template<typename T>
   void write_object(const T& object) noexcept
      requires(std::is_trivially_copyable_v<T> and not std::is_pointer_v<T>)
   {
      write(std::span{reinterpret_cast<const std::byte*>(&object), sizeof(T)});
   }

   // clang-format off
private:
   void vwrite_ln(const fmt::string_view format, fmt::format_args args) noexcept;

   void vwrite(const fmt::string_view format, fmt::format_args args) noexcept;

   void write_impl(const void* data, std::int64_t size) noexcept;

   void flush() noexcept;

   // clang-format on

   struct output_iterator;

   std::int64_t _used_buffer_bytes = 0;
   std::unique_ptr<std::byte[]> _buffer;

   std::unique_ptr<void, void (*)(void*)> _file = {nullptr, [](void*) {}};
};

}
