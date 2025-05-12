#include "bounding_box.hpp"

#include "math/vector_funcs.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

namespace {

auto get_bounding_box(const quaternion& rotation, const float3& position,
                      const float3& size) noexcept -> math::bounding_box
{
   return rotation * math::bounding_box{.min = -size, .max = size} + position;
}

}

auto get_bounding_box(const block_description_box& box) noexcept -> math::bounding_box
{
   return get_bounding_box(box.rotation, box.position, box.size);
}

auto get_bounding_box(const block_description_ramp& ramp) noexcept -> math::bounding_box
{
   return get_bounding_box(ramp.rotation, ramp.position, ramp.size);
}

auto get_bounding_box(const block_description_quad& quad) noexcept -> math::bounding_box
{
   return {
      .min = min(min(min(quad.vertices[0], quad.vertices[1]), quad.vertices[2]),
                 quad.vertices[3]),
      .max = max(max(max(quad.vertices[0], quad.vertices[1]), quad.vertices[2]),
                 quad.vertices[3]),
   };
}

auto get_bounding_box(const block_description_cylinder& cylinder) noexcept
   -> math::bounding_box
{
   return get_bounding_box(cylinder.rotation, cylinder.position, cylinder.size);
}

auto get_bounding_box(const blocks& blocks, const block_type type,
                      const uint32 block_index) noexcept -> math::bounding_box
{
   switch (type) {
   case world::block_type::box: {
      return get_bounding_box(blocks.boxes.description[block_index]);
   } break;
   case world::block_type::ramp: {
      return get_bounding_box(blocks.ramps.description[block_index]);
   } break;
   case world::block_type::quad: {
      return get_bounding_box(blocks.quads.description[block_index]);
   } break;
   case world::block_type::cylinder: {
      return get_bounding_box(blocks.cylinders.description[block_index]);
   } break;
   }

   std::unreachable();
}

}
