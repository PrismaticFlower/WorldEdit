#include "split_segments.hpp"
#include "error.hpp"

#include "math/vector_funcs.hpp"

namespace we::munge {

namespace {

constexpr uint32 min_bones = 3;
constexpr uint32 noindex = 0xffffffffu;

struct segment_split {
   std::vector<uint32> vertex_remap;
   std::vector<uint32> bone_remap;

   std::vector<uint16> vertex_buffer;
   std::vector<std::array<uint16, 3>> index_buffer;
   std::vector<uint8> bone_map;
};

bool try_add_tri_to_soft_skinned_split(const std::array<uint16, 3>& tri,
                                       const model_segment& segment,
                                       const uint32 max_bones, segment_split& split)
{
   // 1. Check for space in the split's bone map for any missing bones needed by the triangle.
   // 1. Add any bones missing from the split needed by the triangle.
   // 2. Add any vertices missing from the split needed by the triangle.
   // 3. Add the triangle to the split.

   const std::array<std::array<uint8, 3>, 3> tri_bones = {
      segment.vertices.bone_indices[tri[0]],
      segment.vertices.bone_indices[tri[1]],
      segment.vertices.bone_indices[tri[2]],
   };

   std::array<bool, 256> visited_bones = {};

   uint32 missing_bones = 0;

   for (const std::array<uint8, 3>& bone_indices : tri_bones) {
      for (const uint8 bone_index : bone_indices) {
         if (visited_bones[bone_index]) continue;

         visited_bones[bone_index] = true;

         if (split.bone_remap[bone_index] == noindex) {
            missing_bones += 1;
         }
      }
   }

   if (split.bone_map.size() + missing_bones > max_bones) return false;

   for (const std::array<uint8, 3>& bone_indices : tri_bones) {
      for (const uint8 bone_index : bone_indices) {
         if (split.bone_remap[bone_index] == noindex) {
            split.bone_remap[bone_index] =
               static_cast<uint32>(split.bone_map.size());
            split.bone_map.push_back(segment.bone_map[bone_index]);
         }
      }
   }

   for (std::size_t i = 0; i < tri.size(); ++i) {
      if (split.vertex_remap[tri[i]] == noindex) {
         split.vertex_remap[tri[i]] =
            static_cast<uint32>(split.vertex_buffer.size());
         split.vertex_buffer.push_back(tri[i]);
      }
   }

   split.index_buffer.push_back({static_cast<uint16>(split.vertex_remap[tri[0]]),
                                 static_cast<uint16>(split.vertex_remap[tri[1]]),
                                 static_cast<uint16>(split.vertex_remap[tri[2]])});

   return true;
}

bool try_add_tri_to_hard_skinned_split(const std::array<uint16, 3>& tri,
                                       const model_segment& segment,
                                       const uint32 max_bones, segment_split& split)
{
   // 1. Check for space in the split's bone map for any missing bones needed by the triangle.
   // 1. Add any bones missing from the split needed by the triangle.
   // 2. Add any vertices missing from the split needed by the triangle.
   // 3. Add the triangle to the split.

   const std::array<uint8, 3> tri_bones = {
      segment.vertices.bone_indices[tri[0]][0],
      segment.vertices.bone_indices[tri[1]][0],
      segment.vertices.bone_indices[tri[2]][0],
   };

   std::array<bool, 256> visited_bones = {};

   uint32 missing_bones = 0;

   for (const uint8 bone_index : tri_bones) {
      if (visited_bones[bone_index]) continue;

      visited_bones[bone_index] = true;

      if (split.bone_remap[bone_index] == noindex) {
         missing_bones += 1;
      }
   }

   if (split.bone_map.size() + missing_bones > max_bones) return false;

   for (const uint8 bone_index : tri_bones) {
      if (split.bone_remap[bone_index] == noindex) {
         split.bone_remap[bone_index] = static_cast<uint32>(split.bone_map.size());
         split.bone_map.push_back(segment.bone_map[bone_index]);
      }
   }

   for (std::size_t i = 0; i < tri.size(); ++i) {
      if (split.vertex_remap[tri[i]] == noindex) {
         split.vertex_remap[tri[i]] =
            static_cast<uint32>(split.vertex_buffer.size());
         split.vertex_buffer.push_back(tri[i]);
      }
   }

   split.index_buffer.push_back({static_cast<uint16>(split.vertex_remap[tri[0]]),
                                 static_cast<uint16>(split.vertex_remap[tri[1]]),
                                 static_cast<uint16>(split.vertex_remap[tri[2]])});

   return true;
}

void split_tri_soft_skinned_segments(const std::array<uint16, 3>& tri,
                                     const model_segment& segment, const uint32 max_bones,
                                     std::vector<segment_split>& splits)
{
   // 1. Try to add triangle to an existing split with all vertices.
   // 2. Try to add vertices to existing split.
   // 3. If those failed, create a new split.

   for (segment_split& split : splits) {
      const uint32 i0 = split.vertex_remap[tri[0]];
      const uint32 i1 = split.vertex_remap[tri[1]];
      const uint32 i2 = split.vertex_remap[tri[2]];

      if (i0 != noindex and i1 != noindex and i2 != noindex) {
         split.index_buffer.push_back({static_cast<uint16>(i0), static_cast<uint16>(i1),
                                       static_cast<uint16>(i2)});

         return;
      }
   }

   for (segment_split& split : splits) {
      if (try_add_tri_to_soft_skinned_split(tri, segment, max_bones, split)) {
         return;
      }
   }

   segment_split& split = splits.emplace_back();

   split.vertex_remap.resize(segment.vertices.vertex_count, noindex);
   split.bone_remap.resize(segment.bone_map.size(), noindex);

   split.vertex_buffer.reserve(segment.vertices.vertex_count);
   split.index_buffer.reserve(segment.vertices.vertex_count);
   split.bone_map.reserve(max_bones);

   if (not try_add_tri_to_soft_skinned_split(tri, segment, max_bones, split)) {
      throw model_error{"Unexpected failure adding tri to new split.",
                        model_ec::model_split_segment_unexpected_failure};
   }
}

void split_tri_hard_skinned_segments(const std::array<uint16, 3>& tri,
                                     const model_segment& segment, const uint32 max_bones,
                                     std::vector<segment_split>& splits)
{
   // 1. Try to add triangle to an existing split with all vertices.
   // 2. Try to add vertices to existing split.
   // 3. If those failed, create a new split.

   for (segment_split& split : splits) {
      const uint32 i0 = split.vertex_remap[tri[0]];
      const uint32 i1 = split.vertex_remap[tri[1]];
      const uint32 i2 = split.vertex_remap[tri[2]];

      if (i0 != noindex and i1 != noindex and i2 != noindex) {
         split.index_buffer.push_back({static_cast<uint16>(i0), static_cast<uint16>(i1),
                                       static_cast<uint16>(i2)});

         return;
      }
   }

   for (segment_split& split : splits) {
      if (try_add_tri_to_hard_skinned_split(tri, segment, max_bones, split)) {
         return;
      }
   }

   segment_split& split = splits.emplace_back();

   split.vertex_remap.resize(segment.vertices.vertex_count, noindex);
   split.bone_remap.resize(segment.bone_map.size(), noindex);

   split.vertex_buffer.reserve(segment.vertices.vertex_count);
   split.index_buffer.reserve(segment.index_buffer.size());
   split.bone_map.reserve(max_bones);

   if (not try_add_tri_to_hard_skinned_split(tri, segment, max_bones, split)) {
      throw model_error{"Unexpected failure adding tri to new split.",
                        model_ec::model_split_segment_unexpected_failure};
   }
}

auto split_soft_skinned_segments(const model_segment& segment, const uint32 max_bones)
   -> std::vector<segment_split>
{
   std::vector<segment_split> splits;
   splits.reserve(8);

   for (const std::array<uint16, 3>& tri : segment.index_buffer) {
      split_tri_soft_skinned_segments(tri, segment, max_bones, splits);
   }

   return splits;
}

auto split_hard_skinned_segments(const model_segment& segment, const uint32 max_bones)
   -> std::vector<segment_split>
{
   std::vector<segment_split> splits;
   splits.reserve(8);

   for (const std::array<uint16, 3>& tri : segment.index_buffer) {
      split_tri_hard_skinned_segments(tri, segment, max_bones, splits);
   }

   return splits;
}

}

auto split_skinned_segments(const model_segment& segment, const uint32 max_bones)
   -> std::vector<model_segment>
{
   if (not segment.vertices.bone_indices or segment.bone_map.empty()) {
      throw model_error{"Attempt to split unskinned segment.",
                        model_ec::model_split_segment_unskinned};
   }

   std::vector<segment_split> splits;

   if (segment.vertices.bone_weights) {
      splits = split_soft_skinned_segments(segment, std::max(min_bones, max_bones));
   }
   else {
      splits = split_hard_skinned_segments(segment, std::max(min_bones, max_bones));
   }

   std::vector<model_segment> segments;
   segments.reserve(splits.size());

   for (segment_split& split : splits) {
      model_segment& split_segment = segments.emplace_back();

      split_segment.material = segment.material;

      const std::size_t vertex_count = split.vertex_buffer.size();

      if (vertex_count == 0) {
         throw model_error{"Split segment has 0 vertices.",
                           model_ec::model_split_segment_bad_vertex_count};
      }

      split_segment.vertices.vertex_count = vertex_count;

      if (segment.vertices.positionSS) {
         split_segment.vertices.positionSS =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (segment.vertices.positionLS) {
         split_segment.vertices.positionLS =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (segment.vertices.bone_weights) {
         split_segment.vertices.bone_weights =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (segment.vertices.bone_indices) {
         split_segment.vertices.bone_indices =
            std::make_unique_for_overwrite<std::array<uint8, 3>[]>(vertex_count);
      }

      if (segment.vertices.normalSS) {
         split_segment.vertices.normalSS =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (segment.vertices.tangents) {
         split_segment.vertices.tangents =
            std::make_unique_for_overwrite<model_segment_vertex_tangents[]>(vertex_count);
      }

      if (segment.vertices.color) {
         split_segment.vertices.color =
            std::make_unique_for_overwrite<uint32[]>(vertex_count);
      }

      if (segment.vertices.texcoords) {
         split_segment.vertices.texcoords =
            std::make_unique_for_overwrite<float2[]>(vertex_count);
      }

      for (uint32 split_index = 0; split_index < split.vertex_buffer.size();
           ++split_index) {
         const uint16 unsplit_index = split.vertex_buffer[split_index];

         if (segment.vertices.positionSS) {
            split_segment.vertices.positionSS[split_index] =
               segment.vertices.positionSS[unsplit_index];
         }

         if (segment.vertices.positionLS) {
            split_segment.vertices.positionLS[split_index] =
               segment.vertices.positionLS[unsplit_index];
         }

         if (segment.vertices.bone_weights) {
            split_segment.vertices.bone_weights[split_index] =
               segment.vertices.bone_weights[unsplit_index];
         }

         if (segment.vertices.bone_indices) {
            const std::array<uint8, 3>& unsplit_bone_indices =
               segment.vertices.bone_indices[unsplit_index];

            if (segment.vertices.bone_weights) {
               for (std::size_t i = 0; i < unsplit_bone_indices.size(); ++i) {
                  split_segment.vertices.bone_indices[split_index][i] =
                     static_cast<uint8>(split.bone_remap[unsplit_bone_indices[i]]);
               }
            }
            else {
               const uint8 bone_index =
                  static_cast<uint8>(split.bone_remap[unsplit_bone_indices[0]]);

               split_segment.vertices.bone_indices[split_index] = {bone_index, bone_index,
                                                                   bone_index};
            }
         }

         if (segment.vertices.normalSS) {
            split_segment.vertices.normalSS[split_index] =
               segment.vertices.normalSS[unsplit_index];
         }

         if (segment.vertices.tangents) {
            split_segment.vertices.tangents[split_index] =
               segment.vertices.tangents[unsplit_index];
         }

         if (segment.vertices.color) {
            split_segment.vertices.color[split_index] =
               segment.vertices.color[unsplit_index];
         }

         if (segment.vertices.texcoords) {
            split_segment.vertices.texcoords[split_index] =
               segment.vertices.texcoords[unsplit_index];
         }
      }

      if (split_segment.vertices.positionSS) {
         split_segment.bboxSS.min = split_segment.vertices.positionSS[0];
         split_segment.bboxSS.max = split_segment.bboxSS.min;

         for (std::size_t i = 1; i < vertex_count; ++i) {
            const float3 positionSS = split_segment.vertices.positionSS[i];

            split_segment.bboxSS.min = min(positionSS, split_segment.bboxSS.min);
            split_segment.bboxSS.max = max(positionSS, split_segment.bboxSS.max);
         }
      }

      if (split_segment.vertices.positionLS) {
         split_segment.bboxLS.min = split_segment.vertices.positionLS[0];
         split_segment.bboxLS.max = split_segment.bboxLS.min;

         for (std::size_t i = 1; i < vertex_count; ++i) {
            const float3 positionLS = split_segment.vertices.positionLS[i];

            split_segment.bboxLS.min = min(positionLS, split_segment.bboxLS.min);
            split_segment.bboxLS.max = max(positionLS, split_segment.bboxLS.max);
         }
      }

      split_segment.index_buffer = std::move(split.index_buffer);
      split_segment.bone_map = std::move(split.bone_map);
   }

   return segments;
}
}