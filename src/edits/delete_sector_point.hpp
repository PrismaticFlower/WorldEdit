#pragma once

#include "edit.hpp"
#include "world/interaction_context.hpp"

#include <memory>

namespace we::edits {

auto make_delete_sector_point(std::vector<float2>* points, const uint32 point_index)
   -> std::unique_ptr<edit<world::edit_context>>;

}