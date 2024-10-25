#pragma once

#include "path.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace we::io {

/// @brief Reads an entire file into memory and returns it.
/// @param path The file path.
/// @return The contents of the file.
auto read_file_to_bytes(const path& path) -> std::vector<std::byte>;

/// @brief Reads an entire file into memory and returns it.
/// @param path The file path.
/// @return The contents of the file.
auto read_file_to_chars(const path& path) -> std::vector<char>;

/// @brief Reads an entire file into memory and returns it as a string. Text encoding is assumed to be UTF-8, if it isn't this isn't the function to be using.
/// @param path The file path.
/// @return The contents of the file.
auto read_file_to_string(const path& path) -> std::string;

}
