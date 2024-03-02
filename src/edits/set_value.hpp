#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"
#include "world/utility/world_utilities.hpp"

namespace we::edits {

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
      context.creation_entity.get<entity_type>().rotation = new_rotation;
      context.creation_entity.get<entity_type>().position = new_position;
      context.euler_rotation = new_euler_rotation;
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.creation_entity.get<entity_type>().rotation = original_rotation;
      context.creation_entity.get<entity_type>().position = original_position;
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
struct set_memory_value final : edit<world::edit_context> {
   set_memory_value(T* value_address, T new_value)
      : value_ptr{value_address}, value{std::move(new_value)}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(value_ptr));

      std::swap(*value_ptr, value);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(value_ptr));

      std::swap(*value_ptr, value);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_memory_value* other =
         dynamic_cast<const set_memory_value*>(&other_unknown);

      if (not other) return false;

      return this->value_ptr == other->value_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_memory_value& other = dynamic_cast<set_memory_value&>(other_unknown);

      value = std::move(other.value);
   }

private:
   T* value_ptr = nullptr;
   T value;
};

template<typename T>
inline auto make_set_memory_value(T* value_address, T new_value)
   -> std::unique_ptr<set_memory_value<T>>
{
   assert(value_address);

   return std::make_unique<set_memory_value<T>>(value_address, std::move(new_value));
}

template<typename T>
inline auto make_set_value(T* value_address, T new_value)
   -> std::unique_ptr<set_memory_value<T>>
{
   assert(value_address);

   return std::make_unique<set_memory_value<T>>(value_address, std::move(new_value));
}

template<typename Vector_value, typename Value>
struct set_vector_value final : edit<world::edit_context> {
   set_vector_value(std::vector<Vector_value>* vector_address, const uint32 index,
                    const uint32 value_offset, Value new_value)
      : vector_ptr{vector_address},
        index{index},
        value_offset{value_offset},
        value{std::move(new_value)}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(vector_ptr));

      std::swap(*value_ptr(), value);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(vector_ptr));

      std::swap(*value_ptr(), value);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_vector_value* other =
         dynamic_cast<const set_vector_value*>(&other_unknown);

      if (not other) return false;

      return this->vector_ptr == other->vector_ptr and this->index == other->index and
             this->value_offset == other->value_offset;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_vector_value& other = dynamic_cast<set_vector_value&>(other_unknown);

      value = std::move(other.value);
   }

private:
   auto value_ptr() const noexcept -> Value*
   {
      return reinterpret_cast<Value*>(
         (reinterpret_cast<char*>((&(*vector_ptr)[index])) + value_offset));
   }

   std::vector<Vector_value>* vector_ptr = nullptr;
   uint32 index = 0;
   uint32 value_offset = 0;
   Value value;
};

template<typename Vector_value, typename Value>
inline auto make_set_vector_value(std::vector<Vector_value>* vector_address,
                                  const uint32 index,
                                  Value Vector_value::*value_member_ptr, Value new_value)
   -> std::unique_ptr<set_vector_value<Vector_value, Value>>
{
   assert(vector_address);
   assert(index < vector_address->size());
   assert(value_member_ptr);

   const uint32 value_offset = static_cast<uint32>(
      reinterpret_cast<const char*>(&((*vector_address)[index].*value_member_ptr)) -
      reinterpret_cast<const char*>(&(*vector_address)[index]));

   return std::make_unique<set_vector_value<Vector_value, Value>>(vector_address,
                                                                  index, value_offset,
                                                                  std::move(new_value));
}

template<typename Value>
inline auto make_set_vector_value(std::vector<Value>* vector_address,
                                  const uint32 index, Value new_value)
   -> std::unique_ptr<set_vector_value<Value, Value>>
{
   assert(vector_address);
   assert(index < vector_address->size());

   return std::make_unique<set_vector_value<Value, Value>>(vector_address, index, 0,
                                                           std::move(new_value));
}

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
