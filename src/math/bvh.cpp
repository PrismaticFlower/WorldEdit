#include "bvh.hpp"
#include "bounding_box.hpp"
#include "frustum.hpp"
#include "intersectors.hpp"
#include "quaternion_funcs.hpp"
#include "vector_funcs.hpp"

#include <bit>
#include <cassert>
#include <vector>

#include <immintrin.h>

namespace we {

namespace {

constexpr int32 split_bin_count = 8;

bool check_indices_in_range(std::span<const std::array<uint16, 3>> indices,
                            std::span<const float3> positions) noexcept
{
   for (const auto& [i0, i1, i2] : indices) {
      if (i0 >= positions.size()) return false;
      if (i1 >= positions.size()) return false;
      if (i2 >= positions.size()) return false;
   }

   return true;
}

bool check_instances(std::span<const top_level_bvh::instance> instances) noexcept
{
   for (const auto& instance : instances) {
      if (not instance.bvh) return false;
   }

   return true;
}

auto area(const math::bounding_box& bbox) noexcept -> float
{
   const float3 extents = bbox.max - bbox.min;

   return extents.x * extents.y + extents.y * extents.z + extents.z * extents.x;
}

[[msvc::forceinline]] auto __vectorcall intersect_aabb(
   const __m128 ray_origin_x,        //
   const __m128 ray_origin_y,        //
   const __m128 ray_origin_z,        //
   const __m128 inv_ray_direction_x, //
   const __m128 inv_ray_direction_y, //
   const __m128 inv_ray_direction_z, //
   const __m128 bbox_min_x,          //
   const __m128 bbox_min_y,          //
   const __m128 bbox_min_z,          //
   const __m128 bbox_max_x,          //
   const __m128 bbox_max_y,          //
   const __m128 bbox_max_z,          //
   const __m128 t_limit,             //
   __m128& t) noexcept -> int
{
   const __m128 ts0_x =
      _mm_mul_ps(_mm_sub_ps(bbox_min_x, ray_origin_x), inv_ray_direction_x);
   const __m128 ts0_y =
      _mm_mul_ps(_mm_sub_ps(bbox_min_y, ray_origin_y), inv_ray_direction_y);
   const __m128 ts0_z =
      _mm_mul_ps(_mm_sub_ps(bbox_min_z, ray_origin_z), inv_ray_direction_z);

   const __m128 ts1_x =
      _mm_mul_ps(_mm_sub_ps(bbox_max_x, ray_origin_x), inv_ray_direction_x);
   const __m128 ts1_y =
      _mm_mul_ps(_mm_sub_ps(bbox_max_y, ray_origin_y), inv_ray_direction_y);
   const __m128 ts1_z =
      _mm_mul_ps(_mm_sub_ps(bbox_max_z, ray_origin_z), inv_ray_direction_z);

   const __m128 ts_min_x = _mm_min_ps(ts0_x, ts1_x);
   const __m128 ts_min_y = _mm_min_ps(ts0_y, ts1_y);
   const __m128 ts_min_z = _mm_min_ps(ts0_z, ts1_z);

   const __m128 ts_max_x = _mm_max_ps(ts0_x, ts1_x);
   const __m128 ts_max_y = _mm_max_ps(ts0_y, ts1_y);
   const __m128 ts_max_z = _mm_max_ps(ts0_z, ts1_z);

   const __m128 t_min = _mm_max_ps(_mm_max_ps(ts_min_x, ts_min_y), ts_min_z);
   const __m128 t_max =
      _mm_min_ps(_mm_min_ps(_mm_min_ps(ts_max_x, ts_max_y), ts_max_z), t_limit);

   const __m128 hit_mask = _mm_cmple_ps(t_min, t_max);

   t = t_min;

   return _mm_movemask_ps(hit_mask);
}

[[msvc::forceinline]] auto __vectorcall inside_plane(const __m128 plane_x, //
                                                     const __m128 plane_y, //
                                                     const __m128 plane_z, //
                                                     const __m128 plane_w, //
                                                     const __m128 point_x, //
                                                     const __m128 point_y, //
                                                     const __m128 point_z) noexcept -> __m128
{
   const __m128 x = _mm_mul_ps(plane_x, point_x);
   const __m128 y = _mm_mul_ps(plane_y, point_y);
   const __m128 z = _mm_mul_ps(plane_z, point_z);

   const __m128 xy_sum = _mm_add_ps(x, y);
   const __m128 zw_sum = _mm_add_ps(z, plane_w);

   const __m128 plane_distance = _mm_add_ps(xy_sum, zw_sum);

   return _mm_cmpge_ps(plane_distance, _mm_setzero_ps());
}

[[msvc::forceinline]] auto __vectorcall inside_corners(
   const frustum& frustum, const float float3::*axis,
   const __m128 bbox_corner_min, const __m128 bbox_corner_max) noexcept -> __m128
{
   __m128 inside_min_mask = _mm_setzero_ps();
   __m128 inside_max_mask = inside_min_mask;

   for (const float3& frustum_corner : frustum.corners) {
      const __m128 corner = _mm_broadcast_ss(&(frustum_corner.*axis));

      const __m128 min_inside = _mm_cmpge_ps(corner, bbox_corner_min);
      const __m128 max_inside = _mm_cmple_ps(corner, bbox_corner_max);

      inside_min_mask = _mm_or_ps(inside_min_mask, min_inside);
      inside_max_mask = _mm_or_ps(inside_max_mask, max_inside);
   }

   return _mm_and_ps(inside_min_mask, inside_max_mask);
}

[[msvc::forceinline]] auto __vectorcall intersects_frustum(
   const frustum& frustum,  //
   const __m128 bbox_min_x, //
   const __m128 bbox_min_y, //
   const __m128 bbox_min_z, //
   const __m128 bbox_max_x, //
   const __m128 bbox_max_y, //
   const __m128 bbox_max_z) noexcept -> int
{
   int intersects = 0b1111;

   for (const auto& plane : frustum.planes) {
      const __m128 plane_x = _mm_broadcast_ss(&plane.x);
      const __m128 plane_y = _mm_broadcast_ss(&plane.y);
      const __m128 plane_z = _mm_broadcast_ss(&plane.z);
      const __m128 plane_w = _mm_broadcast_ss(&plane.w);

      __m128 inside_mask = inside_plane(plane_x, plane_y, plane_z, plane_w,
                                        bbox_min_x, bbox_min_y, bbox_min_z);
      inside_mask = _mm_or_ps(inside_plane(plane_x, plane_y, plane_z, plane_w,
                                           bbox_max_x, bbox_min_y, bbox_min_z),
                              inside_mask);
      inside_mask = _mm_or_ps(inside_plane(plane_x, plane_y, plane_z, plane_w,
                                           bbox_min_x, bbox_max_y, bbox_min_z),
                              inside_mask);
      inside_mask = _mm_or_ps(inside_plane(plane_x, plane_y, plane_z, plane_w,
                                           bbox_max_x, bbox_max_y, bbox_min_z),
                              inside_mask);
      inside_mask = _mm_or_ps(inside_plane(plane_x, plane_y, plane_z, plane_w,
                                           bbox_min_x, bbox_min_y, bbox_max_z),
                              inside_mask);
      inside_mask = _mm_or_ps(inside_plane(plane_x, plane_y, plane_z, plane_w,
                                           bbox_max_x, bbox_min_y, bbox_max_z),
                              inside_mask);
      inside_mask = _mm_or_ps(inside_plane(plane_x, plane_y, plane_z, plane_w,
                                           bbox_min_x, bbox_max_y, bbox_max_z),
                              inside_mask);
      inside_mask = _mm_or_ps(inside_plane(plane_x, plane_y, plane_z, plane_w,
                                           bbox_max_x, bbox_max_y, bbox_max_z),
                              inside_mask);

      intersects &= _mm_movemask_ps(inside_mask);

      if (intersects == 0) return 0;
   }

   intersects &=
      _mm_movemask_ps(inside_corners(frustum, &float3::x, bbox_min_x, bbox_max_x));

   if (intersects == 0) return 0;

   intersects &=
      _mm_movemask_ps(inside_corners(frustum, &float3::y, bbox_min_y, bbox_max_y));

   if (intersects == 0) return 0;

   intersects &=
      _mm_movemask_ps(inside_corners(frustum, &float3::z, bbox_min_z, bbox_max_z));

   return intersects;
}

}

struct detail::bvh_impl {
   bvh_impl(std::span<const std::array<uint16, 3>> indices,
            std::span<const float3> positions, bvh_flags flags) noexcept
      : _indices{indices},
        _positions{positions},
        _no_backface_cull{not flags.backface_cull}
   {
      assert(check_indices_in_range(indices, positions));

      std::vector<float3> centroids;
      centroids.reserve(indices.size());

      for (const auto& [i0, i1, i2] : indices) {
         centroids.emplace_back((positions[i0] + positions[i1] + positions[i2]) *
                                (1.0f / 3.0f));
      }

      _triangles.reserve(indices.size());

      for (uint32 i = 0; i < indices.size(); ++i) {
         _triangles.emplace_back(i);
      }

      _nodes.reserve(std::max(std::ssize(indices) * 2 / 4 - 1, 1ll));

      leaf_node root_node = {.tri_count = static_cast<int32>(indices.size())};

      update_node_bounds(root_node);

      if (std::optional<node_packed_x4> subdivided_root = subdivide(root_node, centroids);
          subdivided_root) {
         _nodes.push_back(*subdivided_root);

         int32 nodes_used = 1;

         for (int32 packed_index = 0; packed_index < nodes_used; ++packed_index) {
            for (int32 i = 0; i < 4; ++i) {
               node_packed_x4& packed = _nodes[packed_index];

               leaf_node child_node = {
                  .bbox = {.min = {packed.bbox.min_x[i], packed.bbox.min_y[i],
                                   packed.bbox.min_z[i]},

                           .max = {packed.bbox.max_x[i], packed.bbox.max_y[i],
                                   packed.bbox.max_z[i]}},

                  .first_tri = packed.children_or_first_tri[i],
                  .tri_count = packed.tri_count[i],
               };

               update_node_bounds(child_node);

               if (std::optional<node_packed_x4> subdivided =
                      subdivide(child_node, centroids);
                   subdivided) {
                  const int32 child_index = nodes_used;

                  _nodes.push_back(*subdivided);

                  packed.children_or_first_tri[i] = child_index;
                  packed.tri_count[i] = 0;

                  nodes_used += 1;
               }
            }
         }
      }
      else {
         _nodes.push_back({.bbox =
                              {
                                 .min_x = {root_node.bbox.min.x},
                                 .min_y = {root_node.bbox.min.y},
                                 .min_z = {root_node.bbox.min.z},
                                 .max_x = {root_node.bbox.max.x},
                                 .max_y = {root_node.bbox.max.y},
                                 .max_z = {root_node.bbox.max.z},
                              },
                           .tri_count = {static_cast<int32>(indices.size()), 1, 1, 1}});
      }

      _nodes.shrink_to_fit();
   }

