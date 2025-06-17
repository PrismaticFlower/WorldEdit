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

auto get_bounding_box_local_space(const block_custom_mesh_description& mesh) noexcept
   -> math::bounding_box
{
   switch (mesh.type) {
   case block_custom_mesh_type::stairway: {
      const float half_width = mesh.stairway.size.x / 2.0f;
      const float half_length = mesh.stairway.size.z / 2.0f;

      return {.min = {-half_width, 0.0f, -half_length},
              .max = {half_width, mesh.stairway.size.y + mesh.stairway.first_step_offset,
                      half_length}};
   }
   case block_custom_mesh_type::ring: {
      const float radius = mesh.ring.inner_radius + mesh.ring.outer_radius * 2.0f;

      return {.min = {-radius, -mesh.ring.height, -radius},
              .max = {radius, mesh.ring.height, radius}};
   }
   case block_custom_mesh_type::beveled_box: {
      return {.min = {-mesh.beveled_box.size}, .max = {mesh.beveled_box.size}};
   }
   }

   std::unreachable();
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

auto get_bounding_box(const block_description_custom& custom_block) noexcept
   -> math::bounding_box
{
   const math::bounding_box bboxLS =
      get_bounding_box_local_space(custom_block.mesh_description);

   return custom_block.rotation *
             math::bounding_box{.min = min(bboxLS.min, bboxLS.max),
                                .max = max(bboxLS.min, bboxLS.max)} +
          custom_block.position;
}

auto get_bounding_box(const block_description_cone& cone) noexcept -> math::bounding_box
{
   return get_bounding_box(cone.rotation, cone.position, cone.size);
}

auto get_bounding_box(const block_description_hemisphere& hemisphere) noexcept
   -> math::bounding_box
{
   return hemisphere.rotation * math::bounding_box{.min = {-hemisphere.size.x, 0.0f,
                                                           -hemisphere.size.z},
                                                   .max = hemisphere.size} +
          hemisphere.position;
}

auto get_bounding_box(const block_description_pyramid& pyramid) noexcept -> math::bounding_box
{
   return get_bounding_box(pyramid.rotation, pyramid.position, pyramid.size);
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
   case world::block_type::custom: {
      return get_bounding_box(blocks.custom.description[block_index]);
   } break;
   case world::block_type::cone: {
      return get_bounding_box(blocks.cones.description[block_index]);
   } break;
   case world::block_type::hemisphere: {
      return get_bounding_box(blocks.hemispheres.description[block_index]);
   } break;
   case world::block_type::pyramid: {
      return get_bounding_box(blocks.pyramids.description[block_index]);
   } break;
   }

   std::unreachable();
}

}
