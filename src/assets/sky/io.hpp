#pragma once

#include "sky.hpp"

#include <string_view>

namespace we::assets::sky {

/// @brief Load the sky from the string.
/// @param str The string to load from.
/// @param platform The platform to load conditional properties for. (PC, PS2 or XBOX)
/// @return The sky config.
auto read(const std::string_view str, const std::string_view platform) -> config;

}
