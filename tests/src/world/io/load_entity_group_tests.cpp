#include "pch.h"

#include "world/io/load_entity_group.hpp"

using namespace std::literals;

namespace we::world::tests {

TEST_CASE("world entity group loading (objects)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_objects.eng", out);

   REQUIRE(group.objects.size() == 2);

   // com_item_healthrecharge
   {
      CHECK(group.objects[0].name == "com_item_healthrecharge"sv);
      CHECK(group.objects[0].class_name == "com_item_healthrecharge"sv);
      CHECK(group.objects[0].position == float3{-32.000f, 0.008f, 32.000f});
      CHECK(group.objects[0].rotation == quaternion{0.000f, 0.000f, 1.000f, 0.000f});
      CHECK(group.objects[0].team == 0);
      CHECK(group.objects[0].layer == 0);

      REQUIRE(group.objects[0].instance_properties.size() == 2);
      CHECK(group.objects[0].instance_properties[0].key == "EffectRegion"sv);
      CHECK(group.objects[0].instance_properties[0].value == ""sv);
      CHECK(group.objects[0].instance_properties[1].key == "Radius"sv);
      CHECK(group.objects[0].instance_properties[1].value == "5.0"sv);
   }

   // com_inv_col_8
   {
      CHECK(group.objects[1].name == "com_inv_col_8"sv);
      CHECK(group.objects[1].class_name == "com_inv_col_8"sv);
      CHECK(group.objects[1].position == float3{68.000f, 0.000f, -4.000f});
      CHECK(group.objects[1].rotation == quaternion{0.000f, 0.000f, 1.000f, 0.000f});
      CHECK(group.objects[1].team == 0);
      CHECK(group.objects[1].layer == 0);
      CHECK(group.objects[1].instance_properties.size() == 0);
   }
}

TEST_CASE("world entity group loading (lights)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_lights.eng", out);

   REQUIRE(group.lights.size() == 5);

   // Light 2
   {
      CHECK(group.lights[0].name == "Light 2"sv);
      CHECK(group.lights[0].position == float3{-128.463806f, 0.855094f, 22.575970f});
      CHECK(group.lights[0].rotation ==
            quaternion{0.000000f, 0.054843f, 0.998519f, 0.000000f});
      CHECK(group.lights[0].layer == 0);
      CHECK(group.lights[0].light_type == light_type::point);
      CHECK(group.lights[0].color == float3{0.501961f, 0.376471f, 0.376471f});
      CHECK(group.lights[0].static_);
      CHECK(group.lights[0].specular_caster);
      CHECK(not group.lights[0].shadow_caster);
      CHECK(group.lights[0].range == 5.0f);
      CHECK(group.lights[0].texture.empty());
   }

   // sun
   {

      CHECK(group.lights[1].name == "sun"sv);
      CHECK(group.lights[1].position == float3{-159.264923f, 19.331013f, 66.727310f});
      CHECK(group.lights[1].rotation ==
            quaternion{-0.039542f, 0.008615f, 0.922373f, -0.384204f});
      CHECK(group.lights[1].layer == 0);
      CHECK(group.lights[1].light_type == light_type::directional);
      CHECK(group.lights[1].color == float3{1.000000f, 0.882353f, 0.752941f});
      CHECK(group.lights[1].static_);
      CHECK(group.lights[1].specular_caster);
      CHECK(group.lights[1].shadow_caster);
      CHECK(group.lights[1].texture.empty());
      CHECK(group.lights[1].directional_texture_tiling == float2{1.0f, 1.0f});
      CHECK(group.lights[1].directional_texture_offset == float2{0.0f, 0.0f});
      CHECK(group.lights[1].region_name.empty());
   }

   // Light 3
   {
      CHECK(group.lights[2].name == "Light 3"sv);
      CHECK(group.lights[2].position == float3{-149.102463f, 0.469788f, -22.194153f});
      CHECK(group.lights[2].rotation ==
            quaternion{0.000000f, 0.000000f, 1.000000f, 0.000000f});
      CHECK(group.lights[2].layer == 0);
      CHECK(group.lights[2].light_type == light_type::spot);
      CHECK(group.lights[2].color == float3{1.000000, 1.000000, 1.000000});
      CHECK(group.lights[2].static_);
      CHECK(not group.lights[2].specular_caster);
      CHECK(group.lights[2].shadow_caster);
      CHECK(group.lights[2].range == 5.0f);
      CHECK(group.lights[2].inner_cone_angle == 0.785398f);
      CHECK(group.lights[2].outer_cone_angle == 0.872665f);
      CHECK(group.lights[2].texture.empty());
      CHECK(group.lights[2].bidirectional);
   }

   // Light 1
   {
      CHECK(group.lights[3].name == "Light 1"sv);
      CHECK(group.lights[3].position == float3{-129.618546f, 5.019108f, 27.300539f});
      CHECK(group.lights[3].rotation ==
            quaternion{0.380202f, 0.000000f, 0.924904f, 0.000000f});
      CHECK(group.lights[3].layer == 0);
      CHECK(group.lights[3].light_type == light_type::point);
      CHECK(group.lights[3].color == float3{0.498039f, 0.498039f, 0.627451f});
      CHECK(group.lights[3].static_);
      CHECK(group.lights[3].specular_caster);
      CHECK(not group.lights[3].shadow_caster);
      CHECK(group.lights[3].range == 16.0f);
      CHECK(group.lights[3].texture.empty());
   }

   // Light 4
   {
      CHECK(group.lights[4].name == "Light 4"sv);
      CHECK(group.lights[4].position == float3{-216.604019f, 2.231649f, 18.720726f});
      CHECK(group.lights[4].rotation ==
            quaternion{0.435918f, 0.610941f, 0.581487f, -0.314004f});
      CHECK(group.lights[4].layer == 0);
      CHECK(group.lights[4].light_type == light_type::directional_region_sphere);
      CHECK(group.lights[4].color == float3{1.000000f, 0.501961f, 0.501961f});
      CHECK(group.lights[4].static_);
      CHECK(not group.lights[4].specular_caster);
      CHECK(not group.lights[4].shadow_caster);
      CHECK(group.lights[4].texture.empty());
      CHECK(group.lights[4].directional_texture_tiling == float2{1.0f, 1.0f});
      CHECK(group.lights[4].directional_texture_offset == float2{0.0f, 0.0f});
      CHECK(group.lights[4].region_name == "lightregion1");
      CHECK(group.lights[4].ps2_blend_mode == ps2_blend_mode::blend);
      CHECK(group.lights[4].region_rotation ==
            quaternion{0.000f, 0.000f, 1.000f, 0.000f});
      CHECK(group.lights[4].region_size == float3{4.591324f, 0.100000f, 1.277475f});
   }
}

