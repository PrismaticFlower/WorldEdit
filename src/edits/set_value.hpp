
#include "edit.hpp"
#include "world/interaction_context.hpp"

namespace we::edits {

template<typename Entity, typename T>
struct set_creation_value final : edit<world::edit_context> {
   using entity_type = Entity;
   using value_type = T;

   set_creation_value(value_type entity_type::*value_member_ptr,
                      value_type new_value, value_type original_value)
      : value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = new_value;
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_value* other =
         dynamic_cast<const set_creation_value*>(&other_unknown);

      if (not other) return false;

      return this->value_member_ptr == other->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_value& other = dynamic_cast<set_creation_value&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   value_type entity_type::*value_member_ptr;

   value_type new_value;
   value_type original_value;
};

template<typename Entity, typename T, typename U>
struct set_creation_value_with_meta final : edit<world::edit_context> {
   using entity_type = Entity;
   using value_type = T;
   using meta_value_type = U;

   set_creation_value_with_meta(value_type entity_type::*value_member_ptr,
                                value_type new_value, value_type original_value,
                                meta_value_type world::edit_context::*meta_value_member_ptr,
                                meta_value_type meta_new_value,
                                meta_value_type meta_original_value)
      : value_member_ptr{value_member_ptr},
        meta_value_member_ptr{meta_value_member_ptr},
        new_value{std::move(new_value)},
        meta_new_value{std::move(meta_new_value)},
        original_value{std::move(original_value)},
        meta_original_value{std::move(meta_original_value)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = new_value;
      context.*meta_value_member_ptr = meta_new_value;
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = original_value;
      context.*meta_value_member_ptr = meta_original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_value_with_meta* other =
         dynamic_cast<const set_creation_value_with_meta*>(&other_unknown);

      if (not other) return false;

      return this->value_member_ptr == other->value_member_ptr and
             this->meta_value_member_ptr == other->meta_value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_value_with_meta& other =
         dynamic_cast<set_creation_value_with_meta&>(other_unknown);

      new_value = std::move(other.new_value);
      meta_new_value = std::move(other.meta_new_value);
   }

   value_type entity_type::*value_member_ptr;
   meta_value_type world::edit_context::*meta_value_member_ptr;

   value_type new_value;
   meta_value_type meta_new_value;

   value_type original_value;
   meta_value_type meta_original_value;
};

template<typename Entity>
struct set_creation_location final : edit<world::edit_context> {
   using entity_type = Entity;

   set_creation_location(quaternion new_rotation, quaternion original_rotation,
                         float3 new_position, float3 original_position,
                         float3 new_euler_rotation, float3 original_euler_rotation)
      : new_rotation{new_rotation},
        new_position{new_position},
        new_euler_rotation{new_euler_rotation},
        original_rotation{original_rotation},
        original_position{original_position},
        original_euler_rotation{original_euler_rotation}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::get<entity_type>(*context.creation_entity).rotation = new_rotation;
      std::get<entity_type>(*context.creation_entity).position = new_position;
      context.euler_rotation = new_euler_rotation;
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::get<entity_type>(*context.creation_entity).rotation = original_rotation;
      std::get<entity_type>(*context.creation_entity).position = original_position;
      context.euler_rotation = original_euler_rotation;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_location* other =
         dynamic_cast<const set_creation_location*>(&other_unknown);

      return other != nullptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_location& other =
         dynamic_cast<set_creation_location&>(other_unknown);

      new_rotation = other.new_rotation;
      new_position = other.new_position;
      new_euler_rotation = other.new_euler_rotation;
   }

   quaternion new_rotation;
   float3 new_position;
   float3 new_euler_rotation;

   quaternion original_rotation;
   float3 original_position;
   float3 original_euler_rotation;
};

template<typename T>
struct set_creation_path_node_value final : edit<world::edit_context> {
   using value_type = T;

