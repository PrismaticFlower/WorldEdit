
#include "pch.h"

#include "world/world.hpp"
#include "world_test_data.hpp"

using namespace std::literals;

namespace we::edits::tests {

using namespace world;

const we::world::world test_world = {
   .name = "Test"s,

   .requirements = {{.file_type = "world", .entries = {"Test"}}},

   .layer_descriptions = {{.name = "[Base]"s}},
   .game_modes = {{.name = "Common"s, .layers = {0}}},

   .terrain = {},
   .global_lights = {.env_map_texture = "sky"},

   .objects = {object{
      .name = "test_object"s,
      .layer = 0,

      .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
      .position = {0.0f, 0.0f, 0.0f},

      .team = 0,

      .class_name = lowercase_string{"test_prop_wall"sv},
      .instance_properties =
         {
            {.key = "MaxHealth"s, .value = "50000"},
            {.key = "SpawnPath"s, .value = "test_path"},
            {.key = "AllyPath"s, .value = "test_path"},
            {.key = "TurretPath"s, .value = "test_path"},
            {.key = "CaptureRegion"s, .value = "test_region"},
            {.key = "ControlRegion"s, .value = "test_region"},
            {.key = "EffectRegion"s, .value = "test_region"},
            {.key = "KillRegion"s, .value = "test_region"},
            {.key = "SpawnRegion"s, .value = "test_region"},
         },
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
      .region_name = ""s,
   }},

   .paths = {path{
      .name = "test_path"s,
      .layer = 0,

      .spline_type = path_spline_type::hermite,
      .properties = {{.key = "Key"s, .value = "Value"s},
                     {.key = "EnableObject"s, .value = "test_object"s}},
      .nodes = {{
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                   .position = {0.0f, 0.0f, 0.0f},
                   .properties = {{.key = "Key"s, .value = "Value"s}},
                },
                {
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                   .position = {1.0f, 1.0f, 1.0f},
                }},
   }},

   .regions = {region{
      .name = "test_region"s,
      .layer = 0,

      .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
      .position = {0.0f, 0.0f, 0.0f},
      .size = {1.0f, 1.0f, 1.0f},
      .shape = region_shape::box,

      .description = "test_region"s,
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

      .command_post = "test_object"s,
   }},

   .barriers = {barrier{
      .name = "test_barrier"s,

      .position = {0.5f, 0.5f},
      .size = {0.5f, 0.5f},
      .rotation_angle = 0.0f,
      .flags = ai_path_flags::soldier | ai_path_flags::hover | ai_path_flags::small |
               ai_path_flags::medium | ai_path_flags::huge | ai_path_flags::flyer,
   }},

   .planning_hubs = {planning_hub{.name = "Hub0",
                                  .position = float2{-63.822487f, -9.202278f},
                                  .radius = 8.0f,
                                  .id = planning_hub_id{0}},

                     planning_hub{.name = "Hub1",
                                  .position = float2{-121.883095f, -30.046543f},
                                  .radius = 7.586431f,
                                  .id = planning_hub_id{1}}},

   .planning_connections =
      {planning_connection{.name = "Connection0",
                           .start = planning_hub_id{0},
                           .end = planning_hub_id{1},
                           .flags = (ai_path_flags::soldier | ai_path_flags::hover |
                                     ai_path_flags::small | ai_path_flags::medium |
                                     ai_path_flags::huge | ai_path_flags::flyer)}},

   .boundaries = {boundary{.name = "boundary"s}},

   .planning_hub_index = {{planning_hub_id{0}, 0}, {planning_hub_id{1}, 1}},
};
}