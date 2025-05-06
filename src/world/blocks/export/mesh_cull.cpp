#include "mesh_cull.hpp"

#include "math/matrix_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <algorithm>

#include <absl/container/flat_hash_map.h>

namespace we::world {

namespace {

constexpr float min_triangle_area = 0.000001f;

struct quantized_plane {
   int16 a;
   int16 b;
   int16 c;
   int64 d;

   bool operator==(const quantized_plane& other) const noexcept = default;

   template<typename H>
   friend H AbslHashValue(H h, const quantized_plane& plane)
   {
      return H::combine(std::move(h), plane.a, plane.b, plane.c, plane.d);
   }
};

struct quad_occluder {
   block_id block_id;

   float area = 0.0f;

   std::array<float3, 4> verticesPS;
};

struct plane_occluders {
   std::vector<quad_occluder> occluders;

   float3x3 plane_from_world;
};

auto quantize(const float4& planeWS) -> quantized_plane
{
   const float3 quantized_normalWS =
      min(max(float3{planeWS.x, planeWS.y, planeWS.z} * 32767.0f,
              {-32767.0f, -32767.0f, -32767.0f}),
          {32767.0f, 32767.0f, 32767.0f}) +
      0.5f;
   const double quantized_d =
      std::min(std::max(planeWS.w * 1000.0, -1125899906842624.0), 1125899906842624.0) + 0.5;

   return {static_cast<int16>(quantized_normalWS.x),
           static_cast<int16>(quantized_normalWS.y),
           static_cast<int16>(quantized_normalWS.z), static_cast<int64>(quantized_d)};
}

bool line_intersection(const float3& a_start, const float3& a_end,
                       const float3& b_start, const float3& b_end) noexcept
{
   const float denominator = (b_end.y - b_start.y) * (a_end.x - a_start.x) -
                             (b_end.x - b_start.x) * (a_end.y - a_start.y);

   const float a_u = (b_end.x - b_start.x) * (a_start.y - b_start.y) -
                     (b_end.y - b_start.y) * (a_start.x - b_start.x);
   const float b_u = (a_end.x - a_start.x) * (a_start.y - b_start.y) -
                     (a_end.y - a_start.y) * (a_start.x - b_start.x);

   const float a_t = a_u / denominator;
   const float b_t = b_u / denominator;

   return (a_t >= 0.0f) and (a_t <= 1.0f) and (b_t >= 0.0f) and (b_t <= 1.0f);
}

bool inside_occluder(const std::array<float3, 4>& occluder,
                     const float3& occluder_min, const float3& point) noexcept
{
   int intersections = 0;

   const float3 point_start = occluder_min - 1.0f;

   for (std::size_t i = 0; i < occluder.size(); ++i) {
      const float3 sector_start = occluder[i];
      const float3 sector_end = occluder[(i + 1) % occluder.size()];

      if (line_intersection(sector_start, sector_end, point_start, point)) {
         intersections += 1;
      }
   }

   return (intersections % 2) == 1;
}

bool is_occluded(const std::array<float3, 3>& trianglePS,
                 const std::array<float3, 4>& occluderPS) noexcept
{
   int matching_vertices = 0;

   for (const float3& tri_pointPS : trianglePS) {
      for (const float3& occluder_pointPS : trianglePS) {
         matching_vertices += (tri_pointPS.x == occluder_pointPS.x) &
                              (tri_pointPS.y == occluder_pointPS.y);
      }
   }

   if (matching_vertices >= 3) return true;

   const float3 occluder_minPS =
      min(min(min(occluderPS[0], occluderPS[1]), occluderPS[2]), occluderPS[2]);

   return inside_occluder(occluderPS, trianglePS[0], occluder_minPS);
}

}

auto cull_hidden_triangles(std::span<const block_world_triangle> triangles,
                           std::span<const block_world_occluder> occluders) noexcept
   -> std::vector<block_world_triangle>
{
   absl::flat_hash_map<quantized_plane, uint32> occluders_count_map;

   for (const block_world_occluder& occluder : occluders) {
      occluders_count_map[quantize(occluder.planeWS)] += 1;
   }

   absl::flat_hash_map<quantized_plane, plane_occluders> occluders_map;
   occluders_map.reserve(occluders_count_map.size());

   for (const auto& [normal, count] : occluders_count_map) {
      occluders_map[normal].occluders.reserve(count);
   }

   for (uint32 index = 0; index < occluders.size(); ++index) {
      const quantized_plane planeWS = quantize(occluders[index].planeWS);

      plane_occluders& plane_occluders = occluders_map[planeWS];

      if (plane_occluders.occluders.empty()) {
         const float3 normalWS = {planeWS.a / 32767.0f, planeWS.b / 32767.0f,
                                  planeWS.c / 32767.0f};

         plane_occluders.plane_from_world = transpose(orthonormal_basis(normalWS));
      }

      plane_occluders.occluders.push_back(
         {.block_id = occluders[index].block_id,
          .area = occluders[index].area,
          .verticesPS = {
             plane_occluders.plane_from_world * occluders[index].verticesWS[0],
             plane_occluders.plane_from_world * occluders[index].verticesWS[1],
             plane_occluders.plane_from_world * occluders[index].verticesWS[2],
             plane_occluders.plane_from_world * occluders[index].verticesWS[3],
          }});
   }

   for (auto& [normal, plane_occluders] : occluders_map) {
      std::sort(plane_occluders.occluders.begin(), plane_occluders.occluders.end(),
                [&](const quad_occluder& left, const quad_occluder& right) noexcept {
                   return left.area < right.area;
                });
   }

   std::vector<block_world_triangle> new_triangles;
   new_triangles.reserve(triangles.size());

   for (const block_world_triangle& triangle : triangles) {
      const float3 edge0WS =
         triangle.vertices[1].positionWS - triangle.vertices[2].positionWS;
      const float3 edge1WS =
         triangle.vertices[0].positionWS - triangle.vertices[2].positionWS;
      const float3 e0_cross_e1 = cross(edge0WS, edge1WS);

      const float e0_cross_e1_length = length(e0_cross_e1);
      const float3 normalWS = e0_cross_e1 / e0_cross_e1_length;
      const float4 planeWS = {normalWS,
                              -dot(normalWS, triangle.vertices[2].positionWS)};

      const float triangle_area = e0_cross_e1_length * 0.5f;

      if (triangle_area < min_triangle_area) continue;

      bool visible = true;

      auto occluder_candidates_it = occluders_map.find(quantize(planeWS));

      if (occluder_candidates_it != occluders_map.end()) {
         plane_occluders& occluder_candidates = occluder_candidates_it->second;

         std::array<float3, 3> trianglePS = {
            occluder_candidates.plane_from_world * triangle.vertices[2].positionWS,
            occluder_candidates.plane_from_world * triangle.vertices[1].positionWS,
            occluder_candidates.plane_from_world * triangle.vertices[0].positionWS,
         };

         for (auto candidate_it =
                 std::upper_bound(occluder_candidates.occluders.begin(),
                                  occluder_candidates.occluders.end(), triangle_area,
                                  [&](const float triangle_area,
                                      const quad_occluder& left) noexcept {
                                     return triangle_area < left.area;
                                  });
              candidate_it != occluder_candidates.occluders.end(); ++candidate_it) {
            const quad_occluder& occluder = *candidate_it;

            if (occluder.block_id == triangle.block_id) continue;

            if (is_occluded(trianglePS, occluder.verticesPS)) {
               visible = false;

               break;
            }
         }
      }

      if (visible) new_triangles.push_back(triangle);
   }

   return new_triangles;
}

}