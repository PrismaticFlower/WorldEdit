#pragma once

#include "frustum.hpp"
#include "math/bounding_box.hpp"
#include "world_mesh_list.hpp"

#include <span>
#include <vector>

namespace we::graphics {

void cull_objects_scalar(const frustum& frustum,
                         std::span<const math::bounding_box> bbox,
                         std::span<const world_pipeline_flags> pipeline_flags,
                         std::vector<uint16>& out_opaque_list,
                         std::vector<uint16>& out_transparent_list) noexcept;

void cull_objects_avx2(const frustum& frustum, std::span<const float> bbox_min_x,
                       std::span<const float> bbox_min_y,
                       std::span<const float> bbox_min_z,
                       std::span<const float> bbox_max_x,
                       std::span<const float> bbox_max_y,
                       std::span<const float> bbox_max_z,
                       std::span<const world_pipeline_flags> pipeline_flags,
                       std::vector<uint16>& out_opaque_list,
                       std::vector<uint16>& out_transparent_list) noexcept;

void cull_objects_shadow_cascade_scalar(const frustum& frustum,
                                        std::span<const math::bounding_box> bbox,
                                        std::span<const world_pipeline_flags> pipeline_flags,
                                        std::vector<uint16>& out_list) noexcept;

void cull_objects_shadow_cascade_avx2(
   const frustum& frustum, std::span<const float> bbox_min_x,
   std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
   std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
   std::span<const float> bbox_max_z,
   std::span<const world_pipeline_flags> pipeline_flags,
   std::vector<uint16>& out_list) noexcept;

}