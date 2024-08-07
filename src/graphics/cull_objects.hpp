#pragma once

#include "math/bounding_box.hpp"
#include "math/frustum.hpp"
#include "world_mesh_list.hpp"

#include <span>
#include <vector>

namespace we::graphics {

void cull_objects_scalar(const frustum& frustum,
                         std::span<const math::bounding_box> bbox,
                         std::vector<uint16>& out_list) noexcept;

void cull_objects_avx2(const frustum& frustum, std::span<const float> bbox_min_x,
                       std::span<const float> bbox_min_y,
                       std::span<const float> bbox_min_z,
                       std::span<const float> bbox_max_x,
                       std::span<const float> bbox_max_y,
                       std::span<const float> bbox_max_z,
                       std::vector<uint16>& out_list) noexcept;

void cull_objects_shadow_cascade_scalar(const frustum& frustum,
                                        std::span<const math::bounding_box> bbox,
                                        std::vector<uint16>& out_list) noexcept;

void cull_objects_shadow_cascade_avx2(
   const frustum& frustum, std::span<const float> bbox_min_x,
   std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
   std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
   std::span<const float> bbox_max_z, std::vector<uint16>& out_list) noexcept;

}