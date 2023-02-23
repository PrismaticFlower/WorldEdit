
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

   void apply(world::edit_context& context) const noexcept
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = new_value;
   }

   void revert(world::edit_context& context) const noexcept
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = original_value;
   }

   bool coalescable(edit<world::edit_context>& edit) const noexcept
   {
      set_creation_value* other = dynamic_cast<set_creation_value*>(&edit);

      if (not other) return false;

      return this->value_member_ptr == other->value_member_ptr;
   }

   value_type entity_type::*value_member_ptr;

   value_type new_value;
   value_type original_value;

   bool closed = false;
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

   void apply(world::edit_context& context) const noexcept
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = new_value;
      context.*meta_value_member_ptr = meta_new_value;
   }

   void revert(world::edit_context& context) const noexcept
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = original_value;
      context.*meta_value_member_ptr = meta_original_value;
   }

   bool coalescable(edit<world::edit_context>& edit) const noexcept
   {
      set_creation_value_with_meta* other =
         dynamic_cast<set_creation_value_with_meta*>(&edit);

      if (not other) return false;

      return this->value_member_ptr == other->value_member_ptr and
             this->meta_value_member_ptr == other->meta_value_member_ptr;
   }

   value_type entity_type::*value_member_ptr;
   meta_value_type world::edit_context::*meta_value_member_ptr;

   value_type new_value;
   meta_value_type meta_new_value;

   value_type original_value;
   meta_value_type meta_original_value;

   bool closed = false;
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

   void apply(world::edit_context& context) const noexcept
   {
      std::get<entity_type>(*context.creation_entity).rotation = new_rotation;
      std::get<entity_type>(*context.creation_entity).position = new_position;
      context.euler_rotation = new_euler_rotation;
   }

   void revert(world::edit_context& context) const noexcept
   {
      std::get<entity_type>(*context.creation_entity).rotation = original_rotation;
      std::get<entity_type>(*context.creation_entity).position = original_position;
      context.euler_rotation = original_euler_rotation;
   }

   bool coalescable(edit<world::edit_context>& edit) const noexcept
   {
      set_creation_location* other = dynamic_cast<set_creation_location*>(&edit);

      return other != nullptr;
   }

   quaternion new_rotation;
   float3 new_position;
   float3 new_euler_rotation;

   quaternion original_rotation;
   float3 original_position;
   float3 original_euler_rotation;

   bool closed = false;
};

}
