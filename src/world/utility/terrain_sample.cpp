#include "terrain_sample.hpp"

#include "math/iq_intersectors.hpp"
#include "math/vector_funcs.hpp"

#include <algorithm>

namespace we::world {

auto sample_terrain_normal(const terrain& terrain, const float3& positionWS) noexcept
   -> float3
{
   if (not terrain.active_flags.terrain) return {0.0f, 1.0f, 0.0f};

   const int32 terrain_half_length = terrain.length / 2;

   const float2 point = round(float2{positionWS.x / terrain.grid_scale,
                                     positionWS.z / terrain.grid_scale});

   const auto get_vertex = [&](const float2 p) {
      return float3{(p.x) * terrain.grid_scale,
                    terrain.height_map[{std::clamp(static_cast<int32>(p.x) + terrain_half_length,
                                                   0, terrain.length - 1),
                                        std::clamp(static_cast<int32>(p.y) + terrain_half_length - 1,
                                                   0, terrain.length - 1)}] *
                       terrain.height_scale,
                    (p.y) * terrain.grid_scale};
   };

   const float3 ray_originWS = {positionWS.x, terrain.height_scale * (INT16_MAX + 1),
                                positionWS.z};

   const std::array vertices{get_vertex(point + float2{0.0f, 0.0f}),
                             get_vertex(point + float2{1.0f, 0.0f}),
                             get_vertex(point + float2{1.0f, 1.0f}),
                             get_vertex(point + float2{0.0f, 1.0f})};

   const bool odd_quad_split =
      (std::clamp(static_cast<int32>(point.y) + terrain_half_length - 1, 0,
                  terrain.length - 1) &
       1) != 0;

   if (odd_quad_split) {
      if (const float intersection =
             triIntersect(ray_originWS, {0.0f, -1.0f, 0.0f}, vertices[0],
                          vertices[3], vertices[1])
                .x;
          intersection >= 0.0f) {
         return normalize(cross(vertices[3] - vertices[0], vertices[1] - vertices[0]));
      }
      else {
         return normalize(cross(vertices[2] - vertices[3], vertices[1] - vertices[3]));
      }
   }
   else {
      if (const float intersection =
             triIntersect(ray_originWS, {0.0f, -1.0f, 0.0f}, vertices[0],
                          vertices[1], vertices[2])
                .x;
          intersection >= 0.0f) {
         return normalize(cross(vertices[0] - vertices[1], vertices[2] - vertices[1]));
      }
      else {
         return normalize(cross(vertices[0] - vertices[2], vertices[3] - vertices[2]));
      }
   }
}

}