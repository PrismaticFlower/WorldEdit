
#include "frustum.hpp"
#include "intersectors.hpp"
#include "matrix_funcs.hpp"
#include "plane_funcs.hpp"
#include "quaternion_funcs.hpp"
#include "vector_funcs.hpp"

#include <array>

#include <functional>

namespace we {

namespace {

bool outside_plane(const float4& plane, const float3& point) noexcept
{
   return dot(plane, float4{point, 1.0f}) < 0.0f;
}

bool outside_plane(const float4& plane, const float3& point, const float radius) noexcept
{
   return dot(plane, float4{point, 1.0f}) < -radius;
}

}

frustum::frustum(const float4x4& inv_view_projection_matrix,
                 const float3 ndc_min, const float3 ndc_max) noexcept
{
   const container::enum_array<float4, frustum_corner> corners_proj =
      container::make_enum_array<float4, frustum_corner>(
         {{frustum_corner::bottom_left_near, {ndc_min.x, ndc_min.y, ndc_max.z, 1.0f}},
          {frustum_corner::bottom_right_near, {ndc_max.x, ndc_min.y, ndc_max.z, 1.0f}},

          {frustum_corner::top_left_near, {ndc_min.x, ndc_max.y, ndc_max.z, 1.0f}},
          {frustum_corner::top_right_near, {ndc_max.x, ndc_max.y, ndc_max.z, 1.0f}},

          {frustum_corner::bottom_left_far, {ndc_min.x, ndc_min.y, ndc_min.z, 1.0f}},
          {frustum_corner::bottom_right_far, {ndc_max.x, ndc_min.y, ndc_min.z, 1.0f}},

          {frustum_corner::top_left_far, {ndc_min.x, ndc_max.y, ndc_min.z, 1.0f}},
          {frustum_corner::top_right_far, {ndc_max.x, ndc_max.y, ndc_min.z, 1.0f}}});

   for (std::size_t i = 0; i < corners.size(); ++i) {
      const float4 position = inv_view_projection_matrix * corners_proj[i];

      corners[i] = float3{position.x, position.y, position.z} / position.w;
   }

   planes[frustum_planes::near_] =
      make_plane(corners[frustum_corner::top_left_near],
                 corners[frustum_corner::top_right_near],
                 corners[frustum_corner::bottom_left_near]);

   planes[frustum_planes::far_] =
      make_plane(corners[frustum_corner::top_left_far],
                 corners[frustum_corner::bottom_left_far],
                 corners[frustum_corner::top_right_far]);

   planes[frustum_planes::bottom] =
      make_plane(corners[frustum_corner::bottom_left_near],
                 corners[frustum_corner::bottom_right_far],
                 corners[frustum_corner::bottom_left_far]);

   planes[frustum_planes::top] =
      make_plane(corners[frustum_corner::top_left_near],
                 corners[frustum_corner::top_left_far],
                 corners[frustum_corner::top_right_far]);

   planes[frustum_planes::left] =
      make_plane(corners[frustum_corner::top_left_near],
                 corners[frustum_corner::bottom_left_far],
                 corners[frustum_corner::top_left_far]);

   planes[frustum_planes::right] =
      make_plane(corners[frustum_corner::top_right_near],
                 corners[frustum_corner::top_right_far],
                 corners[frustum_corner::bottom_right_far]);
}

frustum::frustum(const float4x4& inv_view_projection_matrix, const float z_min,
                 const float z_max) noexcept
   : frustum{inv_view_projection_matrix, {-1.0f, -1.0f, z_min}, {1.0f, 1.0f, z_max}}
{
}

frustum::frustum(const float4x4& inv_view_projection_matrix) noexcept
   : frustum{inv_view_projection_matrix, 0.0f, 1.0f}
{
}

bool intersects(const frustum& frustum, const math::bounding_box& bbox) noexcept
{
   for (const auto& plane : frustum.planes) {
      if (outside_plane(plane, {bbox.min.x, bbox.min.y, bbox.min.z}) &
          outside_plane(plane, {bbox.max.x, bbox.min.y, bbox.min.z}) &
          outside_plane(plane, {bbox.min.x, bbox.max.y, bbox.min.z}) &
          outside_plane(plane, {bbox.max.x, bbox.max.y, bbox.min.z}) &
          outside_plane(plane, {bbox.min.x, bbox.min.y, bbox.max.z}) &
          outside_plane(plane, {bbox.max.x, bbox.min.y, bbox.max.z}) &
          outside_plane(plane, {bbox.min.x, bbox.max.y, bbox.max.z}) &
          outside_plane(plane, {bbox.max.x, bbox.max.y, bbox.max.z})) {
         return false;
      }
   }

   const auto outside_corner = [&](const float float3::*axis, auto comparator,
                                   const float corner) {
      bool outside = true;

      for (const auto& frustum_corner : frustum.corners) {
         outside &= comparator(frustum_corner.*axis, corner);
      }

      return outside;
   };

   if (outside_corner(&float3::x, std::greater<>{}, bbox.max.x)) return false;
   if (outside_corner(&float3::x, std::less<>{}, bbox.min.x)) return false;
   if (outside_corner(&float3::y, std::greater<>{}, bbox.max.y)) return false;
   if (outside_corner(&float3::y, std::less<>{}, bbox.min.y)) return false;
   if (outside_corner(&float3::z, std::greater<>{}, bbox.max.z)) return false;
   if (outside_corner(&float3::z, std::less<>{}, bbox.min.z)) return false;

   return true;
}

bool intersects_shadow_cascade(const frustum& frustum, const math::bounding_box& bbox) noexcept
{
   for (std::size_t i = 0; i < (frustum.planes.size() - 1); ++i) {
      const float4 plane = frustum.planes[i];

      if (outside_plane(plane, {bbox.min.x, bbox.min.y, bbox.min.z}) &
          outside_plane(plane, {bbox.max.x, bbox.min.y, bbox.min.z}) &
          outside_plane(plane, {bbox.min.x, bbox.max.y, bbox.min.z}) &
          outside_plane(plane, {bbox.max.x, bbox.max.y, bbox.min.z}) &
          outside_plane(plane, {bbox.min.x, bbox.min.y, bbox.max.z}) &
          outside_plane(plane, {bbox.max.x, bbox.min.y, bbox.max.z}) &
          outside_plane(plane, {bbox.min.x, bbox.max.y, bbox.max.z}) &
          outside_plane(plane, {bbox.max.x, bbox.max.y, bbox.max.z})) {
         return false;
      }
   }

   return true;
}

bool intersects(const frustum& frustum, const float3& position, const float radius) noexcept
{
   for (const auto& plane : frustum.planes) {
      if (outside_plane(plane, position, radius)) {
         return false;
      }
   }

   return true;
}

bool intersects(const frustum& frustum, const float3& v0, const float3& v1,
                const float3& v2) noexcept
{
   if (intersects(frustum, v0, 0.0f) or intersects(frustum, v1, 0.0f) or
       intersects(frustum, v2, 0.0f)) {
      return true;
   }

   const std::array<float3, 3> tri_edge_vectors = {
      v1 - v0, // origin 0
      v2 - v1, // origin 1
      v0 - v2, // origin 2
   };

   const std::array<float, 3> tri_edge_lengths = {
      length(tri_edge_vectors[0]), // origin 0
      length(tri_edge_vectors[1]), // origin 1
      length(tri_edge_vectors[2]), // origin 2
   };

   const std::array<float3, 3> tri_edge_directions = {
      tri_edge_vectors[0] / tri_edge_lengths[0], // origin 0
      tri_edge_vectors[1] / tri_edge_lengths[1], // origin 1
      tri_edge_vectors[2] / tri_edge_lengths[2], // origin 2
   };

   constexpr static std::array<std::array<frustum_corner, 4>, 6> frustum_faces = {{
      // Near Face
      {frustum_corner::bottom_left_near, frustum_corner::top_left_near,
       frustum_corner::top_right_near, frustum_corner::bottom_right_near},

      // Far Face
      {frustum_corner::bottom_left_far, frustum_corner::top_left_far,
       frustum_corner::top_right_far, frustum_corner::bottom_right_far},

      // Left Face
      {frustum_corner::bottom_left_near, frustum_corner::top_left_near,
       frustum_corner::top_left_far, frustum_corner::bottom_left_far},

      // Right Face
      {frustum_corner::bottom_right_near, frustum_corner::top_right_near,
       frustum_corner::top_right_far, frustum_corner::bottom_right_far},

      // Top Face
      {frustum_corner::top_left_near, frustum_corner::top_left_far,
       frustum_corner::top_right_far, frustum_corner::top_right_near},

      // Bottom Face
      {frustum_corner::bottom_left_near, frustum_corner::bottom_left_far,
       frustum_corner::bottom_right_far, frustum_corner::bottom_right_near},
   }};

   const std::array<float3, 3> tri = {v0, v1, v2};

   for (int32 edge_index = 0; edge_index < 3; ++edge_index) {
      for (const auto& [i0, i1, i2, i3] : frustum_faces) {

         if (float distance = FLT_MAX;
             intersect_tri(tri[edge_index], tri_edge_directions[edge_index],
                           frustum.corners[i0], frustum.corners[i1],
                           frustum.corners[i2], distance) and
             distance <= tri_edge_lengths[edge_index]) {
            return true;
         }

         if (float distance = FLT_MAX;
             intersect_tri(tri[edge_index], tri_edge_directions[edge_index],
                           frustum.corners[i2], frustum.corners[i3],
                           frustum.corners[i0], distance) and
             distance <= tri_edge_lengths[edge_index]) {
            return true;
         }
      }
   }

   constexpr static std::array<std::array<frustum_corner, 2>, 12> frustum_edges = {{
      {frustum_corner::top_left_near, frustum_corner::top_left_far},
      {frustum_corner::top_left_far, frustum_corner::top_right_far},
      {frustum_corner::top_right_far, frustum_corner::top_right_near},
      {frustum_corner::top_right_near, frustum_corner::top_left_near},

      {frustum_corner::bottom_left_near, frustum_corner::bottom_left_far},
      {frustum_corner::bottom_left_far, frustum_corner::bottom_right_far},
      {frustum_corner::bottom_right_far, frustum_corner::bottom_right_near},
      {frustum_corner::bottom_right_near, frustum_corner::bottom_left_near},

      {frustum_corner::bottom_left_near, frustum_corner::top_left_near},
      {frustum_corner::bottom_left_far, frustum_corner::top_left_far},
      {frustum_corner::bottom_right_far, frustum_corner::top_right_far},
      {frustum_corner::bottom_right_near, frustum_corner::top_right_near},
   }};

   for (const auto& [i0, i1] : frustum_edges) {
      const float3 edge_vector = frustum.corners[i1] - frustum.corners[i0];
      const float edge_length = length(edge_vector);
      const float3 edge_direction = edge_vector / edge_length;

      if (float distance = FLT_MAX;
          intersect_tri(frustum.corners[i0], edge_direction, v0, v1, v2, distance) and
          distance <= edge_length) {
         return true;
      }
   }

   return false;
}

auto transform(const frustum& world_frustum, const quaternion& rotation,
               const float3& position) noexcept -> frustum
{
   frustum frustum = world_frustum;

   for (float3& point : frustum.corners) {
      point = rotation * point + position;
   }

   frustum.planes[frustum_planes::near_] =
      make_plane(frustum.corners[frustum_corner::top_left_near],
                 frustum.corners[frustum_corner::top_right_near],
                 frustum.corners[frustum_corner::bottom_left_near]);

   frustum.planes[frustum_planes::far_] =
      make_plane(frustum.corners[frustum_corner::top_left_far],
                 frustum.corners[frustum_corner::bottom_left_far],
                 frustum.corners[frustum_corner::top_right_far]);

   frustum.planes[frustum_planes::bottom] =
      make_plane(frustum.corners[frustum_corner::bottom_left_near],
                 frustum.corners[frustum_corner::bottom_right_far],
                 frustum.corners[frustum_corner::bottom_left_far]);

   frustum.planes[frustum_planes::top] =
      make_plane(frustum.corners[frustum_corner::top_left_near],
                 frustum.corners[frustum_corner::top_left_far],
                 frustum.corners[frustum_corner::top_right_far]);

   frustum.planes[frustum_planes::left] =
      make_plane(frustum.corners[frustum_corner::top_left_near],
                 frustum.corners[frustum_corner::bottom_left_far],
                 frustum.corners[frustum_corner::top_left_far]);

   frustum.planes[frustum_planes::right] =
      make_plane(frustum.corners[frustum_corner::top_right_near],
                 frustum.corners[frustum_corner::top_right_far],
                 frustum.corners[frustum_corner::bottom_right_far]);

   return frustum;
}

}
