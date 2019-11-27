
#include <utility/read_file.hpp>

#include <exceptions.hpp>

#include <fstream>
#include <string_view>

#include <fmt/format.h>

using namespace std::literals;

namespace sk::utility {

auto read_file_to_bytes(const std::filesystem::path& path) -> std::vector<std::byte>
{
   std::ifstream file{path, std::ios::binary | std::ios::ate};

   if (file.bad() or file.fail()) {
      throw file_load_failure{"Failed to open file!", path.filename().string(),
                              fmt::format("Failed to open file '{}'!"sv,
                                          path.string())};
   }

   const auto size = file.tellg();

   std::vector<std::byte> bytes;
   bytes.resize(size);

   file.seekg(0, std::ios::beg);
   file.read(reinterpret_cast<char*>(bytes.data()), size);

   if (file.bad()) {
      throw file_load_failure{"Failed to read file!", path.filename().string(),
                              fmt::format("Failed to read file '{}'! Expected file size '{}'."sv,
                                          path.string(), size)};
   }

   return bytes;
}

}
