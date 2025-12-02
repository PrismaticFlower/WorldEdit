#pragma once

#include "animation.hpp"
#include "barrier.hpp"
#include "blocks.hpp"
#include "boundary.hpp"
#include "configuration.hpp"
#include "effects.hpp"
#include "game_mode_description.hpp"
#include "global_lights.hpp"
#include "hintnode.hpp"
#include "id.hpp"
#include "layer_description.hpp"
#include "light.hpp"
#include "measurement.hpp"
#include "object.hpp"
#include "path.hpp"
#include "planning.hpp"
#include "portal.hpp"
#include "region.hpp"
#include "requirement_list.hpp"
#include "sector.hpp"
#include "terrain.hpp"

#include "container/pinned_vector.hpp"

#include <vector>

namespace we::world {

constexpr std::size_t max_entities = 1'048'576;
constexpr std::size_t reserved_entities = 2048;
constexpr pinned_vector_init entities_init{.max_size = max_entities,
                                           .initial_capacity = reserved_entities};

constexpr std::size_t max_animations = 16'384;
constexpr std::size_t max_animation_groups = 16'384;
constexpr std::size_t max_animation_hierarchies = 16'384;

struct world {
   std::string name;

   configuration configuration;

   std::vector<requirement_list> requirements;

   std::vector<layer_description> layer_descriptions;
   std::vector<game_mode_description> game_modes;
   std::vector<int> common_layers;

   terrain terrain;
   global_lights global_lights;

   pinned_vector<object> objects = entities_init;
   pinned_vector<light> lights = entities_init;
   pinned_vector<path> paths = entities_init;
   pinned_vector<region> regions = entities_init;
   pinned_vector<sector> sectors = entities_init;
   pinned_vector<portal> portals = entities_init;
   pinned_vector<hintnode> hintnodes = entities_init;
   pinned_vector<barrier> barriers = entities_init;
   pinned_vector<planning_hub> planning_hubs = entities_init;
   pinned_vector<planning_connection> planning_connections = entities_init;
   pinned_vector<boundary> boundaries = entities_init;
   pinned_vector<measurement> measurements = entities_init;

   pinned_vector<animation> animations = pinned_vector_init{max_animations, 256};
   pinned_vector<animation_group> animation_groups =
      pinned_vector_init{max_animation_groups, 256};
   pinned_vector<animation_hierarchy> animation_hierarchies =
      pinned_vector_init{max_animation_hierarchies, 256};

   blocks blocks;

   effects effects;

   /// @brief Vector of layers to garbage collect the files of at save time.
   std::vector<std::string> deleted_layers;

   /// @brief Vector of game modes to garbage collect the files of at save time.
   std::vector<std::string> deleted_game_modes;

   struct next_ids {
      id_generator<object> objects;
      id_generator<light> lights;
      id_generator<path> paths;
      id_generator<region> regions;
      id_generator<sector> sectors;
      id_generator<portal> portals;
      id_generator<hintnode> hintnodes;
      id_generator<barrier> barriers;
      id_generator<planning_hub> planning_hubs;
      id_generator<planning_connection> planning_connections;
      id_generator<boundary> boundaries;
      id_generator<measurement> measurements;
      id_generator<animation> animations;
      id_generator<animation_group> animation_groups;
      id_generator<animation_hierarchy> animation_hierarchies;
   } next_id;
};

}
