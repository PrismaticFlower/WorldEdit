#include "insert_entity.hpp"
#include "world/utility/world_utilities.hpp"

#include <algorithm>

namespace we::edits {

namespace {

template<typename T>
struct insert_entity final : edit<world::edit_context> {
   explicit insert_entity(T entity) : _id{entity.id}, _entity{std::move(entity)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      auto& entities = world::select_entities<T>(context.world);

      entities.push_back(_entity);
   }

   void revert(world::edit_context& context) noexcept override
   {
      auto& entities = world::select_entities<T>(context.world);

      entities.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::id<T> _id;
   const T _entity;
};

}

auto make_insert_entity(world::object object)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::object>>(std::move(object));
}

auto make_insert_entity(world::light light)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::light>>(std::move(light));
}

auto make_insert_entity(world::path path) -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::path>>(std::move(path));
}

auto make_insert_entity(world::region region)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::region>>(std::move(region));
}

auto make_insert_entity(world::sector sector)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::sector>>(std::move(sector));
}

auto make_insert_entity(world::portal portal)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::portal>>(std::move(portal));
}

auto make_insert_entity(world::hintnode hintnode)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::hintnode>>(std::move(hintnode));
}

auto make_insert_entity(world::barrier barrier)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::barrier>>(std::move(barrier));
}

auto make_insert_entity(world::planning_hub planning_hub)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::planning_hub>>(std::move(planning_hub));
}

auto make_insert_entity(world::planning_connection planning_connection)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::planning_connection>>(
      std::move(planning_connection));
}

auto make_insert_entity(world::boundary boundary)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::boundary>>(std::move(boundary));
}

}