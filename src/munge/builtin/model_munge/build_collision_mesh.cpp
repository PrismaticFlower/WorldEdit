#include "build_collision_mesh.hpp"
#include "error.hpp"
#include "simplify_mesh.hpp"

#include "math/vector_funcs.hpp"

namespace we::munge {

namespace {

constexpr uint32 split_bin_count = 8;

auto area(const math::bounding_box& bbox) noexcept -> float
{
   const float3 extents = bbox.max - bbox.min;

   return extents.x * extents.y + extents.y * extents.z + extents.z * extents.x;
}

struct collision_bvh {
   collision_bvh(std::span<const math::bounding_box> face_bboxes,
                 std::span<const float3> face_centroids) noexcept
      : _bboxes{face_bboxes}, _centroids{face_centroids}
   {
      assert(face_bboxes.size() == face_centroids.size());

      face_map.reserve(_bboxes.size());

      for (uint32 i = 0; i < _bboxes.size(); ++i) face_map.push_back(i);

      tree.reserve(std::max(std::ssize(_bboxes) * 2 - 1, 1ll));

      node root_node = {.face_count = static_cast<int32>(_bboxes.size())};

      update_node_bounds(root_node);

      std::vector<node> node_stack;
      node_stack.reserve(64);

      node_stack.push_back(root_node);

      while (not node_stack.empty()) {
         node& parent_node = tree.emplace_back(node_stack.back());

         node_stack.pop_back();

         if (parent_node.face_count > 1) {
            const std::array<node, 2> subdivided = subdivide(parent_node);

            node_stack.push_back(subdivided[0]);
            node_stack.push_back(subdivided[1]);

            parent_node.first_face = 0;
            parent_node.face_count = 0;
         }
      }
   }

   collision_bvh(const collision_bvh&) = delete;
   collision_bvh(collision_bvh&&) noexcept = delete;

   auto operator=(const collision_bvh&) -> collision_bvh& = delete;
   auto operator=(collision_bvh&&) noexcept -> collision_bvh& = delete;

   struct node {
      math::bounding_box bbox;
      int32 first_face = 0;
      int32 face_count = 0;

      bool is_leaf() const noexcept
      {
         return face_count != 0;
      }
   };

   std::vector<node> tree;
   std::vector<uint32> face_map;

private:
   std::span<const math::bounding_box> _bboxes;
   std::span<const float3> _centroids;

   using float3_axis = float float3::*;

   void update_node_bounds(node& node) noexcept
   {
      node.bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                   .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};

      const int32 last_face = node.first_face + node.face_count;

      for (int32 i = node.first_face; i < last_face; ++i) {
         const math::bounding_box& child_bbox = _bboxes[face_map[i]];

         node.bbox.min = min(node.bbox.min, child_bbox.min);
         node.bbox.max = max(node.bbox.max, child_bbox.max);
      }
   }

   auto subdivide(const node& root) noexcept -> std::array<node, 2>
   {
      assert(root.face_count > 1);

      float3_axis split_axis = &float3::x;
      float split_position = 0.0f;

      find_best_split_plane(root, split_axis, split_position);

      const int32 right_child_start =
         partition_split(root, split_axis, split_position);
      const int32 left_child_count = right_child_start - root.first_face;

      if (left_child_count == 0 or left_child_count == root.face_count) {
         const int32 brute_split_right_count = (root.face_count / 2);
         const int32 brute_split_left_count =
            brute_split_right_count + (root.face_count % 2);
         const int32 brute_split_right_start = root.first_face + brute_split_left_count;

         node left_child = {.first_face = root.first_face,
                            .face_count = brute_split_left_count};
         node right_child = {.first_face = brute_split_right_start,
                             .face_count = brute_split_right_count};

         update_node_bounds(left_child);
         update_node_bounds(right_child);

         return {left_child, right_child};
      }

      node left_child = {.first_face = root.first_face, .face_count = left_child_count};
      node right_child = {.first_face = right_child_start,
                          .face_count = root.face_count - left_child_count};

      update_node_bounds(left_child);
      update_node_bounds(right_child);

      return std::array{left_child, right_child};
   }

