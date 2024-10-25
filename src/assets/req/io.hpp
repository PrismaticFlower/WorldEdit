#pragma once

#include "io/path.hpp"
#include "requirement_list.hpp"

#include <span>

namespace we::assets::req {

/// @brief Load requirements from a string.
/// @param str The string to load from.
/// @return The requirements.
auto read(std::string_view str) -> std::vector<requirement_list>;

/// @brief Save requires to a file.
/// @param path The path to save the file at.
/// @param requirements The requirements to save.
void save(const io::path& path, const std::span<const requirement_list> requirements);
}
