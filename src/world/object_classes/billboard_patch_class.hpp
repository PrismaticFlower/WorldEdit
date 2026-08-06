#pragma once

#include "types.hpp"

#include "math/bounding_box.hpp"

#include <memory>
#include <span>
#include <string>

namespace we::world {

struct billboard_patch_vertex {
   std::array<int16, 3> position = {};
   int16 darkness = 0;
   uint32 texcoords = 0;
};

struct billboard_patch_class {
   virtual ~billboard_patch_class() = default;

   virtual void update(double delta_time) noexcept = 0;

   virtual void get_quads(
      const float4x4& world_matrix, const float3& light_direction, const bool animated,
      std::span<std::array<billboard_patch_vertex, 4>> out) const noexcept = 0;

   virtual auto num_particles() const noexcept -> std::size_t = 0;

   virtual auto height_scale() const noexcept -> float = 0;

   virtual auto bbox() const noexcept -> const math::bounding_box& = 0;

   virtual auto texture() const noexcept -> const std::string& = 0;
};

}