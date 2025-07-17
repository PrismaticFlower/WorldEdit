#include "bvh.hpp"

namespace we::world {

block_bvh::block_bvh() noexcept = default;

block_bvh::block_bvh(std::vector<std::array<uint16, 3>> triangles,
                     std::vector<float3> vertices) noexcept
   : _triangles{std::move(triangles)}, _vertices{std::move(vertices)}

{
   _bvh = bvh{_triangles, _vertices, bvh_flags{}};
}

block_bvh::block_bvh(const block_bvh& other)
{
   _vertices = other._vertices;
   _triangles = other._triangles;
   _bvh = bvh{_triangles, _vertices, bvh_flags{}};
}

auto block_bvh::operator=(const block_bvh& other) noexcept -> block_bvh&
{
   _vertices = other._vertices;
   _triangles = other._triangles;
   _bvh = bvh{_triangles, _vertices, bvh_flags{}};

   return *this;
}

auto block_bvh::raycast(const float3& ray_origin, const float3& ray_direction,
                        const float max_distance, const bvh_ray_flags flags) const noexcept
   -> std::optional<bvh::ray_hit>
{
   return _bvh.raycast(ray_origin, ray_direction, max_distance, flags);
}

bool block_bvh::intersects(const frustum& frustum) const noexcept
{
   return _bvh.intersects(frustum);
}

}