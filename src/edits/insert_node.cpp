#include "insert_node.hpp"
#include "world/utility/world_utilities.hpp"

namespace we::edits {

namespace {

struct insert_path_node final : edit<world::edit_context> {
   insert_path_node(world::path_id path_id, std::size_t insert_before_index,
                    world::path::node node)
      : _id{path_id}, _insert_before_index{insert_before_index}, _node{std::move(node)}
   {
   }

   void apply(world::edit_context& context) const noexcept override
   {
      auto& nodes = world::find_entity(context.world.paths, _id)->nodes;

      nodes.insert(nodes.begin() + _insert_before_index, _node);
   }

   void revert(world::edit_context& context) const noexcept override
   {
      auto& nodes = world::find_entity(context.world.paths, _id)->nodes;

      nodes.erase(nodes.begin() + _insert_before_index);
   }

private:
   const world::path_id _id;
   const std::size_t _insert_before_index;
   const world::path::node _node;
};

}

auto make_insert_node(world::path_id path_id, std::size_t insert_before_index,
                      world::path::node node)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return std::make_unique<insert_path_node>(path_id, insert_before_index,
                                             std::move(node));
}

}