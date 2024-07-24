#include "add_branch_weight.hpp"

namespace we::edits {

namespace {

struct add_branch_weight final : edit<world::edit_context> {
   add_branch_weight(std::vector<world::planning_branch_weights>* branch_weights,
                     world::planning_branch_weights weights)
      : branch_weights{branch_weights}, weights{weights}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(branch_weights));

      branch_weights->push_back(weights);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(branch_weights));

      branch_weights->pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<world::planning_branch_weights>* branch_weights;
   world::planning_branch_weights weights;
};

}

auto make_add_branch_weight(std::vector<world::planning_branch_weights>* branch_weights,
                            world::planning_branch_weights weights)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_branch_weight>(branch_weights, weights);
}

}
