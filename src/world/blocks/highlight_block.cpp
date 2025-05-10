#include "highlight_block.hpp"
#include "mesh_geometry.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

void highlight_block(const blocks& blocks, const block_type type,
                     const uint32 block_index, tool_visualizers& visualizers) noexcept
{
   switch (type) {
   case block_type::box: {
      const world::block_description_box& box = blocks.boxes.description[block_index];

      const float4x4 scale = {
         {box.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, box.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, box.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(box.rotation);

      float4x4 world_from_object = rotation * scale;
      world_from_object[3] = {box.position, 1.0f};

      visualizers.add_box_additive(world_from_object, {1.0f, 1.0f, 1.0f, 0.125f});
   } break;
   case block_type::ramp: {
      const world::block_description_ramp& ramp =
         blocks.ramps.description[block_index];

      const float4x4 scale = {
         {ramp.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, ramp.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, ramp.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(ramp.rotation);

      float4x4 world_from_object = rotation * scale;
      world_from_object[3] = {ramp.position, 1.0f};

      visualizers.add_ramp_additive(world_from_object, {1.0f, 1.0f, 1.0f, 0.125f});
   } break;
   case block_type::quad: {
      const world::block_description_quad& quad =
         blocks.quads.description[block_index];

      for (const auto& [i0, i1, i2] : quad.quad_split == block_quad_split::regular
                                         ? block_quad_triangles
                                         : block_quad_alternate_triangles) {
         visualizers.add_triangle_additive(quad.vertices[i0], quad.vertices[i1],
                                           quad.vertices[i2], 0x20'ff'ff'ffu);
      }
   } break;
   }
}

}