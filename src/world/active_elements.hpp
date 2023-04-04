#pragma once

#include "container/slim_bitset.hpp"
#include "layer_description.hpp"

namespace we::world {

struct active_entity_types {
   bool objects : 1 = true;
   bool lights : 1 = false;
   bool paths : 1 = false;
   bool regions : 1 = false;
   bool sectors : 1 = false;
   bool portals : 1 = false;
   bool hintnodes : 1 = false;
   bool barriers : 1 = false;
   bool planning_hubs : 1 = false;
   bool planning_connections : 1 = false;
   bool boundaries : 1 = false;
   bool terrain : 1 = true;
};

using active_layers = container::slim_bitset<max_layers>;

}