TEST_CASE("world entity group loading (paths)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_paths.eng", out);

   REQUIRE(group.paths.size() == 2);

   // Path 0
   {
      CHECK(group.paths[0].name == "Path 0"sv);
      CHECK(group.paths[0].type == path_type::none);
      CHECK(group.paths[0].spline_type == path_spline_type::catmull_rom);
      CHECK(group.paths[0].properties.size() == 2);
      CHECK(group.paths[0].properties[0] ==
            path::property{.key = "PropKey"s, .value = "PropValue"s});
      CHECK(group.paths[0].properties[1] ==
            path::property{.key = "PropEmpty"s, .value = ""s});

      constexpr std::array<float3, 3> expected_positions{
         {{-16.041691f, 0.000000f, 31.988783f},
          {-31.982189f, 0.000000f, 48.033310f},
          {-48.012756f, 0.000000f, 31.962399f}}};

      REQUIRE(group.paths[0].nodes.size() == 3);

      CHECK(group.paths[0].nodes[0].position == expected_positions[0]);
      CHECK(group.paths[0].nodes[0].rotation == quaternion{0.0f, 0.0f, 1.0f, 0.0f});
      REQUIRE(group.paths[0].nodes[0].properties.size() == 2);
      CHECK(group.paths[0].nodes[0].properties[0] ==
            path::property{.key = "PropKey"s, .value = "PropValue"s});
      CHECK(group.paths[0].nodes[0].properties[1] ==
            path::property{.key = "PropEmpty"s, .value = ""s});

      CHECK(group.paths[0].nodes[1].position == expected_positions[1]);
      CHECK(group.paths[0].nodes[1].rotation == quaternion{0.0f, 0.0f, 1.0f, 0.0f});
      CHECK(group.paths[0].nodes[1].properties.empty());

      CHECK(group.paths[0].nodes[2].position == expected_positions[2]);
      CHECK(group.paths[0].nodes[2].rotation == quaternion{0.0f, 0.0f, 1.0f, 0.0f});
      CHECK(group.paths[0].nodes[2].properties.empty());
   }

   // Path 1
   {
      CHECK(group.paths[1].name == "Path 1"sv);
      CHECK(group.paths[1].type == path_type::entity_follow);
      CHECK(group.paths[1].spline_type == path_spline_type::none);

      REQUIRE(group.paths[1].nodes.size() == 1);

      CHECK(group.paths[1].nodes[0].position ==
            float3{-16.041691f, 0.000000f, 31.988783f});
      CHECK(group.paths[1].nodes[0].rotation == quaternion{0.0f, 0.0f, 1.0f, 0.0f});
   }
}

