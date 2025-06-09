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

      float4x4 world_from_local = rotation * scale;
      world_from_local[3] = {box.position, 1.0f};

      visualizers.add_box_additive(world_from_local, {1.0f, 1.0f, 1.0f, 0.125f});
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

      float4x4 world_from_local = rotation * scale;
      world_from_local[3] = {ramp.position, 1.0f};

      visualizers.add_ramp_additive(world_from_local, {1.0f, 1.0f, 1.0f, 0.125f});
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
   case block_type::cylinder: {
      const world::block_description_cylinder& cylinder =
         blocks.cylinders.description[block_index];

      const float4x4 scale = {
         {cylinder.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, cylinder.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, cylinder.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(cylinder.rotation);

      float4x4 world_from_local = rotation * scale;
      world_from_local[3] = {cylinder.position, 1.0f};

      visualizers.add_cylinder_additive(world_from_local, {1.0f, 1.0f, 1.0f, 0.125f});
   } break;
   case block_type::stairway: {
      const world::block_description_stairway& stairway =
         blocks.stairways.description[block_index];
      const block_custom_mesh& mesh =
         blocks.custom_meshes[blocks.stairways.mesh[block_index]];

      float4x4 world_from_local = to_matrix(stairway.rotation);
      world_from_local[3] = {stairway.position, 1.0f};

      for (const std::array<uint16, 3>& tri : mesh.triangles) {
         visualizers.add_triangle_additive(world_from_local *
                                              mesh.vertices[tri[0]].position,
                                           world_from_local *
                                              mesh.vertices[tri[1]].position,
                                           world_from_local *
                                              mesh.vertices[tri[2]].position,
                                           0x20'ff'ff'ffu);
      }
   } break;
   case block_type::cone: {
      const world::block_description_cone& cone =
         blocks.cones.description[block_index];

      const float4x4 scale = {
         {cone.size.x, 0.0f, 0.0f, 0.0f},
         {0.0f, cone.size.y, 0.0f, 0.0f},
         {0.0f, 0.0f, cone.size.z, 0.0f},
         {0.0f, 0.0f, 0.0f, 1.0f},
      };
      const float4x4 rotation = to_matrix(cone.rotation);

      float4x4 world_from_local = rotation * scale;
      world_from_local[3] = {cone.position, 1.0f};

      visualizers.add_cone_additive(world_from_local, {1.0f, 1.0f, 1.0f, 0.125f});
   } break;
   }
}
}