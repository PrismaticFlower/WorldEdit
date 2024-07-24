#include "delete_branch_weight.hpp"

namespace we::edits {

namespace {

struct delete_branch_weight final : edit<world::edit_context> {
   delete_branch_weight(std::vector<world::planning_branch_weights>* weights, uint32 index)
      : weights{weights}, index{index}, weight{(*weights)[index]}
   {
      assert(index < keys->size());
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(keys));
      assert(index < weights->size());

      weights->erase(weights->begin() + index);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(weights));
      assert(index <= weights->size());

      weights->insert(weights->begin() + index, weight);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<world::planning_branch_weights>* weights;
   uint32 index;
   world::planning_branch_weights weight;
};

}

auto make_delete_branch_weight(std::vector<world::planning_branch_weights>* branch_weights,
                               uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_branch_weight>(branch_weights, index);
}

}