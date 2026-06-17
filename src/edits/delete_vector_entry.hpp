#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

using T = std::string;

template<typename T>
struct delete_vector_entry final : edit<world::edit_context> {
   delete_vector_entry(std::vector<T>* vector, uint32 index)
      : vector{vector}, index{index}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(vector));
      assert(index < vector->size());

      std::swap((*vector)[index], value);

      vector->erase(vector->begin() + index);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(vector));
      assert(index <= vector->size());

      vector->insert(vector->begin() + index, std::move(value));
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<T>* vector;
   uint32 index;
   T value;
};

template<typename T>
inline auto make_delete_vector_entry(std::vector<T>* vector_address, uint32 index)
   -> std::unique_ptr<delete_vector_entry<T>>
{
   assert(vector_address);

   return std::make_unique<delete_vector_entry<T>>(vector_address, index);
}

}
