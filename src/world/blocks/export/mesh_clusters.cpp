#include "mesh_clusters.hpp"

#include "math/bounding_box.hpp"
#include "math/vector_funcs.hpp"

#include <optional>

namespace we::world {

namespace {

struct triangle_clusterizer {
   triangle_clusterizer(std::span<const block_world_triangle> triangles,
                        int32 min_triangles, int32 max_subdivisions) noexcept
      : _triangle_data{triangles}
   {
      std::vector<float3> centroids;
      centroids.reserve(triangles.size());

      for (const block_world_triangle& tri : triangles) {
         centroids.emplace_back((tri.vertices[0].positionWS + tri.vertices[1].positionWS +
                                 tri.vertices[2].positionWS) *
                                (1.0f / 3.0f));
      }

      _triangles.reserve(triangles.size());

      for (uint32 i = 0; i < triangles.size(); ++i) {
         _triangles.emplace_back(i);
      }

      _nodes.reserve((1ull << max_subdivisions) + 1);

      node root_node = {.tri_count = static_cast<int32>(triangles.size())};

      update_node_bounds(root_node);

      if (std::optional<std::array<node, 2>> subdivided_root =
             subdivide(root_node, centroids);
          subdivided_root) {
         _nodes.push_back((*subdivided_root)[0]);
         _nodes.push_back((*subdivided_root)[1]);

         int32 nodes_used = 1;

         for (int32 index = 0; index < nodes_used; ++index) {
            node& child_node = _nodes[index];

            update_node_bounds(child_node);

            if (child_node.generation >= max_subdivisions) continue;
            if (child_node.tri_count <= min_triangles) continue;

            if (std::optional<std::array<node, 2>> subdivided =
                   subdivide(child_node, centroids);
                subdivided) {
               const int32 child_index = nodes_used;

               _nodes.push_back((*subdivided)[0]);
               _nodes.push_back((*subdivided)[1]);

               child_node.child_or_first_tri = child_index;
               child_node.tri_count = 0;

               nodes_used += 1;
            }
         }
      }
      else {
         _nodes.push_back(root_node);
      }

      _nodes.shrink_to_fit();
   }

   triangle_clusterizer(const triangle_clusterizer&) = delete;
   auto operator=(const triangle_clusterizer&) -> triangle_clusterizer& = delete;

   triangle_clusterizer(triangle_clusterizer&&) noexcept = delete;
   auto operator=(triangle_clusterizer&&) -> triangle_clusterizer& = delete;

   auto clusters() const noexcept -> std::vector<std::vector<uint32>>
   {
      std::vector<std::vector<uint32>> clusters;
      clusters.reserve(_nodes.capacity());

      for (const node& node : _nodes) {
         if (not node.is_leaf()) continue;

         std::vector<uint32>& cluster = clusters.emplace_back();
         cluster.reserve(node.tri_count);

         const int32 last_tri = node.child_or_first_tri + node.tri_count;

         for (int32 i = node.child_or_first_tri; i < last_tri; ++i) {
            const uint32 tri_index = _triangles[i];

            cluster.push_back(tri_index);
         }
      }

      return clusters;
   }

private:
   const static int split_bin_count = 8;

   struct node {
      math::bounding_box bbox;
      int32 child_or_first_tri = 0;
      int32 tri_count = 0;
      int32 generation = 0;

      bool is_leaf() const noexcept
      {
         return tri_count != 0;
      }
   };

   std::vector<node> _nodes;
   std::vector<uint32> _triangles;
   constexpr static int32 _root_node_index = 0;

   std::span<const block_world_triangle> _triangle_data;

   using float3_axis = float float3::*;

   void update_node_bounds(node& node) noexcept
   {
      node.bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                   .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};

      const int32 last_tri = node.child_or_first_tri + node.tri_count;

