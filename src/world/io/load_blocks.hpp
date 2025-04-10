#pragma once

#include "../blocks.hpp"
#include "layer_remap.hpp"

#include "io/path.hpp"

#include "output_stream.hpp"

namespace we::world {

/// @brief Loads blocks.
/// @param path The path to the .blk file
/// @param output The output stream for warnings and errors.
/// @return The loaded blocks.
auto load_blocks(const io::path& path, const layer_remap& layer_remap,
                 output_stream& output) -> blocks;

}