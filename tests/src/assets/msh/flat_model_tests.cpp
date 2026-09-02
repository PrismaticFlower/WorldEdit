#include "pch.h"

#include "approx_test_helpers.hpp"
#include "assets/msh/flat_model.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <string_view>

#include <fmt/format.h>

using namespace std::literals;
using namespace Catch::literals;

namespace we::assets::msh::tests {

namespace {

const scene input_scene{
   .materials = {{
                    .name = "snow"s,
                    .specular_color = {0.75f, 0.75f, 0.75f, 1.0f},
                    .flags = material_flags::specular,
                    .rendertype = rendertype::normalmap,
                    .textures = {"snow"s, "snow_normalmap"s},
                 },

                 {
                    .name = "dirt"s,
                    .specular_color = {0.0f, 0.0f, 0.0f, 1.0f},
                    .flags = material_flags::none,
                    .rendertype = rendertype::normalmap,
                    .textures = {"dirt"s, "dirt_normalmap"s},
                 }},

   .nodes = {
      {.name = "root"s,
       .transform =
          {
             .translation = {0.0f, 0.0f, 0.0f},
             .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
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

      {.name = "terraincutter"s,
       .parent = std::nullopt,
       .transform =
          {
             .translation = {0.0f, 0.0f, 0.0f},
             .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
          },
       .type = node_type::static_mesh,
       .segments =
          {
             {
                .material_index = 0,
                .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
                .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f},
                                               {0.0f, 1.0f, 1.0f},
                                               {0.0f, 1.0f, 0.0f}},
                .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
                .triangles = {{0, 1, 2}, {1, 0, 2}},
             },
             {
                .material_index = 0,
                .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 2.0f}, {0.0f, 2.0f, 2.0f}},
                .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f},
                                               {0.0f, 1.0f, 1.0f},
                                               {0.0f, 1.0f, 0.0f}},
                .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
                .triangles = {{0, 1, 2}, {1, 0, 2}},
             },
          }},
   }};

auto make_huge_segment(const float3 base_position, uint32 count) -> geometry_segment
{
   if (count < 3) std::terminate();

   geometry_segment segment{.material_index = 0};

   segment.positions.resize(count);

   for (uint32 i = 0; i < count; ++i) {
      segment.positions[i] = base_position + float3{i * 1.0f, 0.0f, i * 1.0f};
   }

   segment.triangles.reserve(count);

   for (uint16 i = 2; i < count; ++i) {
      segment.triangles.push_back(
         {static_cast<uint16>(i - 2), static_cast<uint16>(i - 1), i});
   }

   return segment;
}

}

