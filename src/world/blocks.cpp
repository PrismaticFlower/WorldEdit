#include "blocks.hpp"

#include <cassert>

namespace we::world {

void blocks_boxes::reserve(const std::size_t size) noexcept
{
   bbox.min_x.reserve(size);
   bbox.min_y.reserve(size);
   bbox.min_z.reserve(size);
   bbox.max_x.reserve(size);
   bbox.max_y.reserve(size);
   bbox.max_z.reserve(size);
   hidden.reserve(size);
   layer.reserve(size);
   description.reserve(size);
   ids.reserve(size);
}

auto blocks_boxes::size() const noexcept -> std::size_t
{
   assert(is_balanced());

   return bbox.min_x.size();
}

bool blocks_boxes::is_balanced() const noexcept
{
   return bbox.min_x.size() == bbox.min_y.size() and
          bbox.min_x.size() == bbox.min_z.size() and
          bbox.min_x.size() == bbox.max_x.size() and
          bbox.min_x.size() == bbox.max_y.size() and
          bbox.min_x.size() == bbox.max_z.size() and
          bbox.min_x.size() == hidden.size() and
          bbox.min_x.size() == layer.size() and //
          bbox.min_x.size() == description.size() and
          bbox.min_x.size() == ids.size();
}

bool blocks::empty() const noexcept
{
   return boxes.size() == 0;
}

void blocks::untracked_fill_dirty_ranges() noexcept
{
   if (boxes.size() != 0) {
      boxes.dirty.add({0, static_cast<uint32>(boxes.size())});
   }

   materials_dirty.add({0, static_cast<uint32>(materials.size())});
}

void blocks::untracked_clear_dirty_ranges() noexcept
{
   boxes.dirty.clear();
   materials_dirty.clear();
}

auto blocks::get_blank_materials() noexcept -> pinned_vector<block_material>
{
   pinned_vector<block_material> materials{
      {.max_size = max_block_materials, .initial_capacity = max_block_materials}};

   materials.resize(max_block_materials, {});

   return materials;
}

block_id::block_id(block_box_id id) noexcept
   : id_type{block_type::box}, id{.box = id}
{
}

bool block_id::is_box() const noexcept
{
   return id_type == block_type::box;
}

auto block_id::get_box() const noexcept -> block_box_id
{
   assert(id_type == block_type::box);

   return id.box;
}

auto block_id::type() const noexcept -> block_type
{
   return id_type;
}

bool block_id::operator==(const block_id& other) const noexcept
{
   return this->id_type == other.id_type and
          (memcmp(&this->id, &other.id, sizeof(id)) == 0);
}

bool block_id::operator==(const block_box_id box_id) const noexcept
{
   return id_type == block_type::box and box_id == id.box;
}

block_id block_id::none = {};

}