#include "accessors.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

namespace {

bool valid_index(const blocks& blocks, const block_type type,
                 const uint32 block_index) noexcept
{
   switch (type) {
   case block_type::box:
      return block_index < blocks.boxes.size();
   case block_type::ramp:
      return block_index < blocks.ramps.size();
   case block_type::quad:
      return block_index < blocks.quads.size();
   case block_type::cylinder:
      return block_index < blocks.cylinders.size();
   case block_type::custom:
      return block_index < blocks.custom.size();
   case block_type::cone:
      return block_index < blocks.cones.size();
   case block_type::hemisphere:
      return block_index < blocks.hemispheres.size();
   case block_type::pyramid:
      return block_index < blocks.pyramids.size();
   }

   std::unreachable();
}

}

auto get_dirty_tracker(blocks& blocks, const block_type type) noexcept
   -> blocks_dirty_range_tracker&
{
   switch (type) {
   case block_type::box:
      return blocks.boxes.dirty;
   case block_type::ramp:
      return blocks.ramps.dirty;
   case block_type::quad:
      return blocks.quads.dirty;
   case block_type::cylinder:
      return blocks.cylinders.dirty;
   case block_type::custom:
      return blocks.custom.dirty;
   case block_type::cone:
      return blocks.cones.dirty;
   case block_type::hemisphere:
      return blocks.hemispheres.dirty;
   case block_type::pyramid:
      return blocks.pyramids.dirty;
   }

   std::unreachable();
}

auto get_block_hidden(blocks& blocks, const block_type type,
                      const uint32 block_index) noexcept -> bool&
{
   assert(valid_index(blocks, type, block_index));

   switch (type) {
   case block_type::box:
      return blocks.boxes.hidden[block_index];
   case block_type::ramp:
      return blocks.ramps.hidden[block_index];
   case block_type::quad:
      return blocks.quads.hidden[block_index];
   case block_type::cylinder:
      return blocks.cylinders.hidden[block_index];
   case block_type::custom:
      return blocks.custom.hidden[block_index];
   case block_type::cone:
      return blocks.cones.hidden[block_index];
   case block_type::hemisphere:
      return blocks.hemispheres.hidden[block_index];
   case block_type::pyramid:
      return blocks.pyramids.hidden[block_index];
   }

   std::unreachable();
}

auto get_block_layer(blocks& blocks, const block_type type,
                     const uint32 block_index) noexcept -> int8&
{
   assert(valid_index(blocks, type, block_index));

   switch (type) {
   case block_type::box:
      return blocks.boxes.layer[block_index];
   case block_type::ramp:
      return blocks.ramps.layer[block_index];
   case block_type::quad:
      return blocks.quads.layer[block_index];
   case block_type::cylinder:
      return blocks.cylinders.layer[block_index];
   case block_type::custom:
      return blocks.custom.layer[block_index];
   case block_type::cone:
      return blocks.cones.layer[block_index];
   case block_type::hemisphere:
      return blocks.hemispheres.layer[block_index];
   case block_type::pyramid:
      return blocks.pyramids.layer[block_index];
   }

   std::unreachable();
}

auto get_block_surface_material(blocks& blocks, const block_type type,
                                const uint32 block_index,
                                const uint32 surface_index) noexcept -> uint8&
{
   assert(valid_index(blocks, type, block_index));

   switch (type) {
   case block_type::box: {

      std::array<uint8, 6>& surface_materials =
         blocks.boxes.description[block_index].surface_materials;

      return surface_materials[surface_index % surface_materials.size()];
   }
   case block_type::ramp: {

      std::array<uint8, 5>& surface_materials =
         blocks.ramps.description[block_index].surface_materials;

      return surface_materials[surface_index % surface_materials.size()];
   }
   case block_type::quad: {
      std::array<uint8, 1>& surface_materials =
         blocks.quads.description[block_index].surface_materials;

      return surface_materials[surface_index % surface_materials.size()];
   }
   case block_type::cylinder: {
      std::array<uint8, 3>& surface_materials =
         blocks.cylinders.description[block_index].surface_materials;

      return surface_materials[surface_index % surface_materials.size()];
   }
   case block_type::custom: {
      std::array<uint8, 6>& surface_materials =
         blocks.custom.description[block_index].surface_materials;

      return surface_materials[surface_index % surface_materials.size()];
   }
   case block_type::cone: {
      std::array<uint8, 2>& surface_materials =
         blocks.cones.description[block_index].surface_materials;

      return surface_materials[surface_index % surface_materials.size()];
   }
   case block_type::hemisphere: {
      std::array<uint8, 2>& surface_materials =
         blocks.hemispheres.description[block_index].surface_materials;

      return surface_materials[surface_index % surface_materials.size()];
   }
   case block_type::pyramid: {

      std::array<uint8, 5>& surface_materials =
         blocks.pyramids.description[block_index].surface_materials;

      return surface_materials[surface_index % surface_materials.size()];
   }
   }

   std::unreachable();
}

