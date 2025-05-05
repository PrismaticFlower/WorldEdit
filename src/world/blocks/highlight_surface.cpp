#include "highlight_surface.hpp"
#include "mesh_geometry.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

#include <span>

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

namespace {

void highlight_surface_generic(const float4x4& world_from_object,
                               std::span<const std::array<uint16, 3>> triangles,
                               std::span<const block_vertex> vertices,
                               const uint32 surface_index,
                               tool_visualizers& visualizers) noexcept
{
   for (const std::array<uint16, 3>& tri : triangles) {
      if (vertices[tri[0]].surface_index != surface_index) continue;

      const float3 pos0WS = world_from_object * vertices[tri[0]].position;
      const float3 pos1WS = world_from_object * vertices[tri[1]].position;
      const float3 pos2WS = world_from_object * vertices[tri[2]].position;

      visualizers.add_triangle_additive(pos0WS, pos1WS, pos2WS, 0x20'ff'ff'ffu);
   }
}

}

void highlight_surface(const blocks& blocks, const block_type type,
                       const uint32 block_index, const uint32 surface_index,
                       tool_visualizers& visualizers) noexcept
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

      highlight_surface_generic(world_from_object, block_cube_triangles,
                                block_cube_vertices, surface_index, visualizers);
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

      highlight_surface_generic(world_from_object, block_ramp_triangles,
                                block_ramp_vertices, surface_index, visualizers);
   } break;
   }
}

}