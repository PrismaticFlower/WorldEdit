#pragma once

#include "requirement_list.hpp"

#include <filesystem>

namespace we::assets::req {

/// @brief Load requirements from a string.
/// @param str The string to load from.
/// @return The requirements.
auto read(std::string_view str) -> std::vector<requirement_list>;

/// @brief Save requires to a file.
/// @param path The path to save the file at.
/// @param requirements The requirements to save.
void save(const std::filesystem::path& path,
          const std::vector<const requirement_list> requirements);
}
