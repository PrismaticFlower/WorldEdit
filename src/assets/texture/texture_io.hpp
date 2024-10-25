#pragma once

#include "texture.hpp"

#include "io/path.hpp"

namespace we::assets::texture {

auto load_texture(const io::path& path) -> texture;

}
