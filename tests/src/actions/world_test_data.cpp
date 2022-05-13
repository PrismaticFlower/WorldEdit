
#include "pch.h"

#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::actions::tests {

using namespace world;

const we::world::world test_world = {
   .name = "Test"s,

   .layer_descriptions = {{.name = "[Base]"s}},
   .gamemode_descriptions = {},

   .terrain = {},

   .objects = {object{
      .name = "test_object"s,
      .layer = 0,

      .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
      .position = {0.0f, 0.0f, 0.0f},

      .team = 0,

      .class_name = lowercase_string{"test_prop_wall"sv},
      .instance_properties = {{.key = "MaxHealth"s, .value = "50000"}},
   }},

   .lights = {light{
      .name = "test_light"s,
      .layer = 0,

      .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
      .position = {0.0f, 0.0f, 0.0f},

      .color = {1.0f, 1.0f, 1.0f},
      .static_ = false,
      .shadow_caster = false,
      .specular_caster = false,
      .light_type = light_type::point,
      .texture_addressing = texture_addressing::clamp,

      .range = 8.0f,
      .inner_cone_angle = 0.785398f,
      .outer_cone_angle = 0.959931f,

      .directional_texture_tiling = {1.0f, 1.0f},
      .directional_texture_offset = {0.0f, 0.0f},

      .texture = ""s,
      .directional_region = ""s,
   }},

   .paths = {path{
      .name = "test_path"s,
      .layer = 0,

      .spline_type = path_spline_type::hermite,
      .properties = {{.key = "Key"s, .value = "Value"s}},
      .nodes = {{
         .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
         .position = {0.0f, 0.0f, 0.0f},
         .properties = {{.key = "Key"s, .value = "Value"s}},
      }},
   }},

   .regions = {region{
      .name = "test_region"s,
      .layer = 0,

      .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
      .position = {0.0f, 0.0f, 0.0f},
      .size = {1.0f, 1.0f, 1.0f},
      .shape = region_shape::box,

      .description = ""s,
   }},

   .sectors = {sector{
      .name = "test_sector"s,

      .base = 0.0f,
      .height = 0.0f,
      .points = {{0.0f, 0.0f}, {0.0f, 10.0f}, {10.0f, 10.0f}, {10.0f, 0.0f}},
      .objects = {"test_object"s},
   }},

   .portals = {portal{
      .name = "test_portal"s,

      .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
      .position = {0.0f, 10.0f, 0.0f},

      .width = 5.0f,
      .height = 5.0f,

      .sector1 = "test_sector"s,
      .sector2 = ""s,
   }},

   .hintnodes = {hintnode{
      .name = "test_hintnode"s,
      .layer = 0,

      .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
      .position = {0.0f, 0.0f, 0.0f},

      .type = hintnode_type::snipe,
      .mode = hintnode_mode::none,
      .radius = 0.0f,

      .primary_stance = stance_flags::crouch,
      .secondary_stance = stance_flags::none,

      .command_post = ""s,
   }},

   .barriers = {barrier{
      .name = "test_barrier"s,

      .corners = {{{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}}},
      .flags = ai_path_flags::soldier | ai_path_flags::hover | ai_path_flags::small |
               ai_path_flags::medium | ai_path_flags::huge | ai_path_flags::flyer,
   }},

   // TODO: Path Planning!

   .boundaries = {boundary{.name = "test_path"s}},
};
}