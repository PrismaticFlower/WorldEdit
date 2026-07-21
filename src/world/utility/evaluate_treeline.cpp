#include "evaluate_treeline.hpp"

#include "math/vector_funcs.hpp"

namespace we::world {

namespace {

struct random_gen {
   auto operator()() noexcept -> uint32
   {
      const uint32 v = state * 0x19660d + 0x3c6ef35f;
      state = v * 0x19660d + 0x3c6ef35f;

      return state & 0xffff0000 | v >> 0x10;
   }

   uint32 state = 0x94153a94;
};

}

void evaluate_treeline(
   const tree_line& tree_line, std::span<const path> paths,
   function_ptr<void(const float4x4& world_from_object, const object_class_handle class_handle) noexcept> evaluate_callback)
{
   if (tree_line.border_odfs.empty()) return;

   const path& path = paths[tree_line.path_index];

   if (path.nodes.size() < 2) return;

   float3 prev_point = path.nodes[0].position;
   random_gen rand;

   for (std::size_t i = 1; i < path.nodes.size(); ++i) {
      float3 point = path.nodes[i].position;
      float3 path_vector = point - prev_point;
      float path_vector_len_sq = dot(path_vector, path_vector);

      while (tree_line.distance * tree_line.distance < path_vector_len_sq) {
         float path_vector_len = sqrt(path_vector_len_sq);

         path_vector /= path_vector_len;

         float axis_sign = tree_line.flip ? -1.0f : 1.0f;
         float3 up = {0.0f, 1.0f, 0.0f};
         float3 object_axis = cross(path_vector, up) * axis_sign;

         float3 axis_z = normalize(object_axis);
         float3 axis_x = normalize(cross(up, object_axis));
         float3 axis_y = cross(axis_z, axis_x);

         float3 object_position = path_vector * tree_line.distance * 0.5f + prev_point;

         float4x4 world_from_object = {
            float4{axis_x, 0.0f},
            float4{axis_y, 0.0f},
            float4{axis_z, 0.0f},
            float4{object_position, 1.0f},
         };

         // This won't be accurate as to truly get this right we'd need the random state right as the
         // tree line is loaded ingame. Not exactly practical for the editor to try and mimic.
         const std::size_t odf_index = tree_line.border_odfs.size() > 1
                                          ? rand() % tree_line.border_odfs.size()
                                          : 0;

         evaluate_callback(world_from_object, tree_line.border_odfs[odf_index].handle);

         prev_point += path_vector * tree_line.distance;
         path_vector = point - prev_point;
         path_vector_len_sq = dot(path_vector, path_vector);
      }
   }
}

}
