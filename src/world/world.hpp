#pragma once

#include "barrier.hpp"
#include "boundary.hpp"
#include "gamemode_description.hpp"
#include "hintnode.hpp"
#include "id.hpp"
#include "layer_description.hpp"
#include "light.hpp"
#include "lighting_settings.hpp"
#include "object.hpp"
#include "path.hpp"
#include "planning.hpp"
#include "portal.hpp"
#include "region.hpp"
#include "sector.hpp"
#include "terrain.hpp"

#include <vector>

namespace we::world {

struct world {
   std::string name;

   std::vector<layer_description> layer_descriptions;
   std::vector<gamemode_description> gamemode_descriptions;

   terrain terrain;
   lighting_settings lighting_settings;

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

   struct next_ids {
      id_generator<gamemode_description> gamemode_descriptions;
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
   } next_id;
};

}
