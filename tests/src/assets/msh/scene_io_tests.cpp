
#include "pch.h"

#include "approx_test_helpers.hpp"
#include "assets/msh/scene_io.hpp"
#include "utility/read_file.hpp"

#include <string_view>

using namespace std::literals;
using namespace Catch::literals;

namespace sk::assets::msh::tests {

TEST_CASE(".msh reading", "[Assets][MSH]")
{
   auto scene =
      read_scene_from_bytes(utility::read_file_to_bytes("data/sand_test.msh"));

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
      CHECK(material.textures[0] == "sand.tga"sv);
      CHECK(material.textures[1] == "sand_normalmap.tga"sv);
      CHECK(material.textures[2] == "sand_detail.tga"sv);
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
}

}
