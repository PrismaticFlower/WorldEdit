
#include "flat_model_bvh.hpp"
#include "flat_model.hpp"
#include "math/intersectors.hpp"

#include <vector>

#include <FastBVH.h>

namespace we::assets::msh {

namespace detail {

class flat_model_bvh_impl {
public:
   void build(std::span<mesh> meshes) noexcept
   {
      _bvhs.reserve(meshes.size());

      for (mesh& mesh : meshes) {
         FastBVH::BuildStrategy<float, 1> build_strategy;

         _bvhs.emplace_back(
            build_strategy(mesh.triangles, [&](const std::array<uint16, 3>& tri) {
               float3 min{std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max()};
               float3 max{std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest()};

               for (auto& v : tri) {
                  min = we::min(mesh.positions[v], min);
                  max = we::max(mesh.positions[v], max);
               }

               return FastBVH::BBox{FastBVH::Vector3{min.x, min.y, min.z},
                                    FastBVH::Vector3{max.x, max.y, max.z}};
            }));
      }

      _meshes = meshes;
   }

   auto query(const float3 ray_origin, const float3 ray_direction) const noexcept
      -> std::optional<ray_hit>
   {
      std::optional<ray_hit> hit = std::nullopt;

      for (std::size_t i = 0; i < _bvhs.size(); ++i) {
         const FastBVH::BVH<float, std::array<uint16, 3>>& bvh = _bvhs[i];
         const mesh& mesh = _meshes[i];

         FastBVH::Traverser traverser{
            bvh, [&](const std::array<uint16, 3>& tri, const FastBVH::Ray<float>& ray) {
               float3 intersect =
                  triIntersect({ray.o.x, ray.o.y, ray.o.z},
                               {ray.d.x, ray.d.y, ray.d.z}, mesh.positions[tri[0]],
                               mesh.positions[tri[1]], mesh.positions[tri[2]]);

               if (intersect.x < 0.0f) {
                  return FastBVH::Intersection<float, std::array<uint16, 3>>{};
               }

               float3 normal =
                  cross(mesh.positions[tri[1]] - mesh.positions[tri[0]],
                        mesh.positions[tri[2]] - mesh.positions[tri[0]]);

               if (dot(-ray_direction, normal) < 0.0f and
                   not are_flags_set(mesh.material.flags,
                                     material_flags::transparent_doublesided)) {
                  return FastBVH::Intersection<float, std::array<uint16, 3>>{};
               }

               return FastBVH::Intersection<float, std::array<uint16, 3>>{
                  .t = intersect.x,
                  .object = &tri,
                  .normal = {normal.x, normal.y, normal.z}};
            }};

         FastBVH::Intersection intersection = traverser.traverse(
            FastBVH::Ray{FastBVH::Vector3{ray_origin.x, ray_origin.y, ray_origin.z},
                         FastBVH::Vector3{ray_direction.x, ray_direction.y,
                                          ray_direction.z}});

         if (not intersection) continue;

         if (hit and hit->distance > intersection.t) {
            hit = ray_hit{.distance = intersection.t,
                          .normal =
                             float3{intersection.normal.x, intersection.normal.y,
                                    intersection.normal.z}};
         }
         else if (not hit) {
            hit = ray_hit{.distance = intersection.t,
                          .normal =
                             float3{intersection.normal.x, intersection.normal.y,
                                    intersection.normal.z}};
         }
      }

      return hit;
   }

private:
   std::vector<FastBVH::BVH<float, std::array<uint16, 3>>> _bvhs;
   std::span<const mesh> _meshes;
};

}

flat_model_bvh::flat_model_bvh() noexcept
{
   _impl = std::make_unique<detail::flat_model_bvh_impl>();
}

flat_model_bvh::flat_model_bvh(flat_model_bvh&&) noexcept = default;

flat_model_bvh::~flat_model_bvh() = default;

void flat_model_bvh::build(std::span<mesh> meshes) noexcept
{
   _impl->build(meshes);
}

auto flat_model_bvh::query(const float3 ray_origin, const float3 ray_direction) const noexcept
   -> std::optional<ray_hit>
{
   return _impl->query(ray_origin, ray_direction);
}

}