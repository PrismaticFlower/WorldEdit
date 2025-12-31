#pragma once

#include "types.hpp"

#include <string_view>

namespace we::munge {

/// @brief The hash used acorss various munged files. Bitwise ors each byte with 0x20.
/// @param str The str to hash.
/// @return The hash.
auto bf_fnv_1a_hash(const std::string_view str) noexcept -> uint32;

}
