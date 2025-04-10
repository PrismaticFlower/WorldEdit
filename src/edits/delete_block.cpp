#include "delete_block.hpp"

#include "math/bounding_box.hpp"
#include "math/vector_funcs.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::edits {

namespace {

struct delete_block_box final : edit<world::edit_context> {
   delete_block_box(uint32 block_index) : block_index{block_index} {}

   void apply(world::edit_context& context) noexcept override
   {
      world::blocks_boxes& blocks = context.world.blocks.boxes;

      blocks.bbox.min_x.erase(blocks.bbox.min_x.begin() + block_index);
      blocks.bbox.min_y.erase(blocks.bbox.min_y.begin() + block_index);
      blocks.bbox.min_z.erase(blocks.bbox.min_z.begin() + block_index);

      blocks.bbox.max_x.erase(blocks.bbox.max_x.begin() + block_index);
      blocks.bbox.max_y.erase(blocks.bbox.max_y.begin() + block_index);
      blocks.bbox.max_z.erase(blocks.bbox.max_z.begin() + block_index);

      hidden = blocks.hidden[block_index];
      layer = blocks.layer[block_index];
      box = blocks.description[block_index];
      id = blocks.ids[block_index];

      blocks.hidden.erase(blocks.hidden.begin() + block_index);
      blocks.layer.erase(blocks.layer.begin() + block_index);
      blocks.description.erase(blocks.description.begin() + block_index);
      blocks.ids.erase(blocks.ids.begin() + block_index);

      blocks.dirty.remove_index(block_index);

      if (block_index != static_cast<uint32>(blocks.size())) {
         blocks.dirty.add({block_index, static_cast<uint32>(blocks.size())});
      }

      assert(blocks.is_balanced());
   }

   void revert(world::edit_context& context) noexcept override
   {
      world::blocks_boxes& blocks = context.world.blocks.boxes;

      const math::bounding_box bbox =
         box.rotation * math::bounding_box{.min = -box.size, .max = box.size} +
         box.position;

      blocks.bbox.min_x.insert(blocks.bbox.min_x.begin() + block_index, bbox.min.x);
      blocks.bbox.min_y.insert(blocks.bbox.min_y.begin() + block_index, bbox.min.y);
      blocks.bbox.min_z.insert(blocks.bbox.min_z.begin() + block_index, bbox.min.z);

      blocks.bbox.max_x.insert(blocks.bbox.max_x.begin() + block_index, bbox.max.x);
      blocks.bbox.max_y.insert(blocks.bbox.max_y.begin() + block_index, bbox.max.y);
      blocks.bbox.max_z.insert(blocks.bbox.max_z.begin() + block_index, bbox.max.z);

      blocks.hidden.insert(blocks.hidden.begin() + block_index, hidden);
      blocks.layer.insert(blocks.layer.begin() + block_index, layer);
      blocks.description.insert(blocks.description.begin() + block_index, box);
      blocks.ids.insert(blocks.ids.begin() + block_index, id);

      blocks.dirty.add({block_index, static_cast<uint32>(blocks.size())});

      assert(blocks.is_balanced());
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   uint32 block_index;

   bool hidden = false;
   int8 layer = 0;
   world::block_description_box box;
   world::block_box_id id = {};
};

}

auto make_delete_block(world::block_type type, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   switch (type) {
   case world::block_type::box:
      return std::make_unique<delete_block_box>(index);
   }

   std::unreachable();
}

}
