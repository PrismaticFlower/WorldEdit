#pragma once

#include "types.hpp"

#include <string>
#include <vector>

namespace we::world {

enum class animation_transition { pop, linear, spline };

struct position_key {
   float time;
   float3 position;
   animation_transition transition = animation_transition::linear;
   float3 tangent = {};
   float3 tangent_next = {};
};

struct rotation_key {
   float time;
   float3 rotation;
   animation_transition transition = animation_transition::linear;
   float3 tangent = {};
   float3 tangent_next = {};
};

// Animation("{name}", {time}, {loop}, {local_translation})
struct animation {
   std::string name;

   float runtime = 0.0f;
   bool loop = false;
   bool local_translation = false;

   std::vector<position_key> position_keys;
   std::vector<rotation_key> rotation_keys;
};

// AnimationGroup("{name}", {play_when_level_begins}, {stops_when_object_is_controlled})
struct animation_group {
   struct entry {
      std::string animation;
      std::string object;
   };

   std::string name;

   bool play_when_level_begins = false;
   bool stops_when_object_is_controlled = false;
   bool disable_hierarchies = false;

   std::vector<entry> entries;
};

struct animation_hierarchy {
   std::string root_name;

   std::vector<std::string> objects;
};

}
