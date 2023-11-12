#include "add_layer.hpp"
#include "utility/string_icompare.hpp"

#include <fmt/core.h>

namespace we::edits {

namespace {

struct previously_deleted_entry {
   int index = 0;
   std::string name;
};

auto make_previously_deleted(std::string_view new_name,
                             const std::span<const std::string> deleted_layers)
   -> std::optional<previously_deleted_entry>
{
   for (int i = 0; i < deleted_layers.size(); ++i) {
      if (string::iequals(new_name, deleted_layers[i])) {
         return previously_deleted_entry{.index = i, .name = deleted_layers[i]};
      }
   }

   return std::nullopt;
}

struct add_layer final : edit<world::edit_context> {
   add_layer(std::string name, std::optional<previously_deleted_entry> previously_deleted)
      : _name{std::move(name)}, _previously_deleted{std::move(previously_deleted)}
   {
   }

   void apply(world::edit_context& context) noexcept override
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

      if (_previously_deleted) {
         context.world.deleted_layers.erase(context.world.deleted_layers.begin() +
                                            _previously_deleted->index);
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.layer_descriptions.pop_back();
      context.world.game_modes.at(0).layers.pop_back();

      for (auto& requirements : context.world.requirements) {
         if (not string::iequals(requirements.file_type, "world")) continue;
         if (requirements.platform != world::platform::all) continue;

         requirements.entries.pop_back();

         break;
      }

      if (_previously_deleted) {
         context.world.deleted_layers.insert(context.world.deleted_layers.begin() +
                                                _previously_deleted->index,
                                             _previously_deleted->name);
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const std::string _name;
   const std::optional<previously_deleted_entry> _previously_deleted;
};

}

auto make_add_layer(std::string name, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   std::optional<previously_deleted_entry> previously_deleted =
      make_previously_deleted(name, world.deleted_layers);

   return std::make_unique<add_layer>(std::move(name), std::move(previously_deleted));
}

}