   set_creation_path_node_value(value_type world::path::node::*value_member_ptr,
                                value_type new_value, value_type original_value)
      : value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::get<world::path>(*context.creation_entity).nodes[0].*value_member_ptr =
         new_value;
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::get<world::path>(*context.creation_entity).nodes[0].*value_member_ptr =
         original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_path_node_value* other =
         dynamic_cast<const set_creation_path_node_value*>(&other_unknown);

      if (not other) return false;

      return this->value_member_ptr == other->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_path_node_value& other =
         dynamic_cast<set_creation_path_node_value&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   value_type world::path::node::*value_member_ptr;

   value_type new_value;
   value_type original_value;
};

struct set_creation_path_node_location final : edit<world::edit_context> {
   set_creation_path_node_location(quaternion new_rotation, quaternion original_rotation,
                                   float3 new_position, float3 original_position,
                                   float3 new_euler_rotation,
                                   float3 original_euler_rotation)
      : new_rotation{new_rotation},
        new_position{new_position},
        new_euler_rotation{new_euler_rotation},
        original_rotation{original_rotation},
        original_position{original_position},
        original_euler_rotation{original_euler_rotation}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::get<world::path>(*context.creation_entity).nodes[0].rotation = new_rotation;
      std::get<world::path>(*context.creation_entity).nodes[0].position = new_position;
      context.euler_rotation = new_euler_rotation;
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::get<world::path>(*context.creation_entity).nodes[0].rotation =
         original_rotation;
      std::get<world::path>(*context.creation_entity).nodes[0].position =
         original_position;
      context.euler_rotation = original_euler_rotation;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_path_node_location* other =
         dynamic_cast<const set_creation_path_node_location*>(&other_unknown);

      return other != nullptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_path_node_location& other =
         dynamic_cast<set_creation_path_node_location&>(other_unknown);

      new_rotation = other.new_rotation;
      new_position = other.new_position;
      new_euler_rotation = other.new_euler_rotation;
   }

   quaternion new_rotation;
   float3 new_position;
   float3 new_euler_rotation;

   quaternion original_rotation;
   float3 original_position;
   float3 original_euler_rotation;
};

struct set_creation_region_metrics final : edit<world::edit_context> {
   set_creation_region_metrics(quaternion new_rotation, quaternion original_rotation,
                               float3 new_position, float3 original_position,
                               float3 new_size, float3 original_size)
      : new_rotation{new_rotation},
        new_position{new_position},
        new_size{new_size},
        original_rotation{original_rotation},
        original_position{original_position},
        original_size{original_size}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::get<world::region>(*context.creation_entity).rotation = new_rotation;
      std::get<world::region>(*context.creation_entity).position = new_position;
      std::get<world::region>(*context.creation_entity).size = new_size;
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::get<world::region>(*context.creation_entity).rotation = original_rotation;
      std::get<world::region>(*context.creation_entity).position = original_position;
      std::get<world::region>(*context.creation_entity).size = original_size;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_region_metrics* other =
         dynamic_cast<const set_creation_region_metrics*>(&other_unknown);

      return other != nullptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_region_metrics& other =
         dynamic_cast<set_creation_region_metrics&>(other_unknown);

      new_rotation = other.new_rotation;
      new_position = other.new_position;
      new_size = other.new_size;
   }

   quaternion new_rotation;
   float3 new_position;
   float3 new_size;

   quaternion original_rotation;
   float3 original_position;
   float3 original_size;
};

struct set_creation_sector_point final : edit<world::edit_context> {
   set_creation_sector_point(float2 new_position, float2 original_position)
      : new_position{new_position}, original_position{original_position}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      std::get<world::sector>(*context.creation_entity).points[0] = new_position;
   }

   void revert(world::edit_context& context) const noexcept override
   {
      std::get<world::sector>(*context.creation_entity).points[0] = original_position;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_creation_sector_point* other =
         dynamic_cast<const set_creation_sector_point*>(&other_unknown);

      return other != nullptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_creation_sector_point& other =
         dynamic_cast<set_creation_sector_point&>(other_unknown);

      new_position = other.new_position;
   }

   float2 new_position;
   float2 original_position;
};

}