   bvh_impl(const bvh_impl&) = delete;
   auto operator=(const bvh_impl&) -> bvh_impl& = delete;

   bvh_impl(bvh_impl&&) noexcept = delete;
   auto operator=(bvh_impl&&) -> bvh_impl& = delete;

   [[nodiscard]] auto raycast(const float3& ray_origin,
                              const float3& ray_direction, const float max_distance,
                              const bvh_ray_flags flags) const noexcept
      -> std::optional<bvh::ray_hit>
   {
      const float3 inv_ray_direction = 1.0f / ray_direction;

      const __m128 ray_origin_x = _mm_broadcast_ss(&ray_origin.x);
      const __m128 ray_origin_y = _mm_broadcast_ss(&ray_origin.y);
      const __m128 ray_origin_z = _mm_broadcast_ss(&ray_origin.z);

      const __m128 inv_ray_direction_x = _mm_broadcast_ss(&inv_ray_direction.x);
      const __m128 inv_ray_direction_y = _mm_broadcast_ss(&inv_ray_direction.y);
      const __m128 inv_ray_direction_z = _mm_broadcast_ss(&inv_ray_direction.z);

      std::array<int32, 64> stack = {
         _root_node_index,
      };
      int32 stack_ptr = 0;

      __m128 closest_hit = _mm_broadcast_ss(&max_distance);
      float3 hit_normal = {};

      while (stack_ptr >= 0) {
         const node_packed_x4& node = _nodes[stack[stack_ptr]];

         stack_ptr -= 1;

         __m128 hit_distance;

         const int hit_mask =
            intersect_aabb(ray_origin_x, ray_origin_y, ray_origin_z,
                           inv_ray_direction_x, inv_ray_direction_y,
                           inv_ray_direction_z, _mm_load_ps(node.bbox.min_x.data()),
                           _mm_load_ps(node.bbox.min_y.data()),
                           _mm_load_ps(node.bbox.min_z.data()),
                           _mm_load_ps(node.bbox.max_x.data()),
                           _mm_load_ps(node.bbox.max_y.data()),
                           _mm_load_ps(node.bbox.max_z.data()), closest_hit,
                           hit_distance);

         if (hit_mask) {
            alignas(__m128) std::array<float, 4> hits;

            _mm_store_ps(hits.data(), hit_distance);

            // TODO: Sort by hit distance.

            for (int lane_index = 0; lane_index < std::ssize(node.tri_count);
                 ++lane_index) {
               if (not(hit_mask & (1 << lane_index))) continue;

               const bool is_leaf = node.tri_count[lane_index] != 0;

               if (is_leaf) {
                  const int32 last_tri = node.children_or_first_tri[lane_index] +
                                         node.tri_count[lane_index];

                  for (int32 tri_index = node.children_or_first_tri[lane_index];
                       tri_index < last_tri; ++tri_index) {
                     const std::array<uint16, 3>& tri =
                        _indices[_triangles[tri_index]];

                     const float3& v0 = _positions[tri[0]];
                     const float3& v1 = _positions[tri[1]];
                     const float3& v2 = _positions[tri[2]];

                     float closest_hit_scalar;

                     _mm_store_ss(&closest_hit_scalar, closest_hit);

                     [[msvc::forceinline_calls]] //
                     if (float hit = 0.0f;
                         intersect_tri(ray_origin, ray_direction, v0, v1, v2, hit) and
                         hit < closest_hit_scalar) {

                        const float3 normal = cross(v1 - v0, v2 - v0);

                        if (flags.allow_backface_cull and
                            dot(-ray_direction, normal) < 0.0f and
                            not _no_backface_cull) {
                           continue;
                        }

                        closest_hit = _mm_broadcast_ss(&hit);
                        hit_normal = normal;

                        if (flags.accept_first_hit) {
                           return std::optional{
                              bvh::ray_hit{.distance = hit, .normal = hit_normal}};
                        }
                     }
                  }
               }
               else {
                  stack_ptr += 1;

                  stack[stack_ptr] = node.children_or_first_tri[lane_index];
               }
            }
         }
      }

      float closest_hit_scalar;

      _mm_store_ss(&closest_hit_scalar, closest_hit);

      return closest_hit_scalar < max_distance
                ? std::optional{bvh::ray_hit{.distance = closest_hit_scalar,
                                             .normal = hit_normal}}
                : std::nullopt;
   }

