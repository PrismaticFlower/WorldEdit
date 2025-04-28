#pragma once

#include "../blocks.hpp"

#include "io/path.hpp"

namespace we::world {

auto save_blocks_meshes(const io::path& output_directory,
                        const std::string_view world_name, const blocks& blocks)
   -> std::size_t;

}