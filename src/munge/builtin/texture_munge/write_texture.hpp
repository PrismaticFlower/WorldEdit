#pragma once

#include "options.hpp"
#include "texture_transmuted.hpp"

#include "io/path.hpp"

namespace we::munge {

struct write_texture_options {
   texture_type type = texture_type::_2d;
   uint32 detail_bias = 0;
};

void write_texture(const io::path& output_file_path,
                   const texture_transmuted_view& texture,
                   const write_texture_options& options);

}