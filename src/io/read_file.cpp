
#include "read_file.hpp"
#include "error.hpp"

#include <fmt/core.h>
#include <wil/resource.h>

#include <system_error>

namespace we::io {

namespace {

template<typename Out>
auto read_file_impl(const io::path& path) -> Out
{
   static_assert(sizeof(Out::value_type) == 1, "See bytes.resize(file_size);");

   io::wide_path wide_path{path};

   wil::unique_hfile file{
      CreateFileW(wide_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                  FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, nullptr)};

   if (not file) {
      const DWORD system_error = GetLastError();

      throw open_error{fmt::format("Failed to open file '{}'.\n   Reason: {}",
                                   path.string_view(),
                                   std::system_category()
                                      .default_error_condition(system_error)
                                      .message()),
                       map_os_open_error_code(system_error)};
   }

   LARGE_INTEGER file_size = {};

   [[unlikely]] if (not GetFileSizeEx(file.get(), &file_size)) {
      const DWORD system_error = GetLastError();

      throw open_error{fmt::format("Couldn't get size of file '{}'.\n",
                                   path.string_view()),
                       map_os_open_error_code(system_error)};
   }

   [[unlikely]] if (file_size.QuadPart > std::numeric_limits<DWORD>::max()) {
      throw open_error{fmt::format("File '{}' is too big to read.\n   Max "
                                   "readable size is 4294967295, file is {}.",
                                   path.string_view(), file_size.QuadPart)};
   }

   Out bytes;
   bytes.resize(file_size.QuadPart);

   DWORD read_bytes = 0;
   if (not ReadFile(file.get(), bytes.data(), static_cast<DWORD>(bytes.size()),
                    &read_bytes, nullptr)) {
      const DWORD system_error = GetLastError();

      throw read_error{fmt::format(
         "Failed to read file '{}'.\n   Reason: {}\n   Bytes Read: {}/{}",
         path.string_view(),
         std::system_category().default_error_condition(system_error).message(),
         read_bytes, file_size.QuadPart)};
   }

   return bytes;
}

}

auto read_file_to_bytes(const io::path& path) -> std::vector<std::byte>
{
   return read_file_impl<std::vector<std::byte>>(path);
}

auto read_file_to_chars(const io::path& path) -> std::vector<char>
{
   return read_file_impl<std::vector<char>>(path);
}

auto read_file_to_string(const io::path& path) -> std::string
{
   return read_file_impl<std::string>(path);
}

}