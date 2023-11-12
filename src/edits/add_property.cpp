#include "add_property.hpp"
#include "world/utility/world_utilities.hpp"

namespace we::edits {

namespace {

struct add_property_path final : edit<world::edit_context> {
   add_property_path(world::path_id path_id, std::string property)
      : _id{path_id}, _property{std::move(property)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::find_entity(context.world.paths, _id)->properties.emplace_back(_property, "");
   }

   void revert(world::edit_context& context) noexcept override
   {
      world::find_entity(context.world.paths, _id)->properties.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::path_id _id;
   const std::string _property;
};

struct add_property_path_node final : edit<world::edit_context> {
   add_property_path_node(world::path_id path_id, std::size_t node, std::string property)
      : _id{path_id}, _node{node}, _property{std::move(property)}
   {
   }

   void apply(world::edit_context& context) noexcept override
   {
      world::path::node& node =
         world::find_entity(context.world.paths, _id)->nodes[_node];

      node.properties.emplace_back(_property, "");
   }

   void revert(world::edit_context& context) noexcept override
   {
      world::path::node& node =
         world::find_entity(context.world.paths, _id)->nodes[_node];

      node.properties.pop_back();
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

private:
   const world::path_id _id;
   const std::size_t _node;
   const std::string _property;
};

}

auto make_add_property(world::path_id path_id, std::string property)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_property_path>(path_id, std::move(property));
}

auto make_add_property(world::path_id path_id, std::size_t node, std::string property)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<add_property_path_node>(path_id, node, std::move(property));
}

}