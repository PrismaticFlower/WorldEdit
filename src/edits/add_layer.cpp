#include "add_layer.hpp"
#include "utility/string_icompare.hpp"

#include <fmt/core.h>

namespace we::edits {

namespace {

struct add_layer final : edit<world::edit_context> {
   add_layer(std::string name) : _name{std::move(name)} {}

   void apply(world::edit_context& context) const noexcept override
   {
      const int index = static_cast<int>(context.world.layer_descriptions.size());

      context.world.layer_descriptions.push_back({.name = _name});
      context.world.game_modes.at(0).layers.push_back(index);

      for (auto& requirements : context.world.requirements) {
         if (not string::iequals(requirements.file_type, "world")) continue;
         if (requirements.platform != world::platform::all) continue;

         requirements.entries.emplace_back(
            fmt::format("{}_{}", context.world.name, _name));

         break;
      }
   }

   void revert(world::edit_context& context) const noexcept override
   {
      context.world.layer_descriptions.pop_back();
      context.world.game_modes.at(0).layers.pop_back();

      for (auto& requirements : context.world.requirements) {
         if (not string::iequals(requirements.file_type, "world")) continue;
         if (requirements.platform != world::platform::all) continue;

         requirements.entries.pop_back();

         break;
      }
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
