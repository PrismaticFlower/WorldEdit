#include "build_shadow_segments.hpp"
#include "error.hpp"
#include "triangulate_polygon.hpp"

#include "math/vector_funcs.hpp"

#include "math/plane_funcs.hpp"

#include <bit>

#include <absl/container/flat_hash_map.h>

#include <fmt/format.h>

namespace we::munge {

namespace {

const uint32 noindex = UINT32_MAX;

// Used when comparing vertex positions while building half edge structure. Snaps comparisions to around 1mm.
constexpr double position_similarity_precision = 1000.0;

struct hashable_f3 {
   hashable_f3(const float3& v)
   {
      _x = (v.x == 0.0f or v.x != v.x) ? 0 : std::bit_cast<uint32>(v.x);
      _y = (v.y == 0.0f or v.y != v.y) ? 0 : std::bit_cast<uint32>(v.y);
      _z = (v.z == 0.0f or v.z != v.z) ? 0 : std::bit_cast<uint32>(v.z);
   }

   bool operator==(const hashable_f3&) const noexcept = default;

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

struct connectivity_edge_list {
   auto begin() const noexcept -> const uint32*
   {
      return _edges.data();
   }

   auto end() const noexcept -> const uint32*
   {
      return _edges.data() + _edge_count;
   }

   void remove(const uint32 edge_index) noexcept
   {
      if (_edges[0] == edge_index) {
         _edges[0] = _edges[1];
         _edges[1] = noindex;
         _edge_count -= 1;
      }
      else if (_edges[1] == edge_index) {
         _edges[1] = noindex;
         _edge_count -= 1;
      }
   }

   bool add(const uint32 edge_index) noexcept
   {
      if (_edge_count >= 2) return false;

      _edges[_edge_count] = edge_index;
      _edge_count += 1;

      return true;
   }

private:
   uint32 _edge_count = 0;
   std::array<uint32, 2> _edges = {noindex, noindex};
};

struct half_edge {
   uint16 vertex = 0;
   uint32 previous_edge = noindex;
   uint32 next_edge = noindex;
   uint32 twin_edge = noindex;

   uint32 face = noindex;
};

struct shadow_face {
   uint32 first_edge = 0;
   uint32 vertex_count = 0;

   std::array<uint8, 3> normalSS = {};
};

struct shadow_mesh {
   std::size_t vertex_count = 0;

   std::unique_ptr<float3[]> positionSS;
   std::unique_ptr<float3[]> positionLS;
   std::unique_ptr<std::array<uint8, 3>[]> bone_weights;
   std::unique_ptr<std::array<uint8, 3>[]> bone_indices;

   std::vector<half_edge> half_edges;
   std::vector<shadow_face> faces;
};

struct shadow_silouhette_edge {
   std::array<uint32, 2> half_edges;
   std::array<std::array<uint8, 3>, 2> normalSS;
};

struct cached_unskinned_vertex {
   cached_unskinned_vertex(const model_shadow_unskinned_vertex& vertex)
      : _positionSS{vertex.positionSS}, _normalSS{vertex.normalSS}
   {
   }

   bool operator==(const cached_unskinned_vertex&) const noexcept = default;

   template<typename H>
   friend H AbslHashValue(H h, const cached_unskinned_vertex& cached)
   {
      return H::combine(std::move(h), cached._positionSS, cached._normalSS);
   }

private:
   hashable_f3 _positionSS;
   std::array<uint8, 3> _normalSS;
};

struct cached_hard_skinned_vertex {
   cached_hard_skinned_vertex(const model_shadow_hard_skinned_vertex& vertex)
      : _positionSS{vertex.positionSS[0], vertex.positionSS[1], vertex.positionSS[2]},
        _bone_indices{vertex.bone_indices}
   {
   }

   bool operator==(const cached_hard_skinned_vertex&) const noexcept = default;

   template<typename H>
   friend H AbslHashValue(H h, const cached_hard_skinned_vertex& cached)
   {
      return H::combine(std::move(h), cached._positionSS, cached._bone_indices);
   }

private:
   std::array<hashable_f3, 3> _positionSS;
   std::array<uint8, 3> _bone_indices;
};

struct cached_soft_skinned_vertex {
   cached_soft_skinned_vertex(const model_shadow_soft_skinned_vertex& vertex)
      : _positionSS{vertex.positionSS[0], vertex.positionSS[1], vertex.positionSS[2]},
        _bone_indices{vertex.bone_indices},
        _bone_weights{vertex.bone_weights}
   {
   }

   bool operator==(const cached_soft_skinned_vertex&) const noexcept = default;

   template<typename H>
   friend H AbslHashValue(H h, const cached_soft_skinned_vertex& cached)
   {
      return H::combine(std::move(h), cached._positionSS, cached._bone_indices,
                        cached._bone_weights);
   }

private:
   std::array<hashable_f3, 3> _positionSS;
   std::array<std::array<uint8, 3>, 3> _bone_indices;
   std::array<std::array<uint8, 3>, 3> _bone_weights;
};

struct segment_hard_skinned_build_data {
   absl::flat_hash_map<cached_hard_skinned_vertex, uint16> vertex_cache;
   std::vector<uint32> bone_remap;
};

struct segment_hard_skinned_build_context {
   std::vector<model_shadow> segments;
   std::vector<segment_hard_skinned_build_data> segments_data;

   uint32 max_bones = 15;
   const model_segment& input_segment;
   const shadow_mesh& shadow_mesh;
};

struct segment_soft_skinned_build_data {
   absl::flat_hash_map<cached_soft_skinned_vertex, uint16> vertex_cache;
   std::vector<uint32> bone_remap;
};

struct segment_soft_skinned_build_context {
   std::vector<model_shadow> segments;
   std::vector<segment_soft_skinned_build_data> segments_data;

