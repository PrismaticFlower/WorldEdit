
#include "flat_model_terrain_cut_bvh.hpp"
#include "flat_model.hpp"
#include "math/bvh.hpp"
#include "math/vector_funcs.hpp"

#include <vector>

namespace we::assets::msh {

flat_model_terrain_cut_bvh::flat_model_terrain_cut_bvh(
   std::span<flat_model_terrain_cut> cut_meshes) noexcept
{
   _bvhs.reserve(cut_meshes.size());

   for (flat_model_terrain_cut& cut : cut_meshes) {
      _bvhs.emplace_back(cut.triangles, cut.positions,
                         bvh_flags{.backface_cull = false});
   }
}

auto flat_model_terrain_cut_bvh::count_intersections(const float3 ray_origin_start,
                                                     const float3 ray_direction) const noexcept
   -> uint32
{
   uint32 intersections = 0;

   const float ray_pad = 0.001f;

   float3 ray_origin = ray_origin_start;

   for (std::size_t i = 0; i < _bvhs.size(); ++i) {
      const bvh& bvh = _bvhs[i];

      auto hit = bvh.raycast(ray_origin, ray_direction, FLT_MAX);

      if (not hit) continue;

      ray_origin += (hit->distance + ray_pad) * ray_direction;

      intersections += 1;
   }

   return intersections;
}

flat_model_terrain_cut_bvh::flat_model_terrain_cut_bvh() noexcept = default;

flat_model_terrain_cut_bvh::flat_model_terrain_cut_bvh(
   flat_model_terrain_cut_bvh&&) noexcept = default;

auto flat_model_terrain_cut_bvh::operator=(flat_model_terrain_cut_bvh&&) noexcept
   -> flat_model_terrain_cut_bvh& = default;

flat_model_terrain_cut_bvh::~flat_model_terrain_cut_bvh() = default;

}