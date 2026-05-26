#include "simplify_mesh.hpp"

#include "math/plane_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <absl/container/flat_hash_map.h>

#include <algorithm>
#include <bit>

namespace we::munge {

namespace {

constexpr uint32 noindex = UINT32_MAX;

// Max vertices for collision faces.
constexpr uint32 max_face_vertices = 255;

// Precision for the W plane in quantized_plane. The W component is stored to a
// 32-bit int after being multiplied by this.
constexpr float plane_w_precision = 1000.0f;

// When determining if two consecutive edges are colinear we calculate the area
// of the triangle they form, if it's less than this (which is about 1μm) they
// considered colinear and are collapsed.
constexpr float colinear_max_area = 0.000001f;

struct hashable_f3 {
   hashable_f3(const float3& v)
   {
      _x = (v.x == 0.0f or v.x != v.x) ? 0 : std::bit_cast<uint32>(v.x);
      _y = (v.y == 0.0f or v.y != v.y) ? 0 : std::bit_cast<uint32>(v.y);
      _z = (v.z == 0.0f or v.z != v.z) ? 0 : std::bit_cast<uint32>(v.z);
   }

   bool operator==(const hashable_f3&) const noexcept = default;

   explicit operator float3() const noexcept
   {
      return std::bit_cast<float3>(*this);
   }

   template<typename H>
   friend H AbslHashValue(H h, const hashable_f3& cached)
   {
      return H::combine(std::move(h), cached._x, cached._y, cached._z);
   }

private:
   uint32 _x = 0;
   uint32 _y = 0;
   uint32 _z = 0;
};

struct quantized_plane {
   quantized_plane(float4 plane)
   {
      plane *= float4{32767.0f, 32767.0f, 32767.0f, plane_w_precision};
      plane = clamp(plane, float4{-32767.0f, -32767.0f, -32767.0f, plane.w},
                    float4{32767.0f, 32767.0f, 32767.0f, plane.w});

      _x = static_cast<int16>(plane.x >= 0.0f ? plane.x + 0.5f : plane.x - 0.5f);
      _y = static_cast<int16>(plane.y >= 0.0f ? plane.y + 0.5f : plane.y - 0.5f);
      _z = static_cast<int16>(plane.z >= 0.0f ? plane.z + 0.5f : plane.z - 0.5f);
      _w = static_cast<int32>(plane.w);
   }

   bool operator==(const quantized_plane&) const noexcept = default;

   template<typename H>
   friend H AbslHashValue(H h, const quantized_plane& plane)
   {
      return H::combine(std::move(h), plane._x, plane._y, plane._z, plane._w);
   }

private:
   int16 _x = 0;
   int16 _y = 0;
   int16 _z = 0;
   int32 _w = 0;
};

struct half_edge {
   uint16 vertex = 0;
   uint32 previous_edge = 0;
   uint32 next_edge = 0;
   uint32 twin_edge = 0;