   [[nodiscard]] bool intersects(const frustum& frustum) const noexcept
   {
      std::array<int32, 64> stack = {
         _root_node_index,
      };
      int32 stack_ptr = 0;

      while (stack_ptr >= 0) {
         const node_packed_x4& node = _nodes[stack[stack_ptr]];

         stack_ptr -= 1;

         const int intersects_mask =
            intersects_frustum(frustum, _mm_load_ps(node.bbox.min_x.data()),
                               _mm_load_ps(node.bbox.min_y.data()),
                               _mm_load_ps(node.bbox.min_z.data()),
                               _mm_load_ps(node.bbox.max_x.data()),
                               _mm_load_ps(node.bbox.max_y.data()),
                               _mm_load_ps(node.bbox.max_z.data()));

         if (intersects_mask) {
            for (int lane_index = 0; lane_index < std::ssize(node.tri_count);
                 ++lane_index) {
               if (not(intersects_mask & (1 << lane_index))) continue;

               const bool is_leaf = node.tri_count[lane_index] != 0;

               if (is_leaf) {
                  const int32 last_tri = node.children_or_first_tri[lane_index] +
                                         node.tri_count[lane_index];

                  for (int32 tri_index = node.children_or_first_tri[lane_index];
                       tri_index < last_tri; ++tri_index) {
                     const std::array<uint16, 3>& tri_indices =
                        _indices[_triangles[tri_index]];

                     // This probably ain't the "proper" way to to test if a triangle intersects a frustum. But after getting
                     // to the point where a speaker at a talk about the Seperating Axis Theorem said it took them "a few months"
                     // to truly understand these concepts I decided I didn't have that long to implement this feature (precise
                     // selection) and took a step back to look at it afresh instead.
                     //
                     // So what I realised (and hope is correct) is that in our special case (a single tri against a frustum)
                     // there are four possible cases.
                     //
                     // 1. One of the triangle's vertices is inside the frustum.
                     // 2. One of the triangle's edges intersects a face of the frustum.
                     // 3. One of the frustum's edges intersects the triangle.
                     // 3. All of the triangle's vertices are outside the frustum, none of the triangle edges intersect a face
                     // of the frustum and none of frustum's edges intersect the triangle.
                     //
                     // So for 1. we just perform an inside frustum test for each vertex. If any of these return true then we return
                     // true.
                     //
                     // For 2. we build the build the edge vectors starting from each vertex going around in (I hope) winding order.
                     // We then take the length the edge vectors and then divide them by the length to get the normalized edge
                     // directions. (We need the length either way and this way saves us calculating it twice from calling normalize
                     // explicitly).
                     //
                     // After that we use a ray-triangle intersection test against the frustum's faces using the vertex as the
                     // ray origin and the edge direction as the ray direction. If there is a hit and if it'd distance is
                     // less-than or equal the edge length we know the triangle intersects the frustum and we return true.
                     //
                     // The ray-triangle test is used instead of ray-quad as it appears to suffer from less precision issues when used
                     // with the huge frustum faces.
                     //
                     // For 3. we do the same steps as 2. only we use the frustum's edges instead and we intersect the rays against the
                     // triangle.
                     //
                     // For case 4. nothing needs to be done, we continue with the traversal loop.

                     const float3& v0 = _positions[tri_indices[0]];
                     const float3& v1 = _positions[tri_indices[1]];
                     const float3& v2 = _positions[tri_indices[2]];

                     if (we::intersects(frustum, v0, 0.0f) or
                         we::intersects(frustum, v1, 0.0f) or
                         we::intersects(frustum, v2, 0.0f)) {
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

                     // These don't all face out. But it doesn't matter for our use.
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
                               intersect_tri(tri[edge_index],
                                             tri_edge_directions[edge_index],
                                             frustum.corners[i0], frustum.corners[i1],
                                             frustum.corners[i2], distance) and
                               distance <= tri_edge_lengths[edge_index]) {
                              return true;
                           }

                           if (float distance = FLT_MAX;
                               intersect_tri(tri[edge_index],
                                             tri_edge_directions[edge_index],
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
                        const float3 edge_vector =
                           frustum.corners[i1] - frustum.corners[i0];
                        const float edge_length = length(edge_vector);
                        const float3 edge_direction = edge_vector / edge_length;

                        if (float distance = FLT_MAX;
                            intersect_tri(frustum.corners[i0], edge_direction,
                                          v0, v1, v2, distance) and
                            distance <= edge_length) {
                           return true;
                        }
                     }
                  }
               }
               else {
                  stack_ptr += 1;

                  stack[stack_ptr] = node.children_or_first_tri[lane_index];
               }
            }
         }
      }

      return false;
   }

   [[nodiscard]] auto get_debug_boxes() const noexcept -> std::vector<math::bounding_box>
   {
      std::vector<math::bounding_box> boxes;
      boxes.reserve(_nodes.size() * 4);

      for (const node_packed_x4& node : _nodes) {
         const bounding_box_x4& bbox = node.bbox;

         for (int32 i = 0; i < std::ssize(bbox.min_x); ++i) {
            boxes.push_back({.min = {bbox.min_x[i], bbox.min_y[i], bbox.min_z[i]},
                             .max = {bbox.max_x[i], bbox.max_y[i], bbox.max_z[i]}});
         }
      }

      return boxes;
   }

   [[nodiscard]] auto get_bbox() const noexcept -> math::bounding_box
   {
      const bounding_box_x4& bbox = _nodes[_root_node_index].bbox;

      const std::array<float, 4>& min_x = bbox.min_x;
      const std::array<float, 4>& min_y = bbox.min_y;
      const std::array<float, 4>& min_z = bbox.min_z;
      const std::array<float, 4>& max_x = bbox.max_x;
      const std::array<float, 4>& max_y = bbox.max_y;
      const std::array<float, 4>& max_z = bbox.max_z;

      return {
         .min = {std::min(std::min(std::min(min_x[0], min_x[1]), min_x[2]), min_x[3]),
                 std::min(std::min(std::min(min_y[0], min_y[1]), min_y[2]), min_y[3]),
                 std::min(std::min(std::min(min_z[0], min_z[1]), min_z[2]), min_z[3])},

         .max = {std::max(std::max(std::max(max_x[0], max_x[1]), max_x[2]), max_x[3]),
                 std::max(std::max(std::max(max_y[0], max_y[1]), max_y[2]), max_y[3]),
                 std::max(std::max(std::max(max_z[0], max_z[1]), max_z[2]), max_z[3])}};
   }

   [[nodiscard]] auto get_tri_count() const noexcept -> std::size_t
   {
      return _triangles.size();
   }

private:
   struct leaf_node {
      math::bounding_box bbox;
      int32 first_tri = 0;
      int32 tri_count = 0;
   };

   struct bounding_box_x4 {
      alignas(__m128) std::array<float, 4> min_x;
      alignas(__m128) std::array<float, 4> min_y;
      alignas(__m128) std::array<float, 4> min_z;
      alignas(__m128) std::array<float, 4> max_x;
      alignas(__m128) std::array<float, 4> max_y;
      alignas(__m128) std::array<float, 4> max_z;
   };

   struct node_packed_x4 {
      bounding_box_x4 bbox;
      std::array<int32, 4> children_or_first_tri = {};
      std::array<int32, 4> tri_count = {};
   };

   std::vector<node_packed_x4> _nodes;
   std::vector<uint32> _triangles;
   constexpr static int32 _root_node_index = 0;

   std::span<const std::array<uint16, 3>> _indices;
   std::span<const float3> _positions;

   bool _no_backface_cull = false;

   using float3_axis = float float3::*;

   void update_node_bounds(leaf_node& node) noexcept
   {
      node.bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                   .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};

      const int32 last_tri = node.first_tri + node.tri_count;

      for (int32 i = node.first_tri; i < last_tri; ++i) {
         const std::array<uint16, 3>& tri = _indices[_triangles[i]];

         const float3& v0 = _positions[tri[0]];
         const float3& v1 = _positions[tri[1]];
         const float3& v2 = _positions[tri[2]];

         node.bbox.min = min(node.bbox.min, min(v0, min(v1, v2)));
         node.bbox.max = max(node.bbox.max, max(v0, max(v1, v2)));
      }
   }

