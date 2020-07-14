#pragma once

#include "texture.hpp"

namespace sk::assets::texture {

auto generate_normal_maps(const texture& input, const float scale) -> texture;

}