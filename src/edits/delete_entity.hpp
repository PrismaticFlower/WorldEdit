#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_entity(world::object_id object_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::light_id light_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::path_id_node_pair path_id_node, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::region_id region_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::sector_id sector_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::portal_id portal_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::hintnode_id hintnode_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::barrier_id barrier_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::planning_hub_id planning_hub_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::planning_connection_id planning_connection,
                        const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_entity(world::boundary_id boundary_id, const world::world& world)
   -> std::unique_ptr<edit<world::edit_context>>;

}