   uint32 face = 0;
};

struct face {
   uint32 first_edge = 0;
   uint32 vertex_count = 0;
   uint32 next_face = noindex;
   uint32 previous_face = noindex;
};

struct face_bin {
   uint32 first_face = noindex;
   uint32 last_face = noindex;
};

struct reduced_vertices {
   std::vector<float3> vertices;
   std::vector<uint32> remap;
};

/// @brief Reduce input vertices down to the unique set.
/// @param vertices The input vertices.
/// @return The reduced vertex set.
auto reduce_vertices(std::span<const float3> vertices) -> reduced_vertices
{
   absl::flat_hash_map<hashable_f3, uint16> vertex_cache;
   vertex_cache.reserve(vertices.size());

   reduced_vertices reduced;
   reduced.vertices.reserve(vertices.size());
   reduced.remap.reserve(vertices.size());

   for (const float3& v : vertices) {
      auto [it, inserted] =
         vertex_cache.try_emplace(v, static_cast<uint16>(reduced.vertices.size()));

      reduced.remap.push_back(it->second);

      if (inserted) reduced.vertices.push_back(v);
   }

   return reduced;
}

/// @brief Build a remap index for sorting input triangles spatially.
/// @param vertices Vertices for the triangles.
/// @param vertex_remap Remap for the triangle indices.
/// @param triangles The triangles.
/// @return A remap table for the triangles.
auto build_triangle_remap(std::span<const float3> vertices,
                          std::span<const uint32> vertex_remap,
                          std::span<const std::array<uint16, 3>> triangles)
   -> std::vector<uint32>
{
   std::vector<float3> centroids;
   centroids.resize(triangles.size());

   std::vector<uint32> remap;
   remap.resize(triangles.size());

   for (std::size_t i = 0; i < triangles.size(); ++i) {
      const std::array<uint16, 3>& tri = triangles[i];

      centroids[i] =
         (vertices[vertex_remap[tri[0]]] + vertices[vertex_remap[tri[1]]] +
          vertices[vertex_remap[tri[2]]]) /
         3.0f;
      remap[i] = static_cast<uint32>(i);
   }

   std::sort(remap.begin(), remap.end(),
             [&](const uint32 left_index, const uint32 right_index) {
                const float3& left = centroids[left_index];
                const float3& right = centroids[right_index];

                if (left.x < right.x) return true;
                if (right.x < left.x) return false;
                if (left.y < right.y) return true;
                if (right.y < left.y) return false;
                if (left.z < right.z) return true;

                return false;
             });

   return remap;
}

/// @brief Classify a vector's signs.
/// @param v The vector to classify.
/// @return For each component, -1 if negative, 0 if 0, 1 if positive.
auto classify(const float3& v) -> std::array<int, 3>
{
   return {
      v.x < 0.0f ? -1 : v.x > 0.0f,
      v.y < 0.0f ? -1 : v.y > 0.0f,
      v.z < 0.0f ? -1 : v.z > 0.0f,
   };
}

/// @brief Check if two consecutive edges are (approximately) colinear.
/// @param v0 The first vertex of the first edge.
/// @param v1 The shared vertex of the edges.
/// @param v2 The last vertex of the second edge.
/// @return If the edges are colinear.
bool are_edges_colinear(const float3& v0, const float3& v1, const float3& v2)
{
   constexpr float colinear_max_area_sq =
      (colinear_max_area * colinear_max_area) * 2.0f;

   const float3 e0_x_e1 = cross(v1 - v0, v2 - v0);

   return dot(e0_x_e1, e0_x_e1) <= colinear_max_area_sq;
}

/// @brief Check if a half edge loop forms a convex polygon. Only works for simple, coplanar polygons.
/// @param start_edge_index The index of the first half edge of the loop.
/// @param half_edges The mesh half edges.
/// @param vertices The mesh vertices.
/// @return If the loop forms a convex polygon.
bool is_loop_convex(const uint32 start_edge_index, const std::span<half_edge> half_edges,
                    std::span<const float3> vertices)
{
   const half_edge& start_edge = half_edges[start_edge_index];
   const half_edge& start_previous_edge = half_edges[start_edge.previous_edge];
   const half_edge& start_next_edge = half_edges[start_edge.next_edge];

   const float3 face_vector =
      cross(vertices[start_edge.vertex] - vertices[start_previous_edge.vertex],
            vertices[start_next_edge.vertex] - vertices[start_previous_edge.vertex]);

   const std::array<int, 3> face_signs = classify(face_vector);

   for (uint32 next_edge_index = start_edge.next_edge;;) {
      const half_edge& edge = half_edges[next_edge_index];
      const half_edge& previous_edge = half_edges[edge.previous_edge];
      const half_edge& next_edge = half_edges[edge.next_edge];

      const float3 vector =
         cross(vertices[edge.vertex] - vertices[previous_edge.vertex],
               vertices[next_edge.vertex] - vertices[previous_edge.vertex]);
      const std::array<int, 3> signs = classify(vector);

      if (signs != face_signs) return false;

      next_edge_index = edge.next_edge;

      if (next_edge_index == start_edge_index) break;
   }

   return true;
}

/// @brief Check that all twins of a half edge loop are external (point to other faces).
/// This shouldn't be possible during regular processing but it is important to check and prevent as otherwise
/// infinite loops are possible during processing and floating point inprecision scares me.
/// @param start_edge_index The index of the first half edge of the loop.
/// @param half_edges The mesh half edges.
/// @return If all twins are external.
bool are_twins_external(const uint32 start_edge_index, const std::span<half_edge> half_edges)
{
   const uint32 face = half_edges[start_edge_index].face;

   for (uint32 next_edge_index = start_edge_index;;) {
      const half_edge& edge = half_edges[next_edge_index];

      if (edge.twin_edge != noindex) {
         if (half_edges[edge.twin_edge].face == face) return false;
      }

      next_edge_index = edge.next_edge;

      if (next_edge_index == start_edge_index) break;
   }

   return true;
}

}

auto simplify_collision_mesh(std::span<const float3> vertices,
                             std::span<const std::array<uint16, 3>> triangles)
   -> simplified_collision_mesh
{
   reduced_vertices reduced_vertices = reduce_vertices(vertices);

   vertices = reduced_vertices.vertices;

   const std::vector<uint32> triangle_remap =
      build_triangle_remap(vertices, reduced_vertices.remap, triangles);

   std::vector<face> faces;
   faces.reserve(triangles.size());

   std::vector<half_edge> half_edges;
   half_edges.reserve(triangles.size() * 3);

   std::vector<face_bin> face_bins;
   face_bins.reserve(triangles.size());

   absl::flat_hash_map<quantized_plane, uint32> face_bin_map;
   face_bin_map.reserve(triangles.size());

   absl::flat_hash_map<std::array<uint16, 2>, uint32> twin_edge_map;
   twin_edge_map.reserve(triangles.size());

   for (std::size_t tri_index = 0; tri_index < triangles.size(); ++tri_index) {
      std::array<uint16, 3> tri = triangles[triangle_remap[tri_index]];

      tri[0] = static_cast<uint16>(reduced_vertices.remap[tri[0]]);
      tri[1] = static_cast<uint16>(reduced_vertices.remap[tri[1]]);
      tri[2] = static_cast<uint16>(reduced_vertices.remap[tri[2]]);

      const float3& v0 = vertices[tri[0]];
      const float3& v1 = vertices[tri[1]];
      const float3& v2 = vertices[tri[2]];

      float3 normal = cross(v1 - v0, v2 - v0);

      if (normal == float3{}) continue; // Discard degenerate triangles.

      const quantized_plane plane{make_plane_from_point(v0, normalize(normal))};

      const uint32 face_index = static_cast<uint32>(faces.size());

      face& face = faces.emplace_back();

      if (auto bin_it = face_bin_map.find(plane); bin_it != face_bin_map.end()) {
         const uint32 bin_index = bin_it->second;
         face_bin& bin = face_bins[bin_index];

         faces[bin.last_face].next_face = face_index;
         face.previous_face = bin.last_face;
         bin.last_face = face_index;
      }
      else {
         const uint32 bin_index = static_cast<uint32>(face_bins.size());

         face_bin_map.emplace(plane, bin_index);
         face_bins.push_back(face_bin{face_index, face_index});
      }

      const uint32 first_half_edge_index = static_cast<uint32>(half_edges.size());

      half_edges.push_back({
         .vertex = tri[0],
         .previous_edge = first_half_edge_index + 2,
         .next_edge = first_half_edge_index + 1,
         .twin_edge = noindex,

         .face = face_index,
      });
      half_edges.push_back({
         .vertex = tri[1],
         .previous_edge = first_half_edge_index,
         .next_edge = first_half_edge_index + 2,
         .twin_edge = noindex,

         .face = face_index,
      });
      half_edges.push_back({
         .vertex = tri[2],
         .previous_edge = first_half_edge_index + 1,
         .next_edge = first_half_edge_index,
         .twin_edge = noindex,

         .face = face_index,
      });

      face.first_edge = first_half_edge_index;
      face.vertex_count = 3;
   }

   for (face_bin& bin : face_bins) {
      // Populate Twin Edges, this happens in the face bin loop instead of above so the storage for twin_edge_map can be reused.
      twin_edge_map.clear();

      for (uint32 next_face_index = bin.first_face; next_face_index != noindex;) {
         face& current_face = faces[next_face_index];

         for (uint32 next_edge_index = current_face.first_edge;;) {
            const half_edge& current_edge = half_edges[next_edge_index];

            const std::array<uint16, 2> edge =
               {current_edge.vertex, half_edges[current_edge.next_edge].vertex};

            twin_edge_map.emplace(edge, next_edge_index);

            const std::array<uint16, 2> twin_edge = {edge[1], edge[0]};

            if (auto twin_it = twin_edge_map.find(twin_edge);
                twin_it != twin_edge_map.end()) {
               const uint32 twin_edge_index = twin_it->second;

               if (half_edges[twin_edge_index].twin_edge == noindex) {
                  half_edges[next_edge_index].twin_edge = twin_edge_index;
                  half_edges[twin_edge_index].twin_edge = next_edge_index;

                  assert(half_edges[next_edge_index].face !=
                         half_edges[twin_edge_index].face);
               }
            }

            next_edge_index = current_edge.next_edge;

            if (next_edge_index == current_face.first_edge) break;
         }

         next_face_index = current_face.next_face;
      }

      // Edge Collapse, runs until it fails to collapse an edge.
      while (true) {
         bool collapsed_edge = false;

         for (uint32 next_face_index = bin.first_face; next_face_index != noindex;) {
            face& current_face = faces[next_face_index];

            for (uint32 next_edge_index = current_face.first_edge;;) {
               const half_edge& edge = half_edges[next_edge_index];

               if (edge.twin_edge != noindex) {
                  half_edge& previous_edge = half_edges[edge.previous_edge];
                  half_edge& next_edge = half_edges[edge.next_edge];
                  half_edge& end_edge = half_edges[next_edge.next_edge];

                  const half_edge& twin_edge = half_edges[edge.twin_edge];
                  half_edge& twin_previous_edge = half_edges[twin_edge.previous_edge];
                  half_edge& twin_next_edge = half_edges[twin_edge.next_edge];
                  half_edge& twin_end_edge = half_edges[twin_next_edge.next_edge];

                  const float3& previous_vertex = vertices[previous_edge.vertex];
                  const float3& next_vertex = vertices[next_edge.vertex];
                  const float3& end_vertex = vertices[end_edge.vertex];

                  const float3& twin_previous_vertex =
                     vertices[twin_previous_edge.vertex];
                  const float3& twin_next_vertex = vertices[twin_next_edge.vertex];
                  const float3& twin_end_vertex = vertices[twin_end_edge.vertex];

                  const bool collapse_twin_next_edge =
                     are_edges_colinear(previous_vertex, twin_next_vertex,
                                        twin_end_vertex);
                  const bool collapse_next_edge =
                     are_edges_colinear(twin_previous_vertex, next_vertex, end_vertex);

                  face& twin_face = faces[twin_edge.face];

                  for (uint32 next_twin_edge_index = twin_edge.next_edge;;) {
                     half_edge& repoint_edge = half_edges[next_twin_edge_index];

                     repoint_edge.face = edge.face;

                     if (next_twin_edge_index == twin_edge.previous_edge) {
                        break;
                     }

                     next_twin_edge_index = repoint_edge.next_edge;
                  }

                  if (collapse_twin_next_edge) {
                     previous_edge.next_edge = twin_next_edge.next_edge;
                     twin_end_edge.previous_edge = edge.previous_edge;
                  }
                  else {
                     previous_edge.next_edge = twin_edge.next_edge;
                     twin_next_edge.previous_edge = edge.previous_edge;
                  }

                  if (collapse_next_edge) {
                     twin_previous_edge.next_edge = next_edge.next_edge;
                     end_edge.previous_edge = twin_edge.previous_edge;
                  }
                  else {
                     twin_previous_edge.next_edge = edge.next_edge;
                     next_edge.previous_edge = twin_edge.previous_edge;
                  }

                  const uint32 collapsed_vertices =
                     2 + collapse_twin_next_edge + collapse_next_edge;

                  if (is_loop_convex(edge.previous_edge, half_edges, vertices) and
                      are_twins_external(edge.previous_edge, half_edges) and
                      current_face.vertex_count + twin_face.vertex_count - collapsed_vertices <=
                         max_face_vertices) {
                     twin_edge_map.erase({edge.vertex, next_edge.vertex});
                     twin_edge_map.erase({twin_edge.vertex, twin_next_edge.vertex});

                     if (twin_face.previous_face != noindex) {
                        faces[twin_face.previous_face].next_face = twin_face.next_face;
                     }
                     else {
                        bin.first_face = twin_face.next_face;
                     }

                     if (twin_face.next_face != noindex) {
                        faces[twin_face.next_face].previous_face =
                           twin_face.previous_face;
                     }
                     else {
                        bin.last_face = twin_face.previous_face;
                     }

                     if (current_face.first_edge == next_edge_index) {
                        current_face.first_edge = previous_edge.next_edge;
                     }

                     current_face.vertex_count +=
                        twin_face.vertex_count - collapsed_vertices;

                     if (collapse_twin_next_edge) {
                        if (previous_edge.twin_edge != noindex) {
                           half_edges[previous_edge.twin_edge].twin_edge = noindex;
                        }

                        if (twin_next_edge.twin_edge != noindex) {
                           half_edges[twin_next_edge.twin_edge].twin_edge = noindex;
                        }

                        twin_edge_map.erase(
                           {previous_edge.vertex, twin_next_edge.vertex});
                        twin_edge_map.erase(
                           {twin_next_edge.vertex, twin_end_edge.vertex});
                        twin_edge_map.erase({twin_end_edge.vertex,
                                             half_edges[twin_end_edge.next_edge].vertex});

                        previous_edge.twin_edge = noindex;

                        twin_edge_map.emplace(std::array{previous_edge.vertex,
                                                         twin_end_edge.vertex},
                                              edge.previous_edge);

                        if (auto twin_it = twin_edge_map.find(
                               std::array{twin_end_edge.vertex, previous_edge.vertex});
                            twin_it != twin_edge_map.end()) {
                           const uint32 new_twin_edge_index = twin_it->second;

                           if (half_edges[new_twin_edge_index].twin_edge == noindex) {
                              previous_edge.twin_edge = new_twin_edge_index;
                              half_edges[new_twin_edge_index].twin_edge =
                                 edge.previous_edge;
                           }
                        }
                     }

                     if (collapse_next_edge) {
                        if (current_face.first_edge == edge.next_edge) {
                           current_face.first_edge = twin_previous_edge.next_edge;
                        }

                        if (twin_previous_edge.twin_edge != noindex) {
                           half_edges[twin_previous_edge.twin_edge].twin_edge = noindex;
                        }

                        if (next_edge.twin_edge != noindex) {
                           half_edges[next_edge.twin_edge].twin_edge = noindex;
                        }

                        twin_edge_map.erase(
                           {twin_previous_edge.vertex, next_edge.vertex});
                        twin_edge_map.erase({next_edge.vertex, end_edge.vertex});

                        twin_previous_edge.twin_edge = noindex;
                        twin_edge_map.emplace(std::array{twin_previous_edge.vertex,
                                                         end_edge.vertex},
                                              twin_edge.previous_edge);

                        if (auto twin_it = twin_edge_map.find(
                               std::array{end_edge.vertex, twin_previous_edge.vertex});
                            twin_it != twin_edge_map.end()) {
                           const uint32 new_twin_edge_index = twin_it->second;

                           if (half_edges[new_twin_edge_index].twin_edge == noindex) {
                              twin_previous_edge.twin_edge = new_twin_edge_index;
                              half_edges[new_twin_edge_index].twin_edge =
                                 twin_edge.previous_edge;
                           }
                        }
                     }

                     collapsed_edge = true;
                     next_edge_index = previous_edge.next_edge;

                     continue;
                  }
                  else {
                     if (collapse_twin_next_edge) {
                        previous_edge.next_edge = next_edge_index;
                        twin_end_edge.previous_edge = twin_edge.next_edge;
                     }
                     else {
                        previous_edge.next_edge = next_edge_index;
                        twin_next_edge.previous_edge = edge.twin_edge;
                     }

                     if (collapse_next_edge) {
                        twin_previous_edge.next_edge = edge.twin_edge;
                        end_edge.previous_edge = edge.next_edge;
                     }
                     else {
                        twin_previous_edge.next_edge = edge.twin_edge;
                        next_edge.previous_edge = next_edge_index;
                     }

                     for (uint32 next_twin_edge_index = twin_edge.next_edge;;) {
                        half_edge& repoint_edge = half_edges[next_twin_edge_index];

                        repoint_edge.face = twin_edge.face;

                        if (next_twin_edge_index == twin_edge.previous_edge) {
                           break;
                        }

                        next_twin_edge_index = repoint_edge.next_edge;
                     }
                  }
               }

               next_edge_index = edge.next_edge;

               if (next_edge_index == current_face.first_edge) break;
            }

            next_face_index = current_face.next_face;
         }

         if (not collapsed_edge) break;
      }
   }

   simplified_collision_mesh mesh;

   mesh.faces.reserve(triangles.size());
   mesh.face_indices.reserve(triangles.size() * 3);

   for (const face_bin& bin : face_bins) {
      for (uint32 next_face = bin.first_face; next_face != noindex;) {
         const face& face = faces[next_face];

         const uint32 index_begin = static_cast<uint32>(mesh.face_indices.size());
         uint8 index_count = 0;

         for (uint32 next_edge = face.first_edge;;) {
            const half_edge& edge = half_edges[next_edge];

            mesh.face_indices.push_back(edge.vertex);

            next_edge = edge.next_edge;

            index_count += 1;

            if (edge.next_edge == face.first_edge) break;
         }

         mesh.faces.push_back({.index_begin = index_begin, .index_count = index_count});

         next_face = face.next_face;
      }
   }

   // Build a new vertex buffer using only used vertices.
   mesh.vertices.reserve(reduced_vertices.vertices.size());

   // Reuse storage from initial remap to avoid allocating.
   std::vector<uint32>& vertex_final_remap = reduced_vertices.remap;
   vertex_final_remap.clear();
   vertex_final_remap.resize(vertices.size(), noindex);

   for (uint16& vertex_index : mesh.face_indices) {
      uint32& new_vertex_index = vertex_final_remap[vertex_index];

      if (new_vertex_index == noindex) {
         new_vertex_index = static_cast<uint32>(mesh.vertices.size());
         mesh.vertices.push_back(vertices[vertex_index]);
      }

      vertex_index = static_cast<uint16>(new_vertex_index);
   }

   return mesh;
}

}