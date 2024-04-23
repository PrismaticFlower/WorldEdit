#pragma once

#include "id.hpp"
#include "types.hpp"

#include <string>
#include <vector>

namespace we::world {

enum class animation_transition { pop = 0, linear = 1, spline = 2 };

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

// Animation("{name}", {runtime}, {loop}, {local_translation})
struct animation {
   std::string name;

   float runtime = 0.0f;
   bool loop = false;
   bool local_translation = false;

   std::vector<position_key> position_keys;
   std::vector<rotation_key> rotation_keys;

   id<animation> id = {};
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

   id<animation_group> id = {};
};

struct animation_hierarchy {
   std::string root_object;

   std::vector<std::string> objects;

   id<animation_hierarchy> id = {};
};

using animation_id = id<animation>;
using animation_group_id = id<animation_group>;
using animation_hierarchy_id = id<animation_hierarchy>;

}