auto get_block_surface_texture_mode(blocks& blocks, const block_type type,
                                    const uint32 block_index,
                                    const uint32 surface_index) noexcept
   -> block_texture_mode&
{
   assert(valid_index(blocks, type, block_index));

   switch (type) {
   case block_type::box: {
      std::array<block_texture_mode, 6>& surface_texture_mode =
         blocks.boxes.description[block_index].surface_texture_mode;

      return surface_texture_mode[surface_index % surface_texture_mode.size()];
   }
   case block_type::ramp: {
      std::array<block_texture_mode, 5>& surface_texture_mode =
         blocks.ramps.description[block_index].surface_texture_mode;

      return surface_texture_mode[surface_index % surface_texture_mode.size()];
   }
   case block_type::quad: {
      std::array<block_texture_mode, 1>& surface_texture_mode =
         blocks.quads.description[block_index].surface_texture_mode;

      return surface_texture_mode[surface_index % surface_texture_mode.size()];
   }
   case block_type::cylinder: {
      std::array<block_texture_mode, 3>& surface_texture_mode =
         blocks.cylinders.description[block_index].surface_texture_mode;

      return surface_texture_mode[surface_index % surface_texture_mode.size()];
   }
   case block_type::custom: {
      std::array<block_texture_mode, 6>& surface_texture_mode =
         blocks.custom.description[block_index].surface_texture_mode;

      return surface_texture_mode[surface_index % surface_texture_mode.size()];
   }
   case block_type::cone: {
      std::array<block_texture_mode, 2>& surface_texture_mode =
         blocks.cones.description[block_index].surface_texture_mode;

      return surface_texture_mode[surface_index % surface_texture_mode.size()];
   }
   case block_type::hemisphere: {
      std::array<block_texture_mode, 2>& surface_texture_mode =
         blocks.hemispheres.description[block_index].surface_texture_mode;

      return surface_texture_mode[surface_index % surface_texture_mode.size()];
   }
   case block_type::pyramid: {
      std::array<block_texture_mode, 5>& surface_texture_mode =
         blocks.pyramids.description[block_index].surface_texture_mode;

      return surface_texture_mode[surface_index % surface_texture_mode.size()];
   }
   }

   std::unreachable();
}

auto get_block_surface_texture_rotation(blocks& blocks, const block_type type,
                                        const uint32 block_index,
                                        const uint32 surface_index) noexcept
   -> block_texture_rotation&
{
   assert(valid_index(blocks, type, block_index));

   switch (type) {
   case block_type::box: {
      std::array<block_texture_rotation, 6>& surface_texture_rotation =
         blocks.boxes.description[block_index].surface_texture_rotation;

      return surface_texture_rotation[surface_index % surface_texture_rotation.size()];
   }
   case block_type::ramp: {
      std::array<block_texture_rotation, 5>& surface_texture_rotation =
         blocks.ramps.description[block_index].surface_texture_rotation;

      return surface_texture_rotation[surface_index % surface_texture_rotation.size()];
   }
   case block_type::quad: {
      std::array<block_texture_rotation, 1>& surface_texture_rotation =
         blocks.quads.description[block_index].surface_texture_rotation;

      return surface_texture_rotation[surface_index % surface_texture_rotation.size()];
   }
   case block_type::cylinder: {
      std::array<block_texture_rotation, 3>& surface_texture_rotation =
         blocks.cylinders.description[block_index].surface_texture_rotation;

      return surface_texture_rotation[surface_index % surface_texture_rotation.size()];
   }
   case block_type::custom: {
      std::array<block_texture_rotation, 6>& surface_texture_rotation =
         blocks.custom.description[block_index].surface_texture_rotation;

      return surface_texture_rotation[surface_index % surface_texture_rotation.size()];
   }
   case block_type::cone: {
      std::array<block_texture_rotation, 2>& surface_texture_rotation =
         blocks.cones.description[block_index].surface_texture_rotation;

      return surface_texture_rotation[surface_index % surface_texture_rotation.size()];
   }
   case block_type::hemisphere: {
      std::array<block_texture_rotation, 2>& surface_texture_rotation =
         blocks.hemispheres.description[block_index].surface_texture_rotation;

      return surface_texture_rotation[surface_index % surface_texture_rotation.size()];
   }
   case block_type::pyramid: {
      std::array<block_texture_rotation, 5>& surface_texture_rotation =
         blocks.pyramids.description[block_index].surface_texture_rotation;

      return surface_texture_rotation[surface_index % surface_texture_rotation.size()];
   }
   }

   std::unreachable();
}

