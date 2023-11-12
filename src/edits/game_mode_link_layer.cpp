#pragma once

#include "game_mode_link_layer.hpp"
#include "utility/string_icompare.hpp"

#include <fmt/core.h>

namespace we::edits {

namespace {

enum class req_edit_type { none, append, create };

struct req_edit_append {
   int list_index = 0;
};

void get_req_edit_type(const std::span<const world::requirement_list>& requirements,
                       req_edit_type& type_out, req_edit_append& append_out) noexcept
{
   type_out = req_edit_type::create;
   append_out = {.list_index = 0};

   for (int i = 0; i < requirements.size(); ++i) {
      const auto& list = requirements[i];

      if (list.platform != world::platform::all) continue;
      if (not string::iequals(list.file_type, "world")) continue;

      type_out = req_edit_type::append;
      append_out = {.list_index = i};

      break;
   }
}

struct link_layer final : edit<world::edit_context> {
   link_layer(int game_mode_index, int layer_index, req_edit_type req_edit_type,
              req_edit_append req_edit_append)
      : _game_mode_index{game_mode_index},
        _layer_index{layer_index},
        _req_edit_type{req_edit_type},
        _req_edit_append{req_edit_append}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::world& world = context.world;
      world::game_mode_description& game_mode = world.game_modes[_game_mode_index];

      game_mode.layers.push_back(_layer_index);

      if (_req_edit_type == req_edit_type::none) return;

      const std::string req_entry_name =
         fmt::format("{}_{}", world.name, world.layer_descriptions[_layer_index].name);

      if (_req_edit_type == req_edit_type::append) {
         game_mode.requirements[_req_edit_append.list_index].entries.push_back(
            req_entry_name);
      }
      else if (_req_edit_type == req_edit_type::create) {
         game_mode.requirements.push_back(
            {.file_type = "world", .entries = {req_entry_name}});
      }
   }

   void revert(world::edit_context& context) noexcept override
   {
      world::game_mode_description& game_mode =
         context.world.game_modes[_game_mode_index];

      game_mode.layers.pop_back();

      if (_req_edit_type == req_edit_type::append) {
         game_mode.requirements[_req_edit_append.list_index].entries.pop_back();
      }
      else if (_req_edit_type == req_edit_type::create) {
         game_mode.requirements.pop_back();
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const int _game_mode_index;
   const int _layer_index;
   const req_edit_type _req_edit_type;
   const req_edit_append _req_edit_append;
};

}

auto make_game_mode_link_layer(int game_mode_index, int layer_index,
                               const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   req_edit_type req_edit_type = req_edit_type::none;
   req_edit_append req_edit_append{};

   if (game_mode_index != 0) {
      get_req_edit_type(world.game_modes[game_mode_index].requirements,
                        req_edit_type, req_edit_append);
   }

   return std::make_unique<link_layer>(game_mode_index, layer_index,
                                       req_edit_type, req_edit_append);
}

}
