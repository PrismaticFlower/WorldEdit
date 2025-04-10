#include "cull_objects.hpp"

#include <cassert>
#include <ranges>

#include <immintrin.h>

namespace we::graphics {

using std::ranges::views::drop;

void cull_objects_scalar(const frustum& frustum,
                         std::span<const math::bounding_box> bbox,
                         std::vector<uint16>& out_list) noexcept
{
   out_list.clear();
   out_list.reserve(bbox.size());

   for (std::size_t i = 0; i < bbox.size(); ++i) {
      if (not intersects(frustum, bbox[i])) continue;

      out_list.push_back(static_cast<uint16>(i));
   }
}

namespace {

constexpr int avx_width = 8;

auto outside_plane(__m256 plane_x, __m256 plane_y, __m256 plane_z, __m256 plane_w,
                   __m256 point_x, __m256 point_y, __m256 point_z) noexcept -> __m256
{
   const __m256 x = _mm256_mul_ps(plane_x, point_x);
   const __m256 y = _mm256_mul_ps(plane_y, point_y);
   const __m256 z = _mm256_mul_ps(plane_z, point_z);

   const __m256 xy_sum = _mm256_add_ps(x, y);
   const __m256 zw_sum = _mm256_add_ps(z, plane_w);

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

auto outside_corners(const frustum& frustum, const float float3::*axis,
                     __m256 bbox_corner_min, __m256 bbox_corner_max) noexcept -> __m256
{
   const float true_mask = std::bit_cast<float>(0xff'ff'ff'ff);

   __m256 outside_min_mask = _mm256_broadcast_ss(&true_mask);
   __m256 outside_max_mask = outside_min_mask;

   for (const float3& frustum_corner : frustum.corners) {
      const __m256 corner = _mm256_broadcast_ss(&(frustum_corner.*axis));

      const __m256 min_outside = _mm256_cmp_ps(corner, bbox_corner_min, _CMP_LT_OQ);
      const __m256 max_outside = _mm256_cmp_ps(corner, bbox_corner_max, _CMP_GT_OQ);

      outside_min_mask = _mm256_and_ps(outside_min_mask, min_outside);
      outside_max_mask = _mm256_and_ps(outside_max_mask, max_outside);
   }

   return _mm256_or_ps(outside_min_mask, outside_max_mask);
}

}

void cull_objects_avx2(const frustum& frustum, std::span<const float> bbox_min_x,
                       std::span<const float> bbox_min_y,
                       std::span<const float> bbox_min_z,
                       std::span<const float> bbox_max_x,
                       std::span<const float> bbox_max_y,
                       std::span<const float> bbox_max_z,
                       std::vector<uint16>& out_list) noexcept
{
   assert(bbox_min_x.size() == bbox_min_y.size());
   assert(bbox_min_x.size() == bbox_min_z.size());
   assert(bbox_min_x.size() == bbox_max_x.size());
   assert(bbox_min_x.size() == bbox_max_y.size());
   assert(bbox_min_x.size() == bbox_max_z.size());

   out_list.clear();
   out_list.reserve(bbox_min_x.size());

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
         outside_corners(frustum, &float3::x,
                         _mm256_load_ps(&bbox_min_x[i * avx_width]),
                         _mm256_load_ps(&bbox_max_x[i * avx_width]));

      inside_mask &= ~_mm256_movemask_ps(outside_x);

      if (not inside_mask) continue;

      const __m256 outside_y =
         outside_corners(frustum, &float3::y,
                         _mm256_load_ps(&bbox_min_y[i * avx_width]),
                         _mm256_load_ps(&bbox_max_y[i * avx_width]));

      inside_mask &= ~_mm256_movemask_ps(outside_y);

      if (not inside_mask) continue;

      const __m256 outside_z =
         outside_corners(frustum, &float3::z,
                         _mm256_load_ps(&bbox_min_z[i * avx_width]),
                         _mm256_load_ps(&bbox_max_z[i * avx_width]));

      inside_mask &= ~_mm256_movemask_ps(outside_z);

      if (not inside_mask) continue;

      // clang-format off

      if (inside_mask & 0b1) out_list.push_back(static_cast<uint16>((i * avx_width) + 0));
      if (inside_mask & 0b10) out_list.push_back(static_cast<uint16>((i * avx_width) + 1));
      if (inside_mask & 0b100) out_list.push_back((static_cast<uint16>(i * avx_width) + 2));
      if (inside_mask & 0b1000) out_list.push_back(static_cast<uint16>((i * avx_width) + 3));
      if (inside_mask & 0b10000) out_list.push_back(static_cast<uint16>((i * avx_width) + 4));
      if (inside_mask & 0b100000) out_list.push_back(static_cast<uint16>((i * avx_width) + 5));
      if (inside_mask & 0b1000000) out_list.push_back(static_cast<uint16>((i * avx_width) + 6));
      if (inside_mask & 0b10000000) out_list.push_back(static_cast<uint16>((i * avx_width) + 7));

      // clang-format on
   }

   for (std::size_t i = bbox_min_x.size() - scalar_iterations;
        i < bbox_min_x.size(); ++i) {
      if (not intersects(frustum, {{bbox_min_x[i], bbox_min_y[i], bbox_min_z[i]},
                                   {bbox_max_x[i], bbox_max_y[i], bbox_max_z[i]}})) {
         continue;
      }

      out_list.push_back(static_cast<uint16>(i));
   }
}

void cull_objects_avx2(const frustum& frustum, std::span<const float> bbox_min_x,
                       std::span<const float> bbox_min_y,
                       std::span<const float> bbox_min_z,
                       std::span<const float> bbox_max_x,
                       std::span<const float> bbox_max_y,
                       std::span<const float> bbox_max_z,
                       std::span<const bool> hidden, std::span<const int8> layers,
                       const world::active_layers active_layers,
                       uint32& out_count, std::span<uint32> out_list) noexcept
{
   assert(bbox_min_x.size() == bbox_min_y.size());
   assert(bbox_min_x.size() == bbox_min_z.size());
   assert(bbox_min_x.size() == bbox_max_x.size());
   assert(bbox_min_x.size() == bbox_max_y.size());
   assert(bbox_min_x.size() == bbox_max_z.size());
   assert(bbox_min_x.size() == hidden.size());
   assert(bbox_min_x.size() == layers.size());
   assert(bbox_min_x.size() <= out_list.size());

   out_count = 0;

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
         outside_corners(frustum, &float3::x,
                         _mm256_load_ps(&bbox_min_x[i * avx_width]),
                         _mm256_load_ps(&bbox_max_x[i * avx_width]));

      inside_mask &= ~_mm256_movemask_ps(outside_x);

      if (not inside_mask) continue;

      const __m256 outside_y =
         outside_corners(frustum, &float3::y,
                         _mm256_load_ps(&bbox_min_y[i * avx_width]),
                         _mm256_load_ps(&bbox_max_y[i * avx_width]));

      inside_mask &= ~_mm256_movemask_ps(outside_y);

      if (not inside_mask) continue;

      const __m256 outside_z =
         outside_corners(frustum, &float3::z,
                         _mm256_load_ps(&bbox_min_z[i * avx_width]),
                         _mm256_load_ps(&bbox_max_z[i * avx_width]));

      inside_mask &= ~_mm256_movemask_ps(outside_z);

      if (not inside_mask) continue;

      // clang-format off

      if (inside_mask & 0b1        and active_layers[layers[i * avx_width + 0]] and not hidden[i * avx_width + 0]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 0);
      if (inside_mask & 0b10       and active_layers[layers[i * avx_width + 1]] and not hidden[i * avx_width + 1]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 1);
      if (inside_mask & 0b100      and active_layers[layers[i * avx_width + 2]] and not hidden[i * avx_width + 2]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 2);
      if (inside_mask & 0b1000     and active_layers[layers[i * avx_width + 3]] and not hidden[i * avx_width + 3]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 3);
      if (inside_mask & 0b10000    and active_layers[layers[i * avx_width + 4]] and not hidden[i * avx_width + 4]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 4);
      if (inside_mask & 0b100000   and active_layers[layers[i * avx_width + 5]] and not hidden[i * avx_width + 5]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 5);
      if (inside_mask & 0b1000000  and active_layers[layers[i * avx_width + 6]] and not hidden[i * avx_width + 6]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 6);
      if (inside_mask & 0b10000000 and active_layers[layers[i * avx_width + 7]] and not hidden[i * avx_width + 7]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 7);

      // clang-format on
   }

