#include "add_animation_hierarchy_child.hpp"

namespace we::edits {

namespace {

struct add_animation_hierarchy_child final : edit<world::edit_context> {
   add_animation_hierarchy_child(std::vector<std::string>* children, std::string new_child)
      : children{children}, new_child{std::move(new_child)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(children));

      children->push_back(std::move(new_child));
   }

   void revert(world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(children));

      std::swap(new_child, children->back());

      children->pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<std::string>* children;
   std::string new_child;
};

}

auto make_add_animation_hierarchy_child(std::vector<std::string>* children,
                                        std::string new_child)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_animation_hierarchy_child>(children,
                                                          std::move(new_child));
}

}
