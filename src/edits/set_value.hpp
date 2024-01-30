#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"
#include "world/utility/world_utilities.hpp"

namespace we::edits {

template<typename Entity, typename T>
struct set_value final : edit<world::edit_context> {
   using entity_type = Entity;
   using value_type = T;

   set_value(world::id<Entity> id, value_type entity_type::*value_member_ptr,
             value_type new_value, value_type original_value)
      : id{id},
        value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      find_entity<entity_type>(context.world, id)->*value_member_ptr = new_value;
   }

   void revert(world::edit_context& context) noexcept override
   {
      find_entity<entity_type>(context.world, id)->*value_member_ptr = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_value* other = dynamic_cast<const set_value*>(&other_unknown);

      if (not other) return false;

      return this->id == other->id and this->value_member_ptr == other->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_value& other = dynamic_cast<set_value&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   world::id<Entity> id;
   value_type entity_type::*value_member_ptr;

   value_type new_value;
   value_type original_value;
};

template<typename Entity, typename T>
inline auto make_set_value(world::id<Entity> id, T Entity::*value_member_ptr,
                           T new_value, T original_value)
   -> std::unique_ptr<set_value<Entity, T>>
{
   return std::make_unique<set_value<Entity, T>>(id, value_member_ptr,
                                                 std::move(new_value),
                                                 std::move(original_value));
}

template<typename T>
struct set_path_node_value final : edit<world::edit_context> {
   using value_type = T;

   set_path_node_value(world::path_id id, std::size_t node,
                       value_type world::path::node::*value_member_ptr,
                       value_type new_value, value_type original_value)
      : id{id},
        node{node},
        value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      find_entity<world::path>(context.world, id)->nodes[node].*value_member_ptr =
         new_value;
   }

   void revert(world::edit_context& context) noexcept override
   {
      find_entity<world::path>(context.world, id)->nodes[node].*value_member_ptr =
         original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_path_node_value* other =
         dynamic_cast<const set_path_node_value*>(&other_unknown);

      if (not other) return false;

      return this->id == other->id and this->node == other->node and
             this->value_member_ptr == other->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_path_node_value& other = dynamic_cast<set_path_node_value&>(other_unknown);

      new_value = std::move(other.new_value);
   }

private:
   world::path_id id;
   std::size_t node;
   value_type world::path::node::*value_member_ptr;

   value_type new_value;
   value_type original_value;
};

template<typename T>
inline auto make_set_path_node_value(world::path_id id, std::size_t node,
                                     T world::path::node::*value_member_ptr,
                                     T new_value, T original_value)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<set_path_node_value<T>>(id, node, value_member_ptr,
                                                   std::move(new_value),
                                                   std::move(original_value));
}

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

   void apply(world::edit_context& context) noexcept override
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = new_value;
   }

   void revert(world::edit_context& context) noexcept override
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

template<typename Entity, typename T>
inline auto make_set_creation_value(T Entity::*value_member_ptr, T new_value,
                                    T original_value)
   -> std::unique_ptr<set_creation_value<Entity, T>>
{
   return std::make_unique<set_creation_value<Entity, T>>(value_member_ptr,
                                                          std::move(new_value),
                                                          std::move(original_value));
}

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

   void apply(world::edit_context& context) noexcept override
   {
      std::get<entity_type>(*context.creation_entity).*value_member_ptr = new_value;
      context.*meta_value_member_ptr = meta_new_value;
   }

   void revert(world::edit_context& context) noexcept override
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

template<typename Entity, typename T, typename U>
inline auto make_set_creation_value_with_meta(T Entity::*value_member_ptr,
                                              T new_value, T original_value,
                                              U world::edit_context::*meta_value_member_ptr,
                                              U meta_new_value, U meta_original_value)
   -> std::unique_ptr<set_creation_value_with_meta<Entity, T, U>>
{
   return std::make_unique<set_creation_value_with_meta<Entity, T, U>>(
      value_member_ptr, std::move(new_value), std::move(original_value),
      meta_value_member_ptr, std::move(meta_new_value),
      std::move(meta_original_value));
}

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

   void apply(world::edit_context& context) noexcept override
   {
      std::get<entity_type>(*context.creation_entity).rotation = new_rotation;
      std::get<entity_type>(*context.creation_entity).position = new_position;
      context.euler_rotation = new_euler_rotation;
   }

   void revert(world::edit_context& context) noexcept override
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

template<typename Entity>
inline auto make_set_creation_location(quaternion new_rotation,
                                       quaternion original_rotation,
                                       float3 new_position, float3 original_position,
                                       float3 new_euler_rotation,
                                       float3 original_euler_rotation)
   -> std::unique_ptr<set_creation_location<Entity>>
{
   return std::make_unique<set_creation_location<Entity>>(new_rotation, original_rotation,
                                                          new_position, original_position,
                                                          new_euler_rotation,
                                                          original_euler_rotation);
}

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

   void apply(world::edit_context& context) noexcept override
   {
      std::get<world::path>(*context.creation_entity).nodes[0].*value_member_ptr =
         new_value;
   }

   void revert(world::edit_context& context) noexcept override
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