   for (std::size_t i = bbox_min_x.size() - scalar_iterations;
        i < bbox_min_x.size(); ++i) {
      if (not active_layers[layers[i]] or hidden[i] or
          not intersects(frustum, {{bbox_min_x[i], bbox_min_y[i], bbox_min_z[i]},
                                   {bbox_max_x[i], bbox_max_y[i], bbox_max_z[i]}})) {
         continue;
      }

      out_list[out_count++] = static_cast<uint32>(i);
   }
}

void cull_objects_shadow_cascade_scalar(const frustum& frustum,
                                        std::span<const math::bounding_box> bbox,
                                        std::vector<uint16>& out_list) noexcept
{
   out_list.clear();
   out_list.reserve(bbox.size());

   for (std::size_t i = 0; i < bbox.size(); ++i) {
      if (not intersects_shadow_cascade(frustum, bbox[i])) continue;

      out_list.push_back(static_cast<uint16>(i));
   }
}

void cull_objects_shadow_cascade_avx2(
   const frustum& frustum, std::span<const float> bbox_min_x,
   std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
   std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
   std::span<const float> bbox_max_z, std::vector<uint16>& out_list) noexcept
{
   assert(bbox_min_x.size() == bbox_min_y.size());
   assert(bbox_min_x.size() == bbox_min_z.size());
   assert(bbox_min_x.size() == bbox_max_x.size());
   assert(bbox_min_x.size() == bbox_max_y.size());
   assert(bbox_min_x.size() == bbox_max_z.size());

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

      // clang-format off

      if (inside_mask & 0b1) out_list.push_back(static_cast<uint16>((i * avx_width) + 0));
      if (inside_mask & 0b10) out_list.push_back(static_cast<uint16>((i * avx_width) + 1));
      if (inside_mask & 0b100) out_list.push_back((static_cast<uint16>(i * avx_width) + 2));
      if (inside_mask & 0b1000) out_list.push_back(static_cast<uint16>((i * avx_width) + 3));
      if (inside_mask & 0b10000) out_list.push_back(static_cast<uint16>((i * avx_width) + 4));
      if (inside_mask & 0b100000) out_list.push_back(static_cast<uint16>((i * avx_width) + 5));
      if (inside_mask & 0b1000000) out_list.push_back(static_cast<uint16>((i * avx_width) + 6));
      if (inside_mask & 0b10000000) out_list.push_back(static_cast<uint16>((i * avx_width) + 7));

      // clang-format on
   }

