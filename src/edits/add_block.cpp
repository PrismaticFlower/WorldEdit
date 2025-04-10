#include "add_block.hpp"

#include "math/bounding_box.hpp"
#include "math/vector_funcs.hpp"

namespace we::edits {

namespace {

struct add_block final : edit<world::edit_context> {
   add_block(world::block_description_box box, int8 layer, world::block_box_id id)
      : box{box}, layer{layer}, id{id}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::blocks_boxes& blocks = context.world.blocks.boxes;

      const math::bounding_box bbox =
         box.rotation * math::bounding_box{.min = -box.size, .max = box.size} +
         box.position;

      const uint32 block_index = static_cast<uint32>(blocks.size());

      blocks.bbox.min_x.push_back(bbox.min.x);
      blocks.bbox.min_y.push_back(bbox.min.y);
      blocks.bbox.min_z.push_back(bbox.min.z);

      blocks.bbox.max_x.push_back(bbox.max.x);
      blocks.bbox.max_y.push_back(bbox.max.y);
      blocks.bbox.max_z.push_back(bbox.max.z);

      blocks.hidden.push_back(false);
      blocks.layer.push_back(layer);
      blocks.description.push_back(box);
      blocks.ids.push_back(id);

      blocks.dirty.add({block_index, block_index + 1});

      assert(blocks.is_balanced());
   }

   void revert(world::edit_context& context) noexcept override
   {
      world::blocks_boxes& blocks = context.world.blocks.boxes;

      blocks.bbox.min_x.pop_back();
      blocks.bbox.min_y.pop_back();
      blocks.bbox.min_z.pop_back();

      blocks.bbox.max_x.pop_back();
      blocks.bbox.max_y.pop_back();
      blocks.bbox.max_z.pop_back();

      blocks.hidden.pop_back();
      blocks.layer.pop_back();
      blocks.description.pop_back();
      blocks.ids.pop_back();

      const uint32 block_index = static_cast<uint32>(blocks.size());

      blocks.dirty.remove_index(block_index);

      assert(blocks.is_balanced());
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   world::block_description_box box;
   int8 layer;
   world::block_box_id id;
};

}

auto make_add_block(world::block_description_box box, int8 layer, world::block_box_id id)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_block>(box, layer, id);
}

}