   uint32 max_bones = 15;
   const model_segment& input_segment;
   const shadow_mesh& shadow_mesh;
};

auto snap_vertex_for_cache(const float3& v) -> float3
{
   return {
      static_cast<float>(std::round(v.x * position_similarity_precision) /
                         position_similarity_precision),
      static_cast<float>(std::round(v.y * position_similarity_precision) /
                         position_similarity_precision),
      static_cast<float>(std::round(v.z * position_similarity_precision) /
                         position_similarity_precision),
   };
}

/// @brief Fill in any missing normals of faces in the mesh. Used after closing the mesh.
/// @param mesh The mesh to operate on.
void fill_face_normals(shadow_mesh& mesh)
{
   std::vector<uint32> degenerate_faces;

   for (std::size_t i = 0; i < mesh.faces.size(); ++i) {
      shadow_face& face = mesh.faces[i];

      if (face.normalSS != std::array<uint8, 3>{}) continue;

      const half_edge& h0 = mesh.half_edges[face.first_edge];
      const half_edge& h1 = mesh.half_edges[h0.next_edge];
      const half_edge& h2 = mesh.half_edges[h0.previous_edge];

      const float3& v0 = mesh.positionSS[h0.vertex];
      const float3& v1 = mesh.positionSS[h1.vertex];
      const float3& v2 = mesh.positionSS[h2.vertex];

      float3 normalSS = cross(v1 - v0, v2 - v0);

      if (normalSS == float3{}) {
         degenerate_faces.push_back(static_cast<uint32>(i));
      }

      normalSS = normalize(normalSS);

      face.normalSS = {
         static_cast<uint8>(normalSS.x * 127.5f + 127.5f),
         static_cast<uint8>(normalSS.y * 127.5f + 127.5f),
         static_cast<uint8>(normalSS.z * 127.5f + 127.5f),
      };
   }

   if (degenerate_faces.empty()) return;

   // Simple algorithm for filling the normals of degenerate triangles. Has O(n*m) worst case complexity,
   // where n is the number of degenerate tris and m is the total number of triangles.
   //
   // Faster algorithms doubtlessly exists but degenerate triangles will be very rare
   // as inputs and in the common case this one should run fast enough.

   std::vector<bool> visited_faces;
   std::vector<uint32> search_stack;
   search_stack.reserve(32);

   while (not degenerate_faces.empty()) {
      visited_faces.clear();
      visited_faces.resize(mesh.faces.size());

      const uint32 face_index = degenerate_faces.back();

      degenerate_faces.pop_back();

      search_stack.clear();
      search_stack.push_back(face_index);

      while (not search_stack.empty()) {
         const uint32 search_face_index = search_stack.back();

         search_stack.pop_back();

         if (visited_faces[search_face_index]) continue;

         visited_faces[search_face_index] = true;

         const shadow_face& search_face = mesh.faces[search_face_index];

         for (uint32 next_edge_index = search_face.first_edge;;) {
            const half_edge& edge = mesh.half_edges[next_edge_index];
            const half_edge& twin_edge = mesh.half_edges[edge.twin_edge];

            if (visited_faces[twin_edge.face]) continue;

            const shadow_face& twin_face = mesh.faces[twin_edge.face];

            if (twin_face.normalSS != std::array<uint8, 3>{}) {
               mesh.faces[face_index].normalSS = twin_face.normalSS;

               search_stack.clear();

               break;
            }

            search_stack.push_back(twin_edge.face);

            next_edge_index = edge.next_edge;

            if (next_edge_index == search_face.first_edge) break;
         }
      }
   }
}

/// @brief Build an open shadow mesh. This fallback path creates a back facing copy of each triangle in the mesh. This is expensive and
/// only used when closing the mesh fails.
/// @param segment The input model segment.
/// @param context Context for outputting warnings.
/// @return The built shadow mesh.
auto build_shadow_mesh_open_fallback(const model_segment& segment,
                                     const build_shadow_segments_context& context)
   -> shadow_mesh
{
   shadow_mesh mesh;

   mesh.half_edges.reserve(segment.index_buffer.size() * 6);
   mesh.faces.reserve(segment.index_buffer.size() * 2);

   mesh.positionSS =
      std::make_unique_for_overwrite<float3[]>(segment.vertices.vertex_count);
   mesh.positionLS =
      std::make_unique_for_overwrite<float3[]>(segment.vertices.vertex_count);

   if (segment.vertices.bone_weights) {
      mesh.bone_weights = std::make_unique_for_overwrite<std::array<uint8, 3>[]>(
         segment.vertices.vertex_count);
   }

   if (segment.vertices.bone_indices) {
      mesh.bone_indices = std::make_unique_for_overwrite<std::array<uint8, 3>[]>(
         segment.vertices.vertex_count);
   }

   absl::flat_hash_map<hashable_f3, uint16> vertex_cache;
   vertex_cache.reserve(segment.vertices.vertex_count);

   std::vector<uint32> vertex_remap;
   vertex_remap.resize(segment.vertices.vertex_count, noindex);

   for (std::array<uint16, 3> tri : segment.index_buffer) {
      const float3& v0 = segment.vertices.positionSS[tri[0]];
      const float3& v1 = segment.vertices.positionSS[tri[1]];
      const float3& v2 = segment.vertices.positionSS[tri[2]];

      float3 normalSS = cross(v1 - v0, v2 - v0);

      if (normalSS == float3{}) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message = fmt::format(
                "Discarding degenerate triangle from shadow mesh.\n\n{}",
                get_descriptive_message(
                   model_wc::model_shadow_mesh_discarded_degenerate_triangle))});

         continue;
      }

      normalSS = normalize(normalSS);

      for (uint16& vertex_index : tri) {
         if (vertex_remap[vertex_index] == noindex) {
            std::array<uint8, 3> bone_weights = {};

            if (segment.vertices.bone_weights) {
               const float3& flt_bone_weights =
                  segment.vertices.bone_weights[vertex_index];

               bone_weights[0] =
                  static_cast<uint8>(flt_bone_weights.x * 255.0f + 0.5f);
               bone_weights[1] =
                  static_cast<uint8>(flt_bone_weights.y * 255.0f + 0.5f);

               if (bone_weights[0] + bone_weights[1] > 255) {
                  bone_weights[1] = 255 - bone_weights[1];
               }

               bone_weights[2] =
                  static_cast<uint8>((255u - bone_weights[0] - bone_weights[1]));
            }

            auto [it, inserted] =
               vertex_cache.try_emplace(snap_vertex_for_cache(
                                           segment.vertices.positionSS[vertex_index]),
                                        static_cast<uint16>(mesh.vertex_count));

            const uint16 new_vertex_index = it->second;

            if (inserted) {
               mesh.positionSS[new_vertex_index] =
                  segment.vertices.positionSS[vertex_index];
               mesh.positionLS[new_vertex_index] =
                  segment.vertices.positionLS[vertex_index];

               if (segment.vertices.bone_weights) {
                  mesh.bone_weights[new_vertex_index] = bone_weights;
               }

               if (segment.vertices.bone_indices) {
                  mesh.bone_indices[new_vertex_index] =
                     segment.vertices.bone_indices[vertex_index];
               }

               mesh.vertex_count += 1;
            }
            else {
               if (mesh.positionSS[new_vertex_index] !=
                   segment.vertices.positionSS[vertex_index]) {
                  context.feedback.add_warning(
                     {.file = context.path,
                      .tool = "ModelMunge",
                      .message =
                         fmt::format("Merged non-identical vertices. V0: {{{}, "
                                     "{}, {}}} V1: {{{}, {}, {}}} \n\n{}",
                                     mesh.positionSS[new_vertex_index].x,
                                     mesh.positionSS[new_vertex_index].y,
                                     mesh.positionSS[new_vertex_index].z,
                                     segment.vertices.positionSS[vertex_index].x,
                                     segment.vertices.positionSS[vertex_index].y,
                                     segment.vertices.positionSS[vertex_index].z,
                                     get_descriptive_message(
                                        model_wc::model_shadow_mesh_merged_nonidentical_vertices))});
               }

               if (segment.vertices.bone_weights and
                   mesh.bone_weights[new_vertex_index] != bone_weights) {
                  context.feedback.add_warning(
                     {.file = context.path,
                      .tool = "ModelMunge",
                      .message = fmt::format(
                         "Merged vertices with non-identical bone weights. "
                         "V0: "
                         "{{{:d}, {:d}, {:d}}} (Position: {{{}, {}, {}}}) "
                         "V1: "
                         "{{{:d}, {:d}, {:d}}} (Position: {{{}, {}, {}}}) "
                         "\n\n{}",
                         mesh.bone_weights[new_vertex_index][0],
                         mesh.bone_weights[new_vertex_index][1],
                         mesh.bone_weights[new_vertex_index][2],
                         mesh.positionSS[new_vertex_index].x,
                         mesh.positionSS[new_vertex_index].y,
                         mesh.positionSS[new_vertex_index].z, bone_weights[0],
                         bone_weights[1], bone_weights[2],
                         segment.vertices.positionSS[vertex_index].x,
                         segment.vertices.positionSS[vertex_index].y,
                         segment.vertices.positionSS[vertex_index].z,
                         get_descriptive_message(
                            model_wc::model_shadow_mesh_merged_nonidentical_vertices_bone_weights))});
               }

               if (segment.vertices.bone_indices and
                   mesh.bone_indices[new_vertex_index] !=
                      segment.vertices.bone_indices[vertex_index]) {
                  context.feedback.add_warning(
                     {.file = context.path,
                      .tool = "ModelMunge",
                      .message = fmt::format(
                         "Merged vertices with non-identical bone indices. "
                         "V0: "
                         "{{{:d}, {:d}, {:d}}} (Position: {{{}, {}, {}}}) "
                         "V1: "
                         "{{{:d}, {:d}, {:d}}} (Position: {{{}, {}, {}}}) "
                         "\n\n{}",
                         mesh.bone_indices[new_vertex_index][0],
                         mesh.bone_indices[new_vertex_index][1],
                         mesh.bone_indices[new_vertex_index][2],
                         mesh.positionSS[new_vertex_index].x,
                         mesh.positionSS[new_vertex_index].y,
                         mesh.positionSS[new_vertex_index].z,
                         segment.vertices.bone_indices[vertex_index][0],
                         segment.vertices.bone_indices[vertex_index][1],
                         segment.vertices.bone_indices[vertex_index][2],
                         segment.vertices.positionSS[vertex_index].x,
                         segment.vertices.positionSS[vertex_index].y,
                         segment.vertices.positionSS[vertex_index].z,
                         get_descriptive_message(
                            model_wc::model_shadow_mesh_merged_nonidentical_vertices_bone_indices))});
               }
            }

            vertex_remap[vertex_index] = new_vertex_index;
            vertex_index = new_vertex_index;
         }
         else {
            vertex_index = static_cast<uint16>(vertex_remap[vertex_index]);
         }
      }

