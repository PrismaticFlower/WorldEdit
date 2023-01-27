

#include "terrain_collision.hpp"
#include "math/intersectors.hpp"
#include "terrain.hpp"

#include <utility>
#include <vector>

#include <FastBVH.h>

#include <range/v3/view.hpp>

namespace we::assets::terrain {

namespace detail {

class terrain_collision_impl {
public:
   terrain_collision_impl(const terrain& terrain) noexcept
   {
      _half_world_length = terrain.length * terrain.grid_scale / 2.0f;
      _grid_scale = terrain.grid_scale;
      _height_scale = terrain.height_scale;
      _max_length = static_cast<uint16>(terrain.length - 1);
      _terrain = &terrain;

      _indices.clear();
      _indices.reserve(terrain.length * terrain.length);

      for (uint16 z = 0; z < terrain.length; ++z) {
         for (uint16 x = 0; x < terrain.length; ++x) {
            _indices.push_back({x, z});
         }
      }

      FastBVH::BuildStrategy<float, 1> build_strategy;

      _bvh = build_strategy(_indices, [&](const std::array<uint16, 2>& point) {
         float3 min{std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max()};
         float3 max{std::numeric_limits<float>::lowest(),
                    std::numeric_limits<float>::lowest(),
                    std::numeric_limits<float>::lowest()};

         for (auto& v : get_quad(point[0], point[1])) {
            min = we::min(v, min);
            max = we::max(v, max);
         }

         return FastBVH::BBox{FastBVH::Vector3{min.x, min.y, min.z},
                              FastBVH::Vector3{max.x, max.y, max.z}};
      });
   }

   auto raycast(const float3 ray_origin, const float3 ray_direction) const noexcept
      -> std::optional<ray_hit>
   {
      if (not _bvh) return std::nullopt;

      FastBVH::Traverser traverser{
         *_bvh, [&](const std::array<uint16, 2>& point, const FastBVH::Ray<float>& ray) {
            const std::array<float3, 4> quad = get_quad(point[0], point[1]);

            float3 intersect = quadIntersect({ray.o.x, ray.o.y, ray.o.z},
                                             {ray.d.x, ray.d.y, ray.d.z},
                                             quad[0], quad[1], quad[2], quad[3]);

            if (intersect.x < 0.0f) {
               return FastBVH::Intersection<float, std::array<uint16, 2>>{};
            }

            return FastBVH::Intersection<float, std::array<uint16, 2>>{
               .t = intersect.x, .object = &point};
         }};

      FastBVH::Intersection intersection = traverser.traverse(
         FastBVH::Ray{FastBVH::Vector3{ray_origin.x, ray_origin.y, ray_origin.z},
                      FastBVH::Vector3{ray_direction.x, ray_direction.y,
                                       ray_direction.z}});

      if (not intersection) return std::nullopt;

      return ray_hit{.distance = intersection.t};
   }

   auto get_quad(uint16 x, uint16 z) const noexcept -> std::array<float3, 4>
   {
      const auto get_vertex = [&](const uint16 x, const uint16 z) {
         return float3{(x * _grid_scale) - _half_world_length,
                       _terrain->height_map[{std::clamp(x, uint16{0}, _max_length),
                                             std::clamp(z, uint16{0}, _max_length)}] *
                          _height_scale,
                       (z * _grid_scale) - _half_world_length + _grid_scale};
      };

      return {get_vertex(x, z),         //
              get_vertex(x + 1, z),     //
              get_vertex(x + 1, z + 1), //
              get_vertex(x, z + 1)};
   }

private:
   std::optional<FastBVH::BVH<float, std::array<uint16, 2>>> _bvh;
   std::vector<std::array<uint16, 2>> _indices;

   float _half_world_length = 0.0f;
   float _grid_scale = 0.0f;
   float _height_scale = 0.0f;

   uint16 _max_length = 0;

   const terrain* _terrain = nullptr;
};

}

terrain_collision::terrain_collision() = default;

terrain_collision::terrain_collision(const terrain& terrain) noexcept
{
   _impl = std::make_unique<detail::terrain_collision_impl>(terrain);
}

terrain_collision::terrain_collision(terrain_collision&&) noexcept = default;

auto terrain_collision::operator=(terrain_collision&&) noexcept
   -> terrain_collision& = default;

terrain_collision::~terrain_collision() = default;

auto terrain_collision::raycast(const float3 ray_origin,
                                const float3 ray_direction) const noexcept
   -> std::optional<ray_hit>
{
   if (not _impl) return std::nullopt;

   return _impl->raycast(ray_origin, ray_direction);
}

}