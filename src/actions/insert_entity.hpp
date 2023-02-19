#pragma once

#include "edit.hpp"
#include "world/world.hpp"

#include <memory>

namespace we::actions {

auto make_insert_entity(world::object object) -> std::unique_ptr<edit<world::world>>;

auto make_insert_entity(world::light light) -> std::unique_ptr<edit<world::world>>;

auto make_insert_entity(world::path path) -> std::unique_ptr<edit<world::world>>;

auto make_insert_entity(world::region region) -> std::unique_ptr<edit<world::world>>;

auto make_insert_entity(world::sector sector) -> std::unique_ptr<edit<world::world>>;

auto make_insert_entity(world::portal portal) -> std::unique_ptr<edit<world::world>>;

auto make_insert_entity(world::barrier barrier)
   -> std::unique_ptr<edit<world::world>>;

auto make_insert_entity(world::planning_hub planning_hub)
   -> std::unique_ptr<edit<world::world>>;

auto make_insert_entity(world::planning_connection planning_connection)
   -> std::unique_ptr<edit<world::world>>;

auto make_insert_entity(world::boundary boundary)
   -> std::unique_ptr<edit<world::world>>;

}
