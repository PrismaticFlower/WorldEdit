#include "add_block.hpp"

#include "world/blocks/bounding_box.hpp"

namespace we::edits {

namespace {

template<auto type_ptr>
struct add_block final : edit<world::edit_context> {
   using blocks_type =
      std::remove_cvref_t<decltype(std::declval<world::blocks>().*type_ptr)>;
   using description_type = decltype(blocks_type::description)::value_type;
   using id_type = world::id<description_type>;

   add_block(description_type block, int8 layer, id_type id)
      : block{block}, layer{layer}, id{id}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      blocks_type& blocks = context.world.blocks.*type_ptr;

      const math::bounding_box bbox = get_bounding_box(block);

      const uint32 block_index = static_cast<uint32>(blocks.size());

      blocks.bbox.min_x.push_back(bbox.min.x);
      blocks.bbox.min_y.push_back(bbox.min.y);
      blocks.bbox.min_z.push_back(bbox.min.z);

      blocks.bbox.max_x.push_back(bbox.max.x);
      blocks.bbox.max_y.push_back(bbox.max.y);
      blocks.bbox.max_z.push_back(bbox.max.z);

      blocks.hidden.push_back(false);
      blocks.layer.push_back(layer);
      blocks.description.push_back(block);
      blocks.ids.push_back(id);

      blocks.dirty.add({block_index, block_index + 1});

      assert(blocks.is_balanced());
   }

   void revert(world::edit_context& context) noexcept override
   {
      blocks_type& blocks = context.world.blocks.*type_ptr;

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
   description_type block;
   int8 layer;
   id_type id;
};

template<auto type_ptr>
struct add_block_custom_mesh final : edit<world::edit_context> {
   using blocks_type =
      std::remove_cvref_t<decltype(std::declval<world::blocks>().*type_ptr)>;
   using description_type = decltype(blocks_type::description)::value_type;
   using id_type = world::id<description_type>;

   add_block_custom_mesh(description_type block, int8 layer, id_type id)
      : block{block}, layer{layer}, id{id}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      blocks_type& blocks = context.world.blocks.*type_ptr;

      const math::bounding_box bbox = get_bounding_box(block);

      const uint32 block_index = static_cast<uint32>(blocks.size());

      blocks.bbox.min_x.push_back(bbox.min.x);
      blocks.bbox.min_y.push_back(bbox.min.y);
      blocks.bbox.min_z.push_back(bbox.min.z);

      blocks.bbox.max_x.push_back(bbox.max.x);
      blocks.bbox.max_y.push_back(bbox.max.y);
      blocks.bbox.max_z.push_back(bbox.max.z);

      blocks.hidden.push_back(false);
      blocks.layer.push_back(layer);
      blocks.description.push_back(block);
      blocks.mesh.push_back(
         context.world.blocks.custom_meshes.add(block.custom_mesh_desc()));
      blocks.ids.push_back(id);

      blocks.dirty.add({block_index, block_index + 1});

      assert(blocks.is_balanced());
   }

   void revert(world::edit_context& context) noexcept override
   {
      blocks_type& blocks = context.world.blocks.*type_ptr;

      context.world.blocks.custom_meshes.remove(blocks.mesh.back());

      blocks.bbox.min_x.pop_back();
      blocks.bbox.min_y.pop_back();
      blocks.bbox.min_z.pop_back();

      blocks.bbox.max_x.pop_back();
      blocks.bbox.max_y.pop_back();
      blocks.bbox.max_z.pop_back();

      blocks.hidden.pop_back();
      blocks.layer.pop_back();
      blocks.description.pop_back();
      blocks.mesh.pop_back();
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
   description_type block;
   int8 layer;
   id_type id;
};

}

auto make_add_block(world::block_description_box box, int8 layer, world::block_box_id id)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_block<&world::blocks::boxes>>(box, layer, id);
}

auto make_add_block(world::block_description_ramp ramp, int8 layer,
                    world::block_ramp_id id)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_block<&world::blocks::ramps>>(ramp, layer, id);
}

auto make_add_block(world::block_description_quad quad, int8 layer,
                    world::block_quad_id id)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_block<&world::blocks::quads>>(quad, layer, id);
}

auto make_add_block(world::block_description_cylinder cylinder, int8 layer,
                    world::block_cylinder_id id)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_block<&world::blocks::cylinders>>(cylinder, layer, id);
}

auto make_add_block(world::block_description_stairway stairway, int8 layer,
                    world::block_stairway_id id)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_block_custom_mesh<&world::blocks::stairways>>(stairway,
                                                                             layer, id);
}

}
