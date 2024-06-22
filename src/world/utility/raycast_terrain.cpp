#include "raycast_terrain.hpp"
#include "math/iq_intersectors.hpp"
#include "math/vector_funcs.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace we::world {

namespace {

auto raycast_terrain_quad(const float3 ray_origin, const float3 ray_direction,
                          const float2 point, const terrain& terrain) noexcept -> float
{
   const int32 terrain_half_length = terrain.length / 2;

   const auto get_vertex = [&](const float2 p) {
      return float3{(p.x) * terrain.grid_scale,
                    terrain.height_map[{std::clamp(static_cast<int32>(p.x) + terrain_half_length,
                                                   0, terrain.length - 1),
                                        std::clamp(static_cast<int32>(p.y) + terrain_half_length - 1,
                                                   0, terrain.length - 1)}] *
                       terrain.height_scale,
                    (p.y) * terrain.grid_scale};
   };

   const std::array vertices{get_vertex(point + float2{0.0f, 0.0f}),
                             get_vertex(point + float2{1.0f, 0.0f}),
                             get_vertex(point + float2{1.0f, 1.0f}),
                             get_vertex(point + float2{0.0f, 1.0f})};

   if (const float intersection = triIntersect(ray_origin, ray_direction,
                                               vertices[0], vertices[1], vertices[2])
                                     .x;
       intersection >= 0.0f) {
      return intersection;
   }

   return triIntersect(ray_origin, ray_direction, vertices[0], vertices[2],
                       vertices[3])
      .x;
}

auto make_ray_start(const float3 ray_origin, const float3 ray_direction,
                    const terrain& terrain) -> float3
{
   const float terrain_half_length = (terrain.length / 2.0f) * terrain.grid_scale;

   const bool x_inside =
      ray_origin.x < terrain_half_length and ray_origin.x > -terrain_half_length;
   const bool z_inside =
      ray_origin.z < terrain_half_length and ray_origin.z > -terrain_half_length;

   if (x_inside and z_inside) return ray_origin;

   const float terrain_half_length_biased = terrain_half_length - terrain.grid_scale;

   if (const float hit_distance =
          boxIntersection(ray_origin, ray_direction,
                          float3{terrain_half_length_biased, 32768.0f * terrain.height_scale,
                                 terrain_half_length_biased});
       hit_distance >= 0.0f) {
      return ray_origin + ray_direction * hit_distance;
   }

   return ray_origin;
}

}

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const terrain& terrain) noexcept -> std::optional<float>
{
   if (not terrain.active_flags.terrain) return std::nullopt;

   const float3 ray_start = make_ray_start(ray_origin, ray_direction, terrain);

   const float2 start =
      (float2{ray_start.x / terrain.grid_scale, ray_start.z / terrain.grid_scale} - 0.5f);
   const float2 end = ((start + float2{ray_direction.x, ray_direction.z} *
                                   static_cast<float>(terrain.length)));

   const float2 difference = end - start;

   const bool x_step = std::abs(difference.x) > std::abs(difference.y);

   const float step_count =
      x_step ? std::abs(difference.x) : std::abs(difference.y);

   if (step_count == 0.0f) {
      if (const float intersection =
             raycast_terrain_quad(ray_origin, ray_direction, round(start), terrain);
          intersection >= 0.0f) {
         return intersection;
      }
   }

   const float2 increment = difference / step_count;

   float2 point = start;

   const float terrain_bounds = (terrain.length / 2.0f);

   for (float step = 0.0f; step < step_count; ++step) {
      if (point.x > terrain_bounds or point.x < -terrain_bounds or
          point.y > terrain_bounds or point.y < -terrain_bounds) {
         return std::nullopt;
      }

      const float2 rounded_point = round(point);

      if (const float intersection =
             raycast_terrain_quad(ray_origin, ray_direction, rounded_point, terrain);
          intersection >= 0.0f) {
         return intersection;
      }

      if (std::round(point.y - increment.y) != rounded_point.y) {
         if (const float intersection =
                raycast_terrain_quad(ray_origin, ray_direction,
                                     round(point - float2{0.0f, increment.y}), terrain);
             intersection >= 0.0f) {
            return intersection;
         }
      }

      if (std::round(point.x - increment.x) != rounded_point.x) {
         if (const float intersection =
                raycast_terrain_quad(ray_origin, ray_direction,
                                     round(point - float2{increment.x, 0.0f}), terrain);
             intersection >= 0.0f) {
            return intersection;
         }
      }

      point += increment;
   }

   return std::nullopt;
}

}