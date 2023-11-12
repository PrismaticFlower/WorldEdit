#include "add_world_req_entry.hpp"

namespace we::edits {

namespace {

struct add_world_req_entry final : edit<world::edit_context> {
   add_world_req_entry(int list_index, std::string name)
      : _list_index{list_index}, _name{std::move(name)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      context.world.requirements[_list_index].entries.push_back(_name);
   }

   void revert(world::edit_context& context) noexcept override
   {
      context.world.requirements[_list_index].entries.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const int _list_index;
   const std::string _name;
};

}

auto make_add_world_req_entry(int list_index, std::string name)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_world_req_entry>(list_index, std::move(name));
}

}