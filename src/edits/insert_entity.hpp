#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::world {

struct object_class_library;

}

namespace we::edits {

auto make_insert_entity(world::object object,
                        world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::light light)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::path path)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::region region)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::sector sector)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::portal portal)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::hintnode hintnode)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::barrier barrier)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::planning_hub planning_hub)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::planning_connection planning_connection)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::boundary boundary)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_insert_entity(world::measurement measurement)
   -> std::unique_ptr<edit<world::edit_context>>;

}