auto get_block_surface_texture_scale(blocks& blocks, const block_type type,
                                     const uint32 block_index,
                                     const uint32 surface_index) noexcept
   -> std::array<int8, 2>&
{
   assert(valid_index(blocks, type, block_index));

   switch (type) {
   case block_type::box: {
      std::array<std::array<int8, 2>, 6>& surface_texture_scale =
         blocks.boxes.description[block_index].surface_texture_scale;

      return surface_texture_scale[surface_index % surface_texture_scale.size()];
   }
   case block_type::ramp: {
      std::array<std::array<int8, 2>, 5>& surface_texture_scale =
         blocks.ramps.description[block_index].surface_texture_scale;

      return surface_texture_scale[surface_index % surface_texture_scale.size()];
   }
   case block_type::quad: {
      std::array<std::array<int8, 2>, 1>& surface_texture_scale =
         blocks.quads.description[block_index].surface_texture_scale;

      return surface_texture_scale[surface_index % surface_texture_scale.size()];
   }
   case block_type::cylinder: {
      std::array<std::array<int8, 2>, 3>& surface_texture_scale =
         blocks.cylinders.description[block_index].surface_texture_scale;

      return surface_texture_scale[surface_index % surface_texture_scale.size()];
   }
   case block_type::custom: {
      std::array<std::array<int8, 2>, 6>& surface_texture_scale =
         blocks.custom.description[block_index].surface_texture_scale;

      return surface_texture_scale[surface_index % surface_texture_scale.size()];
   }
   case block_type::cone: {
      std::array<std::array<int8, 2>, 2>& surface_texture_scale =
         blocks.cones.description[block_index].surface_texture_scale;

      return surface_texture_scale[surface_index % surface_texture_scale.size()];
   }
   case block_type::hemisphere: {
      std::array<std::array<int8, 2>, 2>& surface_texture_scale =
         blocks.hemispheres.description[block_index].surface_texture_scale;

      return surface_texture_scale[surface_index % surface_texture_scale.size()];
   }
   case block_type::pyramid: {
      std::array<std::array<int8, 2>, 5>& surface_texture_scale =
         blocks.pyramids.description[block_index].surface_texture_scale;

      return surface_texture_scale[surface_index % surface_texture_scale.size()];
   }
   }

   std::unreachable();
}

auto get_block_surface_texture_offset(blocks& blocks, const block_type type,
                                      const uint32 block_index,
                                      const uint32 surface_index) noexcept
   -> std::array<uint16, 2>&
{
   assert(valid_index(blocks, type, block_index));

   switch (type) {
   case block_type::box: {
      std::array<std::array<uint16, 2>, 6>& surface_texture_offset =
         blocks.boxes.description[block_index].surface_texture_offset;

      return surface_texture_offset[surface_index % surface_texture_offset.size()];
   }
   case block_type::ramp: {
      std::array<std::array<uint16, 2>, 5>& surface_texture_offset =
         blocks.ramps.description[block_index].surface_texture_offset;

      return surface_texture_offset[surface_index % surface_texture_offset.size()];
   }
   case block_type::quad: {
      std::array<std::array<uint16, 2>, 1>& surface_texture_offset =
         blocks.quads.description[block_index].surface_texture_offset;

      return surface_texture_offset[surface_index % surface_texture_offset.size()];
   }
   case block_type::cylinder: {
      std::array<std::array<uint16, 2>, 3>& surface_texture_offset =
         blocks.cylinders.description[block_index].surface_texture_offset;

      return surface_texture_offset[surface_index % surface_texture_offset.size()];
   }
   case block_type::custom: {
      std::array<std::array<uint16, 2>, 6>& surface_texture_offset =
         blocks.custom.description[block_index].surface_texture_offset;

      return surface_texture_offset[surface_index % surface_texture_offset.size()];
   }
   case block_type::cone: {
      std::array<std::array<uint16, 2>, 2>& surface_texture_offset =
         blocks.cones.description[block_index].surface_texture_offset;

      return surface_texture_offset[surface_index % surface_texture_offset.size()];
   }
   case block_type::hemisphere: {
      std::array<std::array<uint16, 2>, 2>& surface_texture_offset =
         blocks.hemispheres.description[block_index].surface_texture_offset;

      return surface_texture_offset[surface_index % surface_texture_offset.size()];
   }
   case block_type::pyramid: {
      std::array<std::array<uint16, 2>, 5>& surface_texture_offset =
         blocks.pyramids.description[block_index].surface_texture_offset;

      return surface_texture_offset[surface_index % surface_texture_offset.size()];
   }
   }

   std::unreachable();
}

}