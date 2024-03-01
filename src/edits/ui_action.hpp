#pragma once

#include "edit.hpp"
#include "types.hpp"
#include "world/interaction_context.hpp"
#include "world/utility/world_utilities.hpp"
#include "world/world.hpp"

namespace we::edits {

// Classes for integrating ImGui controls with the Undo-Redo stack.

template<typename Entity, typename T>
struct ui_edit final : edit<world::edit_context> {
   using entity_type = Entity;
   using entity_id_type = world::id<entity_type>;
   using value_type = T;

   ui_edit(entity_id_type id, value_type entity_type::*value_member_ptr,
           value_type new_value, value_type original_value)
      : id{id},
        value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept
   {
      find_entity<entity_type>(context.world, id)->*value_member_ptr = new_value;
   }

   void revert(world::edit_context& context) noexcept
   {
      find_entity<entity_type>(context.world, id)->*value_member_ptr = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const ui_edit* other = dynamic_cast<const ui_edit*>(&other_unknown);

      if (not other) return false;

      return other->id == this->id and other->value_member_ptr == this->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      ui_edit& other = dynamic_cast<ui_edit&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   entity_id_type id;
   value_type entity_type::*value_member_ptr;

   value_type new_value;
   value_type original_value;
};

template<typename Entity, typename T>
struct ui_edit_indexed final : edit<world::edit_context> {
   using entity_type = Entity;
   using entity_id_type = world::id<entity_type>;
   using value_type = T;

   ui_edit_indexed(entity_id_type id,
                   std::vector<value_type> entity_type::*value_member_ptr,
                   std::size_t item_index, value_type new_value,
                   value_type original_value)
      : id{id},
        value_member_ptr{value_member_ptr},
        item_index{item_index},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept
   {
      std::vector<value_type>& vec =
         find_entity<entity_type>(context.world, id)->*value_member_ptr;

      if (item_index >= vec.size()) std::terminate();

      vec[item_index] = new_value;
   }

   void revert(world::edit_context& context) noexcept
   {
      std::vector<value_type>& vec =
         find_entity<entity_type>(context.world, id)->*value_member_ptr;

      if (item_index >= vec.size()) std::terminate();

      vec[item_index] = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const ui_edit_indexed* other =
         dynamic_cast<const ui_edit_indexed*>(&other_unknown);

      if (not other) return false;

      return other->id == this->id and                             //
             other->value_member_ptr == this->value_member_ptr and //
             other->item_index == this->item_index;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      ui_edit_indexed& other = dynamic_cast<ui_edit_indexed&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   entity_id_type id;
   std::vector<value_type> entity_type::*value_member_ptr;
   std::size_t item_index;

   value_type new_value;
   value_type original_value;
};

template<typename T>
struct ui_edit_path_node final : edit<world::edit_context> {
   using entity_type = world::path;
   using node_type = world::path::node;
   using entity_id_type = world::id<entity_type>;
   using value_type = T;

   ui_edit_path_node(entity_id_type id, std::size_t node_index,
                     value_type node_type::*value_member_ptr,
                     value_type new_value, value_type original_value)
      : id{id},
        node_index{node_index},
        value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept
   {
      world::path* path = find_entity<entity_type>(context.world, id);

      if (node_index >= path->nodes.size()) std::terminate();

      world::path::node& node = path->nodes[node_index];

      node.*value_member_ptr = new_value;
   }

   void revert(world::edit_context& context) noexcept
   {
      world::path* path = find_entity<entity_type>(context.world, id);

      if (node_index >= path->nodes.size()) std::terminate();

      world::path::node& node = path->nodes[node_index];

      node.*value_member_ptr = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const ui_edit_path_node* other =
         dynamic_cast<const ui_edit_path_node*>(&other_unknown);

      if (not other) return false;

      return other->id == this->id and                 //
             other->node_index == this->node_index and //
             other->value_member_ptr == this->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      ui_edit_path_node& other = dynamic_cast<ui_edit_path_node&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   entity_id_type id;
   std::size_t node_index;
   value_type node_type::*value_member_ptr;

   value_type new_value;
   value_type original_value;
};

template<typename T>
struct ui_edit_path_node_indexed final : edit<world::edit_context> {
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
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept
   {
      world::path* path = find_entity<entity_type>(context.world, id);

      if (node_index >= path->nodes.size()) std::terminate();

      world::path::node& node = path->nodes[node_index];

      std::vector<value_type>& vec = node.*value_member_ptr;

      if (item_index >= vec.size()) std::terminate();

      vec[item_index] = new_value;
   }

   void revert(world::edit_context& context) noexcept
   {
      world::path* path = find_entity<entity_type>(context.world, id);

      if (node_index >= path->nodes.size()) std::terminate();

      world::path::node& node = path->nodes[node_index];

      std::vector<value_type>& vec = node.*value_member_ptr;

      if (item_index >= vec.size()) std::terminate();

      vec[item_index] = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const ui_edit_path_node_indexed* other =
         dynamic_cast<const ui_edit_path_node_indexed*>(&other_unknown);

      if (not other) return false;

      return other->id == this->id and                             //
             other->node_index == this->node_index and             //
             other->value_member_ptr == this->value_member_ptr and //
             other->item_index == this->item_index;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      ui_edit_path_node_indexed& other =
         dynamic_cast<ui_edit_path_node_indexed&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   entity_id_type id;
   std::size_t node_index;
   std::vector<value_type> node_type::*value_member_ptr;
   std::size_t item_index;

   value_type new_value;
   value_type original_value;

   bool closed = false;
};

template<typename Entity, typename T>
struct ui_creation_edit final : edit<world::edit_context> {
   using entity_type = Entity;
   using value_type = T;

