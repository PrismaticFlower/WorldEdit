#include "pch.h"

#include "approx_test_helpers.hpp"
#include "assets/msh/flat_model.hpp"

#include <string_view>

using namespace std::literals;
using namespace Catch::literals;

namespace we::assets::msh::tests {

namespace {

const scene input_scene{
   .materials = {{
                    .name = "snow"s,
                    .specular_color = {0.75f, 0.75f, 0.75f},
                    .flags = material_flags::specular,
                    .rendertype = rendertype::normalmap,
                    .textures = {"snow"s, "snow_normalmap"s},
                 },

                 {
                    .name = "dirt"s,
                    .specular_color = {0.0f, 0.0f, 0.0f},
                    .flags = material_flags::none,
                    .rendertype = rendertype::normalmap,
                    .textures = {"dirt"s, "dirt_normalmap"s},
                 }},

   .nodes = {
      {.name = "root"s,
       .transform =
          {
             .translation = {2.0f, 0.0f, 0.0f},
             .rotation = {0.924886f, 0.0f, 0.0f, 0.380245f},
          },
       .type = node_type::null},

      {.name = "null00"s,
       .parent = "root"s,
       .transform =
          {
             .translation = {0.0f, 3.0f, 0.0f},
             .rotation = {0.900288f, -0.087112f, -0.211886f, 0.370132f},
          },
       .type = node_type::null},

      {.name = "geometry"s,
       .parent = "null00"s,
       .transform =
          {
             .translation = {0.0f, 0.0f, 1.0f},
             .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
          },
       .type = node_type::static_mesh,
       .segments =
          {{
              .material_index = 0,
              .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
              .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f},
                                             {0.0f, 1.0f, 1.0f},
                                             {0.0f, 1.0f, 0.0f}},
              .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
              .triangles = {{0, 1, 2}},
           },

           {
              .material_index = 1,
              .positions = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
              .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f},
                                             {0.0f, 1.0f, 1.0f},
                                             {0.0f, 1.0f, 0.0f}},
              .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
              .triangles = {{0, 1, 2}},
           }}},

      {.name = "p_box"s,
       .parent = "null00"s,
       .transform =
          {
             .translation = {1.0f, 0.0f, 0.0f},
             .rotation = {0.900288f, -0.087112f, 0.370132f, -0.211886f},
          },
       .type = node_type::null,
       .collision_primitive = collision_primitive{.shape = collision_primitive_shape::box,
                                                  .radius = 1.0f,
                                                  .height = 0.5f,
                                                  .length = 2.5f}},

      {.name = "collision"s,
       .parent = "null00"s,
       .transform =
          {
             .translation = {0.0f, 0.0f, 0.5f},
             .rotation = {0.983883f, 0.178812f, 0.0f, 0.0f},
          },
       .type = node_type::static_mesh,
       .segments = {{
          .material_index = 0,
          .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
          .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
          .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
          .triangles = {{0, 1, 2}},
       }}},
   }};
}

