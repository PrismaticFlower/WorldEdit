#include "add_layer.hpp"

namespace we::edits {

namespace {

struct add_layer final : edit<world::edit_context> {
   add_layer(std::string name) : _name{std::move(name)} {}

   void apply(world::edit_context& context) const noexcept override
   {
      context.world.layer_descriptions.push_back({.name = _name});
   }

   void revert(world::edit_context& context) const noexcept override
   {
      context.world.layer_descriptions.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const std::string _name;
};

}

auto make_add_layer(std::string name) -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_layer>(std::move(name));
}

}