      for (int32 i = node.child_or_first_tri; i < last_tri; ++i) {
         const uint32 tri_index = _triangles[i];

         const float3& v0 = _triangle_data[tri_index].vertices[0].positionWS;
         const float3& v1 = _triangle_data[tri_index].vertices[1].positionWS;
         const float3& v2 = _triangle_data[tri_index].vertices[2].positionWS;

         node.bbox.min = min(node.bbox.min, min(v0, min(v1, v2)));
         node.bbox.max = max(node.bbox.max, max(v0, max(v1, v2)));
      }
   }

   auto subdivide(const node& root, std::span<const float3> centroids) noexcept
      -> std::optional<std::array<node, 2>>
   {
      float3_axis split_axis = &float3::x;
      float split_position = 0.0f;
      float split_cost = FLT_MAX;

      find_best_split_plane(root, centroids, split_axis, split_position, split_cost);

      const float root_unsplit_cost = calculate_node_cost(root);

      if (split_cost >= root_unsplit_cost) return std::nullopt;

      const int32 right_child_start =
         partition_split(root, split_axis, split_position, centroids);
      const int32 left_child_count = right_child_start - root.child_or_first_tri;

      if (left_child_count == 0 or left_child_count == root.tri_count) {
         return std::nullopt;
      }

      node left_child = {.child_or_first_tri = root.child_or_first_tri,
                         .tri_count = left_child_count,
                         .generation = root.generation + 1};
      node right_child = {.child_or_first_tri = right_child_start,
                          .tri_count = root.tri_count - left_child_count,
                          .generation = root.generation + 1};

      update_node_bounds(left_child);
      update_node_bounds(right_child);

      split_axis = &float3::x;
      split_position = 0.0f;
      split_cost = FLT_MAX;

      find_best_split_plane(left_child, centroids, split_axis, split_position,
                            split_cost);

      const float left_child_unsplit_cost = calculate_node_cost(left_child);

      if (split_cost >= left_child_unsplit_cost) return std::nullopt;

      return std::array<node, 2>{left_child, right_child};
   }

   void find_best_split_plane(const node& node, std::span<const float3> centroids,
                              float3_axis& best_split_axis, float& best_split_position,
                              float& best_split_cost) const noexcept
   {
      for (float3_axis axis : {&float3::x, &float3::y, &float3::z}) {
         float bounds_min = FLT_MAX;
         float bounds_max = -FLT_MAX;

         const int32 last_tri = node.child_or_first_tri + node.tri_count;

         for (int32 i = node.child_or_first_tri; i < last_tri; ++i) {
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

         for (int32 i = node.child_or_first_tri; i < last_tri; ++i) {
            const uint32 tri_index = _triangles[i];
            const float3& centroid = centroids[tri_index];

            const int32 bin_index =
               std::min(int32{split_bin_count - 1},
                        static_cast<int32>((centroid.*axis - bounds_min) * bins_scale));

            const float3& v0 = _triangle_data[tri_index].vertices[0].positionWS;
            const float3& v1 = _triangle_data[tri_index].vertices[1].positionWS;
            const float3& v2 = _triangle_data[tri_index].vertices[2].positionWS;

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

   auto partition_split(const node& node, float3_axis split_axis, float split_position,
                        std::span<const float3> centroids) noexcept -> int
   {
      int32 i = node.child_or_first_tri;
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

   auto area(const math::bounding_box& bbox) const noexcept -> float
   {
      const float3 extents = bbox.max - bbox.min;

      return extents.x * extents.y + extents.y * extents.z + extents.z * extents.x;
   }
};

}

auto build_mesh_clusters(std::span<const block_world_triangle> triangles,
                         int32 min_triangles, int32 max_subdivisions) noexcept
   -> std::vector<std::vector<uint32>>
{
   // This is suboptimal. It'd likely be much better to build clusters by picking
   // seed triangles and slow expanding the clusters with nearby triangles. But it was quicker to hack something together using
   // the BVH code we already had and it should be good enough for a start.
   return triangle_clusterizer{triangles, min_triangles, max_subdivisions}.clusters();
}

}
