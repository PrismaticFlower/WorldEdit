#pragma once

#include "world_mesh_list.hpp"

#include "math/bounding_box.hpp"
#include "math/frustum.hpp"

#include "world/active_elements.hpp"

#include <span>
#include <vector>

namespace we::graphics {

auto cull_objects(const frustum& frustum, std::span<const float> bbox_min_x,
                  std::span<const float> bbox_min_y,
                  std::span<const float> bbox_min_z, std::span<const float> bbox_max_x,
                  std::span<const float> bbox_max_y, std::span<const float> bbox_max_z,
                  std::span<uint16> out_list) noexcept -> std::span<uint16>;

auto cull_objects(const frustum& frustum, std::span<const float> bbox_min_x,
                  std::span<const float> bbox_min_y,
                  std::span<const float> bbox_min_z, std::span<const float> bbox_max_x,
                  std::span<const float> bbox_max_y, std::span<const float> bbox_max_z,
                  std::span<uint32> out_list) noexcept -> std::span<uint32>;

auto cull_objects(const frustum& frustum, std::span<const float> bbox_min_x,
                  std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
                  std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
                  std::span<const float> bbox_max_z, std::span<const bool> hidden,
                  std::span<const int8> layers, const world::active_layers active_layers,
                  std::span<uint32> out_list) noexcept -> std::span<uint32>;

auto cull_objects_shadow_cascade(
   const frustum& frustum, std::span<const float> bbox_min_x,
   std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
   std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
   std::span<const float> bbox_max_z, std::span<uint16> out_list) noexcept
   -> std::span<uint16>;

auto cull_objects_shadow_cascade(
   const frustum& frustum, std::span<const float> bbox_min_x,
   std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
   std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
   std::span<const float> bbox_max_z, std::span<const bool> hidden,
   std::span<const int8> layers, const world::active_layers active_layers,
   std::span<uint32> out_list) noexcept -> std::span<uint32>;

auto cull_objects_shadow_cascade(
   const frustum& frustum, std::span<const float> bbox_min_x,
   std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
   std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
   std::span<const float> bbox_max_z, std::span<uint32> out_list) noexcept
   -> std::span<uint32>;

}