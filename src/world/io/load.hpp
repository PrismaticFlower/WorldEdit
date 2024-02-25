#pragma once

#include "../world.hpp"
#include "output_stream.hpp"

#include <filesystem>

namespace we::world {

/// @brief Loads a world.
/// @param path The path to the world.
/// @param output The output stream for warnings and errors.
/// @return The loaded world.
auto load_world(const std::filesystem::path& path, output_stream& output) -> world;

}
