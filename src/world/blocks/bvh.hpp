#pragma once

#include "mesh_vertex.hpp"

#include "math/bvh.hpp"

#include <array>
#include <span>
#include <vector>

namespace we::world {

struct block_bvh {
   block_bvh() noexcept;

   block_bvh(std::vector<std::array<uint16, 3>> triangles,
             std::vector<float3> vertices) noexcept;

   block_bvh(const block_bvh&);

   auto operator=(const block_bvh&) noexcept -> block_bvh&;

   block_bvh(block_bvh&&) = default;

   auto operator=(block_bvh&&) noexcept -> block_bvh& = default;

   [[nodiscard]] auto raycast(const float3& ray_origin,
                              const float3& ray_direction, const float max_distance,
                              const bvh_ray_flags flags = {}) const noexcept
      -> std::optional<bvh::ray_hit>;

   [[nodiscard]] bool intersects(const frustum& frustum) const noexcept;

private:
   std::vector<std::array<uint16, 3>> _triangles;
   std::vector<float3> _vertices;

   bvh _bvh;
};

}