TEST_CASE("world entity group loading (regions)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_regions.eng", out);

   REQUIRE(group.regions.size() == 1);

   CHECK(group.regions[0].name == "Region0"sv);
   CHECK(group.regions[0].description == "foleyfx water"sv);
   CHECK(group.regions[0].shape == region_shape::box);
   CHECK(group.regions[0].position == float3{-32.000000f, 16.000000f, 32.000000f});
   CHECK(group.regions[0].rotation == quaternion{0.000f, 0.000f, 1.000f, 0.000f});
   CHECK(group.regions[0].size == float3{16.000000f, 16.000000f, 16.000000f});
}

TEST_CASE("world entity group loading (sectors)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_sectors.eng", out);

   REQUIRE(group.sectors.size() == 2);

   CHECK(group.sectors[0].name == "sector"sv);

   REQUIRE(group.sectors[0].points.size() == 4);
   CHECK(group.sectors[0].points[0] == float2{-157.581009f, -4.900336f});
   CHECK(group.sectors[0].points[1] == float2{-227.199097f, -7.364827f});
   CHECK(group.sectors[0].points[2] == float2{-228.642029f, 40.347687f});
   CHECK(group.sectors[0].points[3] == float2{-159.451279f, 40.488800f});

   REQUIRE(group.sectors[0].objects.size() == 1);
   CHECK(group.sectors[0].objects[0] == "tat3_bldg_keeper"sv);

   CHECK(group.sectors[1].name == "Sector-1"sv);

   REQUIRE(group.sectors[1].points.size() == 4);
   CHECK(group.sectors[1].points[0] == float2{-196.648041f, 125.908623f});
   CHECK(group.sectors[1].points[1] == float2{-195.826218f, 49.666763f});
   CHECK(group.sectors[1].points[2] == float2{-271.034851f, 48.864563f});
   CHECK(group.sectors[1].points[3] == float2{-274.260132f, 128.690567f});

   REQUIRE(group.sectors[1].objects.size() == 4);
   CHECK(group.sectors[1].objects[0] == "lod_test120"sv);
   CHECK(group.sectors[1].objects[1] == "lod_test2010"sv);
   CHECK(group.sectors[1].objects[2] == "lod_test12"sv);
   CHECK(group.sectors[1].objects[3] == "lod_test201"sv);
}

TEST_CASE("world entity group loading (portals)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_portals.eng", out);

   REQUIRE(group.portals.size() == 1);

   CHECK(group.portals[0].name == "Portal"sv);
   CHECK(group.portals[0].position == float3{-193.661575f, 2.097009f, 31.728502f});
   CHECK(group.portals[0].rotation == quaternion{0.000f, 0.000f, 1.000f, 0.000f});
   CHECK(group.portals[0].width == 2.92f);
   CHECK(group.portals[0].height == 4.12f);
   CHECK(group.portals[0].sector1 == "sector"sv);
   CHECK(group.portals[0].sector2 == "Sector-1"sv);
}

TEST_CASE("world entity group loading (hintnodes)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_hintnodes.eng", out);

   REQUIRE(group.hintnodes.size() == 2);

   // HintNode0
   {
      CHECK(group.hintnodes[0].name == "HintNode0"sv);
      CHECK(group.hintnodes[0].position == float3{-70.045296f, 1.000582f, 19.298828f});
      CHECK(group.hintnodes[0].rotation ==
            quaternion{-0.569245f, 0.651529f, 0.303753f, -0.399004f});
      CHECK(group.hintnodes[0].radius == 7.692307f);
      CHECK(group.hintnodes[0].type == hintnode_type::mine);
      CHECK(group.hintnodes[0].mode == hintnode_mode::attack);
      CHECK(group.hintnodes[0].primary_stance == stance_flags::none);
      CHECK(group.hintnodes[0].secondary_stance == stance_flags::none);
      CHECK(group.hintnodes[0].command_post == "cp1"sv);
   }

   // HintNode1
   {
      CHECK(group.hintnodes[1].name == "HintNode1"sv);
      CHECK(group.hintnodes[1].position == float3{-136.048569f, 0.500000f, 25.761259f});
      CHECK(group.hintnodes[1].rotation ==
            quaternion{-0.995872f, 0.000000f, 0.090763f, 0.000000f});
      CHECK(group.hintnodes[1].radius == 0.0f);
      CHECK(group.hintnodes[1].type == hintnode_type::mine);
      CHECK(group.hintnodes[1].mode == hintnode_mode::both);
      CHECK(group.hintnodes[1].primary_stance ==
            (stance_flags::stand | stance_flags::crouch | stance_flags::prone));
      CHECK(group.hintnodes[1].secondary_stance == stance_flags::none);
      CHECK(group.hintnodes[1].command_post == "cp2"sv);
   }
}

