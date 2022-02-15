#pragma once

#include "ai_path_flags.hpp"
#include "id.hpp"
#include "types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace we::world {

struct planning_hub {
   struct branch_weight {
      std::string target_hub;
      std::string connection;
      float weight = 100.0f;
      ai_path_flags flag = ai_path_flags::soldier;

      bool operator==(const branch_weight&) const noexcept = default;
   };

   std::string name;
   int layer = 0;

   float3 position;
   float radius = 0.0f;

   std::vector<branch_weight> branch_weights;

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
   bool jetjump = false;
   bool oneway = false;
   std::optional<int> dynamic_group;

   id<planning_connection> id{};

   bool operator==(const planning_connection&) const noexcept = default;
};

using planning_connection_id = id<planning_connection>;

}