      const uint32 front_face_index = static_cast<uint32>(mesh.faces.size());
      const uint32 back_face_index = front_face_index + 1;

      const uint32 first_front_half_edge_index =
         static_cast<uint32>(mesh.half_edges.size());

      const uint32 first_back_half_edge_index = first_front_half_edge_index + 3;

      mesh.half_edges.push_back({
         .vertex = tri[0],
         .previous_edge = first_front_half_edge_index + 2,
         .next_edge = first_front_half_edge_index + 1,
         .twin_edge = first_back_half_edge_index + 1,

         .face = front_face_index,
      });
      mesh.half_edges.push_back({
         .vertex = tri[1],
         .previous_edge = first_front_half_edge_index,
         .next_edge = first_front_half_edge_index + 2,
         .twin_edge = first_back_half_edge_index,

         .face = front_face_index,
      });
      mesh.half_edges.push_back({
         .vertex = tri[2],
         .previous_edge = first_front_half_edge_index + 1,
         .next_edge = first_front_half_edge_index,
         .twin_edge = first_back_half_edge_index + 2,

         .face = front_face_index,
      });

      mesh.half_edges.push_back({
         .vertex = tri[2],
         .previous_edge = first_back_half_edge_index + 2,
         .next_edge = first_back_half_edge_index + 1,
         .twin_edge = first_front_half_edge_index + 1,

         .face = back_face_index,
      });
      mesh.half_edges.push_back({
         .vertex = tri[1],
         .previous_edge = first_back_half_edge_index,
         .next_edge = first_back_half_edge_index + 2,
         .twin_edge = first_front_half_edge_index,

         .face = back_face_index,
      });
      mesh.half_edges.push_back({
         .vertex = tri[0],
         .previous_edge = first_back_half_edge_index + 1,
         .next_edge = first_back_half_edge_index,
         .twin_edge = first_front_half_edge_index + 2,

         .face = back_face_index,
      });

      mesh.faces.push_back({
         .first_edge = first_front_half_edge_index,
         .vertex_count = 3,
         .normalSS =
            {
               static_cast<uint8>(normalSS.x * 127.5f + 127.5f),
               static_cast<uint8>(normalSS.y * 127.5f + 127.5f),
               static_cast<uint8>(normalSS.z * 127.5f + 127.5f),
            },
      });

      mesh.faces.push_back({
         .first_edge = first_back_half_edge_index,
         .vertex_count = 3,
         .normalSS =
            {
               static_cast<uint8>(normalSS.x * -127.5f + 127.5f),
               static_cast<uint8>(normalSS.y * -127.5f + 127.5f),
               static_cast<uint8>(normalSS.z * -127.5f + 127.5f),
            },
      });
   }

   return mesh;
}

