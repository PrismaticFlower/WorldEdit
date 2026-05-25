#include "triangulate_polygon.hpp"

#include "math/matrix_funcs.hpp"
#include "math/scalar_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <algorithm>
#include <array>
#include <cassert>

namespace we::munge {

namespace {

// This implementation is based on notes from studying Blender's triangulation algorithm. Given the lack of copying code
// I don't believe I'm stepping on anyone's copyright here). But if you worked on the algorithm in Blender and take issue
// with this then please feel free to reach out, I'm not looking to plagiarize anyone's work and if you feel my methodology of "read code,
// write down how it works, write new code based on those notes" has then I'd like to fix it.
//
// I just wanted nice triangulation and I was struggling to find learning materials on it. So I've tried to learn by studying Blender's
// method and I'm aiming to be transparent about having done so.
//
// Blender also has a couple sources it cites/credits that I'd be remiss to not pass on:
//
// LIBGDX (2013-11-28), what Blender's implementation was originally based on
// Martin Held, "FIST: Fast industrial-strength triangulation of polygons", *Algorithmica(1998),* citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.115.291

enum class classifcation { concave, colinear, convex };

const uint32 noindex = UINT32_MAX;

struct polygon_vertex {
   polygon_vertex* prev = nullptr;
   polygon_vertex* next = nullptr;

   classifcation classifcation = classifcation::concave;

   uint32 index = 0;

   bool cached_hit_index_dirty = false;
   uint32 cached_hit_index = noindex;
};

/// @brief Simple k-d tree used to accelerate vertex in triangle candidate tests.
struct polygon_kd_tree {
   polygon_kd_tree() = default;

   /// @brief Build a k-d tree from the input vertices.
   /// @param vertices Input vertices. Only vertices classified as non-convex will be added to the tree.
   /// @param positions Input positions for the vertices. This will be referenced for the tree's lifetime and so must outlive it.
   polygon_kd_tree(std::span<const polygon_vertex> vertices,
                   std::span<const float2> positions) noexcept
      : _positions{positions}
   {
      std::size_t tree_vertex_count = 0;

      for (const polygon_vertex& vertex : vertices) {
         tree_vertex_count += vertex.classifcation != classifcation::convex;
      }

      if (tree_vertex_count == 0) return;

      _nodes.reserve(tree_vertex_count);

      for (const polygon_vertex& vertex : vertices) {
         if (vertex.classifcation != classifcation::convex) {
            _nodes.push_back({.vertex_index = vertex.index});
         }
      }

      _root_node = construct_tree(_nodes, noindex, 0, 0);

      _vertex_to_node.resize(vertices.size(), noindex);

      for (std::size_t i = 0; i < _nodes.size(); ++i) {
         _vertex_to_node[_nodes[i].vertex_index] = static_cast<uint32>(i);
      }
   }

   /// @brief Remove a vertex from the tree. Doing nothing if the vertex is not in the tree.
   /// @param vertex_index The vertex to remove.
   void remove_vertex(const uint32 vertex_index) noexcept
   {
      if (vertex_index >= _vertex_to_node.size()) return;

      const uint32 node_index = _vertex_to_node[vertex_index];

      if (node_index == noindex) return;

      remove_node(node_index);

      _vertex_to_node[vertex_index] = noindex;
   }

   /// @brief Check if a vertex is inside the tree.
   /// @param vertex_index The vertex to check.
   /// @return Tree if the vertex is inside the tree.
   bool vertex_in_tree(const uint32 vertex_index) noexcept
   {
      if (vertex_index >= _vertex_to_node.size()) return false;

      const uint32 node_index = _vertex_to_node[vertex_index];

      if (node_index == noindex) return false;

      return not _nodes[node_index].is_ghost_branch;
   }

