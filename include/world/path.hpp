#pragma once

#include <types.hpp>

#include <string>
#include <vector>

namespace sk::world {

enum class path_spline_type { none, linear, hermite, catmull_rom };

struct path {
   std::string name;
   int layer = 0;

   struct property {
      std::string key;
      std::string value;
   };

   struct node {
      quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
      vec3 position{};

      std::vector<property> properties;
   };

   path_spline_type spline_type = path_spline_type::hermite;
   std::vector<property> properties;
   std::vector<node> nodes;

   bool operator==(const path&) const noexcept = default;
};

}