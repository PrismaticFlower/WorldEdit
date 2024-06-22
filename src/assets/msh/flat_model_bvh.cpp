
#include "flat_model_bvh.hpp"
#include "flat_model.hpp"
#include "math/bvh.hpp"

namespace we::assets::msh {

flat_model_bvh::flat_model_bvh(std::span<mesh> meshes) noexcept
{
   _bvhs.reserve(meshes.size());

   for (mesh& mesh : meshes) {
      _bvhs.emplace_back(mesh.triangles, mesh.positions,
                         bvh_flags{.backface_cull =
                                      not are_flags_set(mesh.material.flags,
                                                        material_flags::transparent_doublesided)});
   }
}

auto flat_model_bvh::query(const float3 ray_origin, const float3 ray_direction) const noexcept
   -> std::optional<ray_hit>
{
   std::optional<ray_hit> closest_hit = std::nullopt;
   float t = FLT_MAX;

   for (const bvh& bvh : _bvhs) {
      if (auto hit = bvh.raycast(ray_origin, ray_direction, t);
          hit and hit->distance < t) {
         closest_hit = ray_hit{.distance = hit->distance, .normal = hit->normal};
         t = hit->distance;
      }
   }

   return closest_hit;
}

auto flat_model_bvh::get_debug_boxes() const noexcept
   -> std::vector<std::vector<math::bounding_box>>
{
   std::vector<std::vector<math::bounding_box>> boxes;
   boxes.reserve(_bvhs.size());

   for (const bvh& bvh : _bvhs) {
      boxes.emplace_back(bvh.get_debug_boxes());
   }

   return boxes;
}

flat_model_bvh::flat_model_bvh() noexcept = default;

flat_model_bvh::flat_model_bvh(flat_model_bvh&&) noexcept = default;
auto flat_model_bvh::operator=(flat_model_bvh&&) noexcept -> flat_model_bvh& = default;

flat_model_bvh::~flat_model_bvh() = default;

}