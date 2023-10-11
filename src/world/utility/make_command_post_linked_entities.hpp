#pragma once

#include "../object.hpp"
#include "../object_class_library.hpp"
#include "../path.hpp"
#include "../region.hpp"
#include "../terrain.hpp"

namespace we::world {

struct command_post_linked_entities {
   region capture_region;
   region control_region;
   path spawn_path;
};

struct make_command_post_linked_entities_inputs {
   std::string_view name;
   int16 layer = 0;
   float3 position;
   float capture_radius = 0.0f;
   float control_radius = 0.0f;
   float control_height = 0.0f;
   float spawn_radius = 0.0f;
};

auto make_command_post_linked_entities(
   const make_command_post_linked_entities_inputs inputs,
   const std::span<const object> objects, const std::span<const path> paths,
   const std::span<const region> regions, const object_class_library& object_classes,
   const terrain& terrain) noexcept -> command_post_linked_entities;

}
