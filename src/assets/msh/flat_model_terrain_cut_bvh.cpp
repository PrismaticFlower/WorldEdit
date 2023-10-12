
#include "flat_model_terrain_cut_bvh.hpp"
#include "flat_model.hpp"
#include "math/intersectors.hpp"

#include <vector>

#include <FastBVH.h>

namespace we::assets::msh {

namespace detail {

struct flat_model_terrain_cut_bvh_impl {
   void build(std::span<flat_model_terrain_cut> cut_meshes) noexcept
   {
      _bvhs.reserve(cut_meshes.size());

      for (flat_model_terrain_cut& cut : cut_meshes) {
         FastBVH::BuildStrategy<float, 1> build_strategy;

         _bvhs.emplace_back(
            build_strategy(cut.triangles, [&](const std::array<uint16, 3>& tri) {
               float3 min{std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max()};
               float3 max{std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest()};

               for (auto& v : tri) {
                  min = we::min(cut.positions[v], min);
                  max = we::max(cut.positions[v], max);
               }

               return FastBVH::BBox{FastBVH::Vector3{min.x, min.y, min.z},
                                    FastBVH::Vector3{max.x, max.y, max.z}};
            }));
      }

      _cut_meshes = cut_meshes;
   }

   auto count_intersections(const float3 ray_origin_start,
                            const float3 ray_direction) const noexcept -> uint32
   {
      uint32 intersections = 0;

      const float ray_pad = 0.001f;

      float3 ray_origin = ray_origin_start;

      for (std::size_t i = 0; i < _bvhs.size(); ++i) {
         const FastBVH::BVH<float, std::array<uint16, 3>>& bvh = _bvhs[i];
         const flat_model_terrain_cut& cut = _cut_meshes[i];

         FastBVH::Traverser traverser{
            bvh, [&](const std::array<uint16, 3>& tri, const FastBVH::Ray<float>& ray) {
               float3 intersect =
                  triIntersect({ray.o.x, ray.o.y, ray.o.z},
                               {ray.d.x, ray.d.y, ray.d.z}, cut.positions[tri[0]],
                               cut.positions[tri[1]], cut.positions[tri[2]]);

               if (intersect.x < 0.0f) {
                  return FastBVH::Intersection<float, std::array<uint16, 3>>{};
               }

               return FastBVH::Intersection<float, std::array<uint16, 3>>{
                  .t = intersect.x, .object = &tri};
            }};

         FastBVH::Intersection intersection = traverser.traverse(
            FastBVH::Ray{FastBVH::Vector3{ray_origin.x, ray_origin.y, ray_origin.z},
                         FastBVH::Vector3{ray_direction.x, ray_direction.y,
                                          ray_direction.z}});

         if (not intersection) continue;

         ray_origin += (intersection.t + ray_pad) * ray_direction;

         intersections += 1;
      }

      return intersections;
   }

private:
   std::vector<FastBVH::BVH<float, std::array<uint16, 3>>> _bvhs;
   std::span<const flat_model_terrain_cut> _cut_meshes;
};

}

flat_model_terrain_cut_bvh::flat_model_terrain_cut_bvh() noexcept
{
   _impl = std::make_unique<detail::flat_model_terrain_cut_bvh_impl>();
}

flat_model_terrain_cut_bvh::flat_model_terrain_cut_bvh(
   flat_model_terrain_cut_bvh&&) noexcept = default;

flat_model_terrain_cut_bvh::~flat_model_terrain_cut_bvh() = default;

void flat_model_terrain_cut_bvh::build(std::span<flat_model_terrain_cut> cut_meshes) noexcept
{
   _impl->build(cut_meshes);
}

auto flat_model_terrain_cut_bvh::count_intersections(const float3 ray_origin,
                                                     const float3 ray_direction) const noexcept
   -> uint32
{
   return _impl->count_intersections(ray_origin, ray_direction);
}

}