/// @brief Builds a shadow mesh, closing it if needed (and possible). Falls back to build_shadow_mesh_open_fallback on failure.
/// @param segment The input model segment.
/// @param context Context for outputting warnings.
/// @return The built shadow mesh.
auto build_shadow_mesh(const model_segment& segment,
                       const build_shadow_segments_context& context) -> shadow_mesh
{
   shadow_mesh mesh;

   mesh.half_edges.reserve(segment.index_buffer.size() * 3);
   mesh.faces.reserve(segment.index_buffer.size());

   mesh.positionSS =
      std::make_unique_for_overwrite<float3[]>(segment.vertices.vertex_count);
   mesh.positionLS =
      std::make_unique_for_overwrite<float3[]>(segment.vertices.vertex_count);

   if (segment.vertices.bone_weights) {
      mesh.bone_weights = std::make_unique_for_overwrite<std::array<uint8, 3>[]>(
         segment.vertices.vertex_count);
   }

   if (segment.vertices.bone_indices) {
      mesh.bone_indices = std::make_unique_for_overwrite<std::array<uint8, 3>[]>(
         segment.vertices.vertex_count);
   }

   absl::flat_hash_map<hashable_f3, uint16> vertex_cache;
   vertex_cache.reserve(segment.vertices.vertex_count);

   std::vector<uint32> vertex_remap;
   vertex_remap.resize(segment.vertices.vertex_count, noindex);

   for (std::array<uint16, 3> tri : segment.index_buffer) {
      const float3& v0 = segment.vertices.positionSS[tri[0]];
      const float3& v1 = segment.vertices.positionSS[tri[1]];
      const float3& v2 = segment.vertices.positionSS[tri[2]];

      float3 normalSS = cross(v1 - v0, v2 - v0);

      if (normalSS == float3{}) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message = fmt::format(
                "Discarding degenerate triangle from shadow mesh.\n\n{}",
                get_descriptive_message(
                   model_wc::model_shadow_mesh_discarded_degenerate_triangle))});

         continue;
      }

      normalSS = normalize(normalSS);

      for (uint16& vertex_index : tri) {
         if (vertex_remap[vertex_index] == noindex) {
            std::array<uint8, 3> bone_weights = {};

            if (segment.vertices.bone_weights) {
               const float3& flt_bone_weights =
                  segment.vertices.bone_weights[vertex_index];

               bone_weights[0] =
                  static_cast<uint8>(flt_bone_weights.x * 255.0f + 0.5f);
               bone_weights[1] =
                  static_cast<uint8>(flt_bone_weights.y * 255.0f + 0.5f);

               if (bone_weights[0] + bone_weights[1] > 255) {
                  bone_weights[1] = 255 - bone_weights[1];
               }

               bone_weights[2] =
                  static_cast<uint8>((255u - bone_weights[0] - bone_weights[1]));
            }

            auto [it, inserted] =
               vertex_cache.try_emplace(snap_vertex_for_cache(
                                           segment.vertices.positionSS[vertex_index]),
                                        static_cast<uint16>(mesh.vertex_count));

            const uint16 new_vertex_index = it->second;

            if (inserted) {
               mesh.positionSS[new_vertex_index] =
                  segment.vertices.positionSS[vertex_index];
               mesh.positionLS[new_vertex_index] =
                  segment.vertices.positionLS[vertex_index];

               if (segment.vertices.bone_weights) {
                  mesh.bone_weights[new_vertex_index] = bone_weights;
               }

               if (segment.vertices.bone_indices) {
                  mesh.bone_indices[new_vertex_index] =
                     segment.vertices.bone_indices[vertex_index];
               }

               mesh.vertex_count += 1;
            }
            else {
               if (mesh.positionSS[new_vertex_index] !=
                   segment.vertices.positionSS[vertex_index]) {
                  context.feedback.add_warning(
                     {.file = context.path,
                      .tool = "ModelMunge",
                      .message =
                         fmt::format("Merged non-identical vertices. V0: {{{}, "
                                     "{}, {}}} V1: {{{}, {}, {}}} \n\n{}",
                                     mesh.positionSS[new_vertex_index].x,
                                     mesh.positionSS[new_vertex_index].y,
                                     mesh.positionSS[new_vertex_index].z,
                                     segment.vertices.positionSS[vertex_index].x,
                                     segment.vertices.positionSS[vertex_index].y,
                                     segment.vertices.positionSS[vertex_index].z,
                                     get_descriptive_message(
                                        model_wc::model_shadow_mesh_merged_nonidentical_vertices))});
               }

               if (segment.vertices.bone_weights and
                   mesh.bone_weights[new_vertex_index] != bone_weights) {
                  context.feedback.add_warning(
                     {.file = context.path,
                      .tool = "ModelMunge",
                      .message = fmt::format(
                         "Merged vertices with non-identical bone weights. "
                         "V0: "
                         "{{{:d}, {:d}, {:d}}} (Position: {{{}, {}, {}}}) "
                         "V1: "
                         "{{{:d}, {:d}, {:d}}} (Position: {{{}, {}, {}}}) "
                         "\n\n{}",
                         mesh.bone_weights[new_vertex_index][0],
                         mesh.bone_weights[new_vertex_index][1],
                         mesh.bone_weights[new_vertex_index][2],
                         mesh.positionSS[new_vertex_index].x,
                         mesh.positionSS[new_vertex_index].y,
                         mesh.positionSS[new_vertex_index].z, bone_weights[0],
                         bone_weights[1], bone_weights[2],
                         segment.vertices.positionSS[vertex_index].x,
                         segment.vertices.positionSS[vertex_index].y,
                         segment.vertices.positionSS[vertex_index].z,
                         get_descriptive_message(
                            model_wc::model_shadow_mesh_merged_nonidentical_vertices_bone_weights))});
               }

               if (segment.vertices.bone_indices and
                   mesh.bone_indices[new_vertex_index] !=
                      segment.vertices.bone_indices[vertex_index]) {
                  context.feedback.add_warning(
                     {.file = context.path,
                      .tool = "ModelMunge",
                      .message = fmt::format(
                         "Merged vertices with non-identical bone indices. "
                         "V0: "
                         "{{{:d}, {:d}, {:d}}} (Position: {{{}, {}, {}}}) "
                         "V1: "
                         "{{{:d}, {:d}, {:d}}} (Position: {{{}, {}, {}}}) "
                         "\n\n{}",
                         mesh.bone_indices[new_vertex_index][0],
                         mesh.bone_indices[new_vertex_index][1],
                         mesh.bone_indices[new_vertex_index][2],
                         mesh.positionSS[new_vertex_index].x,
                         mesh.positionSS[new_vertex_index].y,
                         mesh.positionSS[new_vertex_index].z,
                         segment.vertices.bone_indices[vertex_index][0],
                         segment.vertices.bone_indices[vertex_index][1],
                         segment.vertices.bone_indices[vertex_index][2],
                         segment.vertices.positionSS[vertex_index].x,
                         segment.vertices.positionSS[vertex_index].y,
                         segment.vertices.positionSS[vertex_index].z,
                         get_descriptive_message(
                            model_wc::model_shadow_mesh_merged_nonidentical_vertices_bone_indices))});
               }
            }

            vertex_remap[vertex_index] = new_vertex_index;
            vertex_index = new_vertex_index;
         }
         else {
            vertex_index = static_cast<uint16>(vertex_remap[vertex_index]);
         }
      }

      const uint32 face_index = static_cast<uint32>(mesh.faces.size());
      const uint32 first_half_edge_index =
         static_cast<uint32>(mesh.half_edges.size());

      mesh.half_edges.push_back({
         .vertex = tri[0],
         .previous_edge = first_half_edge_index + 2,
         .next_edge = first_half_edge_index + 1,
         .twin_edge = noindex,

         .face = static_cast<uint32>(face_index),
      });
      mesh.half_edges.push_back({
         .vertex = tri[1],
         .previous_edge = first_half_edge_index,
         .next_edge = first_half_edge_index + 2,
         .twin_edge = noindex,

         .face = static_cast<uint32>(face_index),
      });
      mesh.half_edges.push_back({
         .vertex = tri[2],
         .previous_edge = first_half_edge_index + 1,
         .next_edge = first_half_edge_index,
         .twin_edge = noindex,

         .face = static_cast<uint32>(face_index),
      });

      mesh.faces.push_back({
         .first_edge = first_half_edge_index,
         .vertex_count = 3,
         .normalSS =
            {
               static_cast<uint8>(normalSS.x * 127.5f + 127.5f),
               static_cast<uint8>(normalSS.y * 127.5f + 127.5f),
               static_cast<uint8>(normalSS.z * 127.5f + 127.5f),
            },
      });
   }

   absl::flat_hash_map<std::array<uint16, 2>, connectivity_edge_list> edge_twin_map;
   edge_twin_map.reserve(mesh.half_edges.size());

   std::size_t closed_edge_count = 0;

   for (std::size_t edge_index = 0; edge_index < mesh.half_edges.size(); ++edge_index) {
      half_edge& edge = mesh.half_edges[edge_index];
      const half_edge& next_edge = mesh.half_edges[edge.next_edge];

      if (edge.twin_edge != noindex) continue;

      const std::array<uint16, 2> vertices = {edge.vertex, next_edge.vertex};
      const std::array<uint16, 2> twin_vertices = {vertices[1], vertices[0]};

      if (auto twin_it = edge_twin_map.find(twin_vertices);
          twin_it != edge_twin_map.end()) {
         const shadow_face& face = mesh.faces[edge.face];

         connectivity_edge_list& candidate_twin_edge_list = twin_it->second;

         uint32 best_twin_edge_candidate = noindex;
         int32 best_twin_edge_distance_sq = INT32_MAX;

         for (uint32 twin_edge_candidate_index : candidate_twin_edge_list) {
            half_edge& candidate_edge = mesh.half_edges[twin_edge_candidate_index];

            const shadow_face& candidate_face = mesh.faces[candidate_edge.face];

            const std::array<int32, 3> normal_difference = {
               face.normalSS[0] - candidate_face.normalSS[0],
               face.normalSS[1] - candidate_face.normalSS[1],
               face.normalSS[2] - candidate_face.normalSS[2],
            };

            const int32 candidate_distance_sq =
               (normal_difference[0] * normal_difference[0]) +
               (normal_difference[1] * normal_difference[1]) +
               (normal_difference[2] * normal_difference[2]);

            if (candidate_distance_sq < best_twin_edge_distance_sq) {
               best_twin_edge_candidate = twin_edge_candidate_index;
               best_twin_edge_distance_sq = candidate_distance_sq;
            }
         }

         if (best_twin_edge_candidate != noindex) {
            half_edge& twin_edge = mesh.half_edges[best_twin_edge_candidate];

            assert(twin_edge.twin_edge == noindex);

            edge.twin_edge = best_twin_edge_candidate;
            twin_edge.twin_edge = static_cast<uint32>(edge_index);

            closed_edge_count += 2;

            candidate_twin_edge_list.remove(best_twin_edge_candidate);

            continue;
         }
      }

      if (not edge_twin_map[vertices].add(static_cast<uint32>(edge_index))) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message =
                fmt::format("Non-manifold edge detected!\n\n{}",
                            get_descriptive_message(
                               model_wc::model_shadow_mesh_non_manifold_edge_detected))});
      }
   }

   if (closed_edge_count != mesh.half_edges.size()) {
      const std::size_t open_edge_count = mesh.half_edges.size() - closed_edge_count;

      std::vector<uint32> open_edges;
      open_edges.reserve(open_edge_count);

      for (std::size_t edge_index = 0; edge_index < mesh.half_edges.size();
           ++edge_index) {
         const half_edge& edge = mesh.half_edges[edge_index];

         if (edge.twin_edge == noindex) {
            open_edges.push_back(static_cast<uint32>(edge_index));
         }
      }

      assert(open_edges.size() == open_edge_count);

      std::vector<std::vector<uint32>> close_faces;
      std::vector<uint32> vertex_open_edge_map;
      std::vector<bool> visited_open_edges;

      while (not open_edges.empty()) {
         vertex_open_edge_map.clear();
         vertex_open_edge_map.resize(mesh.vertex_count, noindex);

         for (std::size_t i = 0; i < open_edges.size(); ++i) {
            const uint32 edge_index = open_edges[i];
            const half_edge& edge = mesh.half_edges[edge_index];

            vertex_open_edge_map[edge.vertex] = static_cast<uint32>(i);
         }

         visited_open_edges.clear();
         visited_open_edges.resize(open_edges.size());

         std::vector<uint32> face;
         face.reserve(16);

         for (uint32 open_edge_index = 0;;) {
            if (visited_open_edges[open_edge_index]) {
               context.feedback.add_warning(
                  {.file = context.path,
                   .tool = "ModelMunge",
                   .message =
                      fmt::format("Failed to close shadow mesh!\n\n{}",
                                  get_descriptive_message(
                                     model_wc::model_shadow_mesh_close_unable_to_loop_back_to_start))});

               return build_shadow_mesh_open_fallback(segment, context);
            }

            visited_open_edges[open_edge_index] = true;

            const uint32 edge_index = open_edges[open_edge_index];
            const half_edge& edge = mesh.half_edges[edge_index];

            face.push_back(edge.vertex);

            const half_edge& next_edge = mesh.half_edges[edge.next_edge];

            if (vertex_open_edge_map[next_edge.vertex] == noindex) {
               context.feedback.add_warning(
                  {.file = context.path,
                   .tool = "ModelMunge",
                   .message =
                      fmt::format("Failed to close shadow mesh!\n\n{}",
                                  get_descriptive_message(
                                     model_wc::model_shadow_mesh_close_unable_to_loop_back_to_start))});

               return build_shadow_mesh_open_fallback(segment, context);
            }

            open_edge_index = vertex_open_edge_map[next_edge.vertex];

            if (open_edge_index == 0) break;
         }

         for (std::ptrdiff_t i = std::ssize(visited_open_edges) - 1; i >= 0; --i) {
            if (visited_open_edges[i]) {
               if (i != std::ssize(visited_open_edges)) {
                  std::swap(open_edges[i], open_edges.back());
               }

               open_edges.pop_back();
            }
         }

         std::reverse(face.begin(), face.end());

         close_faces.push_back(std::move(face));
      }

      for (const std::vector<uint32>& close_face : close_faces) {
         const std::vector<std::array<uint32, 3>> face_triangles =
            triangulate_polygon({mesh.positionSS.get(), mesh.vertex_count}, close_face);

         for (const std::array<uint32, 3>& tri : face_triangles) {
            const uint32 face_index = static_cast<uint32>(mesh.faces.size());

            shadow_face& face = mesh.faces.emplace_back();

            const uint32 first_half_edge_index =
               static_cast<uint32>(mesh.half_edges.size());

            mesh.half_edges.push_back({
               .vertex = static_cast<uint16>(tri[0]),
               .previous_edge = first_half_edge_index + 2,
               .next_edge = first_half_edge_index + 1,
               .twin_edge = noindex,

               .face = face_index,
            });
            mesh.half_edges.push_back({
               .vertex = static_cast<uint16>(tri[1]),
               .previous_edge = first_half_edge_index,
               .next_edge = first_half_edge_index + 2,
               .twin_edge = noindex,

               .face = face_index,
            });
            mesh.half_edges.push_back({
               .vertex = static_cast<uint16>(tri[2]),
               .previous_edge = first_half_edge_index + 1,
               .next_edge = first_half_edge_index,
               .twin_edge = noindex,

               .face = face_index,
            });

            face.first_edge = first_half_edge_index;
            face.vertex_count = 3;
         }
      }

      for (std::size_t edge_index = 0; edge_index < mesh.half_edges.size();
           ++edge_index) {
         half_edge& edge = mesh.half_edges[edge_index];
         const half_edge& next_edge = mesh.half_edges[edge.next_edge];

         if (edge.twin_edge != noindex) continue;

         const std::array<uint16, 2> vertices = {edge.vertex, next_edge.vertex};
         const std::array<uint16, 2> twin_vertices = {vertices[1], vertices[0]};

         if (auto twin_it = edge_twin_map.find(twin_vertices);
             twin_it != edge_twin_map.end()) {
            const shadow_face& face = mesh.faces[edge.face];

            connectivity_edge_list& candidate_twin_edge_list = twin_it->second;

            uint32 best_twin_edge_candidate = noindex;
            int32 best_twin_edge_distance_sq = INT32_MAX;

            for (uint32 twin_edge_candidate_index : candidate_twin_edge_list) {
               half_edge& candidate_edge = mesh.half_edges[twin_edge_candidate_index];

               const shadow_face& candidate_face = mesh.faces[candidate_edge.face];

               const std::array<int32, 3> normal_difference = {
                  face.normalSS[0] - candidate_face.normalSS[0],
                  face.normalSS[1] - candidate_face.normalSS[1],
                  face.normalSS[2] - candidate_face.normalSS[2],
               };

               const int32 candidate_distance_sq =
                  (normal_difference[0] * normal_difference[0]) +
                  (normal_difference[1] * normal_difference[1]) +
                  (normal_difference[2] * normal_difference[2]);

               if (candidate_distance_sq < best_twin_edge_distance_sq) {
                  best_twin_edge_candidate = twin_edge_candidate_index;
                  best_twin_edge_distance_sq = candidate_distance_sq;
               }
            }

            if (best_twin_edge_candidate != noindex) {
               half_edge& twin_edge = mesh.half_edges[best_twin_edge_candidate];

               assert(twin_edge.twin_edge == noindex);

               edge.twin_edge = best_twin_edge_candidate;
               twin_edge.twin_edge = static_cast<uint32>(edge_index);

               closed_edge_count += 2;

               candidate_twin_edge_list.remove(best_twin_edge_candidate);

               continue;
            }
         }

         if (not edge_twin_map[vertices].add(static_cast<uint32>(edge_index))) {
            context.feedback.add_warning(
               {.file = context.path,
                .tool = "ModelMunge",
                .message =
                   fmt::format("Non-manifold edge detected!\n\n{}",
                               get_descriptive_message(
                                  model_wc::model_shadow_mesh_non_manifold_edge_detected))});
         }
      }

      if (closed_edge_count != mesh.half_edges.size()) {
         context.feedback.add_warning(
            {.file = context.path,
             .tool = "ModelMunge",
             .message =
                fmt::format("Failed to close shadow mesh!\n\n{}",
                            get_descriptive_message(
                               model_wc::model_shadow_mesh_close_still_open_after_fill))});

         return build_shadow_mesh_open_fallback(segment, context);
      }

      fill_face_normals(mesh);
   }

   return mesh;
}

