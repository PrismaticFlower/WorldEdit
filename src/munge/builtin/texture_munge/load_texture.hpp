#pragma once

#include "options.hpp"
#include "texture.hpp"

#include "io/path.hpp"

namespace we::munge {

struct load_texture_info {
   texture_type type = texture_type::_2d;
   uint32 mip_levels = 0;
   uint32 depth = 0;
};

struct load_texture_result {
   texture texture;
   texture_traits traits;
};

auto load_texture(const io::path& path, const load_texture_info& info)
   -> load_texture_result;

}