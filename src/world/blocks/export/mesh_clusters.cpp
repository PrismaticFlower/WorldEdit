#include "mesh_clusters.hpp"

#include "math/bounding_box.hpp"
#include "math/vector_funcs.hpp"

#include <optional>

namespace we::world {

namespace {

struct mesh_clusterizer {
   mesh_clusterizer(std::span<const block_world_mesh> meshes,
                    int32 min_triangles, int32 max_subdivisions) noexcept
      : _mesh_data{meshes}
   {
      std::vector<float3> centroids;
      centroids.reserve(meshes.size());

      for (const block_world_mesh& mesh : meshes) {
         centroids.emplace_back((mesh.bboxWS.min + mesh.bboxWS.max) * 2.0f);
      }

      _meshes.reserve(meshes.size());

      for (uint32 i = 0; i < meshes.size(); ++i) {
         _meshes.emplace_back(i);
      }

      _nodes.reserve((1ull << max_subdivisions) + 1);

      node root_node = {.mesh_count = static_cast<int32>(meshes.size())};

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
            if (count_node_tris(child_node) <= min_triangles) continue;

            if (std::optional<std::array<node, 2>> subdivided =
                   subdivide(child_node, centroids);
                subdivided) {
               const int32 child_index = nodes_used;

               _nodes.push_back((*subdivided)[0]);
               _nodes.push_back((*subdivided)[1]);

               child_node.child_or_first_mesh = child_index;
               child_node.mesh_count = 0;

               nodes_used += 1;
            }
         }
      }
      else {
         _nodes.push_back(root_node);
      }

      _nodes.shrink_to_fit();
   }

   mesh_clusterizer(const mesh_clusterizer&) = delete;
   auto operator=(const mesh_clusterizer&) -> mesh_clusterizer& = delete;

   mesh_clusterizer(mesh_clusterizer&&) noexcept = delete;
   auto operator=(mesh_clusterizer&&) -> mesh_clusterizer& = delete;

   auto clusters() const noexcept -> std::vector<std::vector<uint32>>
   {
      std::vector<std::vector<uint32>> clusters;
      clusters.reserve(_nodes.capacity());

      for (const node& node : _nodes) {
         if (not node.is_leaf()) continue;

         std::vector<uint32>& cluster = clusters.emplace_back();
         cluster.reserve(node.mesh_count);

         const int32 last_mesh = node.child_or_first_mesh + node.mesh_count;

         for (int32 i = node.child_or_first_mesh; i < last_mesh; ++i) {
            const uint32 mesh_index = _meshes[i];

            cluster.push_back(mesh_index);
         }
      }

      return clusters;
   }

private:
   const static int split_bin_count = 8;

   struct node {
      math::bounding_box bbox;
      int32 child_or_first_mesh = 0;
      int32 mesh_count = 0;
      int32 generation = 0;

      bool is_leaf() const noexcept
      {
         return mesh_count != 0;
      }
   };

   std::vector<node> _nodes;
   std::vector<uint32> _meshes;
   constexpr static int32 _root_node_index = 0;

   std::span<const block_world_mesh> _mesh_data;

   using float3_axis = float float3::*;

   void update_node_bounds(node& node) noexcept
   {
      node.bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                   .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};

      const int32 last_mesh = node.child_or_first_mesh + node.mesh_count;

