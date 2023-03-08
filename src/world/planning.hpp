#pragma once

#include "ai_path_flags.hpp"
#include "id.hpp"
#include "types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace we::world {

struct planning_branch_weights {
   float soldier = 0.0f;
   float hover = 0.0f;
   float small = 0.0f;
   float medium = 0.0f;
   float huge = 0.0f;
   float flyer = 0.0f;

   bool operator==(const planning_branch_weights&) const noexcept = default;
};

struct planning_hub {
   std::string name;
   int layer = 0;

   float2 position;
   float radius = 0.0f;

   id<planning_hub> id{};

   bool operator==(const planning_hub&) const noexcept = default;
};

using planning_hub_id = id<planning_hub>;

struct planning_connection {
   std::string name;
   int layer = 0;

   std::string start;
   std::string end;

   ai_path_flags flags = ai_path_flags::soldier | ai_path_flags::hover |
                         ai_path_flags::small | ai_path_flags::medium |
                         ai_path_flags::huge | ai_path_flags::flyer;

   bool jump = false;
   bool jet_jump = false;
   bool one_way = false;
   int8 dynamic_group = 0;

   planning_branch_weights forward_weights;
   planning_branch_weights backward_weights;

   id<planning_connection> id{};

   bool operator==(const planning_connection&) const noexcept = default;
};

using planning_connection_id = id<planning_connection>;

}
