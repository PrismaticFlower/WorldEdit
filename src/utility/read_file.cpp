
#include "read_file.hpp"
#include "exceptions.hpp"

#include <fstream>
#include <string_view>

#include <fmt/format.h>

using namespace std::literals;

namespace we::utility {

auto read_file_to_bytes(const std::filesystem::path& path) -> std::vector<std::byte>
{
   std::ifstream file{path, std::ios::binary};

   if (file.bad() or file.fail()) {
      throw file_load_failure{"Failed to open file!", path.filename().string(),
                              fmt::format("Failed to open file '{}'!"sv,
                                          path.string())};
   }

   const auto size = std::filesystem::file_size(path);

   std::vector<std::byte> bytes;
   bytes.resize(size);

   file.read(reinterpret_cast<char*>(bytes.data()), size);

   if (file.bad()) {
      throw file_load_failure{"Failed to read file!", path.filename().string(),
                              fmt::format("Failed to read file '{}'! Expected file size '{}'."sv,
                                          path.string(), size)};
   }

   return bytes;
}

auto read_file_to_string(const std::filesystem::path& path) -> std::string
{
   std::ifstream file{path, std::ios::binary};

   if (file.bad() or file.fail()) {
      throw file_load_failure{"Failed to open file!", path.filename().string(),
                              fmt::format("Failed to open file '{}'!"sv,
                                          path.string())};
   }

   const auto size = std::filesystem::file_size(path);

   std::string str;
   str.resize(size);

   file.read(reinterpret_cast<char*>(str.data()), size);

   if (file.bad()) {
      throw file_load_failure{"Failed to read file!", path.filename().string(),
                              fmt::format("Failed to read file '{}'! Expected file size '{}'."sv,
                                          path.string(), size)};
   }

   return str;
}

}