TEST_CASE(".msh flat model creation", "[Assets][MSH]")
{
   flat_model model{input_scene};

   // hierarchy checks
   {
      REQUIRE(model.node_hierarchy.size() == 1);

      // root
      {
         auto& root = model.node_hierarchy[0];

         CHECK(root.name == input_scene.nodes[0].name);
         CHECK(approx_equals(root.transform.translation,
                             input_scene.nodes[0].transform.translation));
         CHECK(approx_equals(root.transform.rotation,
                             input_scene.nodes[0].transform.rotation));
         CHECK(root.type == input_scene.nodes[0].type);
         CHECK(root.hidden == input_scene.nodes[0].hidden);

         REQUIRE(root.children.size() == 1);

         // null00
         {
            auto& null00 = root.children[0];

            CHECK(null00.name == input_scene.nodes[1].name);
            CHECK(approx_equals(null00.transform.translation,
                                input_scene.nodes[1].transform.translation));
            CHECK(approx_equals(null00.transform.rotation,
                                input_scene.nodes[1].transform.rotation));
            CHECK(null00.type == input_scene.nodes[1].type);
            CHECK(null00.hidden == input_scene.nodes[1].hidden);

            REQUIRE(null00.children.size() == 3);

            // geometry
            {
               auto& geometry = null00.children[0];

               CHECK(geometry.name == input_scene.nodes[2].name);
               CHECK(approx_equals(geometry.transform.translation,
                                   input_scene.nodes[2].transform.translation));
               CHECK(approx_equals(geometry.transform.rotation,
                                   input_scene.nodes[2].transform.rotation));
               CHECK(geometry.type == input_scene.nodes[2].type);
               CHECK(geometry.hidden == input_scene.nodes[2].hidden);
            }

            // p_box
            {
               auto& p_box = null00.children[1];

               CHECK(p_box.name == input_scene.nodes[3].name);
               CHECK(approx_equals(p_box.transform.translation,
                                   input_scene.nodes[3].transform.translation));
               CHECK(approx_equals(p_box.transform.rotation,
                                   input_scene.nodes[3].transform.rotation));
               CHECK(p_box.type == input_scene.nodes[3].type);
               CHECK(p_box.hidden == input_scene.nodes[3].hidden);
            }

            // collision
            {
               auto& collision = null00.children[2];

               CHECK(collision.name == input_scene.nodes[4].name);
               CHECK(approx_equals(collision.transform.translation,
                                   input_scene.nodes[4].transform.translation));
               CHECK(approx_equals(collision.transform.rotation,
                                   input_scene.nodes[4].transform.rotation));
               CHECK(collision.type == input_scene.nodes[4].type);
               CHECK(collision.hidden == input_scene.nodes[4].hidden);
            }
         }
      }
   }

   // mesh checks
   {
      REQUIRE(model.meshes.size() == 2);

      const auto transform_position_from_root = [&](float3 position) {
         return input_scene.nodes[0].transform.rotation *
                   (input_scene.nodes[1].transform.rotation *
                       (input_scene.nodes[2].transform.rotation * position +
                        input_scene.nodes[2].transform.translation) +
                    input_scene.nodes[1].transform.translation) +
                input_scene.nodes[0].transform.translation;
      };

      const auto transform_normal_from_root = [&](float3 normal) {
         return input_scene.nodes[0].transform.rotation *
                (input_scene.nodes[1].transform.rotation *
                 (input_scene.nodes[2].transform.rotation * normal));
      };

      const auto check_mesh = [&](auto& mesh, auto& segment) {
         REQUIRE(mesh.material == input_scene.materials[segment.material_index]);
         REQUIRE(mesh.positions.size() == segment.positions.size());
         REQUIRE(mesh.normals.size() == segment.normals->size());
         REQUIRE(mesh.texcoords.size() == segment.texcoords->size());
         REQUIRE(mesh.triangles.size() == 1);

         CHECK(approx_equals(mesh.positions[0],
                             transform_position_from_root(segment.positions[0])));
         CHECK(approx_equals(mesh.positions[1],
                             transform_position_from_root(segment.positions[1])));
         CHECK(approx_equals(mesh.positions[2],
                             transform_position_from_root(segment.positions[2])));

         auto& segment_normals = *segment.normals;
         CHECK(approx_equals(mesh.normals[0],
                             transform_normal_from_root(segment_normals[0])));
         CHECK(approx_equals(mesh.normals[1],
                             transform_normal_from_root(segment_normals[1])));
         CHECK(approx_equals(mesh.normals[2],
                             transform_normal_from_root(segment_normals[2])));

         CHECK(approx_equals(mesh.colors[0], {1.0f, 1.0f, 1.0f, 1.0f}));
         CHECK(approx_equals(mesh.colors[1], {1.0f, 1.0f, 1.0f, 1.0f}));
         CHECK(approx_equals(mesh.colors[2], {1.0f, 1.0f, 1.0f, 1.0f}));

         auto& segment_texcoords = *segment.texcoords;
         CHECK(approx_equals(mesh.texcoords[0], segment_texcoords[0]));
         CHECK(approx_equals(mesh.texcoords[1], segment_texcoords[1]));
         CHECK(approx_equals(mesh.texcoords[2], segment_texcoords[2]));

         CHECK(mesh.triangles[0][0] == segment.triangles[0][0]);
         CHECK(mesh.triangles[0][1] == segment.triangles[0][1]);
         CHECK(mesh.triangles[0][2] == segment.triangles[0][2]);
      };

      // snow mesh
      check_mesh(model.meshes[0], input_scene.nodes[2].segments[0]);

      // dirt mesh
      check_mesh(model.meshes[1], input_scene.nodes[2].segments[1]);
   }

   // collision checks
   {
      REQUIRE(model.collision.size() == 2);

      // primitive
      {
         REQUIRE(std::holds_alternative<flat_model_collision::primitive>(
            model.collision[0].geometry));

         auto& primitive =
            std::get<flat_model_collision::primitive>(model.collision[0].geometry);

         CHECK(primitive.radius ==
               Approx(input_scene.nodes[3].collision_primitive->radius));
         CHECK(primitive.height ==
               Approx(input_scene.nodes[3].collision_primitive->height));
         CHECK(primitive.length ==
               Approx(input_scene.nodes[3].collision_primitive->length));

         const auto rotation = input_scene.nodes[0].transform.rotation *
                               input_scene.nodes[1].transform.rotation *
                               input_scene.nodes[3].transform.rotation;
         const auto position = input_scene.nodes[0].transform.rotation *
                                  (input_scene.nodes[1].transform.rotation *
                                      input_scene.nodes[3].transform.translation +
                                   input_scene.nodes[1].transform.translation) +
                               input_scene.nodes[0].transform.translation;

         CHECK(approx_equals(primitive.transform.rotation, rotation));
         CHECK(approx_equals(primitive.transform.translation, position));
      }

      // mesh
      {
         REQUIRE(std::holds_alternative<flat_model_collision::mesh>(
            model.collision[1].geometry));

         auto& mesh =
            std::get<flat_model_collision::mesh>(model.collision[1].geometry);

         CHECK(mesh.triangles == input_scene.nodes[4].segments[0].triangles);

         const auto transform_position_from_root = [&](float3 position) {
            return input_scene.nodes[0].transform.rotation *
                      (input_scene.nodes[1].transform.rotation *
                          (input_scene.nodes[4].transform.rotation * position +
                           input_scene.nodes[4].transform.translation) +
                       input_scene.nodes[1].transform.translation) +
                   input_scene.nodes[0].transform.translation;
         };

         auto& segment = input_scene.nodes[4].segments[0];

         CHECK(approx_equals(mesh.positions[0],
                             transform_position_from_root(segment.positions[0])));
         CHECK(approx_equals(mesh.positions[1],
                             transform_position_from_root(segment.positions[1])));
         CHECK(approx_equals(mesh.positions[2],
                             transform_position_from_root(segment.positions[2])));
      }
   }
}

}
