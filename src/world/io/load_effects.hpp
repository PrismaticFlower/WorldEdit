#pragma once

#include "../effects.hpp"

#include "output_stream.hpp"

namespace we::world {

/// @brief Loads world effects from a string.
/// @param str The string containing the world.FX file contents.
/// @param output The output stream for warnings and errors.
/// @return The loaded world effects.
auto load_effects(const std::string_view str, output_stream& output) -> effects;

}