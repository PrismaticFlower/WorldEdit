#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_sector_object(std::vector<uint32>* objects, const uint32 object_index)
   -> std::unique_ptr<edit<world::edit_context>>;

auto make_delete_sector_object(std::vector<std::string>* objects, const uint32 object_index)
   -> std::unique_ptr<edit<world::edit_context>>;

}