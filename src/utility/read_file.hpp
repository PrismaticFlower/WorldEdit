#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace we::utility {

auto read_file_to_bytes(const std::filesystem::path& path) -> std::vector<std::byte>;

auto read_file_to_string(const std::filesystem::path& path) -> std::string;

}