      for (int32 i = node.child_or_first_mesh; i < last_mesh; ++i) {
         const uint32 mesh_index = _meshes[i];

         node.bbox.min = min(node.bbox.min, _mesh_data[mesh_index].bboxWS.min);
         node.bbox.max = max(node.bbox.max, _mesh_data[mesh_index].bboxWS.max);
      }
   }

   auto subdivide(const node& root, std::span<const float3> centroids) noexcept
      -> std::optional<std::array<node, 2>>
   {
      float3_axis split_axis = &float3::x;
      float split_position = 0.0f;
      double split_cost = DBL_MAX;

      find_best_split_plane(root, centroids, split_axis, split_position, split_cost);

      const double root_unsplit_cost = calculate_node_cost(root);

      if (split_cost >= root_unsplit_cost) return std::nullopt;

      const int32 right_child_start =
         partition_split(root, split_axis, split_position, centroids);
      const int32 left_child_count = right_child_start - root.child_or_first_mesh;

      if (left_child_count == 0 or left_child_count == root.mesh_count) {
         return std::nullopt;
      }

      node left_child = {.child_or_first_mesh = root.child_or_first_mesh,
                         .mesh_count = left_child_count,
                         .generation = root.generation + 1};
      node right_child = {.child_or_first_mesh = right_child_start,
                          .mesh_count = root.mesh_count - left_child_count,
                          .generation = root.generation + 1};

      update_node_bounds(left_child);
      update_node_bounds(right_child);

      split_axis = &float3::x;
      split_position = 0.0f;
      split_cost = DBL_MAX;

      find_best_split_plane(left_child, centroids, split_axis, split_position,
                            split_cost);

      const double left_child_unsplit_cost = calculate_node_cost(left_child);

      if (split_cost >= left_child_unsplit_cost) return std::nullopt;

      return std::array<node, 2>{left_child, right_child};
   }

   void find_best_split_plane(const node& node, std::span<const float3> centroids,
                              float3_axis& best_split_axis, float& best_split_position,
                              double& best_split_cost) const noexcept
   {
      for (float3_axis axis : {&float3::x, &float3::y, &float3::z}) {
         float bounds_min = FLT_MAX;
         float bounds_max = -FLT_MAX;

         const int32 last_mesh = node.child_or_first_mesh + node.mesh_count;

         for (int32 i = node.child_or_first_mesh; i < last_mesh; ++i) {
            const float3& centroid = centroids[_meshes[i]];

            bounds_min = std::min(bounds_min, centroid.*axis);
            bounds_max = std::max(bounds_max, centroid.*axis);
         }

         if (bounds_min == bounds_max) continue;

         struct bin {
            math::bounding_box bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                       .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
            double tri_count = 0.0f;
         };

         std::array<bin, split_bin_count> bins = {};

         const float bins_scale =
            static_cast<float>(split_bin_count) / (bounds_max - bounds_min);

         for (int32 i = node.child_or_first_mesh; i < last_mesh; ++i) {
            const uint32 mesh_index = _meshes[i];
            const float3& centroid = centroids[mesh_index];

            const int32 bin_index =
               std::min(int32{split_bin_count - 1},
                        static_cast<int32>((centroid.*axis - bounds_min) * bins_scale));

            bin& bin = bins[bin_index];

            bin.bbox.min = min(bin.bbox.min, _mesh_data[mesh_index].bboxWS.min);
            bin.bbox.max = max(bin.bbox.max, _mesh_data[mesh_index].bboxWS.max);
            bin.tri_count +=
               static_cast<double>(_mesh_data[mesh_index].triangles.size());
         }

         math::bounding_box left_bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                         .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
         math::bounding_box right_bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                          .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
         double left_sum = 0.0f;
         double right_sum = 0.0f;

         std::array<double, split_bin_count - 1> left_area = {};
         std::array<double, split_bin_count - 1> left_count = {};

         std::array<double, split_bin_count - 1> right_area = {};
         std::array<double, split_bin_count - 1> right_count = {};

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
            const double plane_cost =
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

   auto calculate_node_cost(const node& node) const noexcept -> double
   {
      const float3 node_extents = node.bbox.max - node.bbox.min;
      const double node_area = node_extents.x * node_extents.y +
                               node_extents.y * node_extents.z +
                               node_extents.z * node_extents.x;

      return static_cast<double>(count_node_tris(node)) * node_area;
   }

   auto count_node_tris(const node& node) const noexcept -> int32
   {
      if (not node.is_leaf()) return 0;

      const uint32 last_mesh = node.child_or_first_mesh + node.mesh_count;

      int32 count = 0;

      for (uint32 i = node.child_or_first_mesh; i < last_mesh; ++i) {
         count += static_cast<int32>(_mesh_data[_meshes[i]].triangles.size());
      }

      return count;
   }

   auto partition_split(const node& node, float3_axis split_axis, float split_position,
                        std::span<const float3> centroids) noexcept -> int
   {
      int32 i = node.child_or_first_mesh;
      int32 j = i + node.mesh_count - 1;

      while (i <= j) {
         if (centroids[_meshes[i]].*split_axis < split_position) {
            i += 1;
         }
         else {
            std::swap(_meshes[i], _meshes[j]);

            j -= 1;
         }
      }

      return i;
   }

   auto area(const math::bounding_box& bbox) const noexcept -> double
   {
      const float3 extents = bbox.max - bbox.min;

      return extents.x * extents.y + extents.y * extents.z + extents.z * extents.x;
   }
};

}

auto build_mesh_clusters(std::span<const block_world_mesh> meshes,
                         int32 min_meshangles, int32 max_subdivisions) noexcept
   -> std::vector<std::vector<uint32>>
{
   return mesh_clusterizer{meshes, min_meshangles, max_subdivisions}.clusters();
}

}
