#include "pch.h"

#include "assets/msh/merge_flat_models.hpp"

namespace we::assets::msh::tests {

TEST_CASE(".msh merge flat models geometry", "[Assets][MSH]")
{
   flat_model model{
      scene{.materials = {{
               .name = "snow",
               .specular_color = {0.75f, 0.75f, 0.75f},
               .flags = material_flags::specular,
               .rendertype = rendertype::normalmap,
               .textures = {"snow", "snow_normalmap"},
            }},

            .nodes = {
               {.name = "root",
                .transform =
                   {
                      .translation = {2.0f, 0.0f, 0.0f},
                      .rotation = {0.924886f, 0.0f, 0.0f, 0.380245f},
                   },
                .type = node_type::null},

               {.name = "geometry",
                .parent = "root",
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
                                                  {0.0f, 1.0f, 0.0f},
                                                  {0.0f, 1.0f, 0.0f}},
                   .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}},
                   .triangles = {{0, 1, 2}},
                }}},
            }}};

   std::array instances = {
      flat_model_instance{
         .transform = {.translation = {0.0f, 0.0f, 0.0f}},
         .flat_model = &model,
      },
      flat_model_instance{
         .transform = {.translation = {0.0f, 5.0f, 0.0f}},
         .flat_model = &model,
      },
   };

   flat_model merged_model = merge_flat_models(instances);

   REQUIRE(merged_model.meshes.size() == 1);

   const mesh& merged_mesh = merged_model.meshes[0];

   REQUIRE(merged_mesh.positions.size() == 6);

   CHECK(merged_mesh.positions[0] == float3{0.0f, 0.0f, 0.0f});
   CHECK(merged_mesh.positions[1] == float3{0.0f, 0.0f, 1.0f});
   CHECK(merged_mesh.positions[2] == float3{0.0f, 1.0f, 1.0f});
   CHECK(merged_mesh.positions[3] == float3{0.0f, 5.0f, 0.0f});
   CHECK(merged_mesh.positions[4] == float3{0.0f, 5.0f, 1.0f});
   CHECK(merged_mesh.positions[5] == float3{0.0f, 6.0f, 1.0f});

   REQUIRE(merged_mesh.normals.size() == 6);

   CHECK(merged_mesh.normals[0] == float3{0.0f, 1.0f, 0.0f});
   CHECK(merged_mesh.normals[1] == float3{0.0f, 1.0f, 0.0f});
   CHECK(merged_mesh.normals[2] == float3{0.0f, 1.0f, 0.0f});
   CHECK(merged_mesh.normals[3] == float3{0.0f, 1.0f, 0.0f});
   CHECK(merged_mesh.normals[4] == float3{0.0f, 1.0f, 0.0f});
   CHECK(merged_mesh.normals[5] == float3{0.0f, 1.0f, 0.0f});

   REQUIRE(merged_mesh.texcoords.size() == 6);

   CHECK(merged_mesh.texcoords[0] == float2{0.0f, 1.0f});
   CHECK(merged_mesh.texcoords[1] == float2{0.0f, 0.0f});
   CHECK(merged_mesh.texcoords[2] == float2{1.0f, 1.0f});
   CHECK(merged_mesh.texcoords[3] == float2{0.0f, 1.0f});
   CHECK(merged_mesh.texcoords[4] == float2{0.0f, 0.0f});
   CHECK(merged_mesh.texcoords[5] == float2{1.0f, 1.0f});

   REQUIRE(merged_mesh.triangles.size() == 2);

   CHECK(merged_mesh.triangles[0] == std::array<uint16, 3>{0, 1, 2});
   CHECK(merged_mesh.triangles[1] == std::array<uint16, 3>{3, 5, 6});

   CHECK(merged_mesh.material == material{
                                    .name = "snow",
                                    .specular_color = {0.75f, 0.75f, 0.75f},
                                    .flags = material_flags::specular,
                                    .rendertype = rendertype::normalmap,
                                    .textures = {"snow", "snow_normalmap"},
                                 });
}

}