   /// @brief Check if a triangle intersects a vertex from the tree. Returning false if the vertex is no longer in the tree.
   /// @param tri The triangle.
   /// @param vertex_index The vertex index to check against the triangle.
   /// @return True if the vertex part of the tree, intersects the triangle and is not a vertex of the triangle, false otherwise.
   bool tri_intersects_vertex(const std::array<uint32, 3>& tri,
                              const uint32 vertex_index) const noexcept
   {
      if (vertex_index >= _vertex_to_node.size()) return false;

      const uint32 node_index = _vertex_to_node[vertex_index];

      if (node_index == noindex) return false;

      const node& node = _nodes[node_index];

      if (node.is_ghost_branch) return false;

      assert(node.vertex_index == vertex_index);

      if (tri[0] == vertex_index or tri[1] == vertex_index or tri[2] == vertex_index) {
         return false;
      }

      if (tri[0] >= _positions.size() or tri[1] >= _positions.size() or
          tri[2] >= _positions.size()) {
         return false;
      }

      return tri_intersector{{
         _positions[tri[0]],
         _positions[tri[1]],
         _positions[tri[2]],
      }}(_positions[vertex_index]);
   }

   /// @brief Check if the tree intersects a triangle.
   /// @param tri_indices The triangle to test.
   /// @return The index of the vertex that intersects the tree or noindex if there was nointersection.
   auto tri_intersects(const std::array<uint32, 3>& tri_indices) const noexcept -> uint32
   {
      if (_root_node == noindex) return noindex;

      if (tri_indices[0] >= _positions.size() or tri_indices[1] >= _positions.size() or
          tri_indices[2] >= _positions.size()) {
         return noindex;
      }

      const std::array<float2, 3> tri = {
         _positions[tri_indices[0]],
         _positions[tri_indices[1]],
         _positions[tri_indices[2]],
      };

      const float2 tri_min = min(min(tri[0], tri[1]), tri[2]);
      const float2 tri_max = max(max(tri[0], tri[1]), tri[2]);

      return tri_intersects_node(tri_indices, tri_intersector{tri}, tri_min,
                                 tri_max, _root_node);
   }

private:
   enum class axis : int8 { x, y };

   struct node {
      uint32 parent = noindex;
      uint32 vertex_index = noindex;

      bool is_ghost_branch = false;
      axis axis = axis::x;

      uint32 left = noindex;
      uint32 right = noindex;
   };

   struct tri_intersector {
      explicit tri_intersector(const std::array<float2, 3>& tri)
      {
         _e = {
            tri[1] - tri[0],
            tri[2] - tri[1],
            tri[0] - tri[1],
         };
         _c = {
            (_e[0].x * tri[0].y) - (tri[0].x * _e[0].y),
            (_e[1].x * tri[1].y) - (tri[1].x * _e[1].y),
            (_e[2].x * tri[2].y) - (tri[2].x * _e[2].y),
         };
      }

      bool operator()(const float2& v) const noexcept
      {
         return (_e[0].y * v.x) - (_e[0].x * v.y) + _c[0] >= 0.0f and
                (_e[1].y * v.x) - (_e[1].x * v.y) + _c[1] >= 0.0f and
                (_e[2].y * v.x) - (_e[2].x * v.y) + _c[2] >= 0.0f;
      }

   private:
      std::array<float2, 3> _e;
      std::array<float, 3> _c;
   };

   std::span<const float2> _positions;
   std::vector<uint32> _vertex_to_node;
   std::vector<node> _nodes;

   uint32 _root_node = noindex;

   auto construct_tree(std::span<node> nodes, uint32 parent_index, uint32 depth,
                       uint32 node_offset) -> uint32
   {
      if (nodes.empty()) return noindex;

      const axis axis = (depth & 1) == 0 ? axis::x : axis::y;

      if (nodes.size() == 1) {
         nodes[0].parent = parent_index;
         nodes[0].axis = axis;

         return node_offset;
      }

      if (axis == axis::x) {
         std::sort(nodes.begin(), nodes.end(), [this](const node& l, const node& r) {
            return _positions[l.vertex_index].x < _positions[r.vertex_index].x;
         });
      }
      else {
         std::sort(nodes.begin(), nodes.end(), [this](const node& l, const node& r) {
            return _positions[l.vertex_index].y < _positions[r.vertex_index].y;
         });
      }

      const uint32 median_index = static_cast<uint32>(nodes.size() / 2);
      const uint32 right_index = median_index + 1;

      node& median = nodes[median_index];

      median.parent = parent_index;
      median.axis = axis;
      median.left = construct_tree(nodes.subspan(0, median_index),
                                   median_index + node_offset, depth + 1, node_offset);
      median.right =
         construct_tree(nodes.subspan(right_index, nodes.size() - right_index),
                        median_index + node_offset, depth + 1,
                        right_index + node_offset);

      return median_index + node_offset;
   }

