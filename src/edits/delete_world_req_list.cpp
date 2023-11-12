#include "delete_world_req_entry.hpp"

namespace we::edits {

namespace {

struct delete_world_req_list final : edit<world::edit_context> {
   delete_world_req_list(int list_index, world::requirement_list list)
      : _list_index{list_index}, _list{std::move(list)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      auto& requirements = context.world.requirements;

      requirements.erase(requirements.begin() + _list_index);
   }

   void revert(world::edit_context& context) noexcept override
   {
      auto& requirements = context.world.requirements;

      requirements.insert(requirements.begin() + _list_index, _list);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const int _list_index;
   const world::requirement_list _list;
};

}

auto make_delete_world_req_list(int list_index, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_world_req_list>(list_index,
                                                  world.requirements[list_index]);
}

}