/// @brief Build the list of potential silouhette edges for a shadow mesh.
/// @param mesh The shadow mesh.
/// @return The silouhette edge list.
auto build_shadow_silouhette(const shadow_mesh& mesh)
   -> std::vector<shadow_silouhette_edge>
{
   std::vector<bool> visited_edges;
   visited_edges.resize(mesh.half_edges.size());

   std::vector<shadow_silouhette_edge> silouhette_edges;
   silouhette_edges.reserve(mesh.half_edges.size());

   for (const shadow_face& face : mesh.faces) {
      for (uint32 edge_index = face.first_edge;;) {
         const half_edge& edge = mesh.half_edges[edge_index];
         const half_edge& twin_edge = mesh.half_edges[edge.twin_edge];

         const shadow_face& twin_face = mesh.faces[twin_edge.face];

         if (visited_edges[edge_index]) goto next_edge;

         visited_edges[edge_index] = true;

         if (visited_edges[edge.twin_edge]) goto next_edge;

         visited_edges[edge.twin_edge] = true;

         if (not mesh.bone_indices and face.normalSS == twin_face.normalSS) {
            goto next_edge;
         }

         silouhette_edges.push_back({
            .half_edges = {edge_index, edge.twin_edge},
            .normalSS = {face.normalSS, twin_face.normalSS},
         });

      next_edge:
         edge_index = edge.next_edge;

         if (edge_index == face.first_edge) break;
      }
   }

   return silouhette_edges;
}

/// @brief Check if a segment has space for a shadow face to be added to it.
/// @param face The face.
/// @param segment The segment.
/// @return If the face could be added to the segment or not.
bool segment_has_space_for_face_unskinned(const shadow_face& face,
                                          const model_shadow& segment)
{
   const std::size_t segment_split_threshold = 0x10000 - face.vertex_count;

   return segment.vertices.unskinned.size() <= segment_split_threshold;
}

/// @brief Check if a segment has space for a shadow edge to be added to it.
/// @param face The face.
/// @param segment The segment.
/// @return If the edge could be added to the segment or not.
bool segment_has_space_for_edge_unskinned(const model_shadow& segment)
{
   const std::size_t segment_split_threshold = 0x10000 - 4;

   return segment.vertices.unskinned.size() <= segment_split_threshold;
}

bool try_add_triangle_to_segment(std::array<model_shadow_hard_skinned_vertex, 3> tri,
                                 model_shadow& segment,
                                 segment_hard_skinned_build_data& build_data,
                                 segment_hard_skinned_build_context& context)
{
   const std::size_t segment_split_threshold = 0x10000 - 3;

   if (segment.vertices.hard_skinned.size() > segment_split_threshold) {
      return false;
   }

   std::size_t missing_bones = 0;
   std::array<bool, 256> counted_bones = {};

   for (const model_shadow_hard_skinned_vertex& vertex : tri) {
      for (const uint8 bone_index : vertex.bone_indices) {
         if (not counted_bones[bone_index]) {
            missing_bones += build_data.bone_remap[bone_index] == noindex;
            counted_bones[bone_index] = true;
         }
      }
   }

   if (segment.bone_map.size() + missing_bones > context.max_bones) {
      return false;
   }

   std::array<uint16, 3> output_tri = {};

   for (std::size_t i = 0; i < output_tri.size(); ++i) {
      model_shadow_hard_skinned_vertex& vertex = tri[i];

      for (uint8& bone_index : vertex.bone_indices) {
         if (build_data.bone_remap[bone_index] == noindex) {
            build_data.bone_remap[bone_index] =
               static_cast<uint32>(segment.bone_map.size());

            segment.bone_map.push_back(context.input_segment.bone_map[bone_index]);
         }

         bone_index = static_cast<uint8>(build_data.bone_remap[bone_index]);
      }

      auto [it, inserted] =
         build_data.vertex_cache.try_emplace(vertex,
                                             static_cast<uint16>(
                                                segment.vertices.hard_skinned.size()));

      output_tri[i] = it->second;

      if (inserted) {
         segment.vertices.hard_skinned.push_back(vertex);
      }
   }

   segment.index_buffer.push_back(output_tri);

   return true;
}