   void remove_node(uint32 node_index)
   {
      node& node = _nodes[node_index];

      // There are 3 cases we need to handle here.
      //
      // 1. The node has no children and can be removed outright.
      // 2. The node has both children and should just be marked as a ghost.
      // 3. The node has one child and the child can replace the node in the parent.

      const uint32 parent_index = node.parent;

      if (node.left == noindex and node.right == noindex) {
         node = {.is_ghost_branch = true};

         if (parent_index == noindex) {
            assert(_root_node == node_index);

            _root_node = noindex;
         }
         else {
            polygon_kd_tree::node& parent = _nodes[parent_index];

            assert(parent.left == node_index or parent.right == node_index);

            if (parent.left == node_index) {
               parent.left = noindex;
            }
            else {
               parent.right = noindex;
            }

            if (parent.is_ghost_branch and parent.left == noindex and
                parent.right == noindex) {
               return remove_node(parent_index);
            }
         }
      }
      else if (node.left != noindex and node.right != noindex) {
         node.is_ghost_branch = true;
      }
      else {
         const uint32 parent_new_child = node.left != noindex ? node.left : node.right;

         if (parent_index == noindex) {
            assert(_root_node == node_index);

            _root_node = parent_new_child;
         }
         else {
            polygon_kd_tree::node& parent = _nodes[parent_index];

            assert(parent.left == node_index or parent.right == node_index);

            if (parent.left == node_index) {
               parent.left = parent_new_child;
            }
            else {
               parent.right = parent_new_child;
            }
         }

         _nodes[parent_new_child].parent = parent_index;

         node = {.is_ghost_branch = true};
      }
   }

   auto tri_intersects_node(const std::array<uint32, 3>& tri,
                            const tri_intersector& intersector,
                            const float2& tri_min, const float2& tri_max,
                            const uint32 node_index) const noexcept -> uint32
   {
      const node& node = _nodes[node_index];
      const float2& vertex = _positions[node.vertex_index];

      if (not node.is_ghost_branch) {
         if (tri[0] != node.vertex_index and tri[1] != node.vertex_index and
             tri[2] != node.vertex_index) {
            if (intersector(vertex)) return node.vertex_index;
         }
      }

      float float2::* const axis = node.axis == axis::x ? &float2::x : &float2::y;

      if (node.left != noindex and tri_min.*axis <= vertex.*axis) {
         const uint32 child_hit =
            tri_intersects_node(tri, intersector, tri_min, tri_max, node.left);

         if (child_hit != noindex) return child_hit;
      }

      if (node.right != noindex and tri_max.*axis >= vertex.*axis) {
         const uint32 child_hit =
            tri_intersects_node(tri, intersector, tri_min, tri_max, node.right);

         if (child_hit != noindex) return child_hit;
      }

      return noindex;
   }
};

struct polygon_context {
   std::span<const float2> positions;

   uint32 vertex_count = 0;
   uint32 concave_vertex_count = 0;

   bool reverse_winding = false;

   polygon_kd_tree kd_tree;

