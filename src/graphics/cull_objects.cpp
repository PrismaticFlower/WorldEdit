#include "cull_objects.hpp"

#include <cassert>
#include <ranges>

#include <immintrin.h>

namespace we::graphics {

using std::ranges::views::drop;

void cull_objects_scalar(const frustum& frustum,
                         std::span<const math::bounding_box> bbox,
                         std::span<const material_pipeline_flags> pipeline_flags,
                         std::vector<uint16>& out_opaque_list,
                         std::vector<uint16>& out_transparent_list) noexcept
{
   assert(bbox.size() == pipeline_flags.size());

   out_opaque_list.clear();
   out_transparent_list.clear();
   out_opaque_list.reserve(bbox.size());
   out_transparent_list.reserve(bbox.size());

   for (std::size_t i = 0; i < bbox.size(); ++i) {
      if (not intersects(frustum, bbox[i])) continue;

      auto& render_list =
         are_flags_set(pipeline_flags[i], material_pipeline_flags::transparent) or
               are_flags_set(pipeline_flags[i], material_pipeline_flags::additive)
            ? out_transparent_list
            : out_opaque_list;

      render_list.push_back(static_cast<uint16>(i));
   }
}

namespace {

constexpr int avx_width = 8;

auto outside_plane(__m256 plane_x, __m256 plane_y, __m256 plane_z, __m256 plane_w,
                   __m256 point_x, __m256 point_y, __m256 point_z) noexcept -> __m256
{
   const float one = 1.0f;
   const __m256 value_1 = _mm256_broadcast_ss(&one);

   const __m256 x = _mm256_mul_ps(plane_x, point_x);
   const __m256 y = _mm256_mul_ps(plane_y, point_y);
   const __m256 z = _mm256_mul_ps(plane_z, point_z);
   const __m256 w = _mm256_mul_ps(plane_w, value_1);

   const __m256 xy_sum = _mm256_add_ps(x, y);
   const __m256 zw_sum = _mm256_add_ps(z, w);

   const __m256 plane_distance = _mm256_add_ps(xy_sum, zw_sum);

   const float zero = 0.0f;
   const __m256 value_0 = _mm256_broadcast_ss(&zero);

   return _mm256_cmp_ps(plane_distance, value_0, _CMP_LT_OQ);
}

auto outside_plane(__m256 plane_x, __m256 plane_y, __m256 plane_z,
                   __m256 plane_w, __m256 bbox_min_x, __m256 bbox_min_y,
                   __m256 bbox_min_z, __m256 bbox_max_x, __m256 bbox_max_y,
                   __m256 bbox_max_z) noexcept -> __m256
{
   const __m256 corner0_outside = outside_plane(plane_x, plane_y, plane_z, plane_w,
                                                bbox_min_x, bbox_min_y, bbox_min_z);
   const __m256 corner1_outside = outside_plane(plane_x, plane_y, plane_z, plane_w,
                                                bbox_max_x, bbox_min_y, bbox_min_z);
   const __m256 corner2_outside = outside_plane(plane_x, plane_y, plane_z, plane_w,
                                                bbox_min_x, bbox_max_y, bbox_min_z);
   const __m256 corner3_outside = outside_plane(plane_x, plane_y, plane_z, plane_w,
                                                bbox_max_x, bbox_max_y, bbox_min_z);
   const __m256 corner4_outside = outside_plane(plane_x, plane_y, plane_z, plane_w,
                                                bbox_min_x, bbox_min_y, bbox_max_z);
   const __m256 corner5_outside = outside_plane(plane_x, plane_y, plane_z, plane_w,
                                                bbox_max_x, bbox_min_y, bbox_max_z);
   const __m256 corner6_outside = outside_plane(plane_x, plane_y, plane_z, plane_w,
                                                bbox_min_x, bbox_max_y, bbox_max_z);
   const __m256 corner7_outside = outside_plane(plane_x, plane_y, plane_z, plane_w,
                                                bbox_max_x, bbox_max_y, bbox_max_z);

   const __m256 corner01_outside = _mm256_and_ps(corner0_outside, corner1_outside);
   const __m256 corner23_outside = _mm256_and_ps(corner2_outside, corner3_outside);
   const __m256 corner45_outside = _mm256_and_ps(corner4_outside, corner5_outside);
   const __m256 corner67_outside = _mm256_and_ps(corner6_outside, corner7_outside);

   const __m256 corner0123_outside = _mm256_and_ps(corner01_outside, corner23_outside);
   const __m256 corner4567_outside = _mm256_and_ps(corner45_outside, corner67_outside);

   return _mm256_and_ps(corner0123_outside, corner4567_outside);
}

auto outside_corners(std::array<__m256, 8> corners, __m256 bbox_corner_min,
                     __m256 bbox_corner_max) noexcept -> __m256
{
   const float one = 1.0f;
   __m256 outside_mask = _mm256_broadcast_ss(&one);

   for (const __m256 corner : corners) {
      const __m256 min_outside = _mm256_cmp_ps(corner, bbox_corner_min, _CMP_LT_OQ);
      const __m256 max_outside = _mm256_cmp_ps(corner, bbox_corner_max, _CMP_GT_OQ);

      const __m256 min_max_outside = _mm256_or_ps(min_outside, max_outside);

      outside_mask = _mm256_and_ps(outside_mask, min_max_outside);
   }

   return outside_mask;
}

}

void cull_objects_avx2(const frustum& frustum, std::span<const float> bbox_min_x,
                       std::span<const float> bbox_min_y,
                       std::span<const float> bbox_min_z,
                       std::span<const float> bbox_max_x,
                       std::span<const float> bbox_max_y,
                       std::span<const float> bbox_max_z,
                       std::span<const material_pipeline_flags> pipeline_flags,
                       std::vector<uint16>& out_opaque_list,
                       std::vector<uint16>& out_transparent_list) noexcept
{
   assert(bbox_min_x.size() == bbox_min_y.size());
   assert(bbox_min_x.size() == bbox_min_z.size());
   assert(bbox_min_x.size() == bbox_max_x.size());
   assert(bbox_min_x.size() == bbox_max_y.size());
   assert(bbox_min_x.size() == bbox_max_z.size());
   assert(bbox_min_x.size() == pipeline_flags.size());

   out_opaque_list.clear();
   out_transparent_list.clear();
   out_opaque_list.reserve(bbox_min_x.size());
   out_transparent_list.reserve(bbox_min_x.size());

   const std::size_t simd_iterations = bbox_min_x.size() / avx_width;
   const std::size_t scalar_iterations = bbox_min_x.size() % avx_width;

   for (std::size_t i = 0; i < simd_iterations; ++i) {
      int inside_mask = 0xff;

      for (const auto& plane : frustum.planes) {
         const __m256 plane_x = _mm256_broadcast_ss(&plane.x);
         const __m256 plane_y = _mm256_broadcast_ss(&plane.y);
         const __m256 plane_z = _mm256_broadcast_ss(&plane.z);
         const __m256 plane_w = _mm256_broadcast_ss(&plane.w);

         const __m256 outside =
            outside_plane(plane_x, plane_y, plane_z, plane_w,
                          _mm256_load_ps(&bbox_min_x[i * avx_width]),
                          _mm256_load_ps(&bbox_min_y[i * avx_width]),
                          _mm256_load_ps(&bbox_min_z[i * avx_width]),
                          _mm256_load_ps(&bbox_max_x[i * avx_width]),
                          _mm256_load_ps(&bbox_max_y[i * avx_width]),
                          _mm256_load_ps(&bbox_max_z[i * avx_width]));

         inside_mask &= ~_mm256_movemask_ps(outside);
      }

      if (not inside_mask) continue;

      const __m256 outside_x =
         outside_corners({_mm256_broadcast_ss(&frustum.corners[0].x),
                          _mm256_broadcast_ss(&frustum.corners[1].x),
                          _mm256_broadcast_ss(&frustum.corners[2].x),
                          _mm256_broadcast_ss(&frustum.corners[3].x),
                          _mm256_broadcast_ss(&frustum.corners[4].x),
                          _mm256_broadcast_ss(&frustum.corners[5].x)},
                         _mm256_load_ps(&bbox_min_x[i * avx_width]),
                         _mm256_load_ps(&bbox_max_x[i * avx_width]));

      inside_mask &= ~_mm256_movemask_ps(outside_x);

      if (not inside_mask) continue;

      const __m256 outside_y =
         outside_corners({_mm256_broadcast_ss(&frustum.corners[0].y),
                          _mm256_broadcast_ss(&frustum.corners[1].y),
                          _mm256_broadcast_ss(&frustum.corners[2].y),
                          _mm256_broadcast_ss(&frustum.corners[3].y),
                          _mm256_broadcast_ss(&frustum.corners[4].y),
                          _mm256_broadcast_ss(&frustum.corners[5].y)},
                         _mm256_load_ps(&bbox_min_y[i * avx_width]),
                         _mm256_load_ps(&bbox_max_y[i * avx_width]));

      inside_mask &= ~_mm256_movemask_ps(outside_y);

      if (not inside_mask) continue;

      const __m256 outside_z =
         outside_corners({_mm256_broadcast_ss(&frustum.corners[0].z),
                          _mm256_broadcast_ss(&frustum.corners[1].z),
                          _mm256_broadcast_ss(&frustum.corners[2].z),
                          _mm256_broadcast_ss(&frustum.corners[3].z),
                          _mm256_broadcast_ss(&frustum.corners[4].z),
                          _mm256_broadcast_ss(&frustum.corners[5].z)},
                         _mm256_load_ps(&bbox_min_z[i * avx_width]),
                         _mm256_load_ps(&bbox_max_z[i * avx_width]));

      inside_mask &= ~_mm256_movemask_ps(outside_z);

      if (not inside_mask) continue;

      const auto push_index = [&pipeline_flags, &out_opaque_list,
                               &out_transparent_list](const std::size_t i) {
         auto& render_list =
            are_flags_set(pipeline_flags[i], material_pipeline_flags::transparent) or
                  are_flags_set(pipeline_flags[i], material_pipeline_flags::additive)
               ? out_transparent_list
               : out_opaque_list;

         render_list.push_back(static_cast<uint16>(i));
      };

      if (inside_mask & 0b1) push_index((i * avx_width) + 0);
      if (inside_mask & 0b10) push_index((i * avx_width) + 1);
      if (inside_mask & 0b100) push_index((i * avx_width) + 2);
      if (inside_mask & 0b1000) push_index((i * avx_width) + 3);
      if (inside_mask & 0b10000) push_index((i * avx_width) + 4);
      if (inside_mask & 0b100000) push_index((i * avx_width) + 5);
      if (inside_mask & 0b1000000) push_index((i * avx_width) + 6);
      if (inside_mask & 0b10000000) push_index((i * avx_width) + 7);
   }

   for (std::size_t i = bbox_min_x.size() - scalar_iterations;
        i < bbox_min_x.size(); ++i) {
      if (not intersects(frustum, {{bbox_min_x[i], bbox_min_y[i], bbox_min_z[i]},
                                   {bbox_max_x[i], bbox_max_y[i], bbox_max_z[i]}})) {
         continue;
      }

      auto& render_list =
         are_flags_set(pipeline_flags[i], material_pipeline_flags::transparent) or
               are_flags_set(pipeline_flags[i], material_pipeline_flags::additive)
            ? out_transparent_list
            : out_opaque_list;

      render_list.push_back(static_cast<uint16>(i));
   }
}

void cull_objects_shadow_cascade_scalar(const frustum& frustum,
                                        std::span<const math::bounding_box> bbox,
                                        std::span<const material_pipeline_flags> pipeline_flags,
                                        std::vector<uint16>& out_list) noexcept
{
   assert(bbox.size() == pipeline_flags.size());

   out_list.clear();
   out_list.reserve(bbox.size());

   for (std::size_t i = 0; i < bbox.size(); ++i) {
      if (not intersects_shadow_cascade(frustum, bbox[i])) continue;

      if (not(are_flags_set(pipeline_flags[i], material_pipeline_flags::transparent) or
              are_flags_set(pipeline_flags[i], material_pipeline_flags::additive))) {
         out_list.push_back(static_cast<uint16>(i));
      }
   }
}

void cull_objects_shadow_cascade_avx2(
   const frustum& frustum, std::span<const float> bbox_min_x,
   std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
   std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
   std::span<const float> bbox_max_z,
   std::span<const material_pipeline_flags> pipeline_flags,
   std::vector<uint16>& out_list) noexcept
{
   assert(bbox_min_x.size() == bbox_min_y.size());
   assert(bbox_min_x.size() == bbox_min_z.size());
   assert(bbox_min_x.size() == bbox_max_x.size());
   assert(bbox_min_x.size() == bbox_max_y.size());
   assert(bbox_min_x.size() == bbox_max_z.size());
   assert(bbox_min_x.size() == pipeline_flags.size());

   out_list.clear();
   out_list.reserve(bbox_min_x.size());

   const std::size_t simd_iterations = bbox_min_x.size() / avx_width;
   const std::size_t scalar_iterations = bbox_min_x.size() % avx_width;

   for (std::size_t i = 0; i < simd_iterations; ++i) {
      int inside_mask = 0xff;

      for (const auto& plane : frustum.planes | drop(1)) {
         const __m256 plane_x = _mm256_broadcast_ss(&plane.x);
         const __m256 plane_y = _mm256_broadcast_ss(&plane.y);
         const __m256 plane_z = _mm256_broadcast_ss(&plane.z);
         const __m256 plane_w = _mm256_broadcast_ss(&plane.w);

         const __m256 outside =
            outside_plane(plane_x, plane_y, plane_z, plane_w,
                          _mm256_load_ps(&bbox_min_x[i * avx_width]),
                          _mm256_load_ps(&bbox_min_y[i * avx_width]),
                          _mm256_load_ps(&bbox_min_z[i * avx_width]),
                          _mm256_load_ps(&bbox_max_x[i * avx_width]),
                          _mm256_load_ps(&bbox_max_y[i * avx_width]),
                          _mm256_load_ps(&bbox_max_z[i * avx_width]));

         inside_mask &= ~_mm256_movemask_ps(outside);
      }

      if (not inside_mask) continue;

      const auto push_index = [&pipeline_flags, &out_list](const std::size_t i) {
         if (not(are_flags_set(pipeline_flags[i], material_pipeline_flags::transparent) or
                 are_flags_set(pipeline_flags[i], material_pipeline_flags::additive))) {
            out_list.push_back(static_cast<uint16>(i));
         }
      };

      if (inside_mask & 0b1) push_index((i * avx_width) + 0);
      if (inside_mask & 0b10) push_index((i * avx_width) + 1);
      if (inside_mask & 0b100) push_index((i * avx_width) + 2);
      if (inside_mask & 0b1000) push_index((i * avx_width) + 3);
      if (inside_mask & 0b10000) push_index((i * avx_width) + 4);
      if (inside_mask & 0b100000) push_index((i * avx_width) + 5);
      if (inside_mask & 0b1000000) push_index((i * avx_width) + 6);
      if (inside_mask & 0b10000000) push_index((i * avx_width) + 7);
   }

   for (std::size_t i = bbox_min_x.size() - scalar_iterations;
        i < bbox_min_x.size(); ++i) {
      if (not intersects(frustum, {{bbox_min_x[i], bbox_min_y[i], bbox_min_z[i]},
                                   {bbox_max_x[i], bbox_max_y[i], bbox_max_z[i]}})) {
         continue;
      }

      if (not(are_flags_set(pipeline_flags[i], material_pipeline_flags::transparent) or
              are_flags_set(pipeline_flags[i], material_pipeline_flags::additive))) {
         out_list.push_back(static_cast<uint16>(i));
      }
   }
}

}