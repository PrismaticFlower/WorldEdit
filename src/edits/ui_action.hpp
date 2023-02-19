#pragma once

#include "edit.hpp"
#include "types.hpp"
#include "world/utility/world_utilities.hpp"
#include "world/world.hpp"

namespace we::edits {

// Classes for integrating ImGui controls with the Undo-Redo stack.

template<typename Entity, typename T>
struct ui_edit final : edit<world::world> {
   using entity_type = Entity;
   using entity_id_type = world::id<entity_type>;
   using value_type = T;

   ui_edit(entity_id_type id, value_type entity_type::*value_member_ptr,
           value_type new_value, value_type original_value)
      : id{id}, value_member_ptr{value_member_ptr}, new_value{new_value}, original_value{original_value}
   {
   }

   void apply(world::world& world) const noexcept
   {
      find_entity<entity_type>(world, id)->*value_member_ptr = new_value;
   }

   void revert(world::world& world) const noexcept
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

template<typename Entity, typename T>
struct ui_edit_indexed final : edit<world::world> {
   using entity_type = Entity;
   using entity_id_type = world::id<entity_type>;
   using value_type = T;

   ui_edit_indexed(entity_id_type id,
                   std::vector<value_type> entity_type::*value_member_ptr,
                   std::size_t item_index, value_type new_value,
                   value_type original_value)
      : id{id}, value_member_ptr{value_member_ptr}, item_index{item_index}, new_value{new_value}, original_value{original_value}
   {
   }

   void apply(world::world& world) const noexcept
   {
      std::vector<value_type>& vec =
         find_entity<entity_type>(world, id)->*value_member_ptr;

      if (item_index >= vec.size()) std::terminate();

      vec[item_index] = new_value;
   }

   void revert(world::world& world) const noexcept
   {
      std::vector<value_type>& vec =
         find_entity<entity_type>(world, id)->*value_member_ptr;

      if (item_index >= vec.size()) std::terminate();

      vec[item_index] = original_value;
   }

   bool matching(entity_type& object,
                 std::vector<value_type> entity_type::*other_value_member_ptr,
                 std::size_t other_item_index) const noexcept
   {
      return id == object.id and other_value_member_ptr == this->value_member_ptr and
             other_item_index == this->item_index;
   }

   entity_id_type id;
   std::vector<value_type> entity_type::*value_member_ptr;
   std::size_t item_index;

   value_type new_value;
   value_type original_value;

   bool closed = false;
};

template<typename T>
struct ui_edit_path_node final : edit<world::world> {
   using entity_type = world::path;
   using node_type = world::path::node;
   using entity_id_type = world::id<entity_type>;
   using value_type = T;

   ui_edit_path_node(entity_id_type id, std::size_t node_index,
                     value_type node_type::*value_member_ptr,
                     value_type new_value, value_type original_value)
      : id{id}, node_index{node_index}, value_member_ptr{value_member_ptr}, new_value{new_value}, original_value{original_value}
   {
   }

   void apply(world::world& world) const noexcept
   {
      world::path* path = find_entity<entity_type>(world, id);

      if (node_index >= path->nodes.size()) std::terminate();

      world::path::node& node = path->nodes[node_index];

      node.*value_member_ptr = new_value;
   }

   void revert(world::world& world) const noexcept
   {
      world::path* path = find_entity<entity_type>(world, id);

      if (node_index >= path->nodes.size()) std::terminate();

      world::path::node& node = path->nodes[node_index];

      node.*value_member_ptr = original_value;
   }

   bool matching(entity_type& path, std::size_t other_node_index,
                 value_type node_type::*other_value_member_ptr) const noexcept
   {
      return id == path.id and other_node_index == this->node_index and
             other_value_member_ptr == this->value_member_ptr;
   }

   entity_id_type id;
   std::size_t node_index;
   value_type node_type::*value_member_ptr;

   value_type new_value;
   value_type original_value;

   bool closed = false;
};

template<typename T>
struct ui_edit_path_node_indexed final : edit<world::world> {
   using entity_type = world::path;
   using node_type = world::path::node;
   using entity_id_type = world::id<entity_type>;
   using value_type = T;

   ui_edit_path_node_indexed(entity_id_type id, std::size_t node_index,
                             std::vector<value_type> node_type::*value_member_ptr,
                             std::size_t item_index, value_type new_value,
                             value_type original_value)
      : id{id},
        node_index{node_index},
        value_member_ptr{value_member_ptr},
        item_index{item_index},
        new_value{new_value},
        original_value{original_value}
   {
   }

   void apply(world::world& world) const noexcept
   {
      world::path* path = find_entity<entity_type>(world, id);

      if (node_index >= path->nodes.size()) std::terminate();

      world::path::node& node = path->nodes[node_index];

      std::vector<value_type>& vec = node.*value_member_ptr;

      if (item_index >= vec.size()) std::terminate();

      vec[item_index] = new_value;
   }

   void revert(world::world& world) const noexcept
   {
      world::path* path = find_entity<entity_type>(world, id);

      if (node_index >= path->nodes.size()) std::terminate();

      world::path::node& node = path->nodes[node_index];

      std::vector<value_type>& vec = node.*value_member_ptr;

      if (item_index >= vec.size()) std::terminate();

      vec[item_index] = original_value;
   }

   bool matching(entity_type& path, std::size_t other_node_index,
                 std::vector<value_type> node_type::*other_value_member_ptr,
                 std::size_t other_item_index) const noexcept
   {
      return id == path.id and other_node_index == this->node_index and
             other_value_member_ptr == this->value_member_ptr and
             other_item_index == this->item_index;
   }

   entity_id_type id;
   std::size_t node_index;
   std::vector<value_type> node_type::*value_member_ptr;
   std::size_t item_index;

   value_type new_value;
   value_type original_value;

   bool closed = false;
};

}
