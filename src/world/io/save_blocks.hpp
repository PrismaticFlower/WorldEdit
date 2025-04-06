#pragma once

#include "../blocks.hpp"

#include "output_stream.hpp"

#include "io/path.hpp"

namespace we::world {

void save_blocks(const io::path& path, const blocks& blocks);

}