   auto subdivide(const leaf_node& root, std::span<const float3> centroids) noexcept
      -> std::optional<node_packed_x4>
   {
      float3_axis split_axis = &float3::x;
      float split_position = 0.0f;
      float split_cost = FLT_MAX;

      find_best_split_plane(root, centroids, split_axis, split_position, split_cost);

      const float root_unsplit_cost = calculate_node_cost(root);

      if (split_cost >= root_unsplit_cost) return std::nullopt;

      const int32 right_child_start =
         partition_split(root, split_axis, split_position, centroids);
      const int32 left_child_count = right_child_start - root.first_tri;

      if (left_child_count == 0 or left_child_count == root.tri_count) {
         return std::nullopt;
      }

      leaf_node left_child = {.first_tri = root.first_tri, .tri_count = left_child_count};
      leaf_node right_child = {.first_tri = right_child_start,
                               .tri_count = root.tri_count - left_child_count};

      update_node_bounds(left_child);
      update_node_bounds(right_child);

      split_axis = &float3::x;
      split_position = 0.0f;
      split_cost = FLT_MAX;

      find_best_split_plane(left_child, centroids, split_axis, split_position,
                            split_cost);

      const float left_child_unsplit_cost = calculate_node_cost(left_child);

      if (split_cost >= left_child_unsplit_cost) return std::nullopt;

      const int32 child_1_start =
         partition_split(left_child, split_axis, split_position, centroids);
      const int32 child_0_count = child_1_start - left_child.first_tri;

      if (child_0_count == 0 or child_0_count == left_child.tri_count) {
         return std::nullopt;
      }

      leaf_node child_0 = {.first_tri = left_child.first_tri, .tri_count = child_0_count};
      leaf_node child_1 = {.first_tri = child_1_start,
                           .tri_count = left_child.tri_count - child_0_count};

      update_node_bounds(child_0);
      update_node_bounds(child_1);

      split_axis = &float3::x;
      split_position = 0.0f;
      split_cost = FLT_MAX;

      find_best_split_plane(right_child, centroids, split_axis, split_position,
                            split_cost);

      const float right_child_unsplit_cost = calculate_node_cost(right_child);

      if (split_cost >= right_child_unsplit_cost) return std::nullopt;

      const int32 child_3_start =
         partition_split(right_child, split_axis, split_position, centroids);
      const int32 child_2_count = child_3_start - right_child.first_tri;

      if (child_2_count == 0 or child_2_count == right_child.tri_count) {
         return std::nullopt;
      }

      leaf_node child_2 = {.first_tri = right_child.first_tri,
                           .tri_count = child_2_count};
      leaf_node child_3 = {.first_tri = child_3_start,
                           .tri_count = right_child.tri_count - child_2_count};

      update_node_bounds(child_2);
      update_node_bounds(child_3);

      return node_packed_x4{
         .bbox = {.min_x = {child_0.bbox.min.x, child_1.bbox.min.x,
                            child_2.bbox.min.x, child_3.bbox.min.x},
                  .min_y = {child_0.bbox.min.y, child_1.bbox.min.y,
                            child_2.bbox.min.y, child_3.bbox.min.y},
                  .min_z = {child_0.bbox.min.z, child_1.bbox.min.z,
                            child_2.bbox.min.z, child_3.bbox.min.z},

                  .max_x = {child_0.bbox.max.x, child_1.bbox.max.x,
                            child_2.bbox.max.x, child_3.bbox.max.x},
                  .max_y = {child_0.bbox.max.y, child_1.bbox.max.y,
                            child_2.bbox.max.y, child_3.bbox.max.y},
                  .max_z = {child_0.bbox.max.z, child_1.bbox.max.z,
                            child_2.bbox.max.z, child_3.bbox.max.z}},

         .children_or_first_tri = {child_0.first_tri, child_1.first_tri,
                                   child_2.first_tri, child_3.first_tri},
         .tri_count = {child_0.tri_count, child_1.tri_count, child_2.tri_count,
                       child_3.tri_count},
      };
   }