template<typename T>
inline auto make_set_creation_path_node_value(T world::path::node::*value_member_ptr,
                                              T new_value, T original_value)
   -> std::unique_ptr<set_creation_path_node_value<T>>
{
   return std::make_unique<set_creation_path_node_value<T>>(value_member_ptr,
                                                            std::move(new_value),
                                                            std::move(original_value));
}

template<typename Struct, typename Value>
struct set_global_value final : edit<world::edit_context> {
   using value_owner = Struct;
   using value_type = Value;

   set_global_value(value_owner world::world::*value_owner_ptr,
                    value_type value_owner::*value_member_ptr,
                    value_type new_value, value_type original_value)
      : value_owner_ptr{value_owner_ptr},
        value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.world.*value_owner_ptr.*value_member_ptr = new_value;
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.*value_owner_ptr.*value_member_ptr = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_global_value* other =
         dynamic_cast<const set_global_value*>(&other_unknown);

      if (not other) return false;

      return this->value_member_ptr == other->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_global_value& other = dynamic_cast<set_global_value&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   value_owner world::world::*value_owner_ptr;
   value_type value_owner::*value_member_ptr;

   value_type new_value;
   value_type original_value;
};

template<typename Struct, typename Value>
inline auto make_set_global_value(Struct world::world::*value_owner_ptr,
                                  Value Struct::*value_member_ptr,
                                  Value new_value, Value original_value)
   -> std::unique_ptr<set_global_value<Struct, Value>>
{
   return std::make_unique<set_global_value<Struct, Value>>(value_owner_ptr,
                                                            value_member_ptr,
                                                            std::move(new_value),
                                                            std::move(original_value));
}

template<typename Struct, typename Container, typename Value>
struct set_global_value_indexed final : edit<world::edit_context> {
   using value_owner = Struct;
   using value_container = Container;
   using value_type = Value;

   set_global_value_indexed(value_owner world::world::*value_owner_ptr,
                            value_container value_owner::*value_member_ptr,
                            uint32 index, value_type new_value, value_type original_value)
      : index{index},
        value_owner_ptr{value_owner_ptr},
        value_member_ptr{value_member_ptr},
        new_value{std::move(new_value)},
        original_value{std::move(original_value)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      (context.world.*value_owner_ptr.*value_member_ptr)[index] = new_value;
   }

   void revert(world::edit_context& context) noexcept override
   {
      (context.world.*value_owner_ptr.*value_member_ptr)[index] = original_value;
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_global_value_indexed* other =
         dynamic_cast<const set_global_value_indexed*>(&other_unknown);

      if (not other) return false;

      return this->index == other->index and
             this->value_member_ptr == other->value_member_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_global_value_indexed& other =
         dynamic_cast<set_global_value_indexed&>(other_unknown);

      new_value = std::move(other.new_value);
   }

   uint32 index;

   value_owner world::world::*value_owner_ptr;
   value_container value_owner::*value_member_ptr;

   value_type new_value;
   value_type original_value;
};

template<typename Struct, typename Container, typename Value>
inline auto make_set_global_value_indexed(Struct world::world::*value_owner_ptr,
                                          Container Struct::*value_member_ptr,
                                          uint32 index, Value new_value,
                                          Value original_value)
   -> std::unique_ptr<set_global_value_indexed<Struct, Container, Value>>
{
   return std::make_unique<set_global_value_indexed<Struct, Container, Value>>(
      value_owner_ptr, value_member_ptr, index, std::move(new_value),
      std::move(original_value));
}

auto make_set_instance_property_value(world::object_id id, std::size_t property_index,
                                      std::string new_value, std::string original_value)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_sector_point(world::sector_id id, std::size_t point_index,
                           float2 new_position, float2 original_position)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_creation_path_node_location(quaternion new_rotation,
                                          quaternion original_rotation,
                                          float3 new_position, float3 original_position,
                                          float3 new_euler_rotation,
                                          float3 original_euler_rotation)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_creation_region_metrics(quaternion new_rotation,
                                      quaternion original_rotation,
                                      float3 new_position, float3 original_position,
                                      float3 new_size, float3 original_size)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_creation_sector_point(float2 new_position, float2 original_position)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_creation_portal_size(float new_width, float original_width,
                                   float new_height, float original_height)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_creation_barrier_metrics(float new_rotation, float original_rotation,
                                       float3 new_position, float3 original_position,
                                       float2 new_size, float2 original_size)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_set_creation_measurement_points(float3 new_start, float3 original_start,
                                          float3 new_end, float3 original_end)
   -> std::unique_ptr<edit<world::edit_context>>;

}
