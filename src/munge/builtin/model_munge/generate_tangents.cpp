#include "generate_tangents.hpp"
#include "error.hpp"

#include "assets/msh/mikktspace/mikktspace.h"

#include "math/vector_funcs.hpp"

#include <span>

#include <absl/container/flat_hash_map.h>

namespace we::munge {

namespace {

struct tangents_vertex {
   uint16 source_vertex_index;

   std::array<int16, 3> tangentSS;
   std::array<int16, 3> bitangentSS;

   bool operator==(const tangents_vertex&) const noexcept = default;

   template<typename H>
   friend H AbslHashValue(H h, const tangents_vertex& vertex)
   {
      return H::combine(std::move(h), vertex.source_vertex_index,
                        vertex.tangentSS, vertex.bitangentSS);
   }
};

struct tspace_user_data {
   const model_segment& segment;
   std::span<std::array<tangents_vertex, 3>> out;
};

struct indexed_triangle_list {
   std::vector<tangents_vertex> vertex_buffer;
   std::vector<std::array<uint16, 3>> index_buffer;
};

const uint32 max_vertices = 0x10000;

auto pack_snorm_16(float v) noexcept -> int16
{
   v = std::max(std::min(v, 1.0f), -1.0f);
   v *= 32767.0f;

   if (v >= 0.0f) {
      v += 0.5f;
   }
   else {
      v -= 0.5f;
   }

   return static_cast<int16>(v);
}

auto unpack_snorm_16(const std::array<int16, 3>& v) noexcept -> float3
{
   return {std::max(v[0] / 32767.0f, -1.0f), std::max(v[1] / 32767.0f, -1.0f),
           std::max(v[2] / 32767.0f, -1.0f)};
}

const SMikkTSpaceInterface mikktspace_interface{
   .m_getNumFaces = [](const SMikkTSpaceContext* context) -> int {
      const model_segment& segment =
         static_cast<tspace_user_data*>(context->m_pUserData)->segment;

      return static_cast<int>(segment.index_buffer.size());
   },

   .m_getNumVerticesOfFace = [](const SMikkTSpaceContext*, const int) -> int {
      return 3;
   },

   .m_getPosition =
      [](const SMikkTSpaceContext* context, float pos_out[], const int face,
         const int vert) {
         const model_segment& segment =
            static_cast<tspace_user_data*>(context->m_pUserData)->segment;

         std::memcpy(pos_out,
                     &segment.vertices.positionSS[segment.index_buffer[face][vert]],
                     sizeof(float3));
      },

   .m_getNormal =
      [](const SMikkTSpaceContext* context, float norm_out[], const int face,
         const int vert) {
         const model_segment& segment =
            static_cast<tspace_user_data*>(context->m_pUserData)->segment;

         std::memcpy(norm_out,
                     &segment.vertices.normalSS[segment.index_buffer[face][vert]],
                     sizeof(float3));
      },

   .m_getTexCoord =
      [](const SMikkTSpaceContext* context, float texcoord_out[],
         const int face, const int vert) {
         const model_segment& segment =
            static_cast<tspace_user_data*>(context->m_pUserData)->segment;

         std::memcpy(texcoord_out,
                     &segment.vertices.texcoords[segment.index_buffer[face][vert]],
                     sizeof(float2));
      },

   .m_setTSpaceBasic = nullptr,

   .m_setTSpace =
      [](const SMikkTSpaceContext* context, const float tangent[],
         const float bitangent[], [[maybe_unused]] const float mags,
         [[maybe_unused]] const float magt,
         [[maybe_unused]] const tbool is_orientation_preserving, const int face,
         const int vert) {
         const model_segment& segment =
            static_cast<tspace_user_data*>(context->m_pUserData)->segment;
         std::span<std::array<tangents_vertex, 3>>& out =
            static_cast<tspace_user_data*>(context->m_pUserData)->out;

         tangents_vertex& vertex = out[face][vert];

         vertex.source_vertex_index = segment.index_buffer[face][vert];

         vertex.tangentSS[0] = pack_snorm_16(tangent[0]);
         vertex.tangentSS[1] = pack_snorm_16(tangent[1]);
         vertex.tangentSS[2] = pack_snorm_16(tangent[2]);

         vertex.bitangentSS[0] = pack_snorm_16(bitangent[0]);
         vertex.bitangentSS[1] = pack_snorm_16(bitangent[1]);
         vertex.bitangentSS[2] = pack_snorm_16(bitangent[2]);
      }};

auto generate_triangles_tangents(const model_segment& segment)
   -> std::vector<std::array<tangents_vertex, 3>>
{
   std::vector<std::array<tangents_vertex, 3>> triangles;
   triangles.resize(segment.index_buffer.size());

   tspace_user_data user_data{.segment = segment, .out = triangles};

   SMikkTSpaceInterface interface = mikktspace_interface;
   SMikkTSpaceContext context{&interface, &user_data};

   if (not genTangSpaceDefault(&context)) {
      throw model_error{"MikkTSpace failed to generate tangents!",
                        model_ec::model_generate_tangents_unknown_failure};
   }

   return triangles;
}

auto build_indexed_triangle_lists(std::span<const std::array<tangents_vertex, 3>> triangles)
   -> std::vector<indexed_triangle_list>
{
   absl::flat_hash_map<tangents_vertex, uint16> vertex_cache;
   vertex_cache.reserve(triangles.size());

   std::vector<indexed_triangle_list> indexed_triangle_segments;
   indexed_triangle_segments.emplace_back();

   indexed_triangle_segments.back().vertex_buffer.reserve(triangles.size() * 3);
   indexed_triangle_segments.back().index_buffer.reserve(triangles.size());

   for (const std::array<tangents_vertex, 3>& tri : triangles) {
      const std::array<decltype(vertex_cache)::iterator, 3> existing = {
         vertex_cache.find(tri[0]),
         vertex_cache.find(tri[1]),
         vertex_cache.find(tri[2]),
      };

      const std::array<bool, 3> missing = {
         existing[0] == vertex_cache.end(),
         existing[1] == vertex_cache.end(),
         existing[2] == vertex_cache.end(),
      };

      const uint32 missing_count = missing[0] + missing[1] + missing[2];

      if (indexed_triangle_segments.back().vertex_buffer.size() + missing_count >=
          max_vertices) {
         std::size_t used_triangles = 0;

         for (const indexed_triangle_list& indexed : indexed_triangle_segments) {
            used_triangles += indexed.index_buffer.size();
         }

         const std::size_t remaining_triangles = triangles.size() - used_triangles;

         indexed_triangle_segments.emplace_back();

         indexed_triangle_segments.back().vertex_buffer.reserve(remaining_triangles * 3);
         indexed_triangle_segments.back().index_buffer.reserve(remaining_triangles);

         vertex_cache.clear();

         indexed_triangle_segments.back().vertex_buffer.push_back(tri[0]);
         indexed_triangle_segments.back().vertex_buffer.push_back(tri[1]);
         indexed_triangle_segments.back().vertex_buffer.push_back(tri[2]);
         indexed_triangle_segments.back().index_buffer.push_back({0, 1, 2});

         vertex_cache.emplace(tri[0], 0);
         vertex_cache.emplace(tri[1], 1);
         vertex_cache.emplace(tri[2], 2);

         continue;
      }

      std::array<uint16, 3> indexed_tri = {};

      for (std::size_t i = 0; i < indexed_tri.size(); ++i) {
         if (not missing[i]) {
            indexed_tri[i] = existing[i]->second;
         }
         else {
            indexed_tri[i] = static_cast<uint16>(
               indexed_triangle_segments.back().vertex_buffer.size());

            indexed_triangle_segments.back().vertex_buffer.push_back(tri[i]);
         }
      }

      indexed_triangle_segments.back().index_buffer.push_back(indexed_tri);

      for (std::size_t i = 0; i < indexed_tri.size(); ++i) {
         if (missing[i]) vertex_cache.emplace(tri[i], indexed_tri[i]);
      }
   }

   return indexed_triangle_segments;
}

}

auto generate_tangents(const model_segment& segment) -> std::vector<model_segment>
{
   if (not segment.vertices.positionSS or not segment.vertices.normalSS or
       not segment.vertices.texcoords) {
      throw model_error{"Missing vertex inputs needed to generate tangents!",
                        model_ec::model_generate_tangents_missing_inputs};
   }

   if (segment.index_buffer.empty()) {
      throw model_error{"Missing triangles needed to generate tangents!",
                        model_ec::model_generate_tangents_missing_inputs};
   }

   const std::vector<std::array<tangents_vertex, 3>> triangles =
      generate_triangles_tangents(segment);
   std::vector<indexed_triangle_list> indexed_triangle_sets =
      build_indexed_triangle_lists(triangles);

   std::vector<model_segment> segments;
   segments.reserve(indexed_triangle_sets.size());

   for (indexed_triangle_list& list : indexed_triangle_sets) {
      model_segment& new_segment = segments.emplace_back();

      new_segment.material = segment.material;

      const std::size_t vertex_count = list.vertex_buffer.size();

      if (vertex_count == 0) {
         throw model_error{"Segment with generated tangents has 0 vertices.",
                           model_ec::model_generate_tangents_bad_vertex_count};
      }

      new_segment.vertices.vertex_count = vertex_count;

      new_segment.vertices.positionSS =
         std::make_unique_for_overwrite<float3[]>(vertex_count);

      if (segment.vertices.positionLS) {
         new_segment.vertices.positionLS =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (segment.vertices.bone_weights) {
         new_segment.vertices.bone_weights =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (segment.vertices.bone_indices) {
         new_segment.vertices.bone_indices =
            std::make_unique_for_overwrite<std::array<uint8, 3>[]>(vertex_count);
      }

      new_segment.vertices.normalSS =
         std::make_unique_for_overwrite<float3[]>(vertex_count);

      new_segment.vertices.tangents =
         std::make_unique_for_overwrite<model_segment_vertex_tangents[]>(vertex_count);

      if (segment.vertices.color) {
         new_segment.vertices.color =
            std::make_unique_for_overwrite<uint32[]>(vertex_count);
      }

      new_segment.vertices.texcoords =
         std::make_unique_for_overwrite<float2[]>(vertex_count);

      for (uint32 dest_index = 0; dest_index < list.vertex_buffer.size(); ++dest_index) {
         const tangents_vertex& vertex = list.vertex_buffer[dest_index];
         const uint16 source_index = vertex.source_vertex_index;

         new_segment.vertices.positionSS[dest_index] =
            segment.vertices.positionSS[source_index];

         if (segment.vertices.positionLS) {
            new_segment.vertices.positionLS[dest_index] =
               segment.vertices.positionLS[source_index];
         }

         if (segment.vertices.bone_weights) {
            new_segment.vertices.bone_weights[dest_index] =
               segment.vertices.bone_weights[source_index];
         }

         if (segment.vertices.bone_indices) {
            new_segment.vertices.bone_indices[dest_index] =
               segment.vertices.bone_indices[source_index];
         }

         new_segment.vertices.normalSS[dest_index] =
            segment.vertices.normalSS[source_index];

         new_segment.vertices.tangents[dest_index] = {
            .tangentSS = unpack_snorm_16(vertex.tangentSS),
            .bitangentSS = unpack_snorm_16(vertex.bitangentSS),
         };

         if (segment.vertices.color) {
            new_segment.vertices.color[dest_index] =
               segment.vertices.color[source_index];
         }

         new_segment.vertices.texcoords[dest_index] =
            segment.vertices.texcoords[source_index];
      }

      new_segment.bboxSS.min = new_segment.vertices.positionSS[0];
      new_segment.bboxSS.max = new_segment.bboxSS.min;

      for (std::size_t i = 1; i < vertex_count; ++i) {
         const float3 positionSS = new_segment.vertices.positionSS[i];

         new_segment.bboxSS.min = min(positionSS, new_segment.bboxSS.min);
         new_segment.bboxSS.max = max(positionSS, new_segment.bboxSS.max);
      }

      if (new_segment.vertices.positionLS) {
         new_segment.bboxLS.min = new_segment.vertices.positionLS[0];
         new_segment.bboxLS.max = new_segment.bboxLS.min;

         for (std::size_t i = 1; i < vertex_count; ++i) {
            const float3 positionLS = new_segment.vertices.positionLS[i];

            new_segment.bboxLS.min = min(positionLS, new_segment.bboxLS.min);
            new_segment.bboxLS.max = max(positionLS, new_segment.bboxLS.max);
         }
      }

      new_segment.index_buffer = std::move(list.index_buffer);
      new_segment.bone_name = segment.bone_name;
      new_segment.bone_map = segment.bone_map;
   }

   return segments;
}

}