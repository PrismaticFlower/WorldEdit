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

   void apply(world::edit_context& context) const noexcept override
   {
      auto& entities = world::select_entities<T>(context.world);

      entities.push_back(_entity);
   }

   void revert(world::edit_context& context) const noexcept override
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

template<>
struct insert_entity<world::planning_hub> final : edit<world::edit_context> {
   insert_entity(world::planning_hub hub, const std::size_t hub_index)
      : _id{hub.id}, _hub{std::move(hub)}, _index{hub_index}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      context.world.planning_hubs.push_back(_hub);
      context.world.planning_hub_index.emplace(_hub.id, _index);
   }

   void revert(world::edit_context& context) const noexcept override
   {
      context.world.planning_hubs.pop_back();
      context.world.planning_hub_index.erase(_hub.id);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::planning_hub_id _id;
   const world::planning_hub _hub;
   const std::size_t _index;
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

auto make_insert_entity(world::planning_hub planning_hub, std::size_t hub_index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_entity<world::planning_hub>>(std::move(planning_hub),
                                                               hub_index);
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