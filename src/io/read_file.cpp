
#include "read_file.hpp"
#include "error.hpp"

#include <fmt/core.h>
#include <wil/resource.h>

namespace we::io {

namespace {

template<typename Out>
auto read_file_impl(const std::filesystem::path& path) -> Out
{
   static_assert(sizeof(Out::value_type) == 1, "See bytes.resize(file_size);");

   wil::unique_hfile file{
      CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                  FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, nullptr)};

   if (not file) {
      const DWORD system_error = GetLastError();

      throw open_error{fmt::format(
         "Failed to open file '{}'.\n   Reason: {}", path.string(),
         std::system_category().default_error_condition(system_error).message())};
   }

   const std::size_t file_size = std::filesystem::file_size(path.c_str());

   [[unlikely]] if (file_size > std::numeric_limits<DWORD>::max()) {
      throw open_error{fmt::format("File '{}' is too big to read.\n   Max "
                                   "readable size is 4294967295, file is {}.",
                                   path.string(), file_size)};
   }

   Out bytes;
   bytes.resize(file_size);

   DWORD read_bytes = 0;
   if (not ReadFile(file.get(), bytes.data(), static_cast<DWORD>(bytes.size()),
                    &read_bytes, nullptr)) {
      const DWORD system_error = GetLastError();

      throw open_error{fmt::format(
         "Failed to read file '{}'.\n   Reason: {}\n   Bytes Read: {}/{}", path.string(),
         std::system_category().default_error_condition(system_error).message(),
         read_bytes, file_size)};
   }

   return bytes;
}

}

auto read_file_to_bytes(const std::filesystem::path& path) -> std::vector<std::byte>
{
   return read_file_impl<std::vector<std::byte>>(path);
}

auto read_file_to_chars(const std::filesystem::path& path) -> std::vector<char>
{
   return read_file_impl<std::vector<char>>(path);
}

auto read_file_to_string(const std::filesystem::path& path) -> std::string
{
   return read_file_impl<std::string>(path);
}

}