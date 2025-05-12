#pragma once

#include "../blocks.hpp"

#include "math/bounding_box.hpp"

namespace we::world {

auto get_bounding_box(const block_description_box& box) noexcept -> math::bounding_box;

auto get_bounding_box(const block_description_ramp& ramp) noexcept -> math::bounding_box;

auto get_bounding_box(const block_description_quad& quad) noexcept -> math::bounding_box;

auto get_bounding_box(const block_description_cylinder& cylinder) noexcept
   -> math::bounding_box;

auto get_bounding_box(const blocks& blocks, const block_type type,
                      const uint32 block_index) noexcept -> math::bounding_box;

}
