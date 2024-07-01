#include "bvh.hpp"
#include "intersectors.hpp"
#include "math/bounding_box.hpp"
#include "math/frustum.hpp"
#include "vector_funcs.hpp"

#include <cassert>
#include <vector>

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

auto area(const math::bounding_box& bbox) noexcept -> float
{
   const float3 extents = bbox.max - bbox.min;

   return extents.x * extents.y + extents.y * extents.z + extents.z * extents.x;
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

      _nodes.resize(indices.size() * 2 - 1);

      _nodes[_root_node_index] = {.tri_count = static_cast<int32>(indices.size())};

#ifdef BVH_RECURSIVE_BUILD
      update_node_bounds(i);
      subdivide(i, centroids);
#else
      for (int32 i = 0; i < _nodes_used; ++i) {
         update_node_bounds(i);
         subdivide(i, centroids); // Note that subdivide increases _nodes_used.
      }
#endif

      if (_nodes_used > _nodes.size()) std::terminate();

      _nodes.resize(_nodes_used);
      _nodes.shrink_to_fit();
   }

   bvh_impl(const bvh_impl&) = delete;
   auto operator=(const bvh_impl&) -> bvh_impl& = delete;

   bvh_impl(bvh_impl&&) noexcept = delete;
   auto operator=(bvh_impl&&) -> bvh_impl& = delete;

   [[nodiscard]] auto raycast(const float3& ray_origin, const float3& ray_direction,
                              const float max_distance) const noexcept
      -> std::optional<bvh::ray_hit>
   {
      const float3 inv_ray_direction = 1.0f / ray_direction;

      std::array<int32, 64> stack = {
         _root_node_index,
      };
      int32 stack_ptr = 0;

      float closest_hit = max_distance;
      float3 hit_normal = {};

      while (stack_ptr >= 0) {
         const node* node = &_nodes[stack[stack_ptr]];

         stack_ptr -= 1;

         if (node->is_leaf()) {
            const int32 last_tri = node->left_child_or_first_tri + node->tri_count;

            for (int32 i = node->left_child_or_first_tri; i < last_tri; ++i) {
               const std::array<uint16, 3>& tri = _indices[_triangles[i]];

               const float3& v0 = _positions[tri[0]];
               const float3& v1 = _positions[tri[1]];
               const float3& v2 = _positions[tri[2]];

               if (float hit = 0.0f;
                   intersect_tri(ray_origin, ray_direction, v0, v1, v2, hit) and
                   hit < closest_hit) {

                  const float3 normal = cross(v1 - v0, v2 - v0);

                  if (dot(-ray_direction, normal) < 0.0f and not _no_backface_cull) {
                     continue;
                  }

                  closest_hit = hit;
                  hit_normal = normal;
               }
            }
         }
         else {
            int32 child_index0 = node->left_child_or_first_tri;
            int32 child_index1 = node->left_child_or_first_tri + 1;

            float child_distance0 = 0.0f;
            float child_distance1 = 0.0f;

            const bool hit0 = intersect_aabb(ray_origin, inv_ray_direction,
                                             _nodes[child_index0].bbox,
                                             closest_hit, child_distance0);
            const bool hit1 = intersect_aabb(ray_origin, inv_ray_direction,
                                             _nodes[child_index1].bbox,
                                             closest_hit, child_distance1);

            if (hit0 and hit1) {
               if (child_distance0 > child_distance1) {
                  std::swap(child_index0, child_index1);
               }

               stack_ptr += 1;

               stack[stack_ptr] = child_index0;

               stack_ptr += 1;

               stack[stack_ptr] = child_index1;
            }
            else if (hit0) {
               stack_ptr += 1;

               stack[stack_ptr] = child_index0;
            }
            else if (hit1) {
               stack_ptr += 1;

               stack[stack_ptr] = child_index1;
            }
         }
      }

      return closest_hit < max_distance
                ? std::optional{bvh::ray_hit{.distance = closest_hit, .normal = hit_normal}}
                : std::nullopt;
   }

   [[nodiscard]] bool intersects(const frustum& frustum) const noexcept
   {
      std::array<int32, 64> stack = {
         _root_node_index,
      };
      int32 stack_ptr = 0;

      while (stack_ptr >= 0) {
         const node* node = &_nodes[stack[stack_ptr]];

         stack_ptr -= 1;

         if (node->is_leaf()) {
            const int32 last_tri = node->left_child_or_first_tri + node->tri_count;

            for (int32 i = node->left_child_or_first_tri; i < last_tri; ++i) {
               const std::array<uint16, 3>& tri_indices = _indices[_triangles[i]];

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
                  const float3 edge_vector =
                     frustum.corners[i1] - frustum.corners[i0];
                  const float edge_length = length(edge_vector);
                  const float3 edge_direction = edge_vector / edge_length;

                  if (float distance = FLT_MAX;
                      intersect_tri(frustum.corners[i0], edge_direction, v0, v1,
                                    v2, distance) and
                      distance <= edge_length) {
                     return true;
                  }
               }
            }
         }
         else {
            const int32 child_index0 = node->left_child_or_first_tri;
            const int32 child_index1 = node->left_child_or_first_tri + 1;

            if (we::intersects(frustum, _nodes[child_index0].bbox)) {
               stack_ptr += 1;

               stack[stack_ptr] = child_index0;
            }

            if (we::intersects(frustum, _nodes[child_index1].bbox)) {
               stack_ptr += 1;

               stack[stack_ptr] = child_index1;
            }
         }
      }

      return false;
   }

   [[nodiscard]] auto get_debug_boxes() const noexcept -> std::vector<math::bounding_box>
   {
      std::vector<math::bounding_box> boxes;
      boxes.reserve(_nodes_used);

      for (int32 i = 0; i < _nodes_used; ++i) {
         boxes.emplace_back(_nodes[i].bbox);
      }

      return boxes;
   }