   void find_best_split_plane(const leaf_node& node, std::span<const float3> centroids,
                              float3_axis& best_split_axis, float& best_split_position,
                              float& best_split_cost) const noexcept
   {
      for (float3_axis axis : {&float3::x, &float3::y, &float3::z}) {
         float bounds_min = FLT_MAX;
         float bounds_max = -FLT_MAX;

         const int32 last_tri = node.first_tri + node.tri_count;

         for (int32 i = node.first_tri; i < last_tri; ++i) {
            const float3& centroid = centroids[_triangles[i]];

            bounds_min = std::min(bounds_min, centroid.*axis);
            bounds_max = std::max(bounds_max, centroid.*axis);
         }

         if (bounds_min == bounds_max) continue;

         struct bin {
            math::bounding_box bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                       .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
            float tri_count = 0.0f;
         };

         std::array<bin, split_bin_count> bins = {};

         const float bins_scale =
            static_cast<float>(split_bin_count) / (bounds_max - bounds_min);

         for (int32 i = node.first_tri; i < last_tri; ++i) {
            const uint32 tri_index = _triangles[i];
            const float3& centroid = centroids[tri_index];
            const std::array<uint16, 3>& tri = _indices[tri_index];

            const int32 bin_index =
               std::min(int32{split_bin_count - 1},
                        static_cast<int32>((centroid.*axis - bounds_min) * bins_scale));

            const float3& v0 = _positions[tri[0]];
            const float3& v1 = _positions[tri[1]];
            const float3& v2 = _positions[tri[2]];

            bin& bin = bins[bin_index];

            bin.bbox.min = min(bin.bbox.min, min(v0, min(v1, v2)));
            bin.bbox.max = max(bin.bbox.max, max(v0, max(v1, v2)));
            bin.tri_count += 1.0f;
         }

         math::bounding_box left_bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                         .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
         math::bounding_box right_bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                          .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
         float left_sum = 0.0f;
         float right_sum = 0.0f;

         std::array<float, split_bin_count - 1> left_area = {};
         std::array<float, split_bin_count - 1> left_count = {};

         std::array<float, split_bin_count - 1> right_area = {};
         std::array<float, split_bin_count - 1> right_count = {};

         for (int32 i = 0; i < split_bin_count - 1; ++i) {
            left_sum += bins[i].tri_count;
            left_count[i] = left_sum;
            left_bbox = {.min = min(bins[i].bbox.min, left_bbox.min),
                         .max = max(bins[i].bbox.max, left_bbox.max)};
            left_area[i] = area(left_bbox);

            right_sum += bins[split_bin_count - 1 - i].tri_count;
            right_count[split_bin_count - 2 - i] = right_sum;
            right_bbox = {.min = min(bins[split_bin_count - 1 - i].bbox.min,
                                     right_bbox.min),
                          .max = max(bins[split_bin_count - 1 - i].bbox.max,
                                     right_bbox.max)};
            right_area[split_bin_count - 2 - i] = area(right_bbox);
         }

         const float scale =
            (bounds_max - bounds_min) / static_cast<float>(split_bin_count);

         for (int32 i = 0; i < split_bin_count - 1; ++i) {
            const float plane_cost =
               left_count[i] * left_area[i] + right_count[i] * right_area[i];

            if (plane_cost < best_split_cost) {
               best_split_axis = axis;
               best_split_position =
                  bounds_min + scale * static_cast<float>((i + 1));
               best_split_cost = plane_cost;
            }
         }
      }
   }

   auto calculate_node_cost(const leaf_node& node) const noexcept -> float
   {
      const float3 node_extents = node.bbox.max - node.bbox.min;
      const float node_area = node_extents.x * node_extents.y +
                              node_extents.y * node_extents.z +
                              node_extents.z * node_extents.x;

      return static_cast<float>(node.tri_count) * node_area;
   }

