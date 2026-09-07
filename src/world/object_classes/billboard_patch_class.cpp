#include "billboard_patch_class.hpp"

#include "math/matrix_funcs.hpp"

namespace we::world {

auto billboard_patch_class::object_from_world(const quaternion& rotation,
                                              const float3& position) const noexcept
   -> float4x4
{
   float4x4 object_from_world = transpose(world_from_object(rotation, position));

   object_from_world[3] = {float3x3{object_from_world} *
                              -float3{object_from_world[0].w,
                                      object_from_world[1].w,
                                      object_from_world[2].w},
                           1.0f};

   object_from_world[0].w = 0.0f;
   object_from_world[1].w = 0.0f;
   object_from_world[2].w = 0.0f;

   return object_from_world;
}

}