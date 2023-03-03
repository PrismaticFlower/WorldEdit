#pragma once

#include "../object_class.hpp"
#include "../world.hpp"

#include <span>

#include <absl/container/flat_hash_map.h>

namespace we::world {

auto sector_fill(const sector& sector, const std::span<const object> world_objects,
                 const absl::flat_hash_map<lowercase_string, object_class>& object_classes)
   -> std::vector<std::string>;

}
