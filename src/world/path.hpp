#pragma once

#include "id.hpp"
#include "types.hpp"

#include <string>
#include <vector>

namespace we::world {

enum class path_type : int8 { none, entity_follow, formation, patrol };

enum class path_spline_type : int8 { none, linear, hermite, catmull_rom };

struct path {
   std::string name;
   int16 layer = 0;

   struct property {
      std::string key;
      std::string value;

      bool operator==(const property&) const noexcept = default;
   };

   struct node {
      quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
      float3 position{};

      std::vector<property> properties;

      bool operator==(const node&) const noexcept = default;
   };

   path_type type = path_type::none;
   path_spline_type spline_type = path_spline_type::hermite;
   std::vector<property> properties;
   std::vector<node> nodes;

   id<path> id{};

   bool operator==(const path&) const noexcept = default;
};

using path_id = id<path>;

}
