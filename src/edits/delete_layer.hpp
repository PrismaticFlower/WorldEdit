#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::world {

struct object_class_library;

}

namespace we::edits {

auto make_delete_layer(int layer_index, const world::world& world,
                       world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>;

}
