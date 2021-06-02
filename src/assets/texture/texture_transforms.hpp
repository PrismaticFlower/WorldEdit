#pragma once

#include "texture.hpp"

namespace we::assets::texture {

auto generate_normal_maps(const texture& input, const float scale) -> texture;

}