   auto partition_split(const leaf_node& node, float3_axis split_axis,
                        float split_position,
                        std::span<const float3> centroids) noexcept -> int
   {
      int32 i = node.first_tri;
      int32 j = i + node.tri_count - 1;

      while (i <= j) {
         if (centroids[_triangles[i]].*split_axis < split_position) {
            i += 1;
         }
         else {
            std::swap(_triangles[i], _triangles[j]);

            j -= 1;
         }
      }

      return i;
   }
};

bvh::bvh() noexcept {}

bvh::bvh(std::span<const std::array<uint16, 3>> indices,
         std::span<const float3> positions, bvh_flags flags) noexcept
   : _impl{std::make_unique<detail::bvh_impl>(indices, positions, flags)}
{
}

bvh::bvh(bvh&&) noexcept = default;

auto bvh::operator=(bvh&&) -> bvh& = default;

bvh::~bvh() = default;

auto bvh::raycast(const float3& ray_origin, const float3& ray_direction,
                  const float max_distance, const bvh_ray_flags flags) const noexcept
   -> std::optional<ray_hit>
{
   return _impl ? _impl->raycast(ray_origin, ray_direction, max_distance, flags)
                : std::nullopt;
}

bool bvh::intersects(const frustum& frustum) const noexcept
{
   return _impl ? _impl->intersects(frustum) : false;
}

auto bvh::get_debug_boxes() const noexcept -> std::vector<math::bounding_box>
{
   return _impl ? _impl->get_debug_boxes() : std::vector<math::bounding_box>{};
}

struct detail::top_level_bvh_impl {
   using instance = top_level_bvh::instance;

   explicit top_level_bvh_impl(std::span<const instance> instances) noexcept
      : _instances{instances}
   {
      assert(check_instances(instances));

      std::vector<float3> centroids;
      centroids.reserve(instances.size());

      for (const instance& instance : instances) {
         math::bounding_box bbox = instance.bvh->_impl->get_bbox();

         bbox = instance.rotation * bbox + instance.position;

         centroids.emplace_back((bbox.min + bbox.max) * 0.5f);
      }

      _index.reserve(instances.size());

      for (uint32 i = 0; i < instances.size(); ++i) {
         _index.emplace_back(i);
      }

      _nodes.reserve(std::max(std::ssize(instances) * 2 / 4 - 1, 1ll));

      leaf_node root_node = {.instance_count = static_cast<int32>(instances.size())};

      update_node_bounds(root_node);

      if (std::optional<node_packed_x4> subdivided_root = subdivide(root_node, centroids);
          subdivided_root) {
         _nodes.push_back(*subdivided_root);

         int32 nodes_used = 1;

         for (int32 packed_index = 0; packed_index < nodes_used; ++packed_index) {
            for (int32 i = 0; i < 4; ++i) {
               node_packed_x4& packed = _nodes[packed_index];

               leaf_node child_node = {
                  .bbox = {.min = {packed.bbox.min_x[i], packed.bbox.min_y[i],
                                   packed.bbox.min_z[i]},

                           .max = {packed.bbox.max_x[i], packed.bbox.max_y[i],
                                   packed.bbox.max_z[i]}},

                  .first_instance = packed.children_or_first_instance[i],
                  .instance_count = packed.instance_count[i],
               };

               update_node_bounds(child_node);

               if (std::optional<node_packed_x4> subdivided =
                      subdivide(child_node, centroids);
                   subdivided) {
                  const int32 child_index = nodes_used;

                  _nodes.push_back(*subdivided);

                  packed.children_or_first_instance[i] = child_index;
                  packed.instance_count[i] = 0;

                  nodes_used += 1;
               }
            }
         }
      }
      else {
         _nodes.push_back(
            {.bbox =
                {
                   .min_x = {root_node.bbox.min.x},
                   .min_y = {root_node.bbox.min.y},
                   .min_z = {root_node.bbox.min.z},
                   .max_x = {root_node.bbox.max.x},
                   .max_y = {root_node.bbox.max.y},
                   .max_z = {root_node.bbox.max.z},
                },
             .instance_count = {static_cast<int32>(instances.size()), 1, 1, 1}});
      }

      _nodes.shrink_to_fit();
   }

   top_level_bvh_impl(const top_level_bvh_impl&) = delete;
   auto operator=(const top_level_bvh_impl&) -> top_level_bvh_impl& = delete;

   top_level_bvh_impl(top_level_bvh_impl&&) noexcept = delete;
   auto operator=(top_level_bvh_impl&&) -> top_level_bvh_impl& = delete;

   [[nodiscard]] auto raycast(const float3& ray_originWS,
                              const float3& ray_directionWS, const float max_distance,
                              const bvh_ray_flags flags) const noexcept
      -> std::optional<float>
   {
      const float3 inv_ray_directionWS = 1.0f / ray_directionWS;

      const __m128 ray_originWS_x = _mm_broadcast_ss(&ray_originWS.x);
      const __m128 ray_originWS_y = _mm_broadcast_ss(&ray_originWS.y);
      const __m128 ray_originWS_z = _mm_broadcast_ss(&ray_originWS.z);

      const __m128 inv_ray_directionWS_x = _mm_broadcast_ss(&inv_ray_directionWS.x);
      const __m128 inv_ray_directionWS_y = _mm_broadcast_ss(&inv_ray_directionWS.y);
      const __m128 inv_ray_directionWS_z = _mm_broadcast_ss(&inv_ray_directionWS.z);

      std::array<int32, 64> stack = {
         _root_node_index,
      };
      int32 stack_ptr = 0;

      __m128 closest_hit = _mm_broadcast_ss(&max_distance);

      while (stack_ptr >= 0) {
         const node_packed_x4& node = _nodes[stack[stack_ptr]];

         stack_ptr -= 1;

         __m128 hit_distance;

         const int hit_mask =
            intersect_aabb(ray_originWS_x, ray_originWS_y, ray_originWS_z,
                           inv_ray_directionWS_x, inv_ray_directionWS_y,
                           inv_ray_directionWS_z, _mm_load_ps(node.bbox.min_x.data()),
                           _mm_load_ps(node.bbox.min_y.data()),
                           _mm_load_ps(node.bbox.min_z.data()),
                           _mm_load_ps(node.bbox.max_x.data()),
                           _mm_load_ps(node.bbox.max_y.data()),
                           _mm_load_ps(node.bbox.max_z.data()), closest_hit,
                           hit_distance);

         if (hit_mask) {
            alignas(__m128) std::array<float, 4> hits;

            _mm_store_ps(hits.data(), hit_distance);

            // TODO: Sort by hit distance.

            for (int lane_index = 0;
                 lane_index < std::ssize(node.instance_count); ++lane_index) {
               if (not(hit_mask & (1 << lane_index))) continue;

               const bool is_leaf = node.instance_count[lane_index] != 0;

               if (is_leaf) {
                  const int32 last_instance =
                     node.children_or_first_instance[lane_index] +
                     node.instance_count[lane_index];

                  for (int32 instance_index = node.children_or_first_instance[lane_index];
                       instance_index < last_instance; ++instance_index) {
                     const instance& instance = _instances[_index[instance_index]];

                     const float3 ray_originIS =
                        instance.inverse_rotation * ray_originWS +
                        instance.inverse_position;
                     const float3 ray_directionIS =
                        normalize(instance.inverse_rotation * ray_directionWS);

                     float closest_hit_scalar;

                     _mm_store_ss(&closest_hit_scalar, closest_hit);

                     if (std::optional<bvh::ray_hit> hit =
                            instance.bvh->raycast(ray_originIS, ray_directionIS,
                                                  closest_hit_scalar, flags);
                         hit) {
                        closest_hit = _mm_broadcast_ss(&hit->distance);

                        if (flags.accept_first_hit) {
                           return std::optional{hit->distance};
                        }
                     }
                  }
               }
               else {
                  stack_ptr += 1;

                  stack[stack_ptr] = node.children_or_first_instance[lane_index];
               }
            }
         }
      }

      float closest_hit_scalar;

      _mm_store_ss(&closest_hit_scalar, closest_hit);

      return closest_hit_scalar < max_distance ? std::optional{closest_hit_scalar}
                                               : std::nullopt;
   }

