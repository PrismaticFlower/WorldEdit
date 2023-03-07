#include "hintnode_traits.hpp"

namespace we::world {

auto get_hintnode_traits(const hintnode_type type) -> hintnode_traits
{
   switch (type) {
   case hintnode_type::snipe:
      return {.has_primary_stance = true, .has_mode = true, .has_radius = true};
   case hintnode_type::patrol:
      return {.has_command_post = true, .has_primary_stance = true};
   case hintnode_type::cover:
      return {.has_primary_stance = true, .has_secondary_stance = true, .has_radius = true};
   case hintnode_type::access:
      return {};
   case hintnode_type::jet_jump:
      return {};
   case hintnode_type::mine:
      return {.has_command_post = true, .has_radius = true};
   case hintnode_type::land:
      return {.has_command_post = true};
   case hintnode_type::fortification:
      return {.has_primary_stance = true, .has_secondary_stance = true};
   case hintnode_type::vehicle_cover:
      return {.has_primary_stance = true, .has_secondary_stance = true, .has_radius = true};
   default:
      return {};
   }
}

}
