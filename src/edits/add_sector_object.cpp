#include "add_sector_object.hpp"
#include "world/utility/world_utilities.hpp"

namespace we::edits {

namespace {

struct add_sector_object final : edit<world::edit_context> {
   add_sector_object(world::sector_id sector_id, std::string object)
      : _id{sector_id}, _object{std::move(object)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      world::find_entity(context.world.sectors, _id)->objects.emplace_back(_object);
   }

   void revert(world::edit_context& context) const noexcept override
   {
      world::find_entity(context.world.sectors, _id)->objects.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::sector_id _id;
   const std::string _object;
};

}

auto make_add_sector_object(world::sector_id sector_id, std::string object_name)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_sector_object>(sector_id, std::move(object_name));
}

}