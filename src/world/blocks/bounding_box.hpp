#pragma once

#include "../blocks.hpp"

#include "math/bounding_box.hpp"

namespace we::world {

auto get_bounding_box(const block_description_box& box) noexcept -> math::bounding_box;

}
