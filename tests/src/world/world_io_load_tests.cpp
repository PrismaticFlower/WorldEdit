#include "pch.h"

#include "approx_test_helpers.hpp"
#include "world/world_io_load.hpp"

#include <span>

using namespace std::literals;
using namespace Catch::literals;

namespace we::world::tests {

namespace {

/// @brief Checks if the entity at index has a unique ID across all other entities.
/// @param index The index of the entity to check.
/// @param entities All world entities.
/// @return If the entity is unique or not.
bool is_unique_id(std::size_t index, const auto& entities)
{
   if (index >= entities.size()) throw std::out_of_range{"out of range"};

   const auto entity_id = entities[index].id;

   for (std::size_t i = 0; i < entities.size(); ++i) {
      if (i == index) continue;

      if (entity_id == entities[i].id) return false;
   }

   return true;
}

}

TEST_CASE("world loading", "[World][IO]")
{
   null_output_stream out;
   const auto world = load_world("data/world/test.wld"sv, out);

   CHECK(world.name == "test"sv);

   REQUIRE(world.requirements.size() == 7);

   CHECK(world.requirements[0].file_type == "path"sv);
   REQUIRE(world.requirements[0].entries.size() == 1);
   CHECK(world.requirements[0].entries[0] == "test"sv);

   CHECK(world.requirements[1].file_type == "congraph"sv);
   REQUIRE(world.requirements[1].entries.size() == 1);
   CHECK(world.requirements[1].entries[0] == "test"sv);

   CHECK(world.requirements[2].file_type == "envfx"sv);
   REQUIRE(world.requirements[2].entries.size() == 1);
   CHECK(world.requirements[2].entries[0] == "test"sv);

   CHECK(world.requirements[3].file_type == "world"sv);
   REQUIRE(world.requirements[3].entries.size() == 1);
   CHECK(world.requirements[3].entries[0] == "test"sv);

   CHECK(world.requirements[4].file_type == "prop"sv);
   REQUIRE(world.requirements[4].entries.size() == 1);
   CHECK(world.requirements[4].entries[0] == "test"sv);

   CHECK(world.requirements[5].file_type == "povs"sv);
   REQUIRE(world.requirements[5].entries.size() == 1);
   CHECK(world.requirements[5].entries[0] == "test"sv);

   CHECK(world.requirements[6].file_type == "lvl"sv);
   REQUIRE(world.requirements[6].entries.size() == 1);
   CHECK(world.requirements[6].entries[0] == "test_conquest"sv);

   REQUIRE(world.layer_descriptions.size() == 2);
   CHECK(world.layer_descriptions[0].name == "[Base]"sv);
   CHECK(world.layer_descriptions[1].name == "design"sv);

   REQUIRE(world.game_modes.size() == 2);
   CHECK(world.game_modes[0].name == "Common"sv);
   CHECK(world.game_modes[0].layers == std::vector{0, 1});

   CHECK(world.game_modes[1].name == "conquest"sv);
   CHECK(world.game_modes[1].layers == std::vector<int>{});
   REQUIRE(world.game_modes[1].requirements.size() == 1);
   CHECK(world.game_modes[1].requirements[0].file_type == "world"sv);
   REQUIRE(world.game_modes[1].requirements[0].entries.size() == 1);
   CHECK(world.game_modes[1].requirements[0].entries[0] == "test_conquest"sv);

   // object checks
   {
      REQUIRE(world.objects.size() == 2);

      // com_item_healthrecharge
      {
         CHECK(world.objects[0].name == "com_item_healthrecharge"sv);
         CHECK(world.objects[0].class_name == "com_item_healthrecharge"sv);
         CHECK(approx_equals(world.objects[0].position, {-32.000f, 0.008f, 32.000f}));
         CHECK(approx_equals(world.objects[0].rotation,
                             {0.000f, 0.000f, 1.000f, 0.000f}));
         CHECK(world.objects[0].team == 0);
         CHECK(world.objects[0].layer == 0);
         CHECK(is_unique_id(0, world.objects));

         REQUIRE(world.objects[0].instance_properties.size() == 2);
         CHECK(world.objects[0].instance_properties[0].key == "EffectRegion"sv);
         CHECK(world.objects[0].instance_properties[0].value == ""sv);
         CHECK(world.objects[0].instance_properties[1].key == "Radius"sv);
         CHECK(world.objects[0].instance_properties[1].value == "5.0"sv);
      }

      // com_inv_col_8
      {
         CHECK(world.objects[1].name == "com_inv_col_8"sv);
         CHECK(world.objects[1].class_name == "com_inv_col_8"sv);
         CHECK(approx_equals(world.objects[1].position, {68.000f, 0.000f, -4.000f}));
         CHECK(approx_equals(world.objects[1].rotation,
                             {0.000f, 0.000f, 1.000f, 0.000f}));
         CHECK(world.objects[1].team == 0);
         CHECK(world.objects[1].layer == 1);
         CHECK(is_unique_id(1, world.objects));
         CHECK(world.objects[1].instance_properties.size() == 0);
      }
   }

   // light checks
   {
      CHECK(world.global_lights.global_light_1 == "sun"sv);
      CHECK(world.global_lights.global_light_2 == ""sv);
      CHECK(approx_equals(world.global_lights.ambient_sky_color,
                          {0.5490196f, 0.3098039f, 0.2470588f}));
      CHECK(approx_equals(world.global_lights.ambient_ground_color,
                          {0.3137254f, 0.1568627f, 0.1176470f}));

      REQUIRE(world.lights.size() == 5);

      // Light 2
      {
         CHECK(world.lights[0].name == "Light 2"sv);
         CHECK(approx_equals(world.lights[0].position,
                             {-128.463806f, 0.855094f, 22.575970f}));
         CHECK(approx_equals(world.lights[0].rotation,
                             {0.000000f, 0.054843f, 0.998519f, 0.000000f}));
         CHECK(world.lights[0].layer == 0);
         CHECK(world.lights[0].light_type == light_type::point);
         CHECK(approx_equals(world.lights[0].color, {0.501961f, 0.376471f, 0.376471f}));
         CHECK(world.lights[0].static_);
         CHECK(world.lights[0].specular_caster);
         CHECK(not world.lights[0].shadow_caster);
         CHECK(world.lights[0].range == 5.0_a);
         CHECK(world.lights[0].texture.empty());
         CHECK(is_unique_id(0, world.lights));
      }

      // sun
      {

         CHECK(world.lights[1].name == "sun"sv);
         CHECK(approx_equals(world.lights[1].position,
                             {-159.264923f, 19.331013f, 66.727310f}));
         CHECK(approx_equals(world.lights[1].rotation,
                             {-0.039542f, 0.008615f, 0.922373f, -0.384204f}));
         CHECK(world.lights[1].layer == 0);
         CHECK(world.lights[1].light_type == light_type::directional);
         CHECK(approx_equals(world.lights[1].color, {1.000000f, 0.882353f, 0.752941f}));
         CHECK(world.lights[1].static_);
         CHECK(world.lights[1].specular_caster);
         CHECK(world.lights[1].shadow_caster);
         CHECK(world.lights[1].texture.empty());
         CHECK(approx_equals(world.lights[1].directional_texture_tiling, {1.0f, 1.0f}));
         CHECK(approx_equals(world.lights[1].directional_texture_offset, {0.0f, 0.0f}));
         CHECK(world.lights[1].region_name.empty());
         CHECK(is_unique_id(1, world.lights));
      }

      // Light 3
      {
         CHECK(world.lights[2].name == "Light 3"sv);
         CHECK(approx_equals(world.lights[2].position,
                             {-149.102463f, 0.469788f, -22.194153f}));
         CHECK(approx_equals(world.lights[2].rotation,
                             {0.000000f, 0.000000f, 1.000000f, 0.000000f}));
         CHECK(world.lights[2].layer == 0);
         CHECK(world.lights[2].light_type == light_type::spot);
         CHECK(approx_equals(world.lights[2].color, {1.000000, 1.000000, 1.000000}));
         CHECK(world.lights[2].static_);
         CHECK(not world.lights[2].specular_caster);
         CHECK(world.lights[2].shadow_caster);
         CHECK(world.lights[2].range == 5.0_a);
         CHECK(world.lights[2].inner_cone_angle == 0.785398_a);
         CHECK(world.lights[2].outer_cone_angle == 0.872665_a);
         CHECK(world.lights[2].texture.empty());
         CHECK(is_unique_id(2, world.lights));
      }

      // Light 1
      {
         CHECK(world.lights[3].name == "Light 1"sv);
         CHECK(approx_equals(world.lights[3].position,
                             {-129.618546f, 5.019108f, 27.300539f}));
         CHECK(approx_equals(world.lights[3].rotation,
                             {0.380202f, 0.000000f, 0.924904f, 0.000000f}));
         CHECK(world.lights[3].layer == 0);
         CHECK(world.lights[3].light_type == light_type::point);
         CHECK(approx_equals(world.lights[3].color, {0.498039f, 0.498039f, 0.627451f}));
         CHECK(world.lights[3].static_);
         CHECK(world.lights[3].specular_caster);
         CHECK(not world.lights[3].shadow_caster);
         CHECK(world.lights[3].range == 16.0_a);
         CHECK(world.lights[3].texture.empty());
         CHECK(is_unique_id(3, world.lights));
      }

      // Light 4
      {
         CHECK(world.lights[4].name == "Light 4"sv);
         CHECK(approx_equals(world.lights[4].position,
                             {-216.604019f, 2.231649f, 18.720726f}));
         CHECK(approx_equals(world.lights[4].rotation,
                             {0.435918f, 0.610941f, 0.581487f, -0.314004f}));
         CHECK(world.lights[4].layer == 0);
         CHECK(world.lights[4].light_type == light_type::directional_region_sphere);
         CHECK(approx_equals(world.lights[4].color, {1.000000f, 0.501961f, 0.501961f}));
         CHECK(world.lights[4].static_);
         CHECK(not world.lights[4].specular_caster);
         CHECK(not world.lights[4].shadow_caster);
         CHECK(world.lights[4].texture.empty());
         CHECK(approx_equals(world.lights[4].directional_texture_tiling, {1.0f, 1.0f}));
         CHECK(approx_equals(world.lights[4].directional_texture_offset, {0.0f, 0.0f}));
         CHECK(world.lights[4].region_name == "lightregion1");
         CHECK(approx_equals(world.lights[4].region_rotation,
                             {0.000f, 0.000f, 1.000f, 0.000f}));
         CHECK(approx_equals(world.lights[4].region_size,
                             {4.591324f, 0.100000f, 1.277475f}));
         CHECK(is_unique_id(4, world.lights));
      }
   }

   // path checks
   {
      REQUIRE(world.paths.size() == 2);

      // Path 0
      {
         CHECK(world.paths[0].name == "Path 0"sv);
         CHECK(world.paths[0].layer == 0);
         CHECK(world.paths[0].type == path_type::none);
         CHECK(world.paths[0].spline_type == path_spline_type::catmull_rom);
         CHECK(world.paths[0].properties.size() == 2);
         CHECK(world.paths[0].properties[0] ==
               path::property{.key = "PropKey"s, .value = "PropValue"s});
         CHECK(world.paths[0].properties[1] ==
               path::property{.key = "PropEmpty"s, .value = ""s});
         CHECK(is_unique_id(0, world.paths));

         constexpr std::array<float3, 3> expected_positions{
            {{-16.041691f, 0.000000f, 31.988783f},
             {-31.982189f, 0.000000f, 48.033310f},
             {-48.012756f, 0.000000f, 31.962399f}}};

         REQUIRE(world.paths[0].nodes.size() == 3);

         CHECK(approx_equals(world.paths[0].nodes[0].position, expected_positions[0]));
         CHECK(approx_equals(world.paths[0].nodes[0].rotation,
                             {0.0f, 0.0f, 1.0f, 0.0f}));
         REQUIRE(world.paths[0].nodes[0].properties.size() == 2);
         CHECK(world.paths[0].nodes[0].properties[0] ==
               path::property{.key = "PropKey"s, .value = "PropValue"s});
         CHECK(world.paths[0].nodes[0].properties[1] ==
               path::property{.key = "PropEmpty"s, .value = ""s});

         CHECK(approx_equals(world.paths[0].nodes[1].position, expected_positions[1]));
         CHECK(approx_equals(world.paths[0].nodes[1].rotation,
                             {0.0f, 0.0f, 1.0f, 0.0f}));
         CHECK(world.paths[0].nodes[1].properties.empty());

         CHECK(approx_equals(world.paths[0].nodes[2].position, expected_positions[2]));
         CHECK(approx_equals(world.paths[0].nodes[2].rotation,
                             {0.0f, 0.0f, 1.0f, 0.0f}));
         CHECK(world.paths[0].nodes[2].properties.empty());
      }

      // Path 1
      {
         CHECK(world.paths[1].name == "Path 1"sv);
         CHECK(world.paths[1].layer == 0);
         CHECK(world.paths[1].type == path_type::entity_follow);
         CHECK(world.paths[1].spline_type == path_spline_type::none);
         CHECK(is_unique_id(1, world.paths));

         REQUIRE(world.paths[1].nodes.size() == 1);

         CHECK(approx_equals(world.paths[1].nodes[0].position,
                             {-16.041691f, 0.000000f, 31.988783f}));
         CHECK(approx_equals(world.paths[1].nodes[0].rotation,
                             {0.0f, 0.0f, 1.0f, 0.0f}));
      }
   }

   // region checks
   {
      REQUIRE(world.regions.size() == 1);

      CHECK(world.regions[0].name == "Region0"sv);
      CHECK(world.regions[0].layer == 0);
      CHECK(world.regions[0].description == "foleyfx water"sv);
      CHECK(world.regions[0].shape == region_shape::box);
      CHECK(approx_equals(world.regions[0].position,
                          {-32.000000f, 16.000000f, 32.000000f}));
      CHECK(approx_equals(world.regions[0].rotation, {0.000f, 0.000f, 1.000f, 0.000f}));
      CHECK(approx_equals(world.regions[0].size, {16.000000f, 16.000000f, 16.000000f}));
      CHECK(is_unique_id(0, world.regions));

      // lightregion1 would be here but it should've been dropped and added to it's light while loading
      CHECK(world.regions.size() == 1);
   }

   // sector checks
   {
      REQUIRE(world.sectors.size() == 2);

      CHECK(world.sectors[0].name == "sector"sv);

      REQUIRE(world.sectors[0].points.size() == 4);
      CHECK(approx_equals(world.sectors[0].points[0], {-157.581009f, -4.900336f}));
      CHECK(approx_equals(world.sectors[0].points[1], {-227.199097f, -7.364827f}));
      CHECK(approx_equals(world.sectors[0].points[2], {-228.642029f, 40.347687f}));
      CHECK(approx_equals(world.sectors[0].points[3], {-159.451279f, 40.488800f}));

      REQUIRE(world.sectors[0].objects.size() == 1);
      CHECK(world.sectors[0].objects[0] == "tat3_bldg_keeper"sv);

      CHECK(world.sectors[1].name == "Sector-1"sv);

      REQUIRE(world.sectors[1].points.size() == 4);
      CHECK(approx_equals(world.sectors[1].points[0], {-196.648041f, 125.908623f}));
      CHECK(approx_equals(world.sectors[1].points[1], {-195.826218f, 49.666763f}));
      CHECK(approx_equals(world.sectors[1].points[2], {-271.034851f, 48.864563f}));
      CHECK(approx_equals(world.sectors[1].points[3], {-274.260132f, 128.690567f}));

      REQUIRE(world.sectors[1].objects.size() == 4);
      CHECK(world.sectors[1].objects[0] == "lod_test120"sv);
      CHECK(world.sectors[1].objects[1] == "lod_test2010"sv);
      CHECK(world.sectors[1].objects[2] == "lod_test12"sv);
      CHECK(world.sectors[1].objects[3] == "lod_test201"sv);

      CHECK(is_unique_id(1, world.sectors));
   }

   // portal checks
   {
      REQUIRE(world.portals.size() == 1);

      CHECK(world.portals[0].name == "Portal"sv);
      CHECK(approx_equals(world.portals[0].position,
                          {-193.661575f, 2.097009f, 31.728502f}));
      CHECK(approx_equals(world.portals[0].rotation, {0.000f, 0.000f, 1.000f, 0.000f}));
      CHECK(world.portals[0].width == 2.92f);
      CHECK(world.portals[0].height == 4.12f);
      CHECK(world.portals[0].sector1 == "sector"sv);
      CHECK(world.portals[0].sector2 == "Sector-1"sv);

      CHECK(is_unique_id(0, world.portals));
   }

   // hintnodes checks
   {
      REQUIRE(world.hintnodes.size() == 2);

      // HintNode0
      {
         CHECK(world.hintnodes[0].name == "HintNode0"sv);
         CHECK(world.hintnodes[0].layer == 0);
         CHECK(approx_equals(world.hintnodes[0].position,
                             {-70.045296f, 1.000582f, 19.298828f}));
         CHECK(approx_equals(world.hintnodes[0].rotation,
                             {-0.569245f, 0.651529f, 0.303753f, -0.399004f}));
         CHECK(world.hintnodes[0].radius == 7.692307_a);
         CHECK(world.hintnodes[0].type == hintnode_type::mine);
         CHECK(world.hintnodes[0].mode == hintnode_mode::attack);
         CHECK(world.hintnodes[0].primary_stance == stance_flags::none);
         CHECK(world.hintnodes[0].secondary_stance == stance_flags::none);
         CHECK(world.hintnodes[0].command_post == "cp1"sv);
         CHECK(is_unique_id(0, world.hintnodes));
      }

      // HintNode1
      {
         CHECK(world.hintnodes[1].name == "HintNode1"sv);
         CHECK(world.hintnodes[1].layer == 0);
         CHECK(approx_equals(world.hintnodes[1].position,
                             {-136.048569f, 0.500000f, 25.761259f}));
         CHECK(approx_equals(world.hintnodes[1].rotation,
                             {-0.995872f, 0.000000f, 0.090763f, 0.000000f}));
         CHECK(world.hintnodes[1].radius == 0.0f);
         CHECK(world.hintnodes[1].type == hintnode_type::mine);
         CHECK(world.hintnodes[1].mode == hintnode_mode::both);
         CHECK(world.hintnodes[1].primary_stance ==
               (stance_flags::stand | stance_flags::crouch | stance_flags::prone));
         CHECK(world.hintnodes[1].secondary_stance == stance_flags::none);
         CHECK(world.hintnodes[1].command_post == "cp2"sv);
         CHECK(is_unique_id(1, world.hintnodes));
      }
   }

   // barriers checks
   {
      REQUIRE(world.barriers.size() == 1);
      CHECK(world.barriers[0].name == "Barrier0"sv);
      CHECK(world.barriers[0].flags == ai_path_flags::flyer);
      CHECK(approx_equals(world.barriers[0].position, {86.2013702f, 2.0f, 18.6642666f}));
      CHECK(approx_equals(world.barriers[0].size, {7.20497799f, 17.0095882f}));
      CHECK(world.barriers[0].rotation_angle == 2.71437049f);
      CHECK(is_unique_id(0, world.barriers));
   }

   // planning hubs checks
   {
      REQUIRE(world.planning_hubs.size() == 4);

      CHECK(world.planning_hubs[0].name == "Hub0"sv);
      CHECK(world.planning_hubs[0].position == float3{-63.822487f, 0.0f, -9.202278f});
      CHECK(world.planning_hubs[0].radius == 8.0f);
      CHECK(is_unique_id(0, world.planning_hubs));

      CHECK(world.planning_hubs[1].name == "Hub1"sv);
      CHECK(world.planning_hubs[1].position == float3{-121.883095f, 1.0f, -30.046543f});
      CHECK(world.planning_hubs[1].radius == 7.586431f);
      CHECK(is_unique_id(1, world.planning_hubs));

      CHECK(world.planning_hubs[2].name == "Hub2"sv);
      CHECK(world.planning_hubs[2].position == float3{-54.011314f, 2.0f, -194.037018f});
      CHECK(world.planning_hubs[2].radius == 13.120973f);
      CHECK(is_unique_id(2, world.planning_hubs));

      CHECK(world.planning_hubs[3].name == "Hub3"sv);
      CHECK(world.planning_hubs[3].position == float3{-163.852570f, 3.0f, -169.116760f});
      CHECK(world.planning_hubs[3].radius == 12.046540f);
      CHECK(is_unique_id(3, world.planning_hubs));
   }

   // planning connections checks
   {
      REQUIRE(world.planning_connections.size() == 2);

      CHECK(world.planning_connections[0].name == "Connection0"sv);
      CHECK(world.planning_connections[0].start_hub_index == 0);
      CHECK(world.planning_connections[0].end_hub_index == 1);
      CHECK(world.planning_connections[0].flags ==
            (ai_path_flags::soldier | ai_path_flags::hover | ai_path_flags::small |
             ai_path_flags::medium | ai_path_flags::huge | ai_path_flags::flyer));

      CHECK(world.planning_connections[0].backward_weights.soldier == 20.0f);
      CHECK(world.planning_connections[0].backward_weights.hover == 15.0f);
      CHECK(world.planning_connections[0].backward_weights.small == 7.5f);
      CHECK(world.planning_connections[0].backward_weights.medium == 25.0f);
      CHECK(world.planning_connections[0].backward_weights.huge == 75.0f);
      CHECK(world.planning_connections[0].backward_weights.flyer == 100.0f);

      CHECK(is_unique_id(0, world.planning_connections));

      CHECK(world.planning_connections[1].name == "Connection1"sv);
      CHECK(world.planning_connections[1].start_hub_index == 3);
      CHECK(world.planning_connections[1].end_hub_index == 2);
      CHECK(world.planning_connections[1].flags == ai_path_flags::hover);
      CHECK(is_unique_id(1, world.planning_connections));
   }

   // boundaries checks
   {

      REQUIRE(world.boundaries.size() == 1);
      CHECK(world.boundaries[0].name == "boundary"sv);
      CHECK(world.boundaries[0].size == float2{384.000000f, 384.000000f});
      CHECK(world.boundaries[0].position == float2{-0.442565918f, 4.79779053f});
      CHECK(is_unique_id(0, world.boundaries));
   }

   // measurements checks
   {
      REQUIRE(world.measurements.size() == 1);
      CHECK(world.measurements[0].name == "Measurement0"sv);
      CHECK(world.measurements[0].start == float3{1.0f, 0.0f, -0.0f});
      CHECK(world.measurements[0].end == float3{2.0f, 0.0f, -1.0f});
      CHECK(is_unique_id(0, world.measurements));
   }
}
}