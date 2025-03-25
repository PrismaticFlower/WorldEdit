#include "set_block.hpp"

#include "math/bounding_box.hpp"
#include "math/vector_funcs.hpp"

namespace we::edits {

namespace {

struct set_block_box_metrics final : edit<world::edit_context> {
   set_block_box_metrics(const uint32 index, const quaternion& rotation,
                         const float3& position, const float3& size)
      : index{index}, rotation{rotation}, position{position}, size{size}
   {
      bbox = rotation * math::bounding_box{.min = -size, .max = size} + position;
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::blocks_boxes& blocks = context.world.blocks.boxes;

      assert(index < blocks.size());

      std::swap(blocks.bbox.min_x[index], bbox.min.x);
      std::swap(blocks.bbox.min_y[index], bbox.min.y);
      std::swap(blocks.bbox.min_z[index], bbox.min.z);

      std::swap(blocks.bbox.max_x[index], bbox.max.x);
      std::swap(blocks.bbox.max_y[index], bbox.max.y);
      std::swap(blocks.bbox.max_z[index], bbox.max.z);

      std::swap(blocks.description[index].rotation, rotation);
      std::swap(blocks.description[index].position, position);
      std::swap(blocks.description[index].size, size);

      blocks.dirty.add({index, index + 1});
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply(context);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_block_box_metrics* other =
         dynamic_cast<const set_block_box_metrics*>(&other_unknown);

      if (not other) return false;

      return this->index == other->index;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_block_box_metrics& other =
         dynamic_cast<set_block_box_metrics&>(other_unknown);

      this->bbox = other.bbox;
      this->rotation = other.rotation;
      this->position = other.position;
      this->size = other.size;
   }

private:
   const uint32 index = 0;

   math::bounding_box bbox;
   quaternion rotation;
   float3 position;
   float3 size;
};

struct set_block_box_surface final : edit<world::edit_context> {
   set_block_box_surface(const uint32 index, const uint32 surface_index,
                         const world::block_texture_rotation rotation)
      : index{index}, surface_index{surface_index}, rotation{rotation}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::blocks_boxes& blocks = context.world.blocks.boxes;

      assert(index < blocks.size());
      assert(surface_index < blocks.description[index].surface_texture_rotation.size());

      std::swap(blocks.description[index].surface_texture_rotation[surface_index],
                rotation);

      blocks.dirty.add({index, index + 1});
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply(context);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_block_box_surface* other =
         dynamic_cast<const set_block_box_surface*>(&other_unknown);

      if (not other) return false;

      return this->index == other->index and this->surface_index == other->surface_index;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_block_box_surface& other =
         dynamic_cast<set_block_box_surface&>(other_unknown);

      this->rotation = other.rotation;
   }

private:
   const uint32 index = 0;
   const uint32 surface_index = 0;

   world::block_texture_rotation rotation;
};

}

auto make_set_block_box_metrics(const uint32 index, const quaternion& rotation,
                                const float3& position, const float3& size) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_metrics>(index, rotation, position, size);
}

auto make_set_block_box_surface(const uint32 index, const uint32 surface_index,
                                const world::block_texture_rotation rotation) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface>(index, surface_index, rotation);
}

}
