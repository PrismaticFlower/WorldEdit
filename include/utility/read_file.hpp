#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

namespace sk::utility {

auto read_file_to_bytes(const std::filesystem::path& path) -> std::vector<std::byte>;

}
