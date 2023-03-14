#pragma once

#include "../object_class_library.hpp"
#include "../world.hpp"

#include <span>

namespace we::world {

auto sector_fill(const sector& sector, const std::span<const object> world_objects,
                 const object_class_library& object_classes)
   -> std::vector<std::string>;

}