private:
   struct node {
      math::bounding_box bbox;
      int32 left_child_or_first_tri = 0;
      int32 tri_count = 0;

      bool is_leaf() const noexcept
      {
         return tri_count != 0;
      }
   };

   std::vector<node> _nodes;
   std::vector<uint32> _triangles;
   int32 _root_node_index = 0;
   int32 _nodes_used = 1;

   std::span<const std::array<uint16, 3>> _indices;
   std::span<const float3> _positions;

   bool _no_backface_cull = false;

   using float3_axis = float float3::*;

   void update_node_bounds(const int32 node_index) noexcept
   {
      node& node = _nodes[node_index];

      node.bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                   .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};

      const int32 last_tri = node.left_child_or_first_tri + node.tri_count;

      for (int32 i = node.left_child_or_first_tri; i < last_tri; ++i) {
         const std::array<uint16, 3>& tri = _indices[_triangles[i]];

         const float3& v0 = _positions[tri[0]];
         const float3& v1 = _positions[tri[1]];
         const float3& v2 = _positions[tri[2]];

         node.bbox.min = min(node.bbox.min, min(v0, min(v1, v2)));
         node.bbox.max = max(node.bbox.max, max(v0, max(v1, v2)));
      }
   }

   void subdivide(const int32 node_index, std::span<const float3> centroids) noexcept
   {
      node& node = _nodes[node_index];

      float3_axis split_axis = &float3::x;
      float split_position = 0.0f;
      float split_cost = FLT_MAX;

      find_best_split_plane(node, centroids, split_axis, split_position, split_cost);

      const float unsplit_cost = calculate_node_cost(node);

      if (split_cost >= unsplit_cost) return;

      int32 i = node.left_child_or_first_tri;
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

      const int32 left_count = i - node.left_child_or_first_tri;

      if (left_count == 0 or left_count == node.tri_count) return;

      const int32 left_child_index = _nodes_used;
      const int32 right_child_index = _nodes_used + 1;

      _nodes_used += 2;

      _nodes[left_child_index] = {.left_child_or_first_tri = node.left_child_or_first_tri,
                                  .tri_count = left_count};
      _nodes[right_child_index] = {.left_child_or_first_tri = i,
                                   .tri_count = node.tri_count - left_count};

      node.left_child_or_first_tri = left_child_index;
      node.tri_count = 0;

#ifdef BVH_RECURSIVE_BUILD
      update_node_bounds(left_child_index);
      update_node_bounds(right_child_index);

      subdivide(left_child_index, centroids);
      subdivide(right_child_index, centroids);
#endif
   }

   void find_best_split_plane(const node& node, std::span<const float3> centroids,
                              float3_axis& best_split_axis, float& best_split_position,
                              float& best_split_cost) const noexcept
   {
      for (float3_axis axis : {&float3::x, &float3::y, &float3::z}) {
         float bounds_min = FLT_MAX;
         float bounds_max = -FLT_MAX;

         const int32 last_tri = node.left_child_or_first_tri + node.tri_count;

         for (int32 i = node.left_child_or_first_tri; i < last_tri; ++i) {
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

         for (int32 i = node.left_child_or_first_tri; i < last_tri; ++i) {
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

   auto calculate_node_cost(const node& node) const noexcept -> float
   {
      const float3 node_extents = node.bbox.max - node.bbox.min;
      const float node_area = node_extents.x * node_extents.y +
                              node_extents.y * node_extents.z +
                              node_extents.z * node_extents.x;

      return static_cast<float>(node.tri_count) * node_area;
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
                  const float max_distance) const noexcept -> std::optional<ray_hit>
{
   return _impl ? _impl->raycast(ray_origin, ray_direction, max_distance) : std::nullopt;
}

bool bvh::intersects(const frustum& frustum) const noexcept
{
   return _impl ? _impl->intersects(frustum) : false;
}

auto bvh::get_debug_boxes() const noexcept -> std::vector<math::bounding_box>
{
   return _impl ? _impl->get_debug_boxes() : std::vector<math::bounding_box>{};
}
}
