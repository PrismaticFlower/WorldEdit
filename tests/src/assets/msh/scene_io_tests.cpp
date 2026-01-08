
#include "pch.h"

#include "approx_test_helpers.hpp"
#include "assets/msh/scene_io.hpp"

#include <string_view>

using namespace std::literals;
using namespace Catch::literals;

namespace we::assets::msh::tests {

TEST_CASE(".msh reading", "[Assets][MSH]")
{
   auto scene = load_scene("data/sand_test.msh", {});

   // Materials Checks
   {
      REQUIRE(scene.materials.size() == 1);

      const material& material = scene.materials[0];

      CHECK(material.name == "Material"sv);
      CHECK(approx_equals(material.specular_color, {0.0125864427536726f, 0.013322648592293262f,
                                                    0.009387092664837837f}));
      CHECK(material.flags == material_flags::specular);
      CHECK(material.rendertype == rendertype::normalmap);
      CHECK(material.data0 == 0);
      CHECK(material.data1 == 0);
      CHECK(material.textures[0] == "sand"sv);
      CHECK(material.textures[1] == "sand_normalmap"sv);
      CHECK(material.textures[2] == "sand_detail"sv);
   }

   // Node Checks
   {
      REQUIRE(scene.nodes.size() == 3);

      // Node 0
      {
         const node& node = scene.nodes[0];

         REQUIRE(node.name == "root"sv);
         REQUIRE(node.parent == std::nullopt);
         REQUIRE(node.type == node_type::null);
         REQUIRE(node.hidden);

         CHECK(approx_equals(node.transform.translation, {0.0f, 0.0f, 0.0f}));
         CHECK(node.transform.rotation.w == -1.0_a);
         CHECK(node.transform.rotation.x == 0.0_a);
         CHECK(node.transform.rotation.y == 0.0_a);
         CHECK(node.transform.rotation.z == 0.0_a);

         REQUIRE(node.collision_primitive == std::nullopt);
      }

      // Node 1
      {
         const node& node = scene.nodes[1];

         REQUIRE(node.name == "sand"sv);
         REQUIRE(node.parent.value() == "root"sv);
         REQUIRE(node.type == node_type::static_mesh);
         REQUIRE(not node.hidden);

         CHECK(approx_equals(node.transform.translation, {-2.0f, 1.0f, 3.0f}));
         CHECK(node.transform.rotation.w == -0.878902792930603_a);
         CHECK(node.transform.rotation.x == 0.0112990252673626_a);
         CHECK(node.transform.rotation.y == -0.116856157779694_a);
         CHECK(node.transform.rotation.z == -0.462327867746353_a);

         REQUIRE(node.segments.size() == 1);

         // Segment 0
         {
            const geometry_segment& segment = node.segments[0];

            REQUIRE(segment.material_index == 0);
            REQUIRE(segment.normals != std::nullopt);
            REQUIRE(segment.texcoords != std::nullopt);
            REQUIRE(segment.colors == std::nullopt);

            REQUIRE(segment.positions.size() == 4);
            REQUIRE(segment.normals->size() == 4);
            REQUIRE(segment.texcoords->size() == 4);
            REQUIRE(segment.triangles.size() == 2);

            CHECK(approx_equals(segment.positions[0], {1.0f, 0.0f, -1.0f}));
            CHECK(approx_equals(segment.positions[1], {-1.0f, 0.0f, -1.0f}));
            CHECK(approx_equals(segment.positions[2], {-1.0f, 0.0f, 1.0f}));
            CHECK(approx_equals(segment.positions[3], {1.0f, 0.0f, 1.0f}));

            auto& normals = *segment.normals;
            CHECK(approx_equals(normals[0], {0.0f, 1.0f, 0.0f}));
            CHECK(approx_equals(normals[1], {0.0f, 1.0f, 0.0f}));
            CHECK(approx_equals(normals[2], {0.0f, 1.0f, 0.0f}));
            CHECK(approx_equals(normals[3], {0.0f, 1.0f, 0.0f}));

            auto& texcoords = *segment.texcoords;
            CHECK(approx_equals(texcoords[0], {0.0f, 1.0f}));
            CHECK(approx_equals(texcoords[1], {1.0f, 1.0f}));
            CHECK(approx_equals(texcoords[2], {1.0f, 0.0f}));
            CHECK(approx_equals(texcoords[3], {0.0f, 0.0f}));

            REQUIRE(segment.triangles.size() == 2);

            CHECK(segment.triangles[0] == std::array<uint16, 3>{0, 1, 2});
            CHECK(segment.triangles[1] == std::array<uint16, 3>{0, 2, 3});
         }

         REQUIRE(node.collision_primitive == std::nullopt);
      }

      // Node 2
      {
         const node& node = scene.nodes[2];

         REQUIRE(node.name == "p_sand_cube"sv);
         REQUIRE(node.parent.value() == "root"sv);
         REQUIRE(node.type == node_type::static_mesh);
         REQUIRE(node.hidden);

         CHECK(approx_equals(node.transform.translation, {0.0f, -0.0625f, 0.0f}));
         CHECK(node.transform.rotation.w == -0.985783636569977_a);
         CHECK(node.transform.rotation.x == 0.163417175412178_a);
         CHECK(node.transform.rotation.y == -0.0304092466831207_a);
         CHECK(node.transform.rotation.z == -0.0245117712765932_a);

         REQUIRE(node.segments.size() == 1);

         REQUIRE(node.collision_primitive);
         CHECK(node.collision_primitive->shape == collision_primitive_shape::box);
         CHECK(node.collision_primitive->radius == 1.0_a);
         CHECK(node.collision_primitive->height == 0.125_a);
         CHECK(node.collision_primitive->length == 1.0_a);
      }
   }

   // Option Checks
   {
      CHECK(not scene.options.additive_emissive);
      CHECK(not scene.options.vertex_lighting);
      CHECK(scene.options.normal_maps.empty());
      CHECK(scene.options.scale == 1.0f);
   }
}

TEST_CASE(".msh reading scale option", "[Assets][MSH]")
{
   auto scene = load_scene("data/sand_test_scale.msh", {});

   CHECK(scene.options.scale == 2.5f);

   // Sanity Checks (this is just a copy of sand_test.msh with an .option file added, so there is no point in checking it in depth).
   REQUIRE(scene.materials.size() == 1);
   REQUIRE(scene.nodes.size() == 3);
}

TEST_CASE(".msh reading ambientlighting option", "[Assets][MSH]")
{
   auto scene = load_scene("data/sand_test_ambientlighting.msh", {});

   CHECK(scene.options.ambient_lighting == float3{-1.0f, 0.5f, 1.0f});

   // Sanity Checks (this is just a copy of sand_test.msh with an .option file added, so there is no point in checking it in depth).
   REQUIRE(scene.materials.size() == 1);
   REQUIRE(scene.nodes.size() == 3);
}

TEST_CASE(".msh reading CLRB", "[Assets][MSH]")
{
   auto scene = load_scene("data/sand_test_clrb.msh", {});

   // Node Checks
   {
      REQUIRE(scene.nodes.size() == 3);

      // Node 0
      {
         const node& node = scene.nodes[0];

         REQUIRE(node.name == "root"sv);
         REQUIRE(node.parent == std::nullopt);
         REQUIRE(node.type == node_type::null);
         REQUIRE(node.hidden);

         CHECK(approx_equals(node.transform.translation, {0.0f, 0.0f, 0.0f}));
         CHECK(node.transform.rotation.w == -1.0_a);
         CHECK(node.transform.rotation.x == 0.0_a);
         CHECK(node.transform.rotation.y == 0.0_a);
         CHECK(node.transform.rotation.z == 0.0_a);

         REQUIRE(node.collision_primitive == std::nullopt);
      }

      // Node 1
      {
         const node& node = scene.nodes[1];

         REQUIRE(node.name == "sand"sv);
         REQUIRE(node.parent.value() == "root"sv);
         REQUIRE(node.type == node_type::static_mesh);
         REQUIRE(not node.hidden);

         CHECK(approx_equals(node.transform.translation, {-2.0f, 1.0f, 3.0f}));
         CHECK(node.transform.rotation.w == -0.878902792930603_a);
         CHECK(node.transform.rotation.x == 0.0112990252673626_a);
         CHECK(node.transform.rotation.y == -0.116856157779694_a);
         CHECK(node.transform.rotation.z == -0.462327867746353_a);

         REQUIRE(node.segments.size() == 1);

         // Segment 0
         {
            const geometry_segment& segment = node.segments[0];

            REQUIRE(segment.material_index == 0);
            REQUIRE(segment.normals != std::nullopt);
            REQUIRE(segment.texcoords != std::nullopt);
            REQUIRE(segment.colors != std::nullopt);

            REQUIRE(segment.positions.size() == 4);
            REQUIRE(segment.normals->size() == 4);
            REQUIRE(segment.texcoords->size() == 4);
            REQUIRE(segment.colors->size() == 4);
            REQUIRE(segment.triangles.size() == 2);

            CHECK(approx_equals(segment.positions[0], {1.0f, 0.0f, -1.0f}));
            CHECK(approx_equals(segment.positions[1], {-1.0f, 0.0f, -1.0f}));
            CHECK(approx_equals(segment.positions[2], {-1.0f, 0.0f, 1.0f}));
            CHECK(approx_equals(segment.positions[3], {1.0f, 0.0f, 1.0f}));

            auto& normals = *segment.normals;
            CHECK(approx_equals(normals[0], {0.0f, 1.0f, 0.0f}));
            CHECK(approx_equals(normals[1], {0.0f, 1.0f, 0.0f}));
            CHECK(approx_equals(normals[2], {0.0f, 1.0f, 0.0f}));
            CHECK(approx_equals(normals[3], {0.0f, 1.0f, 0.0f}));

            auto& texcoords = *segment.texcoords;
            CHECK(approx_equals(texcoords[0], {0.0f, 1.0f}));
            CHECK(approx_equals(texcoords[1], {1.0f, 1.0f}));
            CHECK(approx_equals(texcoords[2], {1.0f, 0.0f}));
            CHECK(approx_equals(texcoords[3], {0.0f, 0.0f}));

            auto& colors = *segment.colors;
            CHECK(colors[0] == 0x7f7f7f7f);
            CHECK(colors[1] == 0x7f7f7f7f);
            CHECK(colors[2] == 0x7f7f7f7f);
            CHECK(colors[3] == 0x7f7f7f7f);

            REQUIRE(segment.triangles.size() == 2);

            CHECK(segment.triangles[0] == std::array<uint16, 3>{0, 1, 2});
            CHECK(segment.triangles[1] == std::array<uint16, 3>{0, 2, 3});
         }

         REQUIRE(node.collision_primitive == std::nullopt);
      }
   }
}

TEST_CASE(".msh reading envl test", "[Assets][MSH]")
{
   auto scene = load_scene("data/test_envl.msh", {});

   REQUIRE(scene.nodes.size() == 5);

   CHECK(scene.nodes[0].name == "root");
   CHECK(scene.nodes[1].name == "bone_a");
   CHECK(scene.nodes[2].name == "bone_b");
   CHECK(scene.nodes[3].name == "bone_c");
   CHECK(scene.nodes[4].name == "tri");

   REQUIRE(scene.nodes[4].bone_map.size() == 3);
   CHECK(scene.nodes[4].bone_map[0] == 1);
   CHECK(scene.nodes[4].bone_map[1] == 2);
   CHECK(scene.nodes[4].bone_map[2] == 3);
}

TEST_CASE(".msh reading wght test", "[Assets][MSH]")
{
   auto scene = load_scene("data/test_wght.msh", {});

   REQUIRE(scene.nodes.size() == 5);

   CHECK(scene.nodes[0].name == "root");
   CHECK(scene.nodes[1].name == "bone_a");
   CHECK(scene.nodes[2].name == "bone_b");
   CHECK(scene.nodes[3].name == "bone_c");
   CHECK(scene.nodes[4].name == "tri");

   // Node 4
   {
      const node& node = scene.nodes[4];

      REQUIRE(node.bone_map.size() == 3);
      CHECK(node.bone_map[0] == 1);
      CHECK(node.bone_map[1] == 2);
      CHECK(node.bone_map[2] == 3);

      REQUIRE(node.segments.size() == 1);

      const geometry_segment& segment = node.segments[0];

      REQUIRE(segment.positions.size() == 3);
      CHECK(segment.positions[0] == float3{1.0f, 0.0f, 1.0f});
      CHECK(segment.positions[1] == float3{-1.0f, 0.0f, 1.0f});
      CHECK(segment.positions[2] == float3{1.0f, 0.0f, -1.0f});

      REQUIRE(segment.weights);
      REQUIRE(segment.weights->size() == 3);

      CHECK((*segment.weights)[0][0].bone_index == 0);
      CHECK((*segment.weights)[0][0].weight == 0.25f);
      CHECK((*segment.weights)[0][1].bone_index == 1);
      CHECK((*segment.weights)[0][1].weight == 0.5f);
      CHECK((*segment.weights)[0][2].bone_index == 2);
      CHECK((*segment.weights)[0][2].weight == 0.25f);
      CHECK((*segment.weights)[0][3].bone_index == 0);
      CHECK((*segment.weights)[0][3].weight == 0.0f);

      CHECK((*segment.weights)[1][0].bone_index == 1);
      CHECK((*segment.weights)[1][0].weight == 0.35f);
      CHECK((*segment.weights)[1][1].bone_index == 2);
      CHECK((*segment.weights)[1][1].weight == 0.15f);
      CHECK((*segment.weights)[1][2].bone_index == 0);
      CHECK((*segment.weights)[1][2].weight == 0.45f);
      CHECK((*segment.weights)[1][3].bone_index == 0);
      CHECK((*segment.weights)[1][3].weight == 0.0f);

      CHECK((*segment.weights)[2][0].bone_index == 2);
      CHECK((*segment.weights)[2][0].weight == 0.125f);
      CHECK((*segment.weights)[2][1].bone_index == 0);
      CHECK((*segment.weights)[2][1].weight == 0.125f);
      CHECK((*segment.weights)[2][2].bone_index == 1);
      CHECK((*segment.weights)[2][2].weight == 0.125f);
      CHECK((*segment.weights)[2][3].bone_index == 0);
      CHECK((*segment.weights)[2][3].weight == 0.0f);

      REQUIRE(segment.normals);
      REQUIRE(segment.normals->size() == 3);
      CHECK((*segment.normals)[0] == float3{0.0f, -1.0f, 0.0f});
      CHECK((*segment.normals)[1] == float3{0.0f, -1.0f, 0.0f});
      CHECK((*segment.normals)[2] == float3{0.0f, -1.0f, 0.0f});

      REQUIRE(segment.texcoords);
      REQUIRE(segment.texcoords->size() == 3);
      CHECK((*segment.texcoords)[0] == float2{0.0f, 1.0f});
      CHECK((*segment.texcoords)[1] == float2{1.0f, 1.0f});
      CHECK((*segment.texcoords)[2] == float2{1.0f, 0.0f});
   }
}

}
