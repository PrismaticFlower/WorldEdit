#pragma once

#include "output_stream.hpp"
#include "world.hpp"

#include <filesystem>
#include <stdexcept>

namespace we::world {

/// @brief Exception thrown when loading a world fails unexpectedly.
class load_failure : public std::runtime_error {
   using std::runtime_error::runtime_error;
};

/// @brief Loads a world.
/// @param path The patht to the world.
/// @param output The output stream for warnings and errors.
/// @return The loaded world.
auto load_world(const std::filesystem::path& path, output_stream& output) -> world;

}
