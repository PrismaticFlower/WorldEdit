#include "set_block.hpp"

#include "world/blocks/bounding_box.hpp"

namespace we::edits {

namespace {

template<auto type_ptr>
struct set_block_metrics final : edit<world::edit_context> {
   using blocks_type =
      std::remove_cvref_t<decltype(std::declval<world::blocks>().*type_ptr)>;

   set_block_metrics(const uint32 index, const quaternion& rotation,
                     const float3& position, const float3& size)
      : index{index}, rotation{rotation}, position{position}, size{size}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      blocks_type& blocks = context.world.blocks.*type_ptr;

      assert(index < blocks.size());

      std::swap(blocks.description[index].rotation, rotation);
      std::swap(blocks.description[index].position, position);
      std::swap(blocks.description[index].size, size);

      const math::bounding_box bbox = get_bounding_box(blocks.description[index]);

      blocks.bbox.min_x[index] = bbox.min.x;
      blocks.bbox.min_y[index] = bbox.min.y;
      blocks.bbox.min_z[index] = bbox.min.z;
      blocks.bbox.max_x[index] = bbox.max.x;
      blocks.bbox.max_y[index] = bbox.max.y;
      blocks.bbox.max_z[index] = bbox.max.z;

      blocks.dirty.add({index, index + 1});
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply(context);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_block_metrics* other =
         dynamic_cast<const set_block_metrics*>(&other_unknown);

      if (not other) return false;

      return this->index == other->index;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_block_metrics& other = dynamic_cast<set_block_metrics&>(other_unknown);

      this->rotation = other.rotation;
      this->position = other.position;
      this->size = other.size;
   }

private:
   const uint32 index = 0;

   quaternion rotation;
   float3 position;
   float3 size;
};

struct set_block_quad_metrics final : edit<world::edit_context> {
   set_block_quad_metrics(const uint32 index, const std::array<float3, 4>& vertices)
      : index{index}, vertices{vertices}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::blocks_quads& blocks = context.world.blocks.quads;

      assert(index < blocks.size());

      std::swap(blocks.description[index].vertices, vertices);

      const math::bounding_box bbox = get_bounding_box(blocks.description[index]);

      blocks.bbox.min_x[index] = bbox.min.x;
      blocks.bbox.min_y[index] = bbox.min.y;
      blocks.bbox.min_z[index] = bbox.min.z;
      blocks.bbox.max_x[index] = bbox.max.x;
      blocks.bbox.max_y[index] = bbox.max.y;
      blocks.bbox.max_z[index] = bbox.max.z;

      blocks.dirty.add({index, index + 1});
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply(context);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_block_quad_metrics* other =
         dynamic_cast<const set_block_quad_metrics*>(&other_unknown);

      if (not other) return false;

      return this->index == other->index;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_block_quad_metrics& other =
         dynamic_cast<set_block_quad_metrics&>(other_unknown);

      this->vertices = other.vertices;
   }

private:
   const uint32 index = 0;

   std::array<float3, 4> vertices;
};

struct set_block_stairway_metrics final : edit<world::edit_context> {
   set_block_stairway_metrics(const uint32 index, const quaternion& rotation,
                              const float3& position, const float3& size,
                              const float step_height, const float first_step_offset)
      : index{index},
        rotation{rotation},
        position{position},
        size{size},
        step_height{step_height},
        first_step_offset{first_step_offset}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::blocks_stairways& blocks = context.world.blocks.stairways;

      assert(index < blocks.size());

      const world::block_custom_mesh_stairway_desc start_custom_mesh_desc =
         blocks.description[index].custom_mesh_desc();

      std::swap(blocks.description[index].rotation, rotation);
      std::swap(blocks.description[index].position, position);
      std::swap(blocks.description[index].size, size);
      std::swap(blocks.description[index].step_height, step_height);
      std::swap(blocks.description[index].first_step_offset, first_step_offset);

      const math::bounding_box bbox = get_bounding_box(blocks.description[index]);

      blocks.bbox.min_x[index] = bbox.min.x;
      blocks.bbox.min_y[index] = bbox.min.y;
      blocks.bbox.min_z[index] = bbox.min.z;
      blocks.bbox.max_x[index] = bbox.max.x;
      blocks.bbox.max_y[index] = bbox.max.y;
      blocks.bbox.max_z[index] = bbox.max.z;

      // Regenerating a custom mesh can be a heavy operation, skip it if we can. This causes no observable behaviour difference,
      // it just saves some time.
      if (start_custom_mesh_desc != blocks.description[index].custom_mesh_desc()) {
         context.world.blocks.custom_meshes.remove(blocks.mesh[index]);

         blocks.mesh[index] = context.world.blocks.custom_meshes.add(
            blocks.description[index].custom_mesh_desc());
      }

