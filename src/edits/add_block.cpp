#include "add_block.hpp"

#include "math/bounding_box.hpp"
#include "math/vector_funcs.hpp"

namespace we::edits {

namespace {

bool is_balanced(const world::blocks_cubes& blocks) noexcept
{
   return blocks.bbox.min_x.size() == blocks.bbox.min_y.size() and
          blocks.bbox.min_x.size() == blocks.bbox.min_z.size() and
          blocks.bbox.min_x.size() == blocks.bbox.max_x.size() and
          blocks.bbox.min_x.size() == blocks.bbox.max_y.size() and
          blocks.bbox.min_x.size() == blocks.bbox.max_z.size() and
          blocks.bbox.min_x.size() == blocks.hidden.size() and
          blocks.bbox.min_x.size() == blocks.description.size();
}

struct add_block final : edit<world::edit_context> {
   add_block(world::block_description_cube box) : box{box} {}

   void apply(world::edit_context& context) noexcept override
   {
      world::blocks_cubes& blocks = context.world.blocks.cubes;

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
      blocks.description.push_back(box);

      blocks.dirty.add({block_index, block_index + 1});

      assert(is_balanced(blocks));
   }

   void revert(world::edit_context& context) noexcept override
   {
      world::blocks_cubes& blocks = context.world.blocks.cubes;

      blocks.bbox.min_x.pop_back();
      blocks.bbox.min_y.pop_back();
      blocks.bbox.min_z.pop_back();

      blocks.bbox.max_x.pop_back();
      blocks.bbox.max_y.pop_back();
      blocks.bbox.max_z.pop_back();

      blocks.hidden.pop_back();
      blocks.description.pop_back();

      const uint32 block_index = static_cast<uint32>(blocks.size());

      blocks.dirty.remove_index(block_index);

      assert(is_balanced(blocks));
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   world::block_description_cube box;
};

}

auto make_add_block(world::block_description_cube box)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_block>(box);
}

}
