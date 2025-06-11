#include "delete_block.hpp"

#include "world/blocks/bounding_box.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::edits {

namespace {

template<auto type_ptr>
struct delete_block final : edit<world::edit_context> {
   using blocks_type =
      std::remove_cvref_t<decltype(std::declval<world::blocks>().*type_ptr)>;
   using description_type = decltype(blocks_type::description)::value_type;
   using id_type = world::id<description_type>;

   delete_block(uint32 block_index) : block_index{block_index} {}

   void apply(world::edit_context& context) noexcept override
   {
      blocks_type& blocks = context.world.blocks.*type_ptr;

      blocks.bbox.min_x.erase(blocks.bbox.min_x.begin() + block_index);
      blocks.bbox.min_y.erase(blocks.bbox.min_y.begin() + block_index);
      blocks.bbox.min_z.erase(blocks.bbox.min_z.begin() + block_index);

      blocks.bbox.max_x.erase(blocks.bbox.max_x.begin() + block_index);
      blocks.bbox.max_y.erase(blocks.bbox.max_y.begin() + block_index);
      blocks.bbox.max_z.erase(blocks.bbox.max_z.begin() + block_index);

      hidden = blocks.hidden[block_index];
      layer = blocks.layer[block_index];
      block = blocks.description[block_index];
      id = blocks.ids[block_index];

      blocks.hidden.erase(blocks.hidden.begin() + block_index);
      blocks.layer.erase(blocks.layer.begin() + block_index);
      blocks.description.erase(blocks.description.begin() + block_index);
      blocks.ids.erase(blocks.ids.begin() + block_index);

      blocks.dirty.remove_index(block_index);
      blocks.dirty.add({block_index, static_cast<uint32>(blocks.size())});

      assert(blocks.is_balanced());
   }

   void revert(world::edit_context& context) noexcept override
   {
      blocks_type& blocks = context.world.blocks.*type_ptr;

      const math::bounding_box bbox = get_bounding_box(block);

      blocks.bbox.min_x.insert(blocks.bbox.min_x.begin() + block_index, bbox.min.x);
      blocks.bbox.min_y.insert(blocks.bbox.min_y.begin() + block_index, bbox.min.y);
      blocks.bbox.min_z.insert(blocks.bbox.min_z.begin() + block_index, bbox.min.z);

      blocks.bbox.max_x.insert(blocks.bbox.max_x.begin() + block_index, bbox.max.x);
      blocks.bbox.max_y.insert(blocks.bbox.max_y.begin() + block_index, bbox.max.y);
      blocks.bbox.max_z.insert(blocks.bbox.max_z.begin() + block_index, bbox.max.z);

      blocks.hidden.insert(blocks.hidden.begin() + block_index, hidden);
      blocks.layer.insert(blocks.layer.begin() + block_index, layer);
      blocks.description.insert(blocks.description.begin() + block_index, block);
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
   description_type block;
   id_type id = {};
};

template<auto type_ptr>
struct delete_block_custom_mesh final : edit<world::edit_context> {
   using blocks_type =
      std::remove_cvref_t<decltype(std::declval<world::blocks>().*type_ptr)>;
   using description_type = decltype(blocks_type::description)::value_type;
   using id_type = world::id<description_type>;

   delete_block_custom_mesh(uint32 block_index) : block_index{block_index} {}

   void apply(world::edit_context& context) noexcept override
   {
      blocks_type& blocks = context.world.blocks.*type_ptr;

      context.world.blocks.custom_meshes.remove(blocks.mesh[block_index]);

      blocks.bbox.min_x.erase(blocks.bbox.min_x.begin() + block_index);
      blocks.bbox.min_y.erase(blocks.bbox.min_y.begin() + block_index);
      blocks.bbox.min_z.erase(blocks.bbox.min_z.begin() + block_index);

      blocks.bbox.max_x.erase(blocks.bbox.max_x.begin() + block_index);
      blocks.bbox.max_y.erase(blocks.bbox.max_y.begin() + block_index);
      blocks.bbox.max_z.erase(blocks.bbox.max_z.begin() + block_index);

      blocks.mesh.erase(blocks.mesh.begin() + block_index);

      hidden = blocks.hidden[block_index];
      layer = blocks.layer[block_index];
      block = blocks.description[block_index];
      id = blocks.ids[block_index];

      blocks.hidden.erase(blocks.hidden.begin() + block_index);
      blocks.layer.erase(blocks.layer.begin() + block_index);
      blocks.description.erase(blocks.description.begin() + block_index);
      blocks.ids.erase(blocks.ids.begin() + block_index);

      blocks.dirty.remove_index(block_index);
      blocks.dirty.add({block_index, static_cast<uint32>(blocks.size())});

      assert(blocks.is_balanced());
   }

   void revert(world::edit_context& context) noexcept override
   {
      blocks_type& blocks = context.world.blocks.*type_ptr;

      const math::bounding_box bbox = get_bounding_box(block);

      blocks.bbox.min_x.insert(blocks.bbox.min_x.begin() + block_index, bbox.min.x);
      blocks.bbox.min_y.insert(blocks.bbox.min_y.begin() + block_index, bbox.min.y);
      blocks.bbox.min_z.insert(blocks.bbox.min_z.begin() + block_index, bbox.min.z);

      blocks.bbox.max_x.insert(blocks.bbox.max_x.begin() + block_index, bbox.max.x);
      blocks.bbox.max_y.insert(blocks.bbox.max_y.begin() + block_index, bbox.max.y);
      blocks.bbox.max_z.insert(blocks.bbox.max_z.begin() + block_index, bbox.max.z);

      blocks.hidden.insert(blocks.hidden.begin() + block_index, hidden);
      blocks.layer.insert(blocks.layer.begin() + block_index, layer);
      blocks.description.insert(blocks.description.begin() + block_index, block);
      blocks.mesh.insert(blocks.mesh.begin() + block_index,
                         context.world.blocks.custom_meshes.add(
                            block.custom_mesh_desc()));
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
   description_type block;
   id_type id = {};
};

}

auto make_delete_block(world::block_type type, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   switch (type) {
   case world::block_type::box:
      return std::make_unique<delete_block<&world::blocks::boxes>>(index);
   case world::block_type::ramp:
      return std::make_unique<delete_block<&world::blocks::ramps>>(index);
   case world::block_type::quad:
      return std::make_unique<delete_block<&world::blocks::quads>>(index);
   case world::block_type::cylinder:
      return std::make_unique<delete_block<&world::blocks::cylinders>>(index);
   case world::block_type::stairway:
      return std::make_unique<delete_block_custom_mesh<&world::blocks::stairways>>(index);
   case world::block_type::cone:
      return std::make_unique<delete_block<&world::blocks::cones>>(index);
   case world::block_type::hemisphere:
      return std::make_unique<delete_block<&world::blocks::hemispheres>>(index);
   case world::block_type::pyramid:
      return std::make_unique<delete_block<&world::blocks::pyramids>>(index);
   }

   std::unreachable();
}

}
