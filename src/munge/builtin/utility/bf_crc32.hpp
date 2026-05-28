#pragma once

#include "types.hpp"

#include <string_view>

namespace we::munge {

/// @brief Calculates the CRC used as hash in .msh and animation files.
/// @param str The str to calculate the CRC for.
/// @return The CRC.
auto bf_crc32(const std::string_view str) noexcept -> uint32;

}
