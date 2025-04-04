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
   }

   std::unreachable();
}

auto get_block_surface_material(blocks& blocks, const block_type type,
                                const uint32 block_index,
                                const uint32 surface_index) noexcept -> uint8&
{
   assert(valid_index(blocks, type, block_index));

   switch (type) {
   case block_type::box:
      std::array<uint8, 6>& surface_materials =
         blocks.boxes.description[block_index].surface_materials;

      return surface_materials[surface_index % surface_materials.size()];
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
   case block_type::box:
      std::array<block_texture_mode, 6>& surface_texture_mode =
         blocks.boxes.description[block_index].surface_texture_mode;

      return surface_texture_mode[surface_index % surface_texture_mode.size()];
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
   case block_type::box:
      std::array<block_texture_rotation, 6>& surface_texture_rotation =
         blocks.boxes.description[block_index].surface_texture_rotation;

      return surface_texture_rotation[surface_index % surface_texture_rotation.size()];
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
   case block_type::box:
      std::array<std::array<int8, 2>, 6>& surface_texture_scale =
         blocks.boxes.description[block_index].surface_texture_scale;

      return surface_texture_scale[surface_index % surface_texture_scale.size()];
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
   case block_type::box:
      std::array<std::array<uint16, 2>, 6>& surface_texture_offset =
         blocks.boxes.description[block_index].surface_texture_offset;

      return surface_texture_offset[surface_index % surface_texture_offset.size()];
   }

   std::unreachable();
}

}