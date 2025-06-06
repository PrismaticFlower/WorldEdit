#include "mesh_cull.hpp"

#include "math/matrix_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <algorithm>

#include <absl/container/flat_hash_map.h>

namespace we::world {

namespace {

constexpr float min_triangle_area = 0.000001f;
constexpr double quantized_point_precision = 1000.0;

struct snapped_point {
   explicit snapped_point(const float3& point) noexcept
   {
      x = static_cast<int64>(
         std::min(std::max(point.x * quantized_point_precision, -1125899906842624.0),
                  1125899906842624.0) +
         0.5);

      y = static_cast<int64>(
         std::min(std::max(point.y * quantized_point_precision, -1125899906842624.0),
                  1125899906842624.0) +
         0.5);
   }

   int64 x = 0;
   int64 y = 0;

   bool operator==(const snapped_point&) const noexcept = default;
};

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

   std::array<snapped_point, 4> verticesPS;
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
      std::min(std::max(planeWS.w * quantized_point_precision, -1125899906842624.0),
               1125899906842624.0) +
      0.5;

   return {static_cast<int16>(quantized_normalWS.x),
           static_cast<int16>(quantized_normalWS.y),
           static_cast<int16>(quantized_normalWS.z), static_cast<int64>(quantized_d)};
}

bool inside_occluder(const std::array<snapped_point, 4>& occluder,
                     const snapped_point& point) noexcept
{
   bool positive = false;
   bool negative = false;

   for (std::size_t i = 0; i < occluder.size(); ++i) {
      const snapped_point& e0 = occluder[i];
      const snapped_point& e1 = occluder[(i + 1) % occluder.size()];

      const int64 d =
         (point.x - e0.x) * (e1.y - e0.y) - (point.y - e0.y) * (e1.x - e0.x);

      if (d == 0) {
         // Handle the case of a point being along an line as an edge segment but outside of the edge's bounds.
         if (e0.x == e1.x and e0.x == point.x) {
            if (point.y < std::min(e0.y, e1.y) or point.y > std::max(e0.y, e1.y)) {
               continue;
            }
         }
         else if (e0.y == e1.y and e0.y == point.y) {
            if (point.x < std::min(e0.x, e1.x) or point.x > std::max(e0.x, e1.x)) {
               continue;
            }
         }

         return true;
      }

      if (d > 0) positive = true;
      if (d < 0) negative = true;

      if (positive and negative) return false;
   }

   return true;
}

bool is_occluded(const std::array<snapped_point, 3>& trianglePS,
                 const std::array<snapped_point, 4>& occluderPS) noexcept
{
   int matching_vertices = 0;

   for (const snapped_point& tri_pointPS : trianglePS) {
      for (const snapped_point& occluder_pointPS : occluderPS) {
         matching_vertices += tri_pointPS == occluder_pointPS;
      }
   }

   if (matching_vertices >= 3) return true;

   return inside_occluder(occluderPS, trianglePS[0]) and
          inside_occluder(occluderPS, trianglePS[1]) and
          inside_occluder(occluderPS, trianglePS[2]);
}

}

auto cull_hidden_triangles(std::span<const block_world_mesh> meshes,
                           std::span<const block_world_occluder> occluders) noexcept
   -> std::vector<block_world_mesh>
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
             snapped_point{plane_occluders.plane_from_world *
                           occluders[index].verticesWS[0]},
             snapped_point{plane_occluders.plane_from_world *
                           occluders[index].verticesWS[1]},
             snapped_point{plane_occluders.plane_from_world *
                           occluders[index].verticesWS[2]},
             snapped_point{plane_occluders.plane_from_world *
                           occluders[index].verticesWS[3]},
          }});
   }

   for (auto& [normal, plane_occluders] : occluders_map) {
      std::sort(plane_occluders.occluders.begin(), plane_occluders.occluders.end(),
                [&](const quad_occluder& left, const quad_occluder& right) noexcept {
                   return left.area < right.area;
                });
   }

   std::vector<block_world_mesh> new_meshes;
   new_meshes.reserve(meshes.size());

   for (const block_world_mesh& mesh : meshes) {
      std::vector<block_world_triangle> new_triangles;
      new_triangles.reserve(mesh.triangles.size());

      for (const block_world_triangle& triangle : mesh.triangles) {
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

            const std::array<snapped_point, 3> trianglePS = {
               snapped_point{occluder_candidates.plane_from_world *
                             triangle.vertices[2].positionWS},
               snapped_point{occluder_candidates.plane_from_world *
                             triangle.vertices[1].positionWS},
               snapped_point{occluder_candidates.plane_from_world *
                             triangle.vertices[0].positionWS},
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

      if (new_triangles.empty()) continue;

      new_meshes.push_back({
         .triangles = std::move(new_triangles),
         .collision_triangles = mesh.collision_triangles,
         .bboxWS = mesh.bboxWS,
      });
   }

   return new_meshes;
}

}