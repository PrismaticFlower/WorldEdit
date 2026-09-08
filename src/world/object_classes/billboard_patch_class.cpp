#include "billboard_patch_class.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"

namespace we::world {

auto billboard_patch_class::world_from_object(const quaternion& rotation,
                                              const float3& position) noexcept -> float4x4
{
   float4x4 world_from_object = to_matrix(rotation);
   world_from_object[3] = float4{position, 1.0f};

   float3 y_axis = {world_from_object[1].x, world_from_object[1].y,
                    world_from_object[1].z};
   float3 x_axis = normalize(cross({0.0f, 0.0f, -1.0f}, y_axis));
   float3 z_axis = normalize(cross(x_axis, y_axis));

   world_from_object[0] = {x_axis, 0.0f};
   world_from_object[1] = {y_axis, 0.0f};
   world_from_object[2] = {z_axis, 0.0f};

   return world_from_object;
}

auto billboard_patch_class::world_from_object(const float4x4& world_from_object) noexcept
   -> float4x4
{
   float3 y_axis = {world_from_object[1].x, world_from_object[1].y,
                    world_from_object[1].z};
   float3 x_axis = normalize(cross({0.0f, 0.0f, -1.0f}, y_axis));
   float3 z_axis = normalize(cross(x_axis, y_axis));

   return {{x_axis, 0.0f}, {y_axis, 0.0f}, {z_axis, 0.0f}, world_from_object[3]};
}

auto billboard_patch_class::object_from_world(const quaternion& rotation,
                                              const float3& position) noexcept -> float4x4
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