   for (std::size_t i = bbox_min_x.size() - scalar_iterations;
        i < bbox_min_x.size(); ++i) {
      if (not intersects(frustum, {{bbox_min_x[i], bbox_min_y[i], bbox_min_z[i]},
                                   {bbox_max_x[i], bbox_max_y[i], bbox_max_z[i]}})) {
         continue;
      }

      out_list.push_back(static_cast<uint16>(i));
   }
}

void cull_objects_shadow_cascade_avx2(
   const frustum& frustum, std::span<const float> bbox_min_x,
   std::span<const float> bbox_min_y, std::span<const float> bbox_min_z,
   std::span<const float> bbox_max_x, std::span<const float> bbox_max_y,
   std::span<const float> bbox_max_z, std::span<const bool> hidden,
   std::span<const int8> layers, const world::active_layers active_layers,
   uint32& out_count, std::span<uint32> out_list) noexcept
{
   assert(bbox_min_x.size() == bbox_min_y.size());
   assert(bbox_min_x.size() == bbox_min_z.size());
   assert(bbox_min_x.size() == bbox_max_x.size());
   assert(bbox_min_x.size() == bbox_max_y.size());
   assert(bbox_min_x.size() == bbox_max_z.size());
   assert(bbox_min_x.size() == hidden.size());
   assert(bbox_min_x.size() == layers.size());
   assert(bbox_min_x.size() <= out_list.size());

   out_count = 0;

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

      // clang-format off

      if (inside_mask & 0b1        and active_layers[layers[i * avx_width + 0]] and not hidden[i * avx_width + 0]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 0);
      if (inside_mask & 0b10       and active_layers[layers[i * avx_width + 1]] and not hidden[i * avx_width + 1]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 1);
      if (inside_mask & 0b100      and active_layers[layers[i * avx_width + 2]] and not hidden[i * avx_width + 2]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 2);
      if (inside_mask & 0b1000     and active_layers[layers[i * avx_width + 3]] and not hidden[i * avx_width + 3]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 3);
      if (inside_mask & 0b10000    and active_layers[layers[i * avx_width + 4]] and not hidden[i * avx_width + 4]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 4);
      if (inside_mask & 0b100000   and active_layers[layers[i * avx_width + 5]] and not hidden[i * avx_width + 5]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 5);
      if (inside_mask & 0b1000000  and active_layers[layers[i * avx_width + 6]] and not hidden[i * avx_width + 6]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 6);
      if (inside_mask & 0b10000000 and active_layers[layers[i * avx_width + 7]] and not hidden[i * avx_width + 7]) out_list[out_count++] = static_cast<uint32>((i * avx_width) + 7);

      // clang-format on
   }

   for (std::size_t i = bbox_min_x.size() - scalar_iterations;
        i < bbox_min_x.size(); ++i) {
      if (not active_layers[layers[i]] or hidden[i] or
          not intersects(frustum, {{bbox_min_x[i], bbox_min_y[i], bbox_min_z[i]},
                                   {bbox_max_x[i], bbox_max_y[i], bbox_max_z[i]}})) {
         continue;
      }

      out_list[out_count++] = static_cast<uint32>(i);
   }
}

}