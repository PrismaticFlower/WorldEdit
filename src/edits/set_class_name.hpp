#pragma once

#include "edit.hpp"
#include "lowercase_string.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::world {

struct object_class_library;

}

namespace we::edits {

auto make_set_class_name(world::object* object, lowercase_string new_class_name,
                         world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>;

}