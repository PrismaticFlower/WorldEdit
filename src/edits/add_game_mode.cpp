#include "add_game_mode.hpp"
#include "utility/string_icompare.hpp"

#include <fmt/core.h>

namespace we::edits {

namespace {

enum class req_edit_type { append, create };

struct req_edit_append {
   int list_index = 0;
};

void get_req_edit_type(const world::world& world, req_edit_type& type_out,
                       req_edit_append& append_out) noexcept
{
   type_out = req_edit_type::create;
   append_out = {.list_index = 0};

   for (int i = 0; i < world.requirements.size(); ++i) {
      const auto& list = world.requirements[i];

      if (list.platform != world::platform::all) continue;
      if (not string::iequals(list.file_type, "lvl")) continue;

      type_out = req_edit_type::append;
      append_out = {.list_index = i};

      break;
   }
}

struct add_game_mode final : edit<world::edit_context> {
   add_game_mode(std::string name, req_edit_type req_edit_type,
                 req_edit_append req_edit_append)
      : _name{std::move(name)}, _req_edit_type{req_edit_type}, _req_edit_append{req_edit_append}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      world::world& world = context.world;

      const std::string req_entry_name = fmt::format("{}_{}", world.name, _name);

      world.game_modes.push_back({.name = _name,
                                  .requirements = {{
                                     .file_type = "world",
                                     .entries = {req_entry_name},
                                  }}});

      if (_req_edit_type == req_edit_type::append) {
         world.requirements[_req_edit_append.list_index].entries.push_back(req_entry_name);
      }
      else if (_req_edit_type == req_edit_type::create) {
         world.requirements.push_back({.file_type = "lvl", .entries = {req_entry_name}});
      }
   }

   void revert(world::edit_context& context) const noexcept override
   {
      world::world& world = context.world;

      world.game_modes.pop_back();

      if (_req_edit_type == req_edit_type::append) {
         world.requirements[_req_edit_append.list_index].entries.pop_back();
      }
      else if (_req_edit_type == req_edit_type::create) {
         world.requirements.pop_back();
      }
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const std::string _name;
   const req_edit_type _req_edit_type;
   const req_edit_append _req_edit_append;
};

}

auto make_add_game_mode(std::string name, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   req_edit_type req_edit_type = req_edit_type::append;
   req_edit_append req_edit_append{};

   get_req_edit_type(world, req_edit_type, req_edit_append);

   return std::make_unique<add_game_mode>(std::move(name), req_edit_type,
                                          req_edit_append);
}

}
