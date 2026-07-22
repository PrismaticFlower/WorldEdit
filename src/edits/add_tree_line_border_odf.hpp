#pragma once

#include "edit.hpp"

#include "world/interaction_context.hpp"

#include <memory>

namespace we::world {

struct object_class_library;

}

namespace we::edits {

auto make_add_tree_line_border_odf(std::vector<world::tree_line_odf>* odfs,
                                   world::tree_line_odf new_odf,
                                   world::object_class_library& object_class_library)
   -> std::unique_ptr<edit<world::edit_context>>;

}