   std::vector<polygon_vertex> vertices;
};

auto project_polygon(std::span<const float3> positions, std::span<const uint32> face)
   -> std::vector<float2>
{
   if (face.empty()) return {};

   // Calculate the polygon's normal using Newell's Method.
   float3 normal;

   for (std::size_t i = 0; i < face.size(); ++i) {
      const float3& v0 = positions[face[i]];
      const float3& v1 = positions[face[(i + 1) % face.size()]];

      normal.x += (v0.y - v1.y) * (v0.z + v1.z);
      normal.y += (v0.z - v1.z) * (v0.x + v1.x);
      normal.z += (v0.x - v1.x) * (v0.y + v1.y);
   }

   // Provide a fallback normal, just in case.
   if (normal == float3{0.0f, 0.0f, 0.0f}) normal = float3{1.0f, 0.0f, 0.0f};

   normal = normalize(normal);

   const float3 proj_x = {normal.y, -normal.x, 0.0f};
   const float3 proj_y = {-normal.z * proj_x.y, normal.z * proj_x.x,
                          (normal.x * proj_x.y) - (normal.y * proj_x.x)};

   std::vector<float2> projected;
   projected.resize(face.size());

   for (std::size_t i = 0; i < face.size(); ++i) {
      projected[i] = {dot(proj_x, positions[i]), dot(proj_y, positions[i])};
   }

   return projected;
}

auto calc_polygon_sign(std::span<const float2> vertices) -> float
{
   if (vertices.empty()) return 0.0f;

   float sign = 0.0f;

   for (std::size_t i = 0; i < vertices.size(); ++i) {
      const float2& v0 = vertices[i];
      const float2& v1 = vertices[(i + 1) % vertices.size()];

      sign += (v0.x - v1.x) * (v1.y + v0.y);
   }

   return sign;
}

auto classify_vertex(const polygon_vertex& vertex, std::span<const float2> positions)
   -> classifcation
{
   const float2& v0 = positions[vertex.prev->index];
   const float2& v1 = positions[vertex.index];
   const float2& v2 = positions[vertex.next->index];

   const float2 e0 = v1 - v0;
   const float2 e1 = v2 - v0;

   const float sign = (e0.x * e1.y) + (e1.x * e0.y);

   if (sign < 0.0f) return classifcation::concave;
   if (sign == 0.0f) return classifcation::colinear;

   return classifcation::convex;
}

auto make_polygon_context(std::span<const float2> positions) -> polygon_context
{
   polygon_context polygon = {
      .positions = positions,
      .vertex_count = static_cast<uint32>(positions.size()),
   };

   polygon.reverse_winding = calc_polygon_sign(positions) <= 0.0f;
   polygon.vertices.resize(positions.size());

   for (std::size_t i = 0; i < polygon.vertices.size(); ++i) {
      polygon.vertices[i] = {
         .prev = &polygon.vertices[i != 0 ? i - 1 : polygon.vertices.size() - 1],
         .next = &polygon.vertices[i != polygon.vertices.size() - 1 ? i + 1 : 0],
         .index = static_cast<uint32>(
            polygon.reverse_winding ? polygon.vertices.size() - 1 - i : i),
      };
   }

   for (polygon_vertex& vertex : polygon.vertices) {
      vertex.classifcation = classify_vertex(vertex, positions);

      if (vertex.classifcation != classifcation::convex) {
         polygon.concave_vertex_count += 1;
      }
   }

   polygon.kd_tree = polygon_kd_tree{polygon.vertices, polygon.positions};

   return polygon;
}

/// @brief Check that a candidate ear isn't blocked by a vertex.
/// @param polygon The polygon.
/// @param poly_ear_candidate The ear candidate.
/// @return If the candidate is usable (not blocked) or not.
bool check_ear_tip(polygon_context& polygon, polygon_vertex& poly_ear_candidate)
{
   if (polygon.concave_vertex_count == 0) return true;

   const std::array<uint32, 3> tri = {
      poly_ear_candidate.prev->index,
      poly_ear_candidate.index,
      poly_ear_candidate.next->index,
   };

   if (poly_ear_candidate.cached_hit_index != noindex) {
      if (poly_ear_candidate.cached_hit_index_dirty) {
         if (polygon.kd_tree.tri_intersects_vertex(tri, poly_ear_candidate.cached_hit_index)) {
            poly_ear_candidate.cached_hit_index_dirty = false;

            return false;
         }
      }
      else {
         if (polygon.kd_tree.vertex_in_tree(poly_ear_candidate.cached_hit_index)) {
            return false;
         }
      }
   }

   const uint32 hit_index = polygon.kd_tree.tri_intersects(tri);

   if (hit_index != noindex) {
      poly_ear_candidate.cached_hit_index = hit_index;
      poly_ear_candidate.cached_hit_index_dirty = false;

      return false;
   }

   return true;
}

/// @brief Find the "best" ear tip to cut.
/// @param polygon The polygon.
/// @param poly_ear_search_start The vertex to start the search at.
/// @param reverse_search If the search direction should be reversed.
/// @return The ear tip to cut next.
auto find_ear_tip(polygon_context& polygon, polygon_vertex& poly_ear_search_start,
                  bool reverse_search) -> polygon_vertex&
{
   // Try to find a suitable ear, prefering convex ones but accpeting colinear ones if that fails.
   for (const classifcation desired_classifcation :
        {classifcation::convex, classifcation::colinear}) {
      for (polygon_vertex* ear_candidate = &poly_ear_search_start;;) {
         if (ear_candidate->classifcation == desired_classifcation) {
            if (check_ear_tip(polygon, *ear_candidate)) {
               return *ear_candidate;
            }
         }

         ear_candidate = reverse_search ? ear_candidate->prev : ear_candidate->next;

         if (ear_candidate == &poly_ear_search_start) break;
      }
   }

   // If the above failed try to find any non-concave vertex, if this fails we
   // just return the vertex from before poly_ear_search_start.
   for (polygon_vertex* ear_candidate = &poly_ear_search_start;;) {
      if (ear_candidate->classifcation != classifcation::concave) {
         return *ear_candidate;
      }

      ear_candidate = ear_candidate->next;

      if (ear_candidate == &poly_ear_search_start) break;
   }

   return *poly_ear_search_start.prev;
}

/// @brief Cut an ear tip (triangle) from the polygon.
/// @param polygon The polygon.
/// @param poly_ear The ear to cut.
/// @return The triangle that has been cut.
auto cut_ear_tip(polygon_context& polygon, polygon_vertex& poly_ear)
   -> std::array<uint32, 3>
{
   const std::array<uint32, 3> tri =
      polygon.reverse_winding
         ? std::array{poly_ear.next->index, poly_ear.index, poly_ear.prev->index}
         : std::array{poly_ear.prev->index, poly_ear.index, poly_ear.next->index};

   polygon.kd_tree.remove_vertex(poly_ear.index);

   poly_ear.next->prev = poly_ear.prev;
   poly_ear.prev->next = poly_ear.next;

   if (poly_ear.classifcation != classifcation::convex) {
      polygon.concave_vertex_count -= 1;
   }

   polygon.vertex_count -= 1;

   poly_ear.index = noindex;
   poly_ear.next = nullptr;
   poly_ear.prev = nullptr;

   return tri;
}

auto triangulate(std::span<const float2> input_polygon)
   -> std::vector<std::array<uint32, 3>>
{
   if (input_polygon.size() < 3) return {};

   polygon_context polygon = make_polygon_context(input_polygon);

   std::vector<std::array<uint32, 3>> triangles;
   triangles.reserve(polygon.vertices.size() - 2);

   polygon_vertex* poly_ear_search_start = &polygon.vertices[0];
   bool reverse_ear_search = false;
   polygon_vertex* poly_ear_vertex = nullptr;

   while (polygon.vertex_count > 3) {
      poly_ear_vertex =
         &find_ear_tip(polygon, *poly_ear_search_start, reverse_ear_search);

      polygon_vertex& poly_prev_vertex = *poly_ear_vertex->prev;
      polygon_vertex& poly_next_vertex = *poly_ear_vertex->next;

      triangles.push_back(cut_ear_tip(polygon, *poly_ear_vertex));

      for (polygon_vertex* update_vertex : {&poly_prev_vertex, &poly_next_vertex}) {
         update_vertex->cached_hit_index_dirty = true;

         if (update_vertex->classifcation != classifcation::convex) {
            update_vertex->classifcation =
               classify_vertex(*update_vertex, polygon.positions);

            if (update_vertex->classifcation == classifcation::convex) {
               polygon.concave_vertex_count -= 1;
               polygon.kd_tree.remove_vertex(update_vertex->index);
            }
         }
      }

      poly_ear_search_start = reverse_ear_search ? &poly_prev_vertex : &poly_next_vertex;

      if (poly_ear_search_start->classifcation != classifcation::convex) {
         poly_ear_search_start = reverse_ear_search ? poly_ear_search_start->prev
                                                    : poly_ear_search_start->next;

         reverse_ear_search = not reverse_ear_search;
      }
   }

   triangles.push_back(polygon.reverse_winding
                          ? std::array{poly_ear_search_start->next->index,
                                       poly_ear_search_start->index,
                                       poly_ear_search_start->prev->index}
                          : std::array{poly_ear_search_start->prev->index,
                                       poly_ear_search_start->index,
                                       poly_ear_search_start->next->index});

   return triangles;
}

}

auto triangulate_polygon(std::span<const float3> positions, std::span<const uint32> face)
   -> std::vector<std::array<uint32, 3>>
{
   if (face.size() < 3) return {};

   const std::vector<float2> polygon = project_polygon(positions, face);

   std::vector<std::array<uint32, 3>> triangles = triangulate(polygon);

   for (std::array<uint32, 3>& tri : triangles) {
      for (uint32& index : tri) index = face[index];
   }

   return triangles;
}

}