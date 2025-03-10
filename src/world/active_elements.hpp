#pragma once

#include "container/slim_bitset.hpp"
#include "layer_description.hpp"

namespace we::world {

struct active_entity_types {
   unsigned short objects : 1 = true;
   unsigned short lights : 1 = false;
   unsigned short paths : 1 = false;
   unsigned short regions : 1 = false;
   unsigned short sectors : 1 = false;
   unsigned short portals : 1 = false;
   unsigned short hintnodes : 1 = false;
   unsigned short barriers : 1 = false;
   unsigned short planning_hubs : 1 = false;
   unsigned short planning_connections : 1 = false;
   unsigned short boundaries : 1 = false;
   unsigned short measurements : 1 = true;
   unsigned short terrain : 1 = true;
   unsigned short blocks : 1 = true;
};

using active_layers = container::slim_bitset<max_layers>;

}