   [[nodiscard]] auto get_debug_boxes() const noexcept -> std::vector<math::bounding_box>
   {
      std::vector<math::bounding_box> boxes;
      boxes.reserve(_nodes.size() * 4);

      for (const node_packed_x4& node : _nodes) {
         const bounding_box_x4& bbox = node.bbox;

         for (int32 i = 0; i < 4; ++i) {
            boxes.push_back({
               .min = {bbox.min_x[i], bbox.min_y[i], bbox.min_z[i]},
               .max = {bbox.max_x[i], bbox.max_y[i], bbox.max_z[i]},
            });
         }
      }

      return boxes;
   }

private:
   struct leaf_node {
      math::bounding_box bbox;
      int32 first_instance = 0;
      int32 instance_count = 0;
   };

   struct bounding_box_x4 {
      alignas(__m128) std::array<float, 4> min_x;
      alignas(__m128) std::array<float, 4> min_y;
      alignas(__m128) std::array<float, 4> min_z;
      alignas(__m128) std::array<float, 4> max_x;
      alignas(__m128) std::array<float, 4> max_y;
      alignas(__m128) std::array<float, 4> max_z;
   };

   struct node_packed_x4 {
      bounding_box_x4 bbox;
      std::array<int32, 4> children_or_first_instance = {};
      std::array<int32, 4> instance_count = {};
   };

   std::vector<node_packed_x4> _nodes;
   std::vector<uint32> _index;
   constexpr static int32 _root_node_index = 0;

   std::span<const instance> _instances;

   using float3_axis = float float3::*;

   void update_node_bounds(leaf_node& node) noexcept
   {
      node.bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                   .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};

      const int32 last_instance = node.first_instance + node.instance_count;

      for (int32 i = node.first_instance; i < last_instance; ++i) {
         const instance& instance = _instances[_index[i]];

         math::bounding_box bbox = instance.bvh->_impl->get_bbox();

         bbox = instance.rotation * bbox + instance.position;

         node.bbox = combine(node.bbox, bbox);
      }
   }

   auto subdivide(const leaf_node& root, std::span<const float3> centroids) noexcept
      -> std::optional<node_packed_x4>
   {
      float3_axis split_axis = &float3::x;
      float split_position = 0.0f;
      float split_cost = FLT_MAX;

      find_best_split_plane(root, centroids, split_axis, split_position, split_cost);

      const float root_unsplit_cost = calculate_node_cost(root);

      if (split_cost >= root_unsplit_cost) return std::nullopt;

      const int32 right_child_start =
         partition_split(root, split_axis, split_position, centroids);
      const int32 left_child_count = right_child_start - root.first_instance;

      if (left_child_count == 0 or left_child_count == root.instance_count) {
         return std::nullopt;
      }

      leaf_node left_child = {.first_instance = root.first_instance,
                              .instance_count = left_child_count};
      leaf_node right_child = {.first_instance = right_child_start,
                               .instance_count = root.instance_count - left_child_count};

      update_node_bounds(left_child);
      update_node_bounds(right_child);

      split_axis = &float3::x;
      split_position = 0.0f;
      split_cost = FLT_MAX;

      find_best_split_plane(left_child, centroids, split_axis, split_position,
                            split_cost);

      const float left_child_unsplit_cost = calculate_node_cost(left_child);

      if (split_cost >= left_child_unsplit_cost) return std::nullopt;

      const int32 child_1_start =
         partition_split(left_child, split_axis, split_position, centroids);
      const int32 child_0_count = child_1_start - left_child.first_instance;

      if (child_0_count == 0 or child_0_count == left_child.instance_count) {
         return std::nullopt;
      }

      leaf_node child_0 = {.first_instance = left_child.first_instance,
                           .instance_count = child_0_count};
      leaf_node child_1 = {.first_instance = child_1_start,
                           .instance_count = left_child.instance_count - child_0_count};

      update_node_bounds(child_0);
      update_node_bounds(child_1);

      split_axis = &float3::x;
      split_position = 0.0f;
      split_cost = FLT_MAX;

      find_best_split_plane(right_child, centroids, split_axis, split_position,
                            split_cost);

      const float right_child_unsplit_cost = calculate_node_cost(right_child);

      if (split_cost >= right_child_unsplit_cost) return std::nullopt;

      const int32 child_3_start =
         partition_split(right_child, split_axis, split_position, centroids);
      const int32 child_2_count = child_3_start - right_child.first_instance;

      if (child_2_count == 0 or child_2_count == right_child.instance_count) {
         return std::nullopt;
      }

      leaf_node child_2 = {.first_instance = right_child.first_instance,
                           .instance_count = child_2_count};
      leaf_node child_3 = {.first_instance = child_3_start,
                           .instance_count = right_child.instance_count - child_2_count};

      update_node_bounds(child_2);
      update_node_bounds(child_3);

      return node_packed_x4{
         .bbox = {.min_x = {child_0.bbox.min.x, child_1.bbox.min.x,
                            child_2.bbox.min.x, child_3.bbox.min.x},
                  .min_y = {child_0.bbox.min.y, child_1.bbox.min.y,
                            child_2.bbox.min.y, child_3.bbox.min.y},
                  .min_z = {child_0.bbox.min.z, child_1.bbox.min.z,
                            child_2.bbox.min.z, child_3.bbox.min.z},

                  .max_x = {child_0.bbox.max.x, child_1.bbox.max.x,
                            child_2.bbox.max.x, child_3.bbox.max.x},
                  .max_y = {child_0.bbox.max.y, child_1.bbox.max.y,
                            child_2.bbox.max.y, child_3.bbox.max.y},
                  .max_z = {child_0.bbox.max.z, child_1.bbox.max.z,
                            child_2.bbox.max.z, child_3.bbox.max.z}},

         .children_or_first_instance = {child_0.first_instance, child_1.first_instance,
                                        child_2.first_instance, child_3.first_instance},
         .instance_count = {child_0.instance_count, child_1.instance_count,
                            child_2.instance_count, child_3.instance_count},
      };
   }

