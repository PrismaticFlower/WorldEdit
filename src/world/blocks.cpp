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

void blocks_ramps::reserve(const std::size_t size) noexcept
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

auto blocks_ramps::size() const noexcept -> std::size_t
{
   assert(is_balanced());

   return bbox.min_x.size();
}

bool blocks_ramps::is_balanced() const noexcept
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

void blocks_quads::reserve(const std::size_t size) noexcept
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

auto blocks_quads::size() const noexcept -> std::size_t
{
   assert(is_balanced());

   return bbox.min_x.size();
}

bool blocks_quads::is_balanced() const noexcept
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

void blocks_custom::reserve(const std::size_t size) noexcept
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
   mesh.reserve(size);
   ids.reserve(size);
}

auto blocks_custom::size() const noexcept -> std::size_t
{
   assert(is_balanced());

   return bbox.min_x.size();
}

bool blocks_custom::is_balanced() const noexcept
{
   return bbox.min_x.size() == bbox.min_y.size() and
          bbox.min_x.size() == bbox.min_z.size() and
          bbox.min_x.size() == bbox.max_x.size() and
          bbox.min_x.size() == bbox.max_y.size() and
          bbox.min_x.size() == bbox.max_z.size() and
          bbox.min_x.size() == hidden.size() and
          bbox.min_x.size() == layer.size() and //
          bbox.min_x.size() == description.size() and
          bbox.min_x.size() == mesh.size() and //
          bbox.min_x.size() == ids.size();
}

void blocks_cones::reserve(const std::size_t size) noexcept
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

auto blocks_cones::size() const noexcept -> std::size_t
{
   assert(is_balanced());

   return bbox.min_x.size();
}

bool blocks_cones::is_balanced() const noexcept
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

void blocks_hemispheres::reserve(const std::size_t size) noexcept
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

auto blocks_hemispheres::size() const noexcept -> std::size_t
{
   assert(is_balanced());

   return bbox.min_x.size();
}

bool blocks_hemispheres::is_balanced() const noexcept
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

void blocks_pyramids::reserve(const std::size_t size) noexcept
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

auto blocks_pyramids::size() const noexcept -> std::size_t
{
   assert(is_balanced());

   return bbox.min_x.size();
}

bool blocks_pyramids::is_balanced() const noexcept
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
   return boxes.size() == 0 and  //
          ramps.size() == 0 and  //
          quads.size() == 0 and  //
          custom.size() == 0 and //
          cones.size() == 0 and  //
          hemispheres.size() == 0;
}

void blocks::untracked_fill_dirty_ranges() noexcept
{
   if (boxes.size() != 0) {
      boxes.dirty.add({0, static_cast<uint32>(boxes.size())});
   }

   if (ramps.size() != 0) {
      ramps.dirty.add({0, static_cast<uint32>(ramps.size())});
   }

   if (quads.size() != 0) {
      quads.dirty.add({0, static_cast<uint32>(quads.size())});
   }

   if (custom.size() != 0) {
      custom.dirty.add({0, static_cast<uint32>(custom.size())});
   }

   if (cones.size() != 0) {
      cones.dirty.add({0, static_cast<uint32>(cones.size())});
   }

   if (hemispheres.size() != 0) {
      hemispheres.dirty.add({0, static_cast<uint32>(hemispheres.size())});
   }

   if (pyramids.size() != 0) {
      pyramids.dirty.add({0, static_cast<uint32>(pyramids.size())});
   }

   materials_dirty.add({0, static_cast<uint32>(materials.size())});
}

void blocks::untracked_clear_dirty_ranges() noexcept
{
   boxes.dirty.clear();
   ramps.dirty.clear();
   quads.dirty.clear();
   custom.dirty.clear();
   cones.dirty.clear();
   hemispheres.dirty.clear();
   pyramids.dirty.clear();
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

block_id::block_id(block_ramp_id id) noexcept
   : id_type{block_type::ramp}, id{.ramp = id}
{
}

block_id::block_id(block_quad_id id) noexcept
   : id_type{block_type::quad}, id{.quad = id}
{
}

block_id::block_id(block_custom_id id) noexcept
   : id_type{block_type::custom}, id{.custom = id}
{
}

block_id::block_id(block_cone_id id) noexcept
   : id_type{block_type::cone}, id{.cone = id}
{
}

block_id::block_id(block_hemisphere_id id) noexcept
   : id_type{block_type::hemisphere}, id{.hemisphere = id}
{
}

block_id::block_id(block_pyramid_id id) noexcept
   : id_type{block_type::pyramid}, id{.pyramid = id}
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

bool block_id::is_ramp() const noexcept
{
   return id_type == block_type::ramp;
}

auto block_id::get_ramp() const noexcept -> block_ramp_id
{
   assert(id_type == block_type::ramp);

   return id.ramp;
}

bool block_id::is_quad() const noexcept
{
   return id_type == block_type::quad;
}

auto block_id::get_quad() const noexcept -> block_quad_id
{
   assert(id_type == block_type::quad);

   return id.quad;
}

bool block_id::is_custom() const noexcept
{
   return id_type == block_type::custom;
}

auto block_id::get_custom() const noexcept -> block_custom_id
{
   assert(id_type == block_type::custom);

   return id.custom;
}

bool block_id::is_cone() const noexcept
{
   return id_type == block_type::cone;
}

auto block_id::get_cone() const noexcept -> block_cone_id
{
   assert(id_type == block_type::cone);

   return id.cone;
}

bool block_id::is_hemisphere() const noexcept
{
   return id_type == block_type::hemisphere;
}

auto block_id::get_hemisphere() const noexcept -> block_hemisphere_id
{
   assert(id_type == block_type::hemisphere);

   return id.hemisphere;
}

bool block_id::is_pyramid() const noexcept
{
   return id_type == block_type::pyramid;
}

auto block_id::get_pyramid() const noexcept -> block_pyramid_id
{
   assert(id_type == block_type::pyramid);

   return id.pyramid;
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

block_id block_id::none = {};

}