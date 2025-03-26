#include "highlight_surface.hpp"
#include "mesh_geometry.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

namespace we::world {

void highlight_surface(const block_description_box& box, const uint32 surface_index,
                       tool_visualizers& visualizers) noexcept
{
   const float4x4 scale = {
      {box.size.x, 0.0f, 0.0f, 0.0f},
      {0.0f, box.size.y, 0.0f, 0.0f},
      {0.0f, 0.0f, box.size.z, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
   };
   const float4x4 rotation = to_matrix(box.rotation);

   float4x4 world_from_object = rotation * scale;
   world_from_object[3] = {box.position, 1.0f};

   for (const std::array<uint16, 3>& tri : block_cube_triangles) {
      if (block_cube_vertices[tri[0]].surface_index != surface_index) continue;

      const float3 pos0WS = world_from_object * block_cube_vertices[tri[0]].position;
      const float3 pos1WS = world_from_object * block_cube_vertices[tri[1]].position;
      const float3 pos2WS = world_from_object * block_cube_vertices[tri[2]].position;

      visualizers.add_triangle_additive(pos0WS, pos1WS, pos2WS, 0x20'ff'ff'ffu);
   }
}

}