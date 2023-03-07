#pragma once

#include "../hintnode.hpp"

namespace we::world {

struct hintnode_traits {
   bool has_command_post : 1 = false;
   bool has_primary_stance : 1 = false;
   bool has_secondary_stance : 1 = false;
   bool has_mode : 1 = false;
   bool has_radius : 1 = false;
};

auto get_hintnode_traits(const hintnode_type type) -> hintnode_traits;

}