      blocks.dirty.add({index, index + 1});
   }

   void revert(world::edit_context& context) noexcept override
   {
      apply(context);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_block_stairway_metrics* other =
         dynamic_cast<const set_block_stairway_metrics*>(&other_unknown);

      if (not other) return false;

      return this->index == other->index;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_block_stairway_metrics& other =
         dynamic_cast<set_block_stairway_metrics&>(other_unknown);

      this->rotation = other.rotation;
      this->position = other.position;
      this->size = other.size;
   }

private:
   const uint32 index = 0;

   quaternion rotation;
   float3 position;
   float3 size;
   float step_height;
   float first_step_offset;
};

template<typename T>
struct set_block_box_surface final : edit<world::edit_context> {
   set_block_box_surface(T* value_address, T new_value, const uint32 index,
                         world::blocks_dirty_range_tracker* dirt_tracker)
      : value_address{value_address},
        value{std::move(new_value)},
        index{index},
        dirt_tracker{dirt_tracker}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(dirt_tracker));
      assert(context.is_memory_valid(value_address));

      std::swap(*value_address, value);

      dirt_tracker->add({index, index + 1});
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

      return this->value_address == other->value_address and
             this->dirt_tracker == other->dirt_tracker;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_block_box_surface& other =
         dynamic_cast<set_block_box_surface&>(other_unknown);

      this->value = other.value;
   }

private:
   T* value_address;
   T value;
   const uint32 index;
   world::blocks_dirty_range_tracker* dirt_tracker;
};

}

auto make_set_block_box_metrics(const uint32 index, const quaternion& rotation,
                                const float3& position, const float3& size) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_metrics<&world::blocks::boxes>>(index, rotation,
                                                                     position, size);
}

auto make_set_block_ramp_metrics(const uint32 index, const quaternion& rotation,
                                 const float3& position, const float3& size) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_metrics<&world::blocks::ramps>>(index, rotation,
                                                                     position, size);
}

auto make_set_block_quad_metrics(const uint32 index,
                                 const std::array<float3, 4>& vertices) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_quad_metrics>(index, vertices);
}

auto make_set_block_cylinder_metrics(const uint32 index, const quaternion& rotation,
                                     const float3& position, const float3& size) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_metrics<&world::blocks::cylinders>>(index, rotation,
                                                                         position,
                                                                         size);
}

auto make_set_block_stairway_metrics(const uint32 index, const quaternion& rotation,
                                     const float3& position, const float3& size,
                                     const float step_height,
                                     const float first_step_offset) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_stairway_metrics>(index, rotation, position,
                                                       size, step_height,
                                                       first_step_offset);
}

auto make_set_block_surface(uint8* material_index_address,
                            uint8 new_material_index, const uint32 index,
                            world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<uint8>>(material_index_address,
                                                         new_material_index,
                                                         index, dirt_tracker);
}

auto make_set_block_surface(world::block_texture_mode* mode_address,
                            world::block_texture_mode new_mode, const uint32 index,
                            world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<world::block_texture_mode>>(
      mode_address, new_mode, index, dirt_tracker);
}

auto make_set_block_surface(world::block_texture_rotation* rotation_address,
                            world::block_texture_rotation new_rotation,
                            const uint32 index,
                            world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<world::block_texture_rotation>>(
      rotation_address, new_rotation, index, dirt_tracker);
}

auto make_set_block_surface(std::array<int8, 2>* scale_address,
                            std::array<int8, 2> new_scale, const uint32 index,
                            world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<std::array<int8, 2>>>(scale_address,
                                                                       new_scale, index,
                                                                       dirt_tracker);
}

auto make_set_block_surface(std::array<std::uint16_t, 2>* offset_address,
                            std::array<std::uint16_t, 2> new_offset, const uint32 index,
                            world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<std::array<uint16_t, 2>>>(offset_address,
                                                                           new_offset,
                                                                           index,
                                                                           dirt_tracker);
}

auto make_set_block_material(std::string* texture_address,
                             std::string new_texture, const uint32 index,
                             world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<std::string>>(texture_address,
                                                               std::move(new_texture),
                                                               index, dirt_tracker);
}

auto make_set_block_material(std::array<uint8, 2>* tiling_address,
                             std::array<uint8, 2> new_tiling, const uint32 index,
                             world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<std::array<uint8, 2>>>(tiling_address,
                                                                        new_tiling, index,
                                                                        dirt_tracker);
}

auto make_set_block_material(bool* flag_address, bool new_flag, const uint32 index,
                             world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<bool>>(flag_address, new_flag,
                                                        index, dirt_tracker);
}

auto make_set_block_material(float3* color_address, float3 new_color, const uint32 index,
                             world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<float3>>(color_address, new_color,
                                                          index, dirt_tracker);
}

auto make_set_block_material(world::block_material* material_address,
                             world::block_material new_material, const uint32 index,
                             world::blocks_dirty_range_tracker* dirt_tracker) noexcept
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_block_box_surface<world::block_material>>(
      material_address, std::move(new_material), index, dirt_tracker);
}

}
