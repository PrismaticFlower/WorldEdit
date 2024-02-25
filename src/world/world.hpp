#pragma once

#include "barrier.hpp"
#include "boundary.hpp"
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

#include <vector>

namespace we::world {

struct world {
   std::string name;

   std::vector<requirement_list> requirements;

   std::vector<layer_description> layer_descriptions;
   std::vector<game_mode_description> game_modes;

   terrain terrain;
   global_lights global_lights;

   std::vector<object> objects;
   std::vector<light> lights;
   std::vector<path> paths;
   std::vector<region> regions;
   std::vector<sector> sectors;
   std::vector<portal> portals;
   std::vector<hintnode> hintnodes;
   std::vector<barrier> barriers;
   std::vector<planning_hub> planning_hubs;
   std::vector<planning_connection> planning_connections;
   std::vector<boundary> boundaries;
   std::vector<measurement> measurements;

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
   } next_id;
};

}
