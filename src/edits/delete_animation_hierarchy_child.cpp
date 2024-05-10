#include "delete_animation_hierarchy_child.hpp"

namespace we::edits {

namespace {

struct delete_animation_hierarchy_child final : edit<world::edit_context> {
   delete_animation_hierarchy_child(std::vector<std::string>* children, uint32 index)
      : children{children}, index{index}
   {
   }

   void apply([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(children));
      assert(index < children->size());

      std::swap((*children)[index], child);

      children->erase(children->begin() + index);
   }

   void revert([[maybe_unused]] world::edit_context& context) noexcept override
   {
      assert(context.is_memory_valid(children));
      assert(index <= children->size());

      children->insert(children->begin() + index, std::move(child));
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   std::vector<std::string>* children;
   uint32 index;
   std::string child;
};

}

auto make_delete_animation_hierarchy_child(std::vector<std::string>* children, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_animation_hierarchy_child>(children, index);
}

}