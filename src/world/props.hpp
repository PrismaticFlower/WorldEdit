#pragma once

#include "id.hpp"
#include "object.hpp"

#include "types.hpp"

#include "container/pinned_vector.hpp"

#include <array>
#include <string>

namespace we::world {

constexpr std::size_t max_foliage_layer_meshes = 16'384;
constexpr std::size_t max_tree_lines = 16'384;
constexpr std::size_t max_tree_line_border_odfs = 10; // Hard Limit from the game.

struct foliage_file {
   std::string name;
   float max_distance = 1000.0f;
};

struct foliage_sound {
   std::string name;
   float unknown0 = 0.0f;    // WHAT DOES IT DO?
   float unknown1 = FLT_MAX; // WHAT DOES IT DO?
};

struct foliage_mesh {
   foliage_file file;
   foliage_file grass_patch;
   foliage_file leaf_patch;

   float fade_distance = 0.0f;
   float scale = 1.0f;

   foliage_sound sound;
   std::string collision_sound;

   float ai_visibility_factor_min = 1.0f;
   float ai_visibility_factor_max = 1.0f;

   float color_variation = 0.0f;

   bool use_collision = false;
   bool lighting = false;

   float frequency = 1.0f;
   float stiffness = 0.0f;
   float max_distance = 1000.0f;
};

struct foliage_layer {
   float spread_factor = 0.0f;
   pinned_vector<foliage_mesh> meshes =
      pinned_vector_init{.max_size = max_tree_lines, .initial_capacity = 32};
};

struct tree_line_odf {
   std::string name;
   object_class_handle handle = object_class_handle{};
};

struct tree_line {
   float distance = 0.0f;

   std::vector<tree_line_odf> border_odfs;

   bool flip = false;

   uint32 path_index = 0;

   id<tree_line> id;
};

using tree_line_id = id<tree_line>;

}