TEST_CASE("world entity group loading (barriers)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_barriers.eng", out);

   REQUIRE(group.barriers.size() == 1);
   CHECK(group.barriers[0].name == "Barrier0"sv);
   CHECK(group.barriers[0].flags == ai_path_flags::flyer);
   CHECK(group.barriers[0].position == float3{86.2013626f, 2.0f, 18.6642666f});
   CHECK(group.barriers[0].size == float2{7.20497799f, 17.0095882f});
   CHECK(group.barriers[0].rotation_angle == 2.71437049f);
}

TEST_CASE("world entity group loading (planning)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_planning.eng", out);

   REQUIRE(group.planning_hubs.size() == 4);

   CHECK(group.planning_hubs[0].name == "Hub0"sv);
   CHECK(group.planning_hubs[0].position == float3{-63.822487f, 0.0f, -9.202278f});
   CHECK(group.planning_hubs[0].radius == 8.0f);
   CHECK(group.planning_hubs[0].weights.empty());

   CHECK(group.planning_hubs[1].name == "Hub1"sv);
   CHECK(group.planning_hubs[1].position == float3{-121.883095f, 1.0f, -30.046543f});
   CHECK(group.planning_hubs[1].radius == 7.586431f);
   REQUIRE(group.planning_hubs[1].weights.size() == 1);
   CHECK(group.planning_hubs[1].weights[0].connection_index == 0);
   CHECK(group.planning_hubs[1].weights[0].hub_index == 3);
   CHECK(group.planning_hubs[1].weights[0].soldier == 20.0f);
   CHECK(group.planning_hubs[1].weights[0].hover == 15.0f);
   CHECK(group.planning_hubs[1].weights[0].small == 7.5f);
   CHECK(group.planning_hubs[1].weights[0].medium == 25.0f);
   CHECK(group.planning_hubs[1].weights[0].huge == 75.0f);
   CHECK(group.planning_hubs[1].weights[0].flyer == 100.0f);

   CHECK(group.planning_hubs[2].name == "Hub2"sv);
   CHECK(group.planning_hubs[2].position == float3{-54.011314f, 2.0f, -194.037018f});
   CHECK(group.planning_hubs[2].radius == 13.120973f);
   CHECK(group.planning_hubs[2].weights.empty());

   CHECK(group.planning_hubs[3].name == "Hub3"sv);
   CHECK(group.planning_hubs[3].position == float3{-163.852570f, 3.0f, -169.116760f});
   CHECK(group.planning_hubs[3].radius == 12.046540f);
   CHECK(group.planning_hubs[3].weights.empty());

   REQUIRE(group.planning_connections.size() == 2);

   CHECK(group.planning_connections[0].name == "Connection0"sv);
   CHECK(group.planning_connections[0].start_hub_index == 0);
   CHECK(group.planning_connections[0].end_hub_index == 1);
   CHECK(group.planning_connections[0].flags ==
         (ai_path_flags::soldier | ai_path_flags::hover | ai_path_flags::small |
          ai_path_flags::medium | ai_path_flags::huge | ai_path_flags::flyer));

   CHECK(group.planning_connections[1].name == "Connection1"sv);
   CHECK(group.planning_connections[1].start_hub_index == 3);
   CHECK(group.planning_connections[1].end_hub_index == 2);
   CHECK(group.planning_connections[1].flags == ai_path_flags::hover);
}

TEST_CASE("world entity group loading (boundaries)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_boundaries.eng", out);

   REQUIRE(group.boundaries.size() == 1);
   CHECK(group.boundaries[0].name == "boundary"sv);
   CHECK(group.boundaries[0].size == float2{384.000000f, 384.000000f});
   CHECK(group.boundaries[0].position == float3{-0.442565918f, 1.0f, 4.79779053f});
}

TEST_CASE("world entity group loading (measurements)", "[World][IO]")
{
   null_output_stream out;
   const entity_group group =
      load_entity_group("data/entity_groups/test_measurements.eng", out);

   REQUIRE(group.measurements.size() == 1);
   CHECK(group.measurements[0].name == "Measurement0"sv);
   CHECK(group.measurements[0].start == float3{1.0f, 0.0f, -0.0f});
   CHECK(group.measurements[0].end == float3{2.0f, 0.0f, -1.0f});
}

}