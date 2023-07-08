#include "delete_path_property.hpp"
#include "world/utility/world_utilities.hpp"

namespace we::edits {

namespace {

struct delete_path_property final : edit<world::edit_context> {
   delete_path_property(world::path_id id, std::size_t property_index,
                        world::path::property property)
      : _id{id}, _property_index{property_index}, _property{std::move(property)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      auto& properties = world::find_entity(context.world.paths, _id)->properties;

      properties.erase(properties.begin() + _property_index);
   }

   void revert(world::edit_context& context) const noexcept override
   {
      auto& properties = world::find_entity(context.world.paths, _id)->properties;

      properties.insert(properties.begin() + _property_index, _property);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::path_id _id;
   const std::size_t _property_index;
   const world::path::property _property;
};

struct delete_path_node_property final : edit<world::edit_context> {
   delete_path_node_property(world::path_id id, std::size_t node_index,
                             std::size_t property_index, world::path::property property)
      : _id{id}, _node_index{node_index}, _property_index{property_index}, _property{std::move(property)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      auto& properties =
         world::find_entity(context.world.paths, _id)->nodes[_node_index].properties;

      properties.erase(properties.begin() + _property_index);
   }

   void revert(world::edit_context& context) const noexcept override
   {
      auto& properties =
         world::find_entity(context.world.paths, _id)->nodes[_node_index].properties;

      properties.insert(properties.begin() + _property_index, _property);
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::path_id _id;
   const std::size_t _node_index;
   const std::size_t _property_index;
   const world::path::property _property;
};
}

auto make_delete_path_property(world::path_id path_id, const std::size_t property_index,
                               const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_path_property>(path_id, property_index,
                                                 world::find_entity(world.paths, path_id)
                                                    ->properties[property_index]);
}

auto make_delete_path_node_property(world::path_id path_id, const std::size_t node_index,
                                    const std::size_t property_index,
                                    const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<delete_path_node_property>(
      path_id, node_index, property_index,
      world::find_entity(world.paths, path_id)->nodes[node_index].properties[property_index]);
}

}