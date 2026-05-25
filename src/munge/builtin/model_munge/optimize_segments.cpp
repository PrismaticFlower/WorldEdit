#include "optimize_segments.hpp"
#include "error.hpp"

#include "math/vector_funcs.hpp"

#include "utility/string_icompare.hpp"

#include <span>

#include <meshoptimizer.h>

namespace we::munge {

namespace {

constexpr uint32 max_vertices = 0x10000;

struct merging_segment {
   bool to_be_merged = false;
   uint32 vertex_count = 0;
   uint32 triangle_count = 0;

   std::vector<uint32> segments;
   std::vector<uint8> bone_map;
};

struct merge_context {
   std::vector<uint8> bone_remap;
   std::size_t vertex_offset = 0;
};

bool is_same(const material& l, const material& r) noexcept
{
   using string::iequals;

   return l.flags == r.flags and                          //
          l.diffuse_color == r.diffuse_color and          //
          l.specular_color == r.specular_color and        //
          l.specular_exponent == r.specular_exponent and  //
          l.param0 == r.param0 and                        //
          l.param1 == r.param1 and                        //
          iequals(l.attached_light, r.attached_light) and //
          iequals(l.render_type, r.render_type) and       //
          iequals(l.name, r.name) and                     //
          iequals(l.textures[0], r.textures[0]) and       //
          iequals(l.textures[1], r.textures[1]) and       //
          iequals(l.textures[2], r.textures[2]) and       //
          iequals(l.textures[3], r.textures[3]);
}

bool can_merge_segments(const model_segment& l, const model_segment& r)
{
   return is_same(l.material, r.material) and l.bone_name == r.bone_name;
}

auto count_missing_bones(const model_segment& to, const model_segment& from) -> std::size_t
{
   std::size_t missing = from.bone_map.size();

   for (uint8 from_bone : from.bone_map) {
      for (uint8 to_bone : to.bone_map) {
         if (from_bone == to_bone) {
            missing -= 1;

            break;
         }
      }
   }

   return missing;
}

void concatenate_bone_maps(std::vector<uint8>& to, const std::vector<uint8>& from)
{
   for (uint8 from_bone : from) {
      bool found = false;

      for (uint8 to_bone : to) {
         if (from_bone == to_bone) {
            found = true;

            break;
         }
      }

      if (not found) to.push_back(from_bone);
   }
}

void build_bone_remap(const std::vector<uint8>& to,
                      const std::vector<uint8>& from, merge_context& context)
{
   context.bone_remap.clear();
   context.bone_remap.resize(from.size());

   for (std::size_t from_index = 0; from_index < from.size(); ++from_index) {
      const uint8 from_bone = from[from_index];

      for (std::size_t to_index = 0; to_index < to.size(); ++to_index) {
         const uint8 to_bone = to[to_index];

         if (from_bone == to_bone) {
            context.bone_remap[from_index] = static_cast<uint8>(to_index);

            break;
         }
      }
   }
}

void concatenate_segments(model_segment& to, const model_segment& from,
                          merge_context& context)
{
   for (uint32 source_index = 0; source_index < from.vertices.vertex_count;
        ++source_index) {
      const std::size_t dest_index = context.vertex_offset + source_index;

      if (to.vertices.positionSS) {
         to.vertices.positionSS[dest_index] = from.vertices.positionSS[source_index];
      }

      if (from.vertices.positionLS) {
         to.vertices.positionLS[dest_index] = from.vertices.positionLS[source_index];
      }

      if (from.vertices.bone_weights) {
         to.vertices.bone_weights[dest_index] =
            from.vertices.bone_weights[source_index];
      }

      if (from.vertices.bone_indices) {
         std::array<uint8, 3>& to_bone_indices = to.vertices.bone_indices[dest_index];
         const std::array<uint8, 3>& from_bone_indices =
            from.vertices.bone_indices[source_index];

         for (std::size_t i = 0; i < to_bone_indices.size(); ++i) {
            to_bone_indices[i] = context.bone_remap[from_bone_indices[i]];
         }
      }

      if (to.vertices.normalSS) {
         to.vertices.normalSS[dest_index] = from.vertices.normalSS[source_index];
      }

      if (to.vertices.tangents) {
         to.vertices.tangents[dest_index] = from.vertices.tangents[source_index];
      }

      if (from.vertices.color) {
         to.vertices.color[dest_index] = from.vertices.color[source_index];
      }

      if (to.vertices.texcoords) {
         to.vertices.texcoords[dest_index] = from.vertices.texcoords[source_index];
      }
   }

   for (const std::array<uint16, 3>& tri : from.index_buffer) {
      to.index_buffer.push_back(
         {static_cast<uint16>(tri[0] + context.vertex_offset),
          static_cast<uint16>(tri[1] + context.vertex_offset),
          static_cast<uint16>(tri[2] + context.vertex_offset)});
   }

   to.bboxSS = math::combine(to.bboxSS, from.bboxSS);
   to.bboxLS = math::combine(to.bboxLS, from.bboxLS);

   context.vertex_offset += from.vertices.vertex_count;
}

void merge_segments(std::vector<model_segment>& segments, const uint32 max_bones)
{
   std::vector<merging_segment> merging;
   merging.resize(segments.size());

   for (std::size_t segment_index = 0; segment_index < segments.size(); ++segment_index) {
      if (merging[segment_index].to_be_merged) continue;

      const model_segment& segment = segments[segment_index];

      std::size_t vertex_count = segment.vertices.vertex_count;
      std::size_t triangle_count = segment.index_buffer.size();
      std::vector<uint8>& new_bone_map = merging[segment_index].bone_map;

      if (not segment.bone_map.empty()) {
         new_bone_map.reserve(max_bones);
         new_bone_map.append_range(segment.bone_map);
      }

      for (std::size_t other_index = segment_index + 1;
           other_index < segments.size(); ++other_index) {
         if (merging[other_index].to_be_merged) continue;

         const model_segment& other_segment = segments[other_index];
         const std::size_t missing_bones = count_missing_bones(segment, other_segment);

         if (not can_merge_segments(segment, other_segment)) continue;
         if (vertex_count + other_segment.vertices.vertex_count > max_vertices) {
            continue;
         }
         if (missing_bones + new_bone_map.size() > max_bones) continue;

         vertex_count += other_segment.vertices.vertex_count;
         triangle_count += other_segment.index_buffer.size();

         if (missing_bones > 0) {
            concatenate_bone_maps(new_bone_map, other_segment.bone_map);
         }

         merging[other_index].to_be_merged = true;
         merging[segment_index].segments.push_back(static_cast<uint32>(other_index));
      }

      merging[segment_index].vertex_count = static_cast<uint32>(vertex_count);
      merging[segment_index].triangle_count = static_cast<uint32>(triangle_count);
   }

   bool merge_any = false;

   for (const merging_segment& segment : merging) {
      if (segment.to_be_merged) merge_any = true;
   }

   if (not merge_any) return;

   for (std::size_t segment_index = 0; segment_index < segments.size(); ++segment_index) {
      merging_segment& merging_segment = merging[segment_index];

      if (merging_segment.to_be_merged) continue;
      if (merging_segment.segments.empty()) continue;

      model_segment& segment = segments[segment_index];
      model_segment old_segment;

      std::swap(segment, old_segment);

      segment.material = old_segment.material;

      const std::size_t vertex_count = merging_segment.vertex_count;

      if (vertex_count == 0) {
         throw model_error{"Merging segments with 0 vertices.",
                           model_ec::model_optimize_merge_segments_bad_vertex_count};
      }

      segment.vertices.vertex_count = vertex_count;

      if (old_segment.vertices.positionSS) {
         segment.vertices.positionSS =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (old_segment.vertices.positionLS) {
         segment.vertices.positionLS =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (old_segment.vertices.bone_weights) {
         segment.vertices.bone_weights =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (old_segment.vertices.bone_indices) {
         segment.vertices.bone_indices =
            std::make_unique_for_overwrite<std::array<uint8, 3>[]>(vertex_count);
      }

      if (old_segment.vertices.normalSS) {
         segment.vertices.normalSS =
            std::make_unique_for_overwrite<float3[]>(vertex_count);
      }

      if (old_segment.vertices.tangents) {
         segment.vertices.tangents =
            std::make_unique_for_overwrite<model_segment_vertex_tangents[]>(vertex_count);
      }

      if (old_segment.vertices.color) {
         segment.vertices.color =
            std::make_unique_for_overwrite<uint32[]>(vertex_count);
      }

      if (old_segment.vertices.texcoords) {
         segment.vertices.texcoords =
            std::make_unique_for_overwrite<float2[]>(vertex_count);
      }

      segment.bboxSS = old_segment.bboxSS;
      segment.bboxLS = old_segment.bboxLS;

      segment.index_buffer.reserve(merging_segment.triangle_count);
      segment.bone_name = old_segment.bone_name;
      segment.bone_map = std::move(merging_segment.bone_map);

      merge_context merge_context;

      build_bone_remap(segment.bone_map, old_segment.bone_map, merge_context);
      concatenate_segments(segment, old_segment, merge_context);

      for (const uint32 merge_segment_index : merging_segment.segments) {
         const model_segment& other_segment = segments[merge_segment_index];

         build_bone_remap(segment.bone_map, other_segment.bone_map, merge_context);
         concatenate_segments(segment, other_segment, merge_context);
      }
   }

   std::ptrdiff_t erase_offset = 0;

   for (std::ptrdiff_t segment_index = 0; segment_index < std::ssize(merging);
        ++segment_index) {
      if (merging[segment_index].to_be_merged) {
         segments.erase(segments.begin() + (segment_index + erase_offset));

         erase_offset -= 1;
      }
   }
}

void optimize_mesh_buffers(std::span<model_segment> segments)
{
   for (model_segment& segment : segments) {
      if (segment.index_buffer.empty()) {
         throw model_error{"Optimzing index buffer with 0 triangles.",
                           model_ec::model_optimize_vertex_cache_bad_triangle_count};
      }

      if (segment.vertices.vertex_count == 0) {
         throw model_error{"Optimzing vertex buffer with 0 vertices.",
                           model_ec::model_optimize_vertex_cache_bad_vertex_count};
      }

      std::vector<std::array<uint16, 3>> index_buffer;
      index_buffer.resize(segment.index_buffer.size());

      meshopt_optimizeVertexCache(index_buffer[0].data(),
                                  segment.index_buffer[0].data(),
                                  index_buffer.size() * 3,
                                  segment.vertices.vertex_count);

      segment.index_buffer = std::move(index_buffer);

      const uint32 remap_noindex = 0xff'ff'ff'ffu;

      std::vector<uint32> vertex_remap;
      vertex_remap.resize(segment.vertices.vertex_count, remap_noindex);

      std::size_t next_vertex = 0;

      for (std::array<uint16, 3>& tri : segment.index_buffer) {
         for (uint16& index : tri) {
            uint32& remapped_index = vertex_remap[index];

            if (remapped_index == remap_noindex) {
               remapped_index = static_cast<uint32>(next_vertex);

               next_vertex += 1;
            }

            index = static_cast<uint16>(remapped_index);
         }
      }

      const std::size_t new_vertex_count = next_vertex;

      if (new_vertex_count == 0) {
         throw model_error{"Optimzing vertex buffer with 0 vertices.",
                           model_ec::model_optimize_vertex_fetch_reorder_bad_vertex_count};
      }

      model_segment_vertices old_vertices = std::move(segment.vertices);

      segment.vertices.vertex_count = new_vertex_count;

      if (old_vertices.positionSS) {
         segment.vertices.positionSS =
            std::make_unique_for_overwrite<float3[]>(new_vertex_count);
      }

      if (old_vertices.positionLS) {
         segment.vertices.positionLS =
            std::make_unique_for_overwrite<float3[]>(new_vertex_count);
      }

      if (old_vertices.bone_weights) {
         segment.vertices.bone_weights =
            std::make_unique_for_overwrite<float3[]>(new_vertex_count);
      }

      if (old_vertices.bone_indices) {
         segment.vertices.bone_indices =
            std::make_unique_for_overwrite<std::array<uint8, 3>[]>(new_vertex_count);
      }

      if (old_vertices.normalSS) {
         segment.vertices.normalSS =
            std::make_unique_for_overwrite<float3[]>(new_vertex_count);
      }

      if (old_vertices.tangents) {
         segment.vertices.tangents =
            std::make_unique_for_overwrite<model_segment_vertex_tangents[]>(
               new_vertex_count);
      }

      if (old_vertices.color) {
         segment.vertices.color =
            std::make_unique_for_overwrite<uint32[]>(new_vertex_count);
      }

      if (old_vertices.texcoords) {
         segment.vertices.texcoords =
            std::make_unique_for_overwrite<float2[]>(new_vertex_count);
      }

      for (std::size_t i = 0; i < new_vertex_count; ++i) {
         const uint32 new_index = vertex_remap[i];

         if (segment.vertices.positionSS) {
            segment.vertices.positionSS[new_index] = old_vertices.positionSS[i];
         }

         if (segment.vertices.positionLS) {
            segment.vertices.positionLS[new_index] = old_vertices.positionLS[i];
         }

         if (segment.vertices.bone_weights) {
            segment.vertices.bone_weights[new_index] = old_vertices.bone_weights[i];
         }

         if (segment.vertices.bone_indices) {
            segment.vertices.bone_indices[new_index] = old_vertices.bone_indices[i];
         }

         if (segment.vertices.normalSS) {
            segment.vertices.normalSS[new_index] = old_vertices.normalSS[i];
         }

         if (segment.vertices.tangents) {
            segment.vertices.tangents[new_index] = old_vertices.tangents[i];
         }

         if (segment.vertices.color) {
            segment.vertices.color[new_index] = old_vertices.color[i];
         }

         if (segment.vertices.texcoords) {
            segment.vertices.texcoords[new_index] = old_vertices.texcoords[i];
         }
      }
   }
}

void optimize_mesh_buffers(std::span<model_shadow> segments)
{
   for (model_shadow& segment : segments) {
      if (segment.index_buffer.empty()) {
         throw model_error{"Optimzing index buffer with 0 triangles.",
                           model_ec::model_optimize_vertex_cache_bad_triangle_count};
      }

      std::size_t vertex_count = 0;

      switch (segment.vertices.type) {
      case model_shadow_vertex_type::unskinned:
         vertex_count = segment.vertices.unskinned.size();
         break;
      case model_shadow_vertex_type::hard_skinned:
         vertex_count = segment.vertices.hard_skinned.size();
         break;
      case model_shadow_vertex_type::soft_skinned:
         vertex_count = segment.vertices.soft_skinned.size();
         break;
      }

      if (vertex_count == 0) {
         throw model_error{"Optimzing vertex buffer with 0 vertices.",
                           model_ec::model_optimize_vertex_cache_bad_vertex_count};
      }

      std::vector<std::array<uint16, 3>> index_buffer;
      index_buffer.resize(segment.index_buffer.size());

      meshopt_optimizeVertexCache(index_buffer[0].data(),
                                  segment.index_buffer[0].data(),
                                  index_buffer.size() * 3, vertex_count);

      segment.index_buffer = std::move(index_buffer);

      const uint32 remap_noindex = 0xff'ff'ff'ffu;

      std::vector<uint32> vertex_remap;
      vertex_remap.resize(vertex_count, remap_noindex);

      std::size_t next_vertex = 0;

      for (std::array<uint16, 3>& tri : segment.index_buffer) {
         for (uint16& index : tri) {
            uint32& remapped_index = vertex_remap[index];

            if (remapped_index == remap_noindex) {
               remapped_index = static_cast<uint32>(next_vertex);

               next_vertex += 1;
            }

            index = static_cast<uint16>(remapped_index);
         }
      }

      const std::size_t new_vertex_count = next_vertex;

      if (new_vertex_count == 0) {
         throw model_error{"Optimzing vertex buffer with 0 vertices.",
                           model_ec::model_optimize_vertex_fetch_reorder_bad_vertex_count};
      }

      switch (segment.vertices.type) {
      case model_shadow_vertex_type::unskinned: {
         std::vector<model_shadow_unskinned_vertex> new_vertices;
         new_vertices.resize(new_vertex_count);

         for (std::size_t i = 0; i < new_vertex_count; ++i) {
            const uint32 new_index = vertex_remap[i];

            new_vertices[new_index] = segment.vertices.unskinned[i];
         }

         segment.vertices.unskinned = std::move(new_vertices);
      } break;
      case model_shadow_vertex_type::hard_skinned: {
         std::vector<model_shadow_hard_skinned_vertex> new_vertices;
         new_vertices.resize(new_vertex_count);

         for (std::size_t i = 0; i < new_vertex_count; ++i) {
            const uint32 new_index = vertex_remap[i];

            new_vertices[new_index] = segment.vertices.hard_skinned[i];
         }

         segment.vertices.hard_skinned = std::move(new_vertices);
      } break;
      case model_shadow_vertex_type::soft_skinned: {
         std::vector<model_shadow_soft_skinned_vertex> new_vertices;
         new_vertices.resize(new_vertex_count);

         for (std::size_t i = 0; i < new_vertex_count; ++i) {
            const uint32 new_index = vertex_remap[i];

            new_vertices[new_index] = segment.vertices.soft_skinned[i];
         }

         segment.vertices.soft_skinned = std::move(new_vertices);
      } break;
      }
   }
}
}

void optimize_segments(std::vector<model_segment>& segments,
                       const optimization_options& options)
{
   merge_segments(segments, options.max_bones);
   optimize_mesh_buffers(segments);
}

void optimize_segments(std::vector<model_shadow>& segments)
{
   optimize_mesh_buffers(segments);
}

}