bool try_add_triangle_to_segment(std::array<model_shadow_soft_skinned_vertex, 3> tri,
                                 model_shadow& segment,
                                 segment_soft_skinned_build_data& build_data,
                                 segment_soft_skinned_build_context& context)
{
   const std::size_t segment_split_threshold = 0x10000 - 3;

   if (segment.vertices.soft_skinned.size() > segment_split_threshold) {
      return false;
   }

   std::size_t missing_bones = 0;
   std::array<bool, 256> counted_bones = {};

   for (const model_shadow_soft_skinned_vertex& vertex : tri) {
      for (const std::array<uint8, 3>& bone_indices : vertex.bone_indices) {
         for (const uint8 bone_index : bone_indices) {
            if (not counted_bones[bone_index]) {
               missing_bones += build_data.bone_remap[bone_index] == noindex;
               counted_bones[bone_index] = true;
            }
         }
      }
   }

   if (segment.bone_map.size() + missing_bones > context.max_bones) {
      return false;
   }

   std::array<uint16, 3> output_tri = {};

   for (std::size_t i = 0; i < output_tri.size(); ++i) {
      model_shadow_soft_skinned_vertex& vertex = tri[i];

      for (std::array<uint8, 3>& bone_indices : vertex.bone_indices) {
         for (uint8& bone_index : bone_indices) {
            if (build_data.bone_remap[bone_index] == noindex) {
               build_data.bone_remap[bone_index] =
                  static_cast<uint32>(segment.bone_map.size());

               segment.bone_map.push_back(context.input_segment.bone_map[bone_index]);
            }

            bone_index = static_cast<uint8>(build_data.bone_remap[bone_index]);
         }
      }

      auto [it, inserted] =
         build_data.vertex_cache.try_emplace(vertex,
                                             static_cast<uint16>(
                                                segment.vertices.soft_skinned.size()));

      output_tri[i] = it->second;

      if (inserted) {
         segment.vertices.soft_skinned.push_back(vertex);
      }
   }

   segment.index_buffer.push_back(output_tri);

   return true;
}

void add_triangle_to_segments(const std::array<model_shadow_hard_skinned_vertex, 3>& tri,
                              segment_hard_skinned_build_context& context)
{
   for (std::size_t i = 0; i < context.segments.size(); ++i) {
      if (try_add_triangle_to_segment(tri, context.segments[i],
                                      context.segments_data[i], context)) {
         return;
      }
   }

   model_shadow& new_segment = context.segments.emplace_back();

   new_segment.bboxSS = context.input_segment.bboxSS;
   new_segment.bboxLS = context.input_segment.bboxLS;
   new_segment.bone_name = context.input_segment.bone_name;

   new_segment.index_buffer.reserve(context.shadow_mesh.faces.size());
   new_segment.vertices.type = model_shadow_vertex_type::hard_skinned;
   new_segment.vertices.hard_skinned.reserve(context.shadow_mesh.vertex_count);
   new_segment.bone_map.reserve(context.max_bones);

   segment_hard_skinned_build_data& new_build_data =
      context.segments_data.emplace_back();

   new_build_data.bone_remap.resize(context.input_segment.bone_map.size(), noindex);
   new_build_data.vertex_cache.reserve(new_segment.vertices.hard_skinned.capacity());

   if (not try_add_triangle_to_segment(tri, new_segment, new_build_data, context)) {
      throw model_error{"Failed to add triangle to new segment!",
                        model_ec::model_shadow_mesh_unexpected_failure};
   }
}

void add_triangle_to_segments(const std::array<model_shadow_soft_skinned_vertex, 3>& tri,
                              segment_soft_skinned_build_context& context)
{
   for (std::size_t i = 0; i < context.segments.size(); ++i) {
      if (try_add_triangle_to_segment(tri, context.segments[i],
                                      context.segments_data[i], context)) {
         return;
      }
   }

   model_shadow& new_segment = context.segments.emplace_back();

   new_segment.bboxSS = context.input_segment.bboxSS;
   new_segment.bboxLS = context.input_segment.bboxLS;
   new_segment.bone_name = context.input_segment.bone_name;

   new_segment.index_buffer.reserve(context.shadow_mesh.faces.size());
   new_segment.vertices.type = model_shadow_vertex_type::soft_skinned;
   new_segment.vertices.soft_skinned.reserve(context.shadow_mesh.vertex_count);
   new_segment.bone_map.reserve(context.max_bones);

   segment_soft_skinned_build_data& new_build_data =
      context.segments_data.emplace_back();

   new_build_data.bone_remap.resize(context.input_segment.bone_map.size(), noindex);
   new_build_data.vertex_cache.reserve(new_segment.vertices.soft_skinned.capacity());

   if (not try_add_triangle_to_segment(tri, new_segment, new_build_data, context)) {
      throw model_error{"Failed to add triangle to new segment!",
                        model_ec::model_shadow_mesh_unexpected_failure};
   }
}

void build_bboxes(std::span<model_shadow> segments)
{
   for (model_shadow& segment : segments) {
      switch (segment.vertices.type) {
      case model_shadow_vertex_type::unskinned: {
         segment.bboxLS.min = segment.vertices.unskinned[0].positionLS;
         segment.bboxLS.max = segment.bboxLS.min;

         segment.bboxSS.min = segment.vertices.unskinned[0].positionSS;
         segment.bboxSS.max = segment.bboxSS.min;

         for (std::size_t i = 1; i < segment.vertices.unskinned.size(); ++i) {
            const model_shadow_unskinned_vertex& vertex =
               segment.vertices.unskinned[i];

            segment.bboxSS.min = min(vertex.positionSS, segment.bboxSS.min);
            segment.bboxSS.max = max(vertex.positionSS, segment.bboxSS.max);
            segment.bboxLS.min = min(vertex.positionLS, segment.bboxLS.min);
            segment.bboxLS.max = max(vertex.positionLS, segment.bboxLS.max);
         }
      } break;
      case model_shadow_vertex_type::hard_skinned: {
         segment.bboxLS.min = segment.vertices.hard_skinned[0].positionLS;
         segment.bboxLS.max = segment.bboxLS.min;

         segment.bboxSS.min = segment.vertices.hard_skinned[0].positionSS[0];
         segment.bboxSS.max = segment.bboxSS.min;

         for (std::size_t i = 1; i < segment.vertices.hard_skinned.size(); ++i) {
            const model_shadow_hard_skinned_vertex& vertex =
               segment.vertices.hard_skinned[i];

            segment.bboxSS.min = min(vertex.positionSS[0], segment.bboxSS.min);
            segment.bboxSS.max = max(vertex.positionSS[0], segment.bboxSS.max);
            segment.bboxLS.min = min(vertex.positionLS, segment.bboxLS.min);
            segment.bboxLS.max = max(vertex.positionLS, segment.bboxLS.max);
         }
      } break;
      case model_shadow_vertex_type::soft_skinned: {
         segment.bboxLS.min = segment.vertices.soft_skinned[0].positionLS;
         segment.bboxLS.max = segment.bboxLS.min;

         segment.bboxSS.min = segment.vertices.soft_skinned[0].positionSS[0];
         segment.bboxSS.max = segment.bboxSS.min;

         for (std::size_t i = 1; i < segment.vertices.soft_skinned.size(); ++i) {
            const model_shadow_soft_skinned_vertex& vertex =
               segment.vertices.soft_skinned[i];

            segment.bboxSS.min = min(vertex.positionSS[0], segment.bboxSS.min);
            segment.bboxSS.max = max(vertex.positionSS[0], segment.bboxSS.max);
            segment.bboxLS.min = min(vertex.positionLS, segment.bboxLS.min);
            segment.bboxLS.max = max(vertex.positionLS, segment.bboxLS.max);
         }
      } break;
      }
   }
}

