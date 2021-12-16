
#include "output_file.hpp"

#include <cstring>
#include <stdexcept>
#include <system_error>
#include <utility>

#include <Windows.h>

namespace we::io {

namespace {

constexpr auto buffer_max_size = 16384;
constexpr char linebreak_char = '\n';

auto desired_access(output_open_mode output_mode) noexcept -> int
{
   if (output_mode == output_open_mode::create) return GENERIC_WRITE;
   if (output_mode == output_open_mode::append) return FILE_APPEND_DATA;

   std::terminate();
}

auto creation_disposition(output_open_mode output_mode) noexcept -> int
{
   if (output_mode == output_open_mode::create) return CREATE_ALWAYS;
   if (output_mode == output_open_mode::append) return OPEN_ALWAYS;

   std::terminate();
}

void close_file(HANDLE file) noexcept
{
   if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
}

}

struct output_file::output_iterator {
   output_file* file;

   auto operator=(const char c) noexcept -> output_iterator&
   {
      file->write_impl(&c, sizeof(c));

      return *this;
   }

   auto operator*() noexcept -> output_iterator&
   {
      return *this;
   }

   auto operator++() noexcept -> output_iterator&
   {
      return *this;
   }

   auto operator++(int) noexcept -> output_iterator&
   {
      return *this;
   }

   using iterator_category = std::output_iterator_tag;
   using value_type = void;
   using difference_type = std::ptrdiff_t;
   using pointer = void;
   using reference = void;
};

output_file::output_file(const std::filesystem::path& path, output_open_mode output_mode)
{
   _file = {CreateFileW(path.c_str(), desired_access(output_mode), 0x0, nullptr,
                        creation_disposition(output_mode),
                        FILE_ATTRIBUTE_NORMAL, nullptr),
            &close_file};

   if (_file.get() == INVALID_HANDLE_VALUE) {
      const DWORD system_error = GetLastError();

      throw std::runtime_error{fmt::format(
         "Failed to open file '{}'.\n   Reason: {}", path.string(),
         std::system_category().default_error_condition(system_error).message())};
   }

   _buffer = std::make_unique<std::byte[]>(buffer_max_size);
}

output_file::~output_file()
{
   flush();
}

void output_file::write_ln(const std::string_view str) noexcept
{
   write_impl(str.data(), str.size());
   write_impl(&linebreak_char, sizeof(linebreak_char));
}

void output_file::write(const std::string_view str) noexcept
{
   write_impl(str.data(), str.size());
}

void output_file::write(const std::span<const std::byte> bytes) noexcept
{
   write_impl(bytes.data(), bytes.size());
}

void output_file::vwrite_ln(const fmt::string_view format, fmt::format_args args) noexcept
{
   fmt::vformat_to(output_iterator{.file = this}, format, args);

   write_impl(&linebreak_char, sizeof(linebreak_char));
}

void output_file::vwrite(const fmt::string_view format, fmt::format_args args) noexcept
{
   fmt::vformat_to(output_iterator{.file = this}, format, args);
}

void output_file::write_impl(const void* data, std::int64_t size) noexcept
{
   const std::int64_t remaining_buffer_bytes = buffer_max_size - _used_buffer_bytes;

   if (remaining_buffer_bytes < size) {
      flush(); // empty the buffer

      if (size >= buffer_max_size) { // if the data doesn't fit in the buffer write directly to the file
         DWORD num_bytes_written = 0;

         WriteFile(_file.get(), data, static_cast<DWORD>(size),
                   &num_bytes_written, nullptr);

         return;
      } // else fallthrough back to writing to the buffer below
   }

   std::memcpy(&_buffer[_used_buffer_bytes], data, size);

   _used_buffer_bytes += size;
}

void output_file::flush() noexcept
{
   if (_used_buffer_bytes <= 0) return;

   DWORD num_bytes_written = 0;

   WriteFile(_file.get(), _buffer.get(),
             static_cast<DWORD>(std::exchange(_used_buffer_bytes, 0)),
             &num_bytes_written, nullptr);
}

}