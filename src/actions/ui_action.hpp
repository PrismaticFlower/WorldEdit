#pragma once

#include "action.hpp"
#include "types.hpp"
#include "world/world.hpp"
#include "world/world_utilities.hpp"

namespace we::actions {

template<typename Entity, typename T>
struct ui_action final : action {
   using entity_type = Entity;
   using entity_id_type = world::id<entity_type>;
   using value_type = T;

   ui_action(entity_id_type id, value_type entity_type::*value_member_ptr,
             value_type new_value, value_type original_value)
      : id{id}, value_member_ptr{value_member_ptr}, new_value{new_value}, original_value{original_value}
   {
   }

   void apply(world::world& world) noexcept
   {
      find_entity<entity_type>(world, id)->*value_member_ptr = new_value;
   }

   void revert(world::world& world) noexcept
   {
      find_entity<entity_type>(world, id)->*value_member_ptr = original_value;
   }

   bool matching(entity_type& object,
                 value_type entity_type::*other_value_member_ptr) const noexcept
   {
      return id == object.id and other_value_member_ptr == this->value_member_ptr;
   }

   entity_id_type id;
   value_type entity_type::*value_member_ptr;

   value_type new_value;
   value_type original_value;

   bool closed = false;
};

}