auto build_segments_unskinned(const shadow_mesh& shadow_mesh,
                              std::span<const shadow_silouhette_edge> shadow_silouhette,
                              const std::string_view bone_name)
   -> std::vector<model_shadow>
{
   std::vector<model_shadow> shadow_segments;

   const std::size_t total_triangle_count =
      shadow_mesh.faces.size() + shadow_silouhette.size() * 2;
   std::size_t output_triangle_count = 0;

   absl::flat_hash_map<cached_unskinned_vertex, uint16> vertex_cache;
   vertex_cache.reserve(shadow_mesh.faces.size() * 3);

   for (const shadow_face& face : shadow_mesh.faces) {
      if (shadow_segments.empty() or
          not segment_has_space_for_face_unskinned(face, shadow_segments.back())) {
         if (not shadow_segments.empty()) {
            output_triangle_count += shadow_segments.back().index_buffer.size();
         }

         shadow_segments.push_back({
            .bone_name = std::string{bone_name},
         });

         shadow_segments.back().index_buffer.reserve(total_triangle_count -
                                                     output_triangle_count);
         shadow_segments.back().vertices.type = model_shadow_vertex_type::unskinned;
         shadow_segments.back().vertices.unskinned.reserve(
            std::min(0x10000ull, shadow_mesh.vertex_count));

         vertex_cache.clear();
         vertex_cache.reserve(shadow_segments.back().vertices.unskinned.capacity());
      }

      const half_edge& fanning_edge = shadow_mesh.half_edges[face.first_edge];
      half_edge previous_edge = shadow_mesh.half_edges[fanning_edge.next_edge];

      for (uint32 next_edge_index = previous_edge.next_edge;
           next_edge_index != face.first_edge;) {
         const half_edge& edge = shadow_mesh.half_edges[next_edge_index];

         const std::array<uint16, 3> tri = {fanning_edge.vertex,
                                            previous_edge.vertex, edge.vertex};
         std::array<uint16, 3> output_tri = {};

         for (std::size_t i = 0; i < output_tri.size(); ++i) {
            const model_shadow_unskinned_vertex vertex = {
               .positionSS = shadow_mesh.positionSS[tri[i]],
               .normalSS = face.normalSS,
               .positionLS = shadow_mesh.positionLS[tri[i]],
            };

            auto [it, inserted] = vertex_cache.try_emplace(
               vertex, static_cast<uint16>(
                          shadow_segments.back().vertices.unskinned.size()));

            output_tri[i] = it->second;

            if (inserted) {
               shadow_segments.back().vertices.unskinned.push_back(vertex);
            }
         }

         shadow_segments.back().index_buffer.push_back(output_tri);

         previous_edge = edge;
         next_edge_index = edge.next_edge;
      }
   }

   for (const shadow_silouhette_edge& edge : shadow_silouhette) {
      if (shadow_segments.empty() or
          not segment_has_space_for_edge_unskinned(shadow_segments.back())) {
         if (not shadow_segments.empty()) {
            output_triangle_count += shadow_segments.back().index_buffer.size();
         }

         shadow_segments.push_back({
            .bone_name = std::string{bone_name},
         });

         shadow_segments.back().index_buffer.reserve(total_triangle_count -
                                                     output_triangle_count);
         shadow_segments.back().vertices.type = model_shadow_vertex_type::unskinned;
         shadow_segments.back().vertices.unskinned.reserve(
            std::min(0x10000ull, shadow_mesh.vertex_count));

         vertex_cache.clear();
         vertex_cache.reserve(shadow_segments.back().vertices.unskinned.capacity());
      }

      struct silhouette_edge_vertex {
         uint16 index = 0;
         std::array<uint8, 3> normalSS;
      };

      const std::array<std::array<uint8, 3>, 2> normalSS = {
         shadow_mesh.faces[shadow_mesh.half_edges[edge.half_edges[0]].face].normalSS,
         shadow_mesh.faces[shadow_mesh.half_edges[edge.half_edges[1]].face].normalSS,
      };

      const std::array<uint16, 2> indices = {
         shadow_mesh.half_edges[edge.half_edges[0]].vertex,
         shadow_mesh.half_edges[edge.half_edges[1]].vertex,
      };

      const std::array<silhouette_edge_vertex, 4> quad = {{
         {indices[0], normalSS[1]},
         {indices[1], normalSS[1]},
         {indices[1], normalSS[0]},
         {indices[0], normalSS[0]},
      }};
      std::array<uint16, 4> output_quad = {};

      for (std::size_t quad_index = 0; quad_index < quad.size(); ++quad_index) {
         const silhouette_edge_vertex& input_vertex = quad[quad_index];

         const model_shadow_unskinned_vertex vertex = {
            .positionSS = shadow_mesh.positionSS[input_vertex.index],
            .normalSS = input_vertex.normalSS,
            .positionLS = shadow_mesh.positionLS[input_vertex.index],
         };

         auto [it, inserted] = vertex_cache.try_emplace(
            vertex,
            static_cast<uint16>(shadow_segments.back().vertices.unskinned.size()));

         output_quad[quad_index] = it->second;

         if (inserted) {
            shadow_segments.back().vertices.unskinned.push_back(vertex);
         }
      }

      shadow_segments.back().index_buffer.push_back(
         {output_quad[0], output_quad[1], output_quad[2]});
      shadow_segments.back().index_buffer.push_back(
         {output_quad[0], output_quad[2], output_quad[3]});
   }

   build_bboxes(shadow_segments);

   return shadow_segments;
}

auto build_segments_hard_skinned(const model_segment& segment,
                                 const shadow_mesh& shadow_mesh,
                                 std::span<const shadow_silouhette_edge> shadow_silouhette,
                                 const uint32 max_bones) -> std::vector<model_shadow>
{
   segment_hard_skinned_build_context context = {
      .max_bones = max_bones,
      .input_segment = segment,
      .shadow_mesh = shadow_mesh,
   };

   for (const shadow_face& face : shadow_mesh.faces) {
      const half_edge& fanning_edge = shadow_mesh.half_edges[face.first_edge];
      half_edge previous_edge = shadow_mesh.half_edges[fanning_edge.next_edge];

      for (uint32 next_edge_index = previous_edge.next_edge;
           next_edge_index != face.first_edge;) {
         const half_edge& edge = shadow_mesh.half_edges[next_edge_index];

         const std::array<uint16, 3> indices = {fanning_edge.vertex,
                                                previous_edge.vertex, edge.vertex};
         std::array<model_shadow_hard_skinned_vertex, 3> tri = {};

         for (std::size_t i = 0; i < indices.size(); ++i) {
            const uint16 next_vertex = indices[(i + 1) % 3];
            const uint16 previous_vertex = indices[(i + 2) % 3];

            tri[i] = {
               .positionSS = {shadow_mesh.positionSS[indices[i]],
                              shadow_mesh.positionSS[next_vertex],
                              shadow_mesh.positionSS[previous_vertex]},
               .bone_indices = {shadow_mesh.bone_indices[indices[i]][0],
                                shadow_mesh.bone_indices[next_vertex][0],
                                shadow_mesh.bone_indices[previous_vertex][0]},
               .positionLS = shadow_mesh.positionLS[indices[i]],
            };
         }

         add_triangle_to_segments(tri, context);

         previous_edge = edge;
         next_edge_index = edge.next_edge;
      }
   }

   for (const shadow_silouhette_edge& edge : shadow_silouhette) {
      const std::array<uint32, 4> quad_edges = {{
         shadow_mesh.half_edges[edge.half_edges[1]].next_edge,
         edge.half_edges[1],
         shadow_mesh.half_edges[edge.half_edges[0]].next_edge,
         edge.half_edges[0],
      }};
      std::array<model_shadow_hard_skinned_vertex, 4> quad = {};

      for (std::size_t quad_index = 0; quad_index < quad_edges.size(); ++quad_index) {
         const uint32 edge_index = quad_edges[quad_index];

         const half_edge& vertex_edge = shadow_mesh.half_edges[edge_index];

         const uint16 next_vertex =
            shadow_mesh.half_edges[vertex_edge.next_edge].vertex;
         const uint16 previous_vertex =
            shadow_mesh.half_edges[vertex_edge.previous_edge].vertex;

         quad[quad_index] = {
            .positionSS = {shadow_mesh.positionSS[vertex_edge.vertex],
                           shadow_mesh.positionSS[next_vertex],
                           shadow_mesh.positionSS[previous_vertex]},
            .bone_indices = {shadow_mesh.bone_indices[vertex_edge.vertex][0],
                             shadow_mesh.bone_indices[next_vertex][0],
                             shadow_mesh.bone_indices[previous_vertex][0]},
            .positionLS = shadow_mesh.positionLS[vertex_edge.vertex],
         };
      }

      add_triangle_to_segments({quad[0], quad[1], quad[2]}, context);
      add_triangle_to_segments({quad[0], quad[2], quad[3]}, context);
   }

   build_bboxes(context.segments);

   return std::move(context.segments);
}

