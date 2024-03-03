#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"
#include "world/utility/world_utilities.hpp"

namespace we::edits {

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

template<typename T1, typename T2>
struct set_multi_value2 final : edit<world::edit_context> {
   set_multi_value2(T1* value1_address, T1 new_value1, T2* value2_address, T2 new_value2)
      : value1_ptr{value1_address},
        value2_ptr{value2_address},
        value1{std::move(new_value1)},
        value2{std::move(new_value2)}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(value1_ptr));
      assert(context.is_memory_valid(value2_ptr));

      std::swap(*value1_ptr, value1);
      std::swap(*value2_ptr, value2);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(value1_ptr));
      assert(context.is_memory_valid(value2_ptr));

      std::swap(*value1_ptr, value1);
      std::swap(*value2_ptr, value2);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_multi_value2* other =
         dynamic_cast<const set_multi_value2*>(&other_unknown);

      if (not other) return false;

      return this->value1_ptr == other->value1_ptr and
             this->value2_ptr == other->value2_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_multi_value2& other = dynamic_cast<set_multi_value2&>(other_unknown);

      value1 = std::move(other.value1);
      value2 = std::move(other.value2);
   }

private:
   T1* value1_ptr = nullptr;
   T2* value2_ptr = nullptr;

   T1 value1;
   T2 value2;
};

template<typename T1, typename T2, typename T3>
struct set_multi_value3 final : edit<world::edit_context> {
   set_multi_value3(T1* value1_address, T1 new_value1, //
                    T2* value2_address, T2 new_value2, //
                    T3* value3_address, T3 new_value3)
      : value1_ptr{value1_address},
        value2_ptr{value2_address},
        value3_ptr{value3_address},
        value1{std::move(new_value1)},
        value2{std::move(new_value2)},
        value3{std::move(new_value3)}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(value1_ptr));
      assert(context.is_memory_valid(value2_ptr));
      assert(context.is_memory_valid(value3_ptr));

      std::swap(*value1_ptr, value1);
      std::swap(*value2_ptr, value2);
      std::swap(*value3_ptr, value3);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(value1_ptr));
      assert(context.is_memory_valid(value2_ptr));
      assert(context.is_memory_valid(value3_ptr));

      std::swap(*value1_ptr, value1);
      std::swap(*value2_ptr, value2);
      std::swap(*value3_ptr, value3);
   }

   bool is_coalescable(const edit& other_unknown) const noexcept override
   {
      const set_multi_value3* other =
         dynamic_cast<const set_multi_value3*>(&other_unknown);

      if (not other) return false;

      return this->value1_ptr == other->value1_ptr and
             this->value2_ptr == other->value2_ptr and
             this->value3_ptr == other->value3_ptr;
   }

   void coalesce(edit& other_unknown) noexcept override
   {
      set_multi_value3& other = dynamic_cast<set_multi_value3&>(other_unknown);

      value1 = std::move(other.value1);
      value2 = std::move(other.value2);
      value3 = std::move(other.value3);
   }

private:
   T1* value1_ptr = nullptr;
   T2* value2_ptr = nullptr;
   T3* value3_ptr = nullptr;

   T1 value1;
   T2 value2;
   T3 value3;
};

template<typename T1, typename T2>
inline auto make_set_multi_value(T1* value1_address, T1 new_value1,
                                 T2* value2_address, T2 new_value2)
   -> std::unique_ptr<set_multi_value2<T1, T2>>
{
   assert(value1_address and value2_address);

   return std::make_unique<set_multi_value2<T1, T2>>(value1_address,
                                                     std::move(new_value1),
                                                     value2_address,
                                                     std::move(new_value2));
}

template<typename T1, typename T2, typename T3>
inline auto make_set_multi_value(T1* value1_address, T1 new_value1, //
                                 T2* value2_address, T2 new_value2, //
                                 T3* value3_address, T3 new_value3)
   -> std::unique_ptr<set_multi_value3<T1, T2, T3>>
{
   assert(value1_address and value2_address and value3_address);

   return std::make_unique<set_multi_value3<T1, T2, T3>>(value1_address,
                                                         std::move(new_value1),
                                                         value2_address,
                                                         std::move(new_value2),
                                                         value3_address,
                                                         std::move(new_value3));
}

auto make_set_creation_path_node_location(quaternion new_rotation, float3 new_position,
                                          float3 new_euler_rotation)
   -> std::unique_ptr<edit<world::edit_context>>;

}