TEST_CASE(".msh flat model creation", "[Assets][MSH]")
{
   flat_model model{input_scene};

   REQUIRE(model.nodes.size() == 6);

   CHECK(model.nodes[0].name == "root");
   CHECK(model.nodes[0].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[0].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[0].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[0].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x0p-1022f, 0x1p+0f});

   CHECK(model.nodes[1].name == "null00");
   CHECK(model.nodes[1].local_from_vertex[0] ==
         float4{0x1.45bdcp-1f, 0x1.681f9cp-1f, 0x1.44a3bcp-2f, 0x0p-1022f});
   CHECK(model.nodes[1].local_from_vertex[1] ==
         float4{-0x1.425268p-1f, 0x1.6bf19cp-1f, -0x1.413b6cp-2f, 0x0p-1022f});
   CHECK(model.nodes[1].local_from_vertex[2] ==
         float4{-0x1.c8b4e6p-2f, 0x1.bffff6p-23f, 0x1.ca4196p-1f, 0x0p-1022f});
   CHECK(model.nodes[1].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x1.8p+1f, 0x0p-1022f, 0x1p+0f});

   CHECK(model.nodes[2].name == "geometry");
   CHECK(model.nodes[2].local_from_vertex[0] ==
         float4{0x1.45bdcp-1f, 0x1.681f9cp-1f, 0x1.44a3bcp-2f, 0x0p-1022f});
   CHECK(model.nodes[2].local_from_vertex[1] ==
         float4{-0x1.425268p-1f, 0x1.6bf19cp-1f, -0x1.413b6cp-2f, 0x0p-1022f});
   CHECK(model.nodes[2].local_from_vertex[2] ==
         float4{-0x1.c8b4e6p-2f, 0x1.bffff6p-23f, 0x1.ca4196p-1f, 0x0p-1022f});
   CHECK(model.nodes[2].local_from_vertex[3] ==
         float4{-0x1.c8b4e6p-2f, 0x1.800002p+1f, 0x1.ca4196p-1f, 0x1p+0f});

   CHECK(model.nodes[3].name == "p_box");
   CHECK(model.nodes[3].local_from_vertex[0] ==
         float4{0x1.eec0fp-1f, 0x1.0b2e7ep-3f, -0x1.c65518p-3f, 0x0p-1022f});
   CHECK(model.nodes[3].local_from_vertex[1] ==
         float4{-0x1.c65514p-3f, 0x1.b7e954p-1f, -0x1.d81ae4p-2f, 0x0p-1022f});
   CHECK(model.nodes[3].local_from_vertex[2] ==
         float4{0x1.0b2e8p-3f, 0x1.fa98fep-2f, 0x1.b7e954p-1f, 0x0p-1022f});
   CHECK(model.nodes[3].local_from_vertex[3] ==
         float4{0x1.45bdcp-1f, 0x1.da07e8p+1f, 0x1.44a3bcp-2f, 0x1p+0f});

   CHECK(model.nodes[4].name == "collision");
   CHECK(model.nodes[4].local_from_vertex[0] ==
         float4{0x1.45bdcp-1f, 0x1.681f9cp-1f, 0x1.44a3bcp-2f, 0x0p-1022f});
   CHECK(model.nodes[4].local_from_vertex[1] ==
         float4{-0x1.7e0f06p-1f, 0x1.54aba8p-1f, 0x1.5cb54p-6f, 0x0p-1022f});
   CHECK(model.nodes[4].local_from_vertex[2] ==
         float4{-0x1.915a84p-3f, -0x1.001d54p-2f, 0x1.e5775ep-1f, 0x0p-1022f});
   CHECK(model.nodes[4].local_from_vertex[3] ==
         float4{-0x1.c8b4e6p-3f, 0x1.8p+1f, 0x1.ca4196p-2f, 0x1p+0f});

   CHECK(model.nodes[5].name == "terraincutter");
   CHECK(model.nodes[5].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[5].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[5].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[5].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x0p-1022f, 0x1p+0f});

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

         CHECK(mesh.colors[0] == 0xffffffffu);
         CHECK(mesh.colors[1] == 0xffffffffu);
         CHECK(mesh.colors[2] == 0xffffffffu);

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

   // terrain cut checks
   {
      REQUIRE(model.terrain_cuts.size() == 1);

      REQUIRE(model.terrain_cuts[0].positions.size() == 6);
      CHECK(model.terrain_cuts[0].positions[0] == float3{0.0f, 0.0f, 0.0f});
      CHECK(model.terrain_cuts[0].positions[1] == float3{0.0f, 0.0f, 1.0f});
      CHECK(model.terrain_cuts[0].positions[2] == float3{0.0f, 1.0f, 1.0f});
      CHECK(model.terrain_cuts[0].positions[3] == float3{0.0f, 0.0f, 0.0f});
      CHECK(model.terrain_cuts[0].positions[4] == float3{0.0f, 0.0f, 2.0f});
      CHECK(model.terrain_cuts[0].positions[5] == float3{0.0f, 2.0f, 2.0f});

      REQUIRE(model.terrain_cuts[0].triangles.size() == 4);
      CHECK(model.terrain_cuts[0].triangles[0] == std::array<uint16, 3>{0, 1, 2});
      CHECK(model.terrain_cuts[0].triangles[1] == std::array<uint16, 3>{1, 0, 2});
      CHECK(model.terrain_cuts[0].triangles[2] == std::array<uint16, 3>{3, 4, 5});
      CHECK(model.terrain_cuts[0].triangles[3] == std::array<uint16, 3>{4, 3, 5});

      REQUIRE(model.terrain_cuts[0].planes.size() == 2);
      CHECK(model.terrain_cuts[0].planes[0] == float4{-1.0f, 0.0f, 0.0f, 0.0f});
      CHECK(model.terrain_cuts[0].planes[1] == float4{1.0f, 0.0f, 0.0f, -0.0f});
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
                               input_scene.nodes[2].transform.rotation *
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

TEST_CASE(".msh flat model creation with scale", "[Assets][MSH]")
{
   const scene input_scene{
      .materials = {{
         .name = "snow"s,
         .specular_color = {0.75f, 0.75f, 0.75f, 1.0f},
         .flags = material_flags::specular,
         .rendertype = rendertype::normalmap,
         .textures = {"snow"s, "snow_normalmap"s},
      }},

      .nodes =
         {
            {.name = "root"s,
             .transform =
                {
                   .translation = {0.0f, 0.0f, 0.0f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::null},

            {.name = "null00"s,
             .parent = "root"s,
             .transform =
                {
                   .translation = {0.0f, 3.0f, 0.0f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
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
             .segments = {{
                .material_index = 0,
                .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
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
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::null,
             .collision_primitive =
                collision_primitive{.shape = collision_primitive_shape::box,
                                    .radius = 1.0f,
                                    .height = 0.5f,
                                    .length = 2.5f}},

            {.name = "collision"s,
             .parent = "null00"s,
             .transform =
                {
                   .translation = {0.0f, 0.0f, 0.5f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::static_mesh,
             .segments =
                {
                   {
                      .material_index = 0,
                      .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
                      .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f},
                                                     {0.0f, 1.0f, 1.0f},
                                                     {0.0f, 1.0f, 0.0f}},
                      .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
                      .triangles = {{0, 1, 2}},
                   }}},

            {.name = "terraincutter"s,
             .parent = std::nullopt,
             .transform =
                {
                   .translation = {0.0f, 0.0f, 0.0f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::static_mesh,
             .segments =
                {
                   {
                      .material_index = 0,
                      .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
                      .normals =
                         std::vector<float3>{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
                      .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
                      .triangles = {{0, 1, 2}, {1, 0, 2}},
                   }}},
         },

      .options = {.scale = 2.5f},
   };

   flat_model model{input_scene};

   REQUIRE(model.nodes.size() == 6);

   CHECK(model.nodes[0].name == "root");
   CHECK(model.nodes[0].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[0].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[0].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[0].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x0p-1022f, 0x1p+0f});

   CHECK(model.nodes[1].name == "null00");
   CHECK(model.nodes[1].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[1].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[1].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[1].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x1.ep+2f, 0x0p-1022f, 0x1p+0f});

   CHECK(model.nodes[2].name == "geometry");
   CHECK(model.nodes[2].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[2].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[2].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[2].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x1.ep+2f, 0x1.4p+1f, 0x1p+0f});

   CHECK(model.nodes[3].name == "p_box");
   CHECK(model.nodes[3].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[3].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[3].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[3].local_from_vertex[3] ==
         float4{0x1.4p+1f, 0x1.ep+2f, 0x0p-1022f, 0x1p+0f});

   CHECK(model.nodes[4].name == "collision");
   CHECK(model.nodes[4].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[4].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[4].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[4].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x1.ep+2f, 0x1.4p+0f, 0x1p+0f});

   CHECK(model.nodes[5].name == "terraincutter");
   CHECK(model.nodes[5].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[5].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[5].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[5].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x0p-1022f, 0x1p+0f});

   // mesh checks
   {
      REQUIRE(model.meshes.size() == 1);

      const auto translate_position_from_root = [&](float3 position) {
         return position + input_scene.nodes[2].transform.translation * 2.5f +
                input_scene.nodes[1].transform.translation * 2.5f +
                input_scene.nodes[0].transform.translation * 2.5f;
      };

      const auto& mesh = model.meshes[0];
      const auto& segment = input_scene.nodes[2].segments[0];

      REQUIRE(mesh.material == input_scene.materials[segment.material_index]);
      REQUIRE(mesh.positions.size() == segment.positions.size());
      REQUIRE(mesh.normals.size() == segment.normals->size());
      REQUIRE(mesh.texcoords.size() == segment.texcoords->size());
      REQUIRE(mesh.triangles.size() == 1);

      CHECK(mesh.positions[0] == float3{0.0f, 7.5f, 2.5f});
      CHECK(mesh.positions[1] == float3{0.0f, 7.5f, 5.0f});
      CHECK(mesh.positions[2] == float3{0.0f, 10.0f, 5.0f});

      CHECK(mesh.triangles[0][0] == segment.triangles[0][0]);
      CHECK(mesh.triangles[0][1] == segment.triangles[0][1]);
      CHECK(mesh.triangles[0][2] == segment.triangles[0][2]);
   }

   // terrain cut checks
   {
      REQUIRE(model.terrain_cuts.size() == 1);

      REQUIRE(model.terrain_cuts[0].positions.size() == 3);
      CHECK(model.terrain_cuts[0].positions[0] == float3{0.0f, 0.0f, 0.0f} * 2.5f);
      CHECK(model.terrain_cuts[0].positions[1] == float3{0.0f, 0.0f, 1.0f} * 2.5f);
      CHECK(model.terrain_cuts[0].positions[2] == float3{0.0f, 1.0f, 1.0f} * 2.5f);

      REQUIRE(model.terrain_cuts[0].triangles.size() == 2);
      CHECK(model.terrain_cuts[0].triangles[0] == std::array<uint16, 3>{0, 1, 2});
      CHECK(model.terrain_cuts[0].triangles[1] == std::array<uint16, 3>{1, 0, 2});

      REQUIRE(model.terrain_cuts[0].planes.size() == 2);
      CHECK(model.terrain_cuts[0].planes[0] == float4{-1.0f, 0.0f, 0.0f, -0.0f});
      CHECK(model.terrain_cuts[0].planes[1] == float4{1.0f, 0.0f, 0.0f, -0.0f});
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

         CHECK(primitive.radius == 2.5f);
         CHECK(primitive.height == 1.25f);
         CHECK(primitive.length == 6.25f);

         const auto rotation = input_scene.nodes[0].transform.rotation *
                               input_scene.nodes[1].transform.rotation *
                               input_scene.nodes[3].transform.rotation;
         const auto position = input_scene.nodes[3].transform.translation * 2.5f +
                               input_scene.nodes[1].transform.translation * 2.5f +
                               input_scene.nodes[0].transform.translation * 2.5f;

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
            return position + input_scene.nodes[4].transform.translation * 2.5f +
                   input_scene.nodes[1].transform.translation * 2.5f +
                   input_scene.nodes[0].transform.translation * 2.5f;
         };

         CHECK(mesh.positions[0] == float3{0.0f, 7.5f, 1.25f});
         CHECK(mesh.positions[1] == float3{0.0f, 7.5f, 3.75f});
         CHECK(mesh.positions[2] == float3{0.0f, 10.0f, 3.75f});
      }
   }
}

TEST_CASE(".msh flat model creation root transform skip", "[Assets][MSH]")
{
   const scene input_scene{
      .materials = {{
         .name = "snow"s,
         .specular_color = {0.75f, 0.75f, 0.75f, 1.0f},
         .flags = material_flags::specular,
         .rendertype = rendertype::normalmap,
         .textures = {"snow"s, "snow_normalmap"s},
      }},

      .nodes =
         {
            {.name = "root"s,
             .transform =
                {
                   .translation = {0.0f, 5.0f, 0.0f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::null},

            {.name = "geometry"s,
             .parent = "root"s,
             .transform =
                {
                   .translation = {0.0f, 0.0f, 0.0f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::static_mesh,
             .segments = {{
                .material_index = 0,
                .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
                .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f},
                                               {0.0f, 1.0f, 1.0f},
                                               {0.0f, 1.0f, 0.0f}},
                .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
                .triangles = {{0, 1, 2}},
             }}},
         },
   };

   flat_model model{input_scene};

   REQUIRE(model.nodes.size() == 2);

   CHECK(model.nodes[0].name == "root");
   CHECK(model.nodes[0].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[0].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[0].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[0].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x0p-1022f, 0x1p+0f});

   CHECK(model.nodes[1].name == "geometry");
   CHECK(model.nodes[1].local_from_vertex[0] ==
         float4{0x1p+0f, 0x0p-1022f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[1].local_from_vertex[1] ==
         float4{0x0p-1022f, 0x1p+0f, 0x0p-1022f, 0x0p-1022f});
   CHECK(model.nodes[1].local_from_vertex[2] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x1p+0f, 0x0p-1022f});
   CHECK(model.nodes[1].local_from_vertex[3] ==
         float4{0x0p-1022f, 0x0p-1022f, 0x0p-1022f, 0x1p+0f});

   // mesh checks
   {
      REQUIRE(model.meshes.size() == 1);

      const auto& mesh = model.meshes[0];
      const auto& segment = input_scene.nodes[1].segments[0];

      REQUIRE(mesh.material == input_scene.materials[segment.material_index]);
      REQUIRE(mesh.positions.size() == segment.positions.size());
      REQUIRE(mesh.normals.size() == segment.normals->size());
      REQUIRE(mesh.texcoords.size() == segment.texcoords->size());
      REQUIRE(mesh.triangles.size() == 1);

      CHECK(approx_equals(mesh.positions[0], segment.positions[0]));
      CHECK(approx_equals(mesh.positions[1], segment.positions[1]));
      CHECK(approx_equals(mesh.positions[2], segment.positions[2]));

      CHECK(mesh.triangles[0][0] == segment.triangles[0][0]);
      CHECK(mesh.triangles[0][1] == segment.triangles[0][1]);
      CHECK(mesh.triangles[0][2] == segment.triangles[0][2]);
   }
}

TEST_CASE(".msh flat model creation with ambient lighting", "[Assets][MSH]")
{
   const scene input_scene{
      .materials = {{
         .name = "snow"s,
         .specular_color = {0.75f, 0.75f, 0.75f, 1.0f},
         .flags = material_flags::specular,
         .rendertype = rendertype::normalmap,
         .textures = {"snow"s, "snow_normalmap"s},
      }},

      .nodes =
         {
            {.name = "root"s,
             .transform =
                {
                   .translation = {0.0f, 0.0f, 0.0f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::null},

            {.name = "geometry"s,
             .parent = "root"s,
             .transform =
                {
                   .translation = {0.0f, 0.0f, 0.0f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::static_mesh,
             .segments = {{
                .material_index = 0,
                .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
                .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f},
                                               {0.0f, 1.0f, 1.0f},
                                               {0.0f, 1.0f, 0.0f}},
                .colors = std::vector<uint32>{0xff'ff'ff'ffu, 0xff'ff'ff'ffu, 0xff'ff'ff'ffu},
                .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
                .triangles = {{0, 1, 2}},
             }}},
         },

      .options = {.vertex_lighting = true,
                  .ambient_lighting = float3{-0.25f, -0.5f, -1.0f}},
   };

   flat_model model{input_scene};

   // mesh checks
   {
      REQUIRE(model.meshes.size() == 1);

      const auto& mesh = model.meshes[0];
      const auto& segment = input_scene.nodes[1].segments[0];

      REQUIRE(mesh.material == input_scene.materials[segment.material_index]);
      REQUIRE(mesh.positions.size() == segment.positions.size());
      REQUIRE(mesh.normals.size() == segment.normals->size());
      REQUIRE(mesh.colors.size() == segment.colors->size());
      REQUIRE(mesh.texcoords.size() == segment.texcoords->size());
      REQUIRE(mesh.triangles.size() == 1);

      CHECK(mesh.colors[0] == 0xff'bf'80'00u);
      CHECK(mesh.colors[1] == 0xff'bf'80'00u);
      CHECK(mesh.colors[2] == 0xff'bf'80'00u);
   }
}

TEST_CASE(".msh flat model creation ground points", "[Assets][MSH]")
{
   const scene input_scene{
      .materials = {{
         .name = "snow"s,
         .specular_color = {0.75f, 0.75f, 0.75f, 1.0f},
         .flags = material_flags::specular,
         .rendertype = rendertype::normalmap,
         .textures = {"snow"s, "snow_normalmap"s},
      }},

      .nodes =
         {
            {.name = "root"s,
             .transform =
                {
                   .translation = {0.0f, 0.0f, 0.0f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::null},

            {.name = "geometry"s,
             .parent = "root"s,
             .transform =
                {
                   .translation = {0.0f, 0.0f, 0.0f},
                   .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                },
             .type = node_type::static_mesh,
             .segments = {{
                .material_index = 0,
                .positions = {{0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
                .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f},
                                               {0.0f, 1.0f, 1.0f},
                                               {0.0f, 1.0f, 0.0f}},
                .colors = std::vector<uint32>{0xff'ff'ff'ffu, 0xff'ff'ff'ffu, 0xff'ff'ff'ffu},
                .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
                .triangles = {{0, 1, 2}},
             }}},
         },

      .options = {.vertex_lighting = true,
                  .ambient_lighting = float3{-0.25f, -0.5f, -1.0f}},
   };

   flat_model model{input_scene};

   REQUIRE(model.ground_points.size() == 1);
   CHECK(model.ground_points[0] == float3{0.0f, -1.0f, 0.0f});
}

TEST_CASE(".msh flat excessive vertices test", "[Assets][MSH]")
{
   scene input_scene{
      .materials = {{
                       .name = "snow"s,
                       .specular_color = {0.75f, 0.75f, 0.75f, 1.0f},
                       .flags = material_flags::specular,
                       .rendertype = rendertype::normalmap,
                       .textures = {"snow"s, "snow_normalmap"s},
                    },

                    {
                       .name = "dirt"s,
                       .specular_color = {0.0f, 0.0f, 0.0f, 1.0f},
                       .flags = material_flags::none,
                       .rendertype = rendertype::normalmap,
                       .textures = {"dirt"s, "dirt_normalmap"s},
                    }},

      .nodes = {
         {.name = "geometry"s,
          .transform =
             {
                .translation = {0.0f, 0.0f, 1.0f},
                .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
             },
          .type = node_type::static_mesh,
          .segments = {make_huge_segment(float3{0.0f, 0.0f, 0.0f},
                                         geometry_segment::max_vertex_count),
                       make_huge_segment(float3{0.0f, 1.0f, 0.0f},
                                         geometry_segment::max_vertex_count),
                       make_huge_segment(float3{0.0f, 2.0f, 0.0f},
                                         geometry_segment::max_vertex_count)}},
      }};

   flat_model model{input_scene};

   REQUIRE(model.meshes.size() == 3);
}

TEST_CASE(".msh flat excessive terrain cut test", "[Assets][MSH]")
{
   scene input_scene{
      .materials = {{
                       .name = "snow"s,
                       .specular_color = {0.75f, 0.75f, 0.75f, 1.0f},
                       .flags = material_flags::specular,
                       .rendertype = rendertype::normalmap,
                       .textures = {"snow"s, "snow_normalmap"s},
                    },

                    {
                       .name = "dirt"s,
                       .specular_color = {0.0f, 0.0f, 0.0f, 1.0f},
                       .flags = material_flags::none,
                       .rendertype = rendertype::normalmap,
                       .textures = {"dirt"s, "dirt_normalmap"s},
                    }},

      .nodes = {
         {.name = "terraincutter"s,
          .transform =
             {
                .translation = {0.0f, 0.0f, 1.0f},
                .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
             },
          .type = node_type::static_mesh,
          .segments = {make_huge_segment(float3{0.0f, 0.0f, 0.0f},
                                         geometry_segment::max_vertex_count),
                       make_huge_segment(float3{0.0f, 1.0f, 0.0f},
                                         geometry_segment::max_vertex_count),
                       make_huge_segment(float3{0.0f, 2.0f, 0.0f},
                                         geometry_segment::max_vertex_count)}},
      }};

   REQUIRE_THROWS(flat_model{input_scene});
}

}