   void find_best_split_plane(const node& node, float3_axis& best_split_axis,
                              float& best_split_position) const noexcept
   {
      float best_split_cost = FLT_MAX;

      for (float3_axis axis : {&float3::x, &float3::y, &float3::z}) {
         float bounds_min = FLT_MAX;
         float bounds_max = -FLT_MAX;

         const int32 last_face = node.first_face + node.face_count;

         for (int32 i = node.first_face; i < last_face; ++i) {
            const float3& centroid = _centroids[face_map[i]];

            bounds_min = std::min(bounds_min, centroid.*axis);
            bounds_max = std::max(bounds_max, centroid.*axis);
         }

         if (bounds_min == bounds_max) continue;

         struct bin {
            math::bounding_box bbox = {.min = {FLT_MAX, FLT_MAX, FLT_MAX},
                                       .max = {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
            float face_count = 0.0f;
         };

         std::array<bin, split_bin_count> bins = {};

         const float bins_scale =
            static_cast<float>(split_bin_count) / (bounds_max - bounds_min);

         for (int32 i = node.first_face; i < last_face; ++i) {
            const uint32 face_index = face_map[i];
            const float3& centroid = _centroids[face_index];
            const math::bounding_box& bbox = _bboxes[face_index];

            const int32 bin_index =
               std::min(int32{split_bin_count - 1},
                        static_cast<int32>((centroid.*axis - bounds_min) * bins_scale));

            bin& bin = bins[bin_index];

            bin.bbox.min = min(bin.bbox.min, bbox.min);
            bin.bbox.max = max(bin.bbox.max, bbox.max);
            bin.face_count += 1.0f;
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
            left_sum += bins[i].face_count;
            left_count[i] = left_sum;
            left_bbox = {.min = min(bins[i].bbox.min, left_bbox.min),
                         .max = max(bins[i].bbox.max, left_bbox.max)};
            left_area[i] = area(left_bbox);

            right_sum += bins[split_bin_count - 1 - i].face_count;
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

   auto partition_split(const node& node, float3_axis split_axis,
                        float split_position) noexcept -> int
   {
      int32 i = node.first_face;
      int32 j = i + node.face_count - 1;

      while (i <= j) {
         if (_centroids[face_map[i]].*split_axis < split_position) {
            i += 1;
         }
         else {
            std::swap(face_map[i], face_map[j]);

            j -= 1;
         }
      }

      return i;
   }
};

}

auto build_collision_mesh(collision_mesh_input input) -> collision_mesh
{
   simplified_collision_mesh simplified;

   if (not input.do_not_simplify) {
      simplified = simplify_collision_mesh(input.vertices, input.triangles);
   }

   std::vector<math::bounding_box> face_bboxes;
   std::vector<float3> face_centroids;

   if (not input.do_not_simplify) {
      face_bboxes.resize(simplified.faces.size());
      face_centroids.resize(simplified.faces.size());

      for (std::size_t i = 0; i < simplified.faces.size(); ++i) {
         const simplified_mesh_face& face = simplified.faces[i];

         if (face.index_count == 0) {
            throw model_error{"Simplified collision face has 0 vertices!",
                              model_ec::collision_mesh_simplified_face_invalid};
         }

         assert(face.index_begin < simplified.face_indices.size());
         assert(face.index_begin + face.index_count <= simplified.face_indices.size());

         float3 centroid =
            simplified.vertices[simplified.face_indices[face.index_begin]];

         math::bounding_box bbox = {
            .min = centroid,
            .max = bbox.min,
         };

         for (uint16 vertex_index :
              std::span{simplified.face_indices}.subspan(face.index_begin,
                                                         face.index_count)) {
            const float3& vertex = simplified.vertices[vertex_index];

            bbox = math::integrate(bbox, simplified.vertices[vertex_index]);
            centroid += vertex;
         }

         face_bboxes[i] = bbox;
         face_centroids[i] = centroid / static_cast<float>(face.index_count);
      }
   }
   else {
      face_bboxes.resize(input.triangles.size());
      face_centroids.resize(input.triangles.size());

      for (std::size_t i = 0; i < input.triangles.size(); ++i) {
         const std::array<uint16, 3>& tri = input.triangles[i];

         const float3& v0 = input.vertices[tri[0]];
         const float3& v1 = input.vertices[tri[1]];
         const float3& v2 = input.vertices[tri[2]];

         face_bboxes[i] = {.min = min(min(v0, v1), v2), .max = max(max(v0, v1), v2)};
         face_centroids[i] = (v0 + v1 + v2) / 3.0f;
      }
   }

   collision_bvh bvh{face_bboxes, face_centroids};
   collision_mesh mesh = {
      .node_name = input.node_name,
      .mask = input.mask,
      .bbox = not bvh.tree.empty() ? bvh.tree[0].bbox : math::bounding_box{},
   };

   if (not input.do_not_simplify) {
      mesh.vertices = std::move(simplified.vertices);
      mesh.face_indices = std::move(simplified.face_indices);
   }
   else {
      mesh.vertices = std::move(input.vertices);

      mesh.face_indices.resize(input.triangles.size() * 3);

      for (std::size_t i = 0; i < input.triangles.size(); ++i) {
         const std::array<uint16, 3>& tri = input.triangles[bvh.face_map[i]];

         mesh.face_indices[i * 3 + 0] = tri[0];
         mesh.face_indices[i * 3 + 1] = tri[1];
         mesh.face_indices[i * 3 + 2] = tri[2];
      }
   }

   mesh.tree.resize(bvh.tree.size());

   std::vector<math::bounding_box> process_stack;
   process_stack.reserve(64);

   if (not bvh.tree.empty()) process_stack.push_back(mesh.bbox);

   for (std::size_t i = 0; i < bvh.tree.size(); ++i) {
      assert(not process_stack.empty());

      const math::bounding_box parent_bbox = process_stack.back();

      process_stack.pop_back();

      const collision_bvh::node& node = bvh.tree[i];

      math::bounding_box node_bbox = node.bbox;

      const float3 parent_size = parent_bbox.max - parent_bbox.min;

      math::bounding_box relative_bbox = {
         .min = clamp((node_bbox.min - parent_bbox.min) / parent_size * 255.0f,
                      0.0f, 255.0f),
         .max = clamp((node_bbox.max - parent_bbox.min) / parent_size * 255.0f + 1.0f,
                      0.0f, 255.0f),
      };

      collision_mesh_bbox_unorm bbox_unorm = {
         .min_x = static_cast<uint8>(relative_bbox.min.x),
         .min_y = static_cast<uint8>(relative_bbox.min.y),
         .min_z = static_cast<uint8>(relative_bbox.min.z),

         .max_x = static_cast<uint8>(relative_bbox.max.x),
         .max_y = static_cast<uint8>(relative_bbox.max.y),
         .max_z = static_cast<uint8>(relative_bbox.max.z),
      };

      if (bbox_unorm.min_x == bbox_unorm.max_x) {
         if (bbox_unorm.min_x == 0) {
            bbox_unorm.max_x += 1;
         }
         else {
            bbox_unorm.min_x -= 1;
         }
      }

      if (bbox_unorm.min_y == bbox_unorm.max_y) {
         if (bbox_unorm.min_y == 0) {
            bbox_unorm.max_y += 1;
         }
         else {
            bbox_unorm.min_y -= 1;
         }
      }

      if (bbox_unorm.min_z == bbox_unorm.max_z) {
         if (bbox_unorm.min_z == 0) {
            bbox_unorm.max_z += 1;
         }
         else {
            bbox_unorm.min_z -= 1;
         }
      }

      collision_mesh_node& out = mesh.tree[i];

      out.bbox = bbox_unorm;

      if (node.is_leaf()) {
         out.type = collision_mesh_node_type::leaf;

         if (not input.do_not_simplify) {
            const simplified_mesh_face& face =
               simplified.faces[bvh.face_map[node.first_face]];

            out.face_index_count = face.index_count;
            out.face_index_begin = face.index_begin;
         }
         else {
            out.face_index_count = 3;
            out.face_index_begin = node.first_face * 3;
         }
      }
      else {
         const math::bounding_box quantized_bbox = math::combine(
            {
               // The implicit casts are deliberate here, we're converting a float can
               // always hold a uint8 and this makes for more readable code.
               .min = float3(bbox_unorm.min_x, bbox_unorm.min_y, bbox_unorm.min_z) *
                         parent_size / 255.0f +
                      parent_bbox.min,
               .max = float3(bbox_unorm.max_x, bbox_unorm.max_y, bbox_unorm.max_z) *
                         parent_size / 255.0f +
                      parent_bbox.min,
            },
            node.bbox);

         process_stack.push_back(quantized_bbox);
         process_stack.push_back(quantized_bbox);
      }
   }

   assert(process_stack.empty());

   return mesh;
}
}