#pragma once

#include "../../blocks.hpp"

#include <optional>

namespace we::world {

auto find_block(const blocks& blocks, const block_id id) -> std::optional<uint32>;

/// @brief Try to find a material that is equivalent to the passed in one.
/// @param blocks The blocks.
/// @param material The material to compare for equivalence.
/// @return The index of the material or nullopt.
auto find_block_equivalent_material(const blocks& blocks, const block_material& material)
   -> std::optional<uint8>;

/// @brief Try to find an empty material.
/// @param blocks The blocks.
/// @return The index of the material or nullopt.
auto find_block_empty_material(const blocks& blocks) -> std::optional<uint8>;

}
