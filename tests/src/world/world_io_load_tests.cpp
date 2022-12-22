#include "pch.h"

#include "approx_test_helpers.hpp"
#include "utility/srgb_conversion.hpp"
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

   REQUIRE(world.layer_descriptions.size() == 2);
   CHECK(world.layer_descriptions[0].name == "[Base]"sv);
   CHECK(world.layer_descriptions[1].name == "design"sv);

   REQUIRE(world.gamemode_descriptions.size() == 1);
   CHECK(world.gamemode_descriptions[0].name == "Common"sv);
   CHECK(world.gamemode_descriptions[0].layers == std::vector{0, 1});

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
      CHECK(world.lighting_settings.global_lights[0] == "sun"sv);
      CHECK(world.lighting_settings.global_lights[1] == ""sv);
      CHECK(approx_equals(world.lighting_settings.ambient_sky_color,
                          utility::decompress_srgb({0.5490196f, 0.3098039f, 0.2470588f})));
      CHECK(approx_equals(world.lighting_settings.ambient_ground_color,
                          utility::decompress_srgb({0.3137254f, 0.1568627f, 0.1176470f})));

      REQUIRE(world.lights.size() == 4);

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
         CHECK(world.lights[1].directional_region.empty());
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

      // Light 2
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
   }

   // path checks
   {
      REQUIRE(world.paths.size() == 2);

      // boundary
      {
         CHECK(world.paths[0].name == "boundary"sv);
         CHECK(world.paths[0].layer == 0);
         CHECK(world.paths[0].properties.empty());
         CHECK(is_unique_id(0, world.paths));

         constexpr std::array<float3, 12> expected_positions{
            {{383.557434f, 0.000000f, 4.797800f},
             {332.062256f, 0.000000f, -187.287064f},
             {191.642288f, 0.000000f, -327.707031f},
             {-0.442575f, 0.000000f, -379.202209f},
             {-192.527451f, 0.000000f, -327.707031f},
             {-332.947418f, 0.000000f, -187.287064f},
             {-384.442566f, 0.000000f, 4.797800f},
             {-332.947021f, 0.000000f, 196.882675f},
             {-192.527451f, 0.000000f, 337.302643f},
             {-0.442575f, 0.000000f, 388.797791f},
             {191.642288f, 0.000000f, 337.302246f},
             {332.062256f, 0.000000f, 196.882675f}}};

         REQUIRE(world.paths[0].nodes.size() == 12);

         for (int i = 0; i < world.paths[0].nodes.size(); ++i) {
            CHECK(approx_equals(world.paths[0].nodes[i].position,
                                expected_positions[i]));
            CHECK(approx_equals(world.paths[0].nodes[i].rotation,
                                {0.0f, 0.0f, 1.0f, 0.0f}));
            CHECK(world.paths[0].nodes[i].properties.empty());
         }
      }

      // Path 0
      {
         CHECK(world.paths[1].name == "Path 0"sv);
         CHECK(world.paths[1].layer == 0);
         CHECK(world.paths[1].properties.size() == 1);
         CHECK(world.paths[1].properties[0] ==
               path::property{.key = "PropKey"s, .value = "PropValue"s});
         CHECK(is_unique_id(1, world.paths));

         constexpr std::array<float3, 3> expected_positions{
            {{-16.041691f, 0.000000f, 31.988783f},
             {-31.982189f, 0.000000f, 48.033310f},
             {-48.012756f, 0.000000f, 31.962399f}}};

         REQUIRE(world.paths[1].nodes.size() == 3);

         CHECK(approx_equals(world.paths[1].nodes[0].position, expected_positions[0]));
         CHECK(approx_equals(world.paths[1].nodes[0].rotation,
                             {0.0f, 0.0f, 1.0f, 0.0f}));
         REQUIRE(world.paths[1].nodes[0].properties.size() == 1);
         CHECK(world.paths[1].nodes[0].properties[0] ==
               path::property{.key = "PropKey"s, .value = "PropValue"s});

         CHECK(approx_equals(world.paths[1].nodes[1].position, expected_positions[1]));
         CHECK(approx_equals(world.paths[1].nodes[1].rotation,
                             {0.0f, 0.0f, 1.0f, 0.0f}));
         CHECK(world.paths[1].nodes[1].properties.empty());

         CHECK(approx_equals(world.paths[1].nodes[2].position, expected_positions[2]));
         CHECK(approx_equals(world.paths[1].nodes[2].rotation,
                             {0.0f, 0.0f, 1.0f, 0.0f}));
         CHECK(world.paths[1].nodes[2].properties.empty());
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
   }

   // sector checks
   {
      // TODO: create a test .pvs file
   }

   // portal checks
   {
      // TODO: create a test .pvs file
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
      CHECK(approx_equals(world.barriers[0].corners[0], {72.596146f, 31.159695f}));
      CHECK(approx_equals(world.barriers[0].corners[1], {86.691795f, 0.198154f}));
      CHECK(approx_equals(world.barriers[0].corners[2], {99.806587f, 6.168838f}));
      CHECK(approx_equals(world.barriers[0].corners[3], {85.710938f, 37.130379f}));
      CHECK(is_unique_id(0, world.barriers));
   }

   // planning hubs checks
   {
      // TODO: create a test .pln file
   }

   // planning connections checks
   {
      // TODO: create a test .pln file
   }

   // boundaries checks
   {
      REQUIRE(world.boundaries.size() == 1);
      CHECK(world.boundaries[0].name == "boundary"sv);
      CHECK(is_unique_id(0, world.boundaries));
   }
}

}