auto build_segments_soft_skinned(const model_segment& segment,
                                 const shadow_mesh& shadow_mesh,
                                 std::span<const shadow_silouhette_edge> shadow_silouhette,
                                 const uint32 max_bones) -> std::vector<model_shadow>
{
   segment_soft_skinned_build_context context = {
      .max_bones = max_bones,
      .input_segment = segment,
      .shadow_mesh = shadow_mesh,
   };

   for (const shadow_face& face : shadow_mesh.faces) {
      const half_edge& fanning_edge = shadow_mesh.half_edges[face.first_edge];
      half_edge previous_edge = shadow_mesh.half_edges[fanning_edge.next_edge];

      for (uint32 next_edge_index = previous_edge.next_edge;
           next_edge_index != face.first_edge;) {
         const half_edge& edge = shadow_mesh.half_edges[next_edge_index];

         const std::array<uint16, 3> indices = {fanning_edge.vertex,
                                                previous_edge.vertex, edge.vertex};
         std::array<model_shadow_soft_skinned_vertex, 3> tri = {};

         for (std::size_t i = 0; i < indices.size(); ++i) {
            const uint16 next_vertex = indices[(i + 1) % 3];
            const uint16 previous_vertex = indices[(i + 2) % 3];

            tri[i] = {
               .positionSS = {shadow_mesh.positionSS[indices[i]],
                              shadow_mesh.positionSS[next_vertex],
                              shadow_mesh.positionSS[previous_vertex]},
               .bone_weights = {shadow_mesh.bone_weights[indices[i]],
                                shadow_mesh.bone_weights[next_vertex],
                                shadow_mesh.bone_weights[previous_vertex]},
               .bone_indices = {shadow_mesh.bone_indices[indices[i]],
                                shadow_mesh.bone_indices[next_vertex],
                                shadow_mesh.bone_indices[previous_vertex]},
               .positionLS = shadow_mesh.positionLS[indices[i]],
            };
         }

         add_triangle_to_segments(tri, context);

         previous_edge = edge;
         next_edge_index = edge.next_edge;
      }
   }

   for (const shadow_silouhette_edge& edge : shadow_silouhette) {
      const std::array<uint32, 4> quad_edges = {{
         shadow_mesh.half_edges[edge.half_edges[1]].next_edge,
         edge.half_edges[1],
         shadow_mesh.half_edges[edge.half_edges[0]].next_edge,
         edge.half_edges[0],
      }};
      std::array<model_shadow_soft_skinned_vertex, 4> quad = {};

      for (std::size_t quad_index = 0; quad_index < quad_edges.size(); ++quad_index) {
         const uint32 edge_index = quad_edges[quad_index];

         const half_edge& vertex_edge = shadow_mesh.half_edges[edge_index];

         const uint16 next_vertex =
            shadow_mesh.half_edges[vertex_edge.next_edge].vertex;
         const uint16 previous_vertex =
            shadow_mesh.half_edges[vertex_edge.previous_edge].vertex;

         quad[quad_index] = {
            .positionSS = {shadow_mesh.positionSS[vertex_edge.vertex],
                           shadow_mesh.positionSS[next_vertex],
                           shadow_mesh.positionSS[previous_vertex]},
            .bone_weights = {shadow_mesh.bone_weights[vertex_edge.vertex],
                             shadow_mesh.bone_weights[next_vertex],
                             shadow_mesh.bone_weights[previous_vertex]},
            .bone_indices = {shadow_mesh.bone_indices[vertex_edge.vertex],
                             shadow_mesh.bone_indices[next_vertex],
                             shadow_mesh.bone_indices[previous_vertex]},
            .positionLS = shadow_mesh.positionLS[vertex_edge.vertex],
         };
      }

      add_triangle_to_segments({quad[0], quad[1], quad[2]}, context);
      add_triangle_to_segments({quad[0], quad[2], quad[3]}, context);
   }

   build_bboxes(context.segments);

   return std::move(context.segments);
}

}

auto build_shadow_segments(const model_segment& segment,
                           const build_shadow_segments_context& context)
   -> std::vector<model_shadow>
{
   if (not segment.vertices.positionSS or not segment.vertices.positionLS) {
      throw model_error{"Missing vertex inputs needed to build shadow mesh!",
                        model_ec::model_shadow_mesh_missing_inputs};
   }

   if (segment.index_buffer.empty()) return {};

   const shadow_mesh shadow_mesh = build_shadow_mesh(segment, context);
   const std::vector<shadow_silouhette_edge> shadow_silouhette =
      build_shadow_silouhette(shadow_mesh);

   const uint32 max_bones =
      std::max(context.max_bones,
               segment.vertices.bone_weights ? uint32{9} : uint32{3});

   if (shadow_mesh.bone_weights and shadow_mesh.bone_indices) {
      return build_segments_soft_skinned(segment, shadow_mesh,
                                         shadow_silouhette, max_bones);
   }
   else if (shadow_mesh.bone_indices) {
      return build_segments_hard_skinned(segment, shadow_mesh,
                                         shadow_silouhette, max_bones);
   }
   else {
      return build_segments_unskinned(shadow_mesh, shadow_silouhette, segment.bone_name);
   }
}

auto build_shadow_segments(std::span<const assets::msh::shadow_volume_half_edge> half_edges,
                           build_shadow_volume_vertices vertices,
                           const std::string_view bone_name) -> std::vector<model_shadow>
{
   shadow_mesh mesh = {
      .vertex_count = vertices.vertex_count,

      .positionSS = std::move(vertices.positionSS),
      .positionLS = std::move(vertices.positionLS),
   };

   mesh.half_edges.resize(half_edges.size());
   mesh.faces.reserve(half_edges.size() + 2 / 3);

   for (std::size_t i = 0; i < half_edges.size(); ++i) {
      mesh.half_edges[i] = {
         .vertex = half_edges[i].vertex,
         .next_edge = half_edges[i].next,
         .twin_edge = half_edges[i].twin,
      };
   }

   // Fill in missing shadow mesh info (face indices, previous edge indices) and build face list

   std::vector<bool> visited_edges;
   visited_edges.resize(half_edges.size());

   for (std::size_t edge_face_index = 0; edge_face_index < half_edges.size();
        ++edge_face_index) {
      if (visited_edges[edge_face_index]) continue;

      const uint32 face_index = static_cast<uint32>(mesh.faces.size());

      uint32 vertex_count = 1;

      float3 normalSS;

      for (uint32 edge_index = static_cast<uint32>(edge_face_index);;) {
         half_edge& edge = mesh.half_edges[edge_index];
         half_edge& next_edge = mesh.half_edges[edge.next_edge];

         next_edge.previous_edge = edge_index;

         edge.face = face_index;

         visited_edges[edge_index] = true;

         // Calculate the polygon's normal using Newell's Method.
         const float3& v0 = mesh.positionSS[edge.vertex];
         const float3& v1 = mesh.positionSS[next_edge.vertex];

         normalSS.x += (v0.y - v1.y) * (v0.z + v1.z);
         normalSS.y += (v0.z - v1.z) * (v0.x + v1.x);
         normalSS.z += (v0.x - v1.x) * (v0.y + v1.y);

         vertex_count += 1;

         edge_index = edge.next_edge;

         if (edge_index == edge_face_index) break;
      }

      normalSS = normalize(normalSS);

      mesh.faces.push_back({
         .first_edge = static_cast<uint32>(edge_face_index),
         .vertex_count = vertex_count,
         .normalSS =
            {
               static_cast<uint8>(normalSS.x * 127.5f + 127.5f),
               static_cast<uint8>(normalSS.y * 127.5f + 127.5f),
               static_cast<uint8>(normalSS.z * 127.5f + 127.5f),
            },
      });
   }

   // The shadow_mesh is now valid.

   const std::vector<shadow_silouhette_edge> shadow_silouhette =
      build_shadow_silouhette(mesh);

   return build_segments_unskinned(mesh, shadow_silouhette, bone_name);
}

}