   ui_creation_edit(value_type entity_type::*value_member_ptr,
                    value_type new_value, value_type original_value)
      : value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept
   {
      context.creation_entity.get<entity_type>().*value_member_ptr = new_value;
   }

   void revert(world::edit_context& context) noexcept
   {
      context.creation_entity.get<entity_type>().*value_member_ptr = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const ui_creation_edit* other =
         dynamic_cast<const ui_creation_edit*>(&other_unknown);

      if (not other) return false;

      return other->value_member_ptr == this->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      ui_creation_edit& other = dynamic_cast<ui_creation_edit&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   value_type entity_type::*value_member_ptr;

   value_type new_value;
   value_type original_value;
};

template<typename Entity, typename T, typename U>
struct ui_creation_edit_with_meta final : edit<world::edit_context> {
   using entity_type = Entity;
   using value_type = T;
   using meta_value_type = U;

   ui_creation_edit_with_meta(value_type entity_type::*value_member_ptr,
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

   void apply(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<entity_type>().*value_member_ptr = new_value;
      context.*meta_value_member_ptr = meta_new_value;
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<entity_type>().*value_member_ptr = original_value;
      context.*meta_value_member_ptr = meta_original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const ui_creation_edit_with_meta* other =
         dynamic_cast<const ui_creation_edit_with_meta*>(&other_unknown);

      if (not other) return false;

      return other->value_member_ptr == this->value_member_ptr and
             other->meta_value_member_ptr == this->meta_value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      ui_creation_edit_with_meta& other =
         dynamic_cast<ui_creation_edit_with_meta&>(other_unknown);

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

template<typename T>
struct ui_creation_path_node_edit final : edit<world::edit_context> {
   using value_type = T;

   ui_creation_path_node_edit(value_type world::path::node::*value_member_ptr,
                              value_type new_value, value_type original_value)
      : value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::path>().nodes[0].*value_member_ptr = new_value;
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::path>().nodes[0].*value_member_ptr =
         original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const ui_creation_path_node_edit* other =
         dynamic_cast<const ui_creation_path_node_edit*>(&other_unknown);

      if (not other) return false;

      return other->value_member_ptr == this->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      ui_creation_path_node_edit& other =
         dynamic_cast<ui_creation_path_node_edit&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   value_type world::path::node::*value_member_ptr;
   value_type new_value;
   value_type original_value;
};

template<typename T, typename U>
struct ui_creation_path_node_edit_with_meta final : edit<world::edit_context> {
   using value_type = T;
   using meta_value_type = U;

   ui_creation_path_node_edit_with_meta(
      value_type world::path::node::*value_member_ptr, value_type new_value,
      value_type original_value,
      meta_value_type world::edit_context::*meta_value_member_ptr,
      meta_value_type meta_new_value, meta_value_type meta_original_value)
      : value_member_ptr{value_member_ptr},
        meta_value_member_ptr{meta_value_member_ptr},
        new_value{std::move(new_value)},
        meta_new_value{std::move(meta_new_value)},
        original_value{std::move(original_value)},
        meta_original_value{std::move(meta_original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::path>().nodes[0].*value_member_ptr = new_value;
      context.*meta_value_member_ptr = meta_new_value;
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::path>().nodes[0].*value_member_ptr =
         original_value;
      context.*meta_value_member_ptr = meta_original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const ui_creation_path_node_edit_with_meta* other =
         dynamic_cast<const ui_creation_path_node_edit_with_meta*>(&other_unknown);

      if (not other) return false;

      return other->value_member_ptr == this->value_member_ptr and
             other->meta_value_member_ptr == this->meta_value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      ui_creation_path_node_edit_with_meta& other =
         dynamic_cast<ui_creation_path_node_edit_with_meta&>(other_unknown);

      new_value = std::move(other.new_value);
      meta_new_value = std::move(other.meta_new_value);
   }

   value_type world::path::node::*value_member_ptr;
   meta_value_type world::edit_context::*meta_value_member_ptr;

   value_type new_value;
   meta_value_type meta_new_value;

   value_type original_value;
   meta_value_type meta_original_value;
};

struct ui_creation_sector_point_edit final : edit<world::edit_context> {
   using value_type = float2;

   ui_creation_sector_point_edit(value_type new_value, value_type original_value)
      : new_value{std::move(new_value)}, original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::sector>().points[0] = new_value;
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<world::sector>().points[0] = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const ui_creation_sector_point_edit* other =
         dynamic_cast<const ui_creation_sector_point_edit*>(&other_unknown);

      return other != nullptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      ui_creation_sector_point_edit& other =
         dynamic_cast<ui_creation_sector_point_edit&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   value_type new_value;
   value_type original_value;
};

}
