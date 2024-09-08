#pragma once

#include "../entity_group.hpp"

#include "output_stream.hpp"

#include <filesystem>
#include <string_view>

namespace we::world {

/// @brief Loads an entity group.
/// @param entity_group_data A view of a string containing the entity group's data.
/// @param output The output stream for warnings and errors.
/// @return The loaded entity group.
auto load_entity_group_from_string(const std::string_view entity_group_data,
                                   output_stream& output) -> entity_group;

/// @brief Loads an entity group.
/// @param path The path to the entity group.
/// @param output The output stream for warnings and errors.
/// @return The loaded entity group.
auto load_entity_group(const std::filesystem::path& path, output_stream& output)
   -> entity_group;

}
