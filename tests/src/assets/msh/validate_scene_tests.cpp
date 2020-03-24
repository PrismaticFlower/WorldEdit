#include "pch.h"

#include "assets/msh/validate_scene.hpp"

#include <string_view>

using namespace std::literals;
using namespace Catch::literals;

namespace sk::assets::msh::tests {

namespace {

const scene input_scene{
   .materials = {{
                    .name = "snow"s,
                    .specular_color = {0.75f, 0.75f, 0.75f},
                    .flags = material_flags::specular,
                    .rendertype = rendertype::normalmap,
                    .textures = {"snow.png"s, "snow_normalmap.png"s},
                 },

                 {
                    .name = "dirt"s,
                    .specular_color = {0.0f, 0.0f, 0.0f},
                    .flags = material_flags::none,
                    .rendertype = rendertype::normalmap,
                    .textures = {"dirt.png"s, "dirt_normalmap.png"s},
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

TEST_CASE(".msh scene validation name uniqueness",
          "[Assets][MSH][SceneValidation]")
{
   const scene bad_scene{.nodes = {{.name = "node"s}, {.name = "node"s}}};

   REQUIRE_THROWS(validate_scene(bad_scene));
}

TEST_CASE(".msh scene validation node type validity",
          "[Assets][MSH][SceneValidation]")
{
   const scene bad_scene{.nodes = {{.name = "node"s, .type = node_type{-1}}}};

   REQUIRE_THROWS(validate_scene(bad_scene));
}

TEST_CASE(".msh scene validation parents validity",
          "[Assets][MSH][SceneValidation]")
{
   const scene bad_scene{
      .nodes = {{.name = "parent"s}, {.name = "child"s, .parent = "root"s}}};

   REQUIRE_THROWS(validate_scene(bad_scene));
}

TEST_CASE(".msh scene validation parents noncircular",
          "[Assets][MSH][SceneValidation]")
{
   const scene bad_scene{.nodes = {{.name = "node_0"s, .parent = "node_1"s},
                                   {.name = "node_1"s, .parent = "node_0"s}}};
   const scene bad_scene_self_parent{.nodes = {{.name = "node"s, .parent = "node"s}}};

   REQUIRE_THROWS(validate_scene(bad_scene));
   REQUIRE_THROWS(validate_scene(bad_scene_self_parent));
}

TEST_CASE(".msh geometry segment material index validity",
          "[Assets][MSH][SceneValidation]")
{
   const scene bad_scene{.materials = {{.name = "snow"s}, {.name = "dirt"s}},
                         .nodes = {{.name = "node"s,
                                    .segments = {{
                                       .material_index = 4,
                                       .positions = {{0.0f, 0.0f, 0.0f},
                                                     {0.0f, 0.0f, 1.0f},
                                                     {0.0f, 1.0f, 1.0f}},
                                       .triangles = {{0, 1, 2}},
                                    }}}}};
   const scene bad_scene_negative{.materials = {{.name = "snow"s}, {.name = "dirt"s}},
                                  .nodes = {{.name = "node"s,
                                             .segments = {{
                                                .material_index = -1,
                                                .positions = {{0.0f, 0.0f, 0.0f},
                                                              {0.0f, 0.0f, 1.0f},
                                                              {0.0f, 1.0f, 1.0f}},
                                                .triangles = {{0, 1, 2}},
                                             }}}}};

   REQUIRE_THROWS(validate_scene(bad_scene));
   REQUIRE_THROWS(validate_scene(bad_scene_negative));
}

TEST_CASE(".msh geometry segment attibutes count matches",
          "[Assets][MSH][SceneValidation]")
{
   const scene bad_scene{
      .materials = {{.name = "snow"s}},
      .nodes = {
         {.name = "node"s,
          .segments = {{
             .material_index = 0,
             .positions = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}}, // 3 positions
             .normals = std::vector<float3>{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}}, // 2 normals
             .texcoords = std::vector<float2>{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}}, // 3 texcoords
             .triangles = {{0, 1, 2}},
          }}}}};

   REQUIRE_THROWS(validate_scene(bad_scene));
}

TEST_CASE(".msh geometry segment vertex count limit",
          "[Assets][MSH][SceneValidation]")
{
   scene bad_scene{.materials = {{.name = "snow"s}},
                   .nodes = {{.name = "node"s,
                              .segments = {{
                                 .material_index = 0,
                                 .triangles = {{0, 1, 2}},
                              }}}}};

   bad_scene.nodes[0].segments[0].positions.resize(
      geometry_segment::max_vertex_count + 1);

   REQUIRE_THROWS(validate_scene(bad_scene));
}

TEST_CASE(".msh geometry segment triangles index validity",
          "[Assets][MSH][SceneValidation]")
{
   scene bad_scene{.materials = {{.name = "snow"s}},
                   .nodes = {{.name = "node"s,
                              .segments = {{
                                 .material_index = 0,
                                 .positions = {{0.0f, 0.0f, 0.0f},
                                               {0.0f, 0.0f, 1.0f},
                                               {0.0f, 1.0f, 1.0f}},
                                 .triangles = {{
                                    0, // valid
                                    1, // valid
                                    5  // out of range
                                 }},
                              }}}}};

   REQUIRE_THROWS(validate_scene(bad_scene));
}

TEST_CASE(".msh scene primitive shape validity",
          "[Assets][MSH][SceneValidation]")
{
   const scene bad_scene{.nodes = {{.name = "p_node"s,
                                    .collision_primitive = collision_primitive{
                                       .shape = collision_primitive_shape{5}}}}};

   REQUIRE_THROWS(validate_scene(bad_scene));
}

}