   void find_best_split_plane(const leaf_node& node, std::span<const float3> centroids,
                              float3_axis& best_split_axis, float& best_split_position,
                              float& best_split_cost) const noexcept
   {
      for (float3_axis axis : {&float3::x, &float3::y, &float3::z}) {
         float bounds_min = FLT_MAX;
         float bounds_max = -FLT_MAX;

         const int32 last_tri = node.first_instance + node.instance_count;

         for (int32 i = node.first_instance; i < last_tri; ++i) {
            const float3& centroid = centroids[_index[i]];

            bounds_min = std::min(bounds_min, centroid.*axis);
            bounds_max = std::max(bounds_max, centroid.*axis);
         }

         if (bounds_min == bounds_max) continue;

         struct bin {
            math::bounding_box bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                       .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
            float tri_count = 0.0f;
         };

         std::array<bin, split_bin_count> bins = {};

         const float bins_scale =
            static_cast<float>(split_bin_count) / (bounds_max - bounds_min);

         for (int32 i = node.first_instance; i < last_tri; ++i) {
            const uint32 instance_index = _index[i];
            const float3& centroid = centroids[instance_index];
            const instance& instance = _instances[_index[i]];

            math::bounding_box bbox = instance.bvh->_impl->get_bbox();

            bbox = instance.rotation * bbox + instance.position;

            const int32 bin_index =
               std::min(int32{split_bin_count - 1},
                        static_cast<int32>((centroid.*axis - bounds_min) * bins_scale));

            bin& bin = bins[bin_index];

            bin.bbox = combine(bin.bbox, bbox);
            bin.tri_count +=
               static_cast<float>(instance.bvh->_impl->get_tri_count());
         }

         math::bounding_box left_bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                         .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
         math::bounding_box right_bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                          .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
         float left_sum = 0.0f;
         float right_sum = 0.0f;

         std::array<float, split_bin_count - 1> left_area = {};
         std::array<float, split_bin_count - 1> left_count = {};

         std::array<float, split_bin_count - 1> right_area = {};
         std::array<float, split_bin_count - 1> right_count = {};

         for (int32 i = 0; i < split_bin_count - 1; ++i) {
            left_sum += bins[i].tri_count;
            left_count[i] = left_sum;
            left_bbox = {.min = min(bins[i].bbox.min, left_bbox.min),
                         .max = max(bins[i].bbox.max, left_bbox.max)};
            left_area[i] = area(left_bbox);

            right_sum += bins[split_bin_count - 1 - i].tri_count;
            right_count[split_bin_count - 2 - i] = right_sum;
            right_bbox = {.min = min(bins[split_bin_count - 1 - i].bbox.min,
                                     right_bbox.min),
                          .max = max(bins[split_bin_count - 1 - i].bbox.max,
                                     right_bbox.max)};
            right_area[split_bin_count - 2 - i] = area(right_bbox);
         }

         const float scale =
            (bounds_max - bounds_min) / static_cast<float>(split_bin_count);

         for (int32 i = 0; i < split_bin_count - 1; ++i) {
            const float plane_cost =
               left_count[i] * left_area[i] + right_count[i] * right_area[i];

            if (plane_cost < best_split_cost) {
               best_split_axis = axis;
               best_split_position =
                  bounds_min + scale * static_cast<float>((i + 1));
               best_split_cost = plane_cost;
            }
         }
      }
   }

   auto calculate_node_cost(const leaf_node& node) const noexcept -> float
   {
      const float3 node_extents = node.bbox.max - node.bbox.min;
      const float node_area = node_extents.x * node_extents.y +
                              node_extents.y * node_extents.z +
                              node_extents.z * node_extents.x;

      std::size_t tri_count = 0;

      const int32 last_instance = node.first_instance + node.instance_count;

      for (int32 i = node.first_instance; i < last_instance; ++i) {
         tri_count += _instances[_index[i]].bvh->_impl->get_tri_count();
      }

      return static_cast<float>(tri_count) * node_area;
   }

   auto partition_split(const leaf_node& node, float3_axis split_axis,
                        float split_position,
                        std::span<const float3> centroids) noexcept -> int
   {
      int32 i = node.first_instance;
      int32 j = i + node.instance_count - 1;

      while (i <= j) {
         if (centroids[_index[i]].*split_axis < split_position) {
            i += 1;
         }
         else {
            std::swap(_index[i], _index[j]);

            j -= 1;
         }
      }

      return i;
   }
};

top_level_bvh::top_level_bvh() noexcept {}

top_level_bvh::top_level_bvh(std::span<const instance> instances) noexcept
   : _impl{std::make_unique<detail::top_level_bvh_impl>(instances)}
{
}

top_level_bvh::top_level_bvh(top_level_bvh&&) noexcept = default;

auto top_level_bvh::operator=(top_level_bvh&&) -> top_level_bvh& = default;

top_level_bvh::~top_level_bvh() = default;

auto top_level_bvh::raycast(const float3& ray_origin,
                            const float3& ray_direction, const float max_distance,
                            const bvh_ray_flags flags) const noexcept
   -> std::optional<float>
{
   return _impl ? _impl->raycast(ray_origin, ray_direction, max_distance, flags)
                : std::nullopt;
}

auto top_level_bvh::get_debug_boxes() const noexcept -> std::vector<math::bounding_box>
{
   return _impl ? _impl->get_debug_boxes() : std::vector<math::bounding_box>{};
}

}
