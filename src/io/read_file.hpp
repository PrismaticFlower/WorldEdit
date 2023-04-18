#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace we::io {

/// @brief Reads an entire file into memory and returns it.
/// @param path The file path.
/// @return The contents of the file.
auto read_file_to_bytes(const std::filesystem::path& path) -> std::vector<std::byte>;

/// @brief Reads an entire file into memory and returns it.
/// @param path The file path.
/// @return The contents of the file.
auto read_file_to_chars(const std::filesystem::path& path) -> std::vector<char>;

/// @brief Reads an entire file into memory and returns it as a string. Text encoding is assumed to be UTF-8, if it isn't this isn't the function to be using.
/// @param path The file path.
/// @return The contents of the file.
auto read_file_to_string(const std::filesystem::path& path) -> std::string;

/// @brief Check if a file is readable. Useless you say? Mostly except for the very specific thing we need it for in asset_libraries.cpp
/// @param path The file path.
/// @return True if the file can currently be read, false if it can't. This information may already be out of date when the call returns so again it is mostly useless.
bool is_readable(const std::filesystem::path& path) noexcept;

}
