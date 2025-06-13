#include "pch.h"

#include "world/blocks/mesh_generate.hpp"

namespace we::world::tests {

// Reference tests for block mesh generators. The code in the #if block below can be used to generate the contents of the
// test after you've validated the mesh generator changes.

#if 0
world::block_custom_mesh mesh = world::generate_mesh(description);

std::string output;

output +=
   fmt::format("REQUIRE(mesh.vertices.size() == {});\n\n", mesh.vertices.size());

for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
   output += fmt::format("CHECK(mesh.vertices[{}].position == "
                         "float3({:a}f, {:a}f, {:a}f));\n",
                         i, mesh.vertices[i].position.x,
                         mesh.vertices[i].position.y, mesh.vertices[i].position.z);
   output += fmt::format("CHECK(mesh.vertices[{}].normal == float3({:a}f, "
                         "{:a}f, {:a}f));\n",
                         i, mesh.vertices[i].normal.x,
                         mesh.vertices[i].normal.y, mesh.vertices[i].normal.z);
   output += fmt::format(
      "CHECK(mesh.vertices[{}].texcoords == float2({:a}f, {:a}f));\n", i,
      mesh.vertices[i].texcoords.x, mesh.vertices[i].texcoords.y);
   output += fmt::format("CHECK(mesh.vertices[{}].surface_index == {});\n", i,
                         mesh.vertices[i].surface_index);
}

output += fmt::format("\nREQUIRE(mesh.triangles.size() == {});\n\n",
                      mesh.triangles.size());

for (std::size_t i = 0; i < mesh.triangles.size(); ++i) {
   output += fmt::format("CHECK(mesh.triangles[{}] == std::array<uint16, "
                         "3>{{{}, {}, {}}});\n",
                         i, mesh.triangles[i][0], mesh.triangles[i][1],
                         mesh.triangles[i][2]);
}

output += fmt::format("\nREQUIRE(mesh.occluders.size() == {});\n\n",
                      mesh.occluders.size());

for (std::size_t i = 0; i < mesh.occluders.size(); ++i) {
   output += fmt::format("CHECK(mesh.occluders[{}] == std::array<uint16, "
                         "4>{{{}, {}, {}, {}}});\n",
                         i, mesh.occluders[i][0], mesh.occluders[i][1],
                         mesh.occluders[i][2], mesh.occluders[i][3]);
}

output += fmt::format("\nREQUIRE(mesh.collision_vertices.size() == {});\n\n",
                      mesh.collision_vertices.size());

for (std::size_t i = 0; i < mesh.collision_vertices.size(); ++i) {
   output +=
      fmt::format("CHECK(mesh.collision_vertices[{}].position == "
                  "float3({:a}f, {:a}f, {:a}f));\n",
                  i, mesh.collision_vertices[i].position.x, mesh.collision_vertices[i].position.y,
                  mesh.collision_vertices[i].position.z);
   output +=
      fmt::format("CHECK(mesh.collision_vertices[{}].surface_index == {});\n",
                  i, mesh.collision_vertices[i].surface_index);
}

output += fmt::format("\nREQUIRE(mesh.collision_triangles.size() == {});\n\n",
                      mesh.collision_triangles.size());

for (std::size_t i = 0; i < mesh.collision_triangles.size(); ++i) {
   output +=
      fmt::format("CHECK(mesh.collision_triangles[{}] == std::array<uint16, "
                  "3>{{{}, {}, {}}});\n",
                  i, mesh.collision_triangles[i][0],
                  mesh.collision_triangles[i][1], mesh.collision_triangles[i][2]);
}

output += fmt::format("\nREQUIRE(mesh.collision_occluders.size() == {});\n\n",
                      mesh.collision_occluders.size());

for (std::size_t i = 0; i < mesh.collision_occluders.size(); ++i) {
   output +=
      fmt::format("CHECK(mesh.collision_occluders[{}] == std::array<uint16, "
                  "4>{{{}, {}, {}, {}}});\n",
                  i, mesh.collision_occluders[i][0], mesh.collision_occluders[i][1],
                  mesh.collision_occluders[i][2], mesh.collision_occluders[i][3]);
}

output += fmt::format("\nREQUIRE(mesh.snap_points.size() == {});\n\n",
                      mesh.snap_points.size());

for (std::size_t i = 0; i < mesh.snap_points.size(); ++i) {
   output += fmt::format("CHECK(mesh.snap_points[{}] == "
                         "float3({:a}f, {:a}f, {:a}f));\n",
                         i, mesh.snap_points[i].x, mesh.snap_points[i].y,
                         mesh.snap_points[i].z);
}

output += fmt::format("\nREQUIRE(mesh.snap_edges.size() == {});\n\n",
                      mesh.snap_edges.size());

for (std::size_t i = 0; i < mesh.snap_edges.size(); ++i) {
   output += fmt::format("CHECK(mesh.snap_edges[{}] == std::array<uint16, "
                         "2>{{{}, {}}});\n",
                         i, mesh.snap_edges[i][0], mesh.snap_edges[i][1]);
}
#endif

TEST_CASE("world blocks generate_mesh stairway", "[World][Blocks]")
{
   block_custom_mesh mesh = world::generate_mesh(
      block_custom_mesh_description_stairway{.size = {10.0f, 1.0f, 10.0f},
                                             .step_height = 0.25f,
                                             .first_step_offset = -0.05f});

   REQUIRE(mesh.vertices.size() == 72);

   CHECK(mesh.vertices[0].position == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.vertices[0].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[0].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[0].surface_index == 2);
   CHECK(mesh.vertices[1].position == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.vertices[1].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[1].texcoords == float2(0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[1].surface_index == 2);
   CHECK(mesh.vertices[2].position == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+1f));
   CHECK(mesh.vertices[2].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[2].texcoords == float2(0x1p+0f, 0x1p-2f));
   CHECK(mesh.vertices[2].surface_index == 2);
   CHECK(mesh.vertices[3].position == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+1f));
   CHECK(mesh.vertices[3].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[3].texcoords == float2(0x0p-1022f, 0x1p-2f));
   CHECK(mesh.vertices[3].surface_index == 2);
   CHECK(mesh.vertices[4].position == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.vertices[4].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[4].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[4].surface_index == 5);
   CHECK(mesh.vertices[5].position == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.vertices[5].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[5].texcoords == float2(0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[5].surface_index == 5);
   CHECK(mesh.vertices[6].position == float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.vertices[6].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[6].texcoords == float2(0x1p+0f, 0x1.99999ap-3f));
   CHECK(mesh.vertices[6].surface_index == 5);
   CHECK(mesh.vertices[7].position == float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.vertices[7].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[7].texcoords == float2(0x0p-1022f, 0x1.99999ap-3f));
   CHECK(mesh.vertices[7].surface_index == 5);
   CHECK(mesh.vertices[8].position == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.vertices[8].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[8].texcoords == float2(0x1.99999ap-3f, 0x0p-1022f));
   CHECK(mesh.vertices[8].surface_index == 1);
   CHECK(mesh.vertices[9].position == float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.vertices[9].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[9].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[9].surface_index == 1);
   CHECK(mesh.vertices[10].position == float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.vertices[10].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[10].texcoords == float2(0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[10].surface_index == 1);
   CHECK(mesh.vertices[11].position == float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.vertices[11].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[11].texcoords == float2(0x1.99999ap-3f, 0x1p+0f));
   CHECK(mesh.vertices[11].surface_index == 1);
   CHECK(mesh.vertices[12].position == float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.vertices[12].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[12].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[12].surface_index == 0);
   CHECK(mesh.vertices[13].position == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.vertices[13].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[13].texcoords == float2(0x1.99999ap-3f, 0x0p-1022f));
   CHECK(mesh.vertices[13].surface_index == 0);
   CHECK(mesh.vertices[14].position == float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.vertices[14].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[14].texcoords == float2(0x1.99999ap-3f, 0x1p+0f));
   CHECK(mesh.vertices[14].surface_index == 0);
   CHECK(mesh.vertices[15].position == float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.vertices[15].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[15].texcoords == float2(0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[15].surface_index == 0);
   CHECK(mesh.vertices[16].position == float3(0x1.4p+2f, 0x1.ccccccp-2f, -0x1.4p+1f));
   CHECK(mesh.vertices[16].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[16].texcoords == float2(0x0p-1022f, 0x1p-2f));
   CHECK(mesh.vertices[16].surface_index == 2);
   CHECK(mesh.vertices[17].position == float3(-0x1.4p+2f, 0x1.ccccccp-2f, -0x1.4p+1f));
   CHECK(mesh.vertices[17].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[17].texcoords == float2(0x1p+0f, 0x1p-2f));
   CHECK(mesh.vertices[17].surface_index == 2);
   CHECK(mesh.vertices[18].position == float3(-0x1.4p+2f, 0x1.ccccccp-2f, 0x0p-1022f));
   CHECK(mesh.vertices[18].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[18].texcoords == float2(0x1p+0f, 0x1p-1f));
   CHECK(mesh.vertices[18].surface_index == 2);
   CHECK(mesh.vertices[19].position == float3(0x1.4p+2f, 0x1.ccccccp-2f, 0x0p-1022f));
   CHECK(mesh.vertices[19].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[19].texcoords == float2(0x0p-1022f, 0x1p-1f));
   CHECK(mesh.vertices[19].surface_index == 2);
   CHECK(mesh.vertices[20].position == float3(-0x1.4p+2f, 0x1.ccccccp-2f, -0x1.4p+1f));
   CHECK(mesh.vertices[20].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[20].texcoords == float2(0x0p-1022f, 0x1.99999ap-3f));
   CHECK(mesh.vertices[20].surface_index == 5);
   CHECK(mesh.vertices[21].position == float3(0x1.4p+2f, 0x1.ccccccp-2f, -0x1.4p+1f));
   CHECK(mesh.vertices[21].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[21].texcoords == float2(0x1p+0f, 0x1.99999ap-3f));
   CHECK(mesh.vertices[21].surface_index == 5);
   CHECK(mesh.vertices[22].position == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+1f));
   CHECK(mesh.vertices[22].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[22].texcoords == float2(0x1p+0f, 0x1.ccccccp-2f));
   CHECK(mesh.vertices[22].surface_index == 5);
   CHECK(mesh.vertices[23].position == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+1f));
   CHECK(mesh.vertices[23].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[23].texcoords == float2(0x0p-1022f, 0x1.ccccccp-2f));
   CHECK(mesh.vertices[23].surface_index == 5);
   CHECK(mesh.vertices[24].position == float3(-0x1.4p+2f, 0x1.ccccccp-2f, -0x1.4p+1f));
   CHECK(mesh.vertices[24].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[24].texcoords == float2(0x1.ccccccp-2f, 0x1p-2f));
   CHECK(mesh.vertices[24].surface_index == 1);
   CHECK(mesh.vertices[25].position == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+1f));
   CHECK(mesh.vertices[25].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[25].texcoords == float2(0x1.99999ap-3f, 0x1p-2f));
   CHECK(mesh.vertices[25].surface_index == 1);
   CHECK(mesh.vertices[26].position == float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.vertices[26].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[26].texcoords == float2(0x1.99999ap-3f, 0x1p+0f));
   CHECK(mesh.vertices[26].surface_index == 1);
   CHECK(mesh.vertices[27].position == float3(-0x1.4p+2f, 0x1.ccccccp-2f, 0x1.4p+2f));
   CHECK(mesh.vertices[27].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[27].texcoords == float2(0x1.ccccccp-2f, 0x1p+0f));
   CHECK(mesh.vertices[27].surface_index == 1);
   CHECK(mesh.vertices[28].position == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+1f));
   CHECK(mesh.vertices[28].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[28].texcoords == float2(0x1.99999ap-3f, 0x1p-2f));
   CHECK(mesh.vertices[28].surface_index == 0);
   CHECK(mesh.vertices[29].position == float3(0x1.4p+2f, 0x1.ccccccp-2f, -0x1.4p+1f));
   CHECK(mesh.vertices[29].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[29].texcoords == float2(0x1.ccccccp-2f, 0x1p-2f));
   CHECK(mesh.vertices[29].surface_index == 0);
   CHECK(mesh.vertices[30].position == float3(0x1.4p+2f, 0x1.ccccccp-2f, 0x1.4p+2f));
   CHECK(mesh.vertices[30].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[30].texcoords == float2(0x1.ccccccp-2f, 0x1p+0f));
   CHECK(mesh.vertices[30].surface_index == 0);
   CHECK(mesh.vertices[31].position == float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.vertices[31].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[31].texcoords == float2(0x1.99999ap-3f, 0x1p+0f));
   CHECK(mesh.vertices[31].surface_index == 0);
   CHECK(mesh.vertices[32].position == float3(0x1.4p+2f, 0x1.666666p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[32].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[32].texcoords == float2(0x0p-1022f, 0x1p-1f));
   CHECK(mesh.vertices[32].surface_index == 2);
   CHECK(mesh.vertices[33].position == float3(-0x1.4p+2f, 0x1.666666p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[33].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[33].texcoords == float2(0x1p+0f, 0x1p-1f));
   CHECK(mesh.vertices[33].surface_index == 2);
   CHECK(mesh.vertices[34].position == float3(-0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[34].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[34].texcoords == float2(0x1p+0f, 0x1.8p-1f));
   CHECK(mesh.vertices[34].surface_index == 2);
   CHECK(mesh.vertices[35].position == float3(0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[35].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[35].texcoords == float2(0x0p-1022f, 0x1.8p-1f));
   CHECK(mesh.vertices[35].surface_index == 2);
   CHECK(mesh.vertices[36].position == float3(-0x1.4p+2f, 0x1.666666p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[36].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[36].texcoords == float2(0x0p-1022f, 0x1.ccccccp-2f));
   CHECK(mesh.vertices[36].surface_index == 5);
   CHECK(mesh.vertices[37].position == float3(0x1.4p+2f, 0x1.666666p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[37].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[37].texcoords == float2(0x1p+0f, 0x1.ccccccp-2f));
   CHECK(mesh.vertices[37].surface_index == 5);
   CHECK(mesh.vertices[38].position == float3(0x1.4p+2f, 0x1.ccccccp-2f, 0x0p-1022f));
   CHECK(mesh.vertices[38].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[38].texcoords == float2(0x1p+0f, 0x1.666666p-1f));
   CHECK(mesh.vertices[38].surface_index == 5);
   CHECK(mesh.vertices[39].position == float3(-0x1.4p+2f, 0x1.ccccccp-2f, 0x0p-1022f));
   CHECK(mesh.vertices[39].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[39].texcoords == float2(0x0p-1022f, 0x1.666666p-1f));
   CHECK(mesh.vertices[39].surface_index == 5);
   CHECK(mesh.vertices[40].position == float3(-0x1.4p+2f, 0x1.666666p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[40].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[40].texcoords == float2(0x1.666666p-1f, 0x1p-1f));
   CHECK(mesh.vertices[40].surface_index == 1);
   CHECK(mesh.vertices[41].position == float3(-0x1.4p+2f, 0x1.ccccccp-2f, 0x0p-1022f));
   CHECK(mesh.vertices[41].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[41].texcoords == float2(0x1.ccccccp-2f, 0x1p-1f));
   CHECK(mesh.vertices[41].surface_index == 1);
   CHECK(mesh.vertices[42].position == float3(-0x1.4p+2f, 0x1.ccccccp-2f, 0x1.4p+2f));
   CHECK(mesh.vertices[42].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[42].texcoords == float2(0x1.ccccccp-2f, 0x1p+0f));
   CHECK(mesh.vertices[42].surface_index == 1);
   CHECK(mesh.vertices[43].position == float3(-0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[43].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[43].texcoords == float2(0x1.666666p-1f, 0x1p+0f));
   CHECK(mesh.vertices[43].surface_index == 1);
   CHECK(mesh.vertices[44].position == float3(0x1.4p+2f, 0x1.ccccccp-2f, 0x0p-1022f));
   CHECK(mesh.vertices[44].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[44].texcoords == float2(0x1.ccccccp-2f, 0x1p-1f));
   CHECK(mesh.vertices[44].surface_index == 0);
   CHECK(mesh.vertices[45].position == float3(0x1.4p+2f, 0x1.666666p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[45].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[45].texcoords == float2(0x1.666666p-1f, 0x1p-1f));
   CHECK(mesh.vertices[45].surface_index == 0);
   CHECK(mesh.vertices[46].position == float3(0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[46].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[46].texcoords == float2(0x1.666666p-1f, 0x1p+0f));
   CHECK(mesh.vertices[46].surface_index == 0);
   CHECK(mesh.vertices[47].position == float3(0x1.4p+2f, 0x1.ccccccp-2f, 0x1.4p+2f));
   CHECK(mesh.vertices[47].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[47].texcoords == float2(0x1.ccccccp-2f, 0x1p+0f));
   CHECK(mesh.vertices[47].surface_index == 0);
   CHECK(mesh.vertices[48].position == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[48].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[48].texcoords == float2(0x0p-1022f, 0x1.8p-1f));
   CHECK(mesh.vertices[48].surface_index == 2);
   CHECK(mesh.vertices[49].position == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[49].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[49].texcoords == float2(0x1p+0f, 0x1.8p-1f));
   CHECK(mesh.vertices[49].surface_index == 2);
   CHECK(mesh.vertices[50].position == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[50].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[50].texcoords == float2(0x1p+0f, 0x1p+0f));
   CHECK(mesh.vertices[50].surface_index == 2);
   CHECK(mesh.vertices[51].position == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[51].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[51].texcoords == float2(0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[51].surface_index == 2);
   CHECK(mesh.vertices[52].position == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[52].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[52].texcoords == float2(0x0p-1022f, 0x1.666666p-1f));
   CHECK(mesh.vertices[52].surface_index == 5);
   CHECK(mesh.vertices[53].position == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[53].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[53].texcoords == float2(0x1p+0f, 0x1.666666p-1f));
   CHECK(mesh.vertices[53].surface_index == 5);
   CHECK(mesh.vertices[54].position == float3(0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[54].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[54].texcoords == float2(0x1p+0f, 0x1.e66666p-1f));
   CHECK(mesh.vertices[54].surface_index == 5);
   CHECK(mesh.vertices[55].position == float3(-0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[55].normal == float3(0x0p-1022f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[55].texcoords == float2(0x0p-1022f, 0x1.e66666p-1f));
   CHECK(mesh.vertices[55].surface_index == 5);
   CHECK(mesh.vertices[56].position == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[56].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[56].texcoords == float2(0x1.e66666p-1f, 0x1.8p-1f));
   CHECK(mesh.vertices[56].surface_index == 1);
   CHECK(mesh.vertices[57].position == float3(-0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[57].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[57].texcoords == float2(0x1.666666p-1f, 0x1.8p-1f));
   CHECK(mesh.vertices[57].surface_index == 1);
   CHECK(mesh.vertices[58].position == float3(-0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[58].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[58].texcoords == float2(0x1.666666p-1f, 0x1p+0f));
   CHECK(mesh.vertices[58].surface_index == 1);
   CHECK(mesh.vertices[59].position == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[59].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[59].texcoords == float2(0x1.e66666p-1f, 0x1p+0f));
   CHECK(mesh.vertices[59].surface_index == 1);
   CHECK(mesh.vertices[60].position == float3(0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[60].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[60].texcoords == float2(0x1.666666p-1f, 0x1.8p-1f));
   CHECK(mesh.vertices[60].surface_index == 0);
   CHECK(mesh.vertices[61].position == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.vertices[61].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[61].texcoords == float2(0x1.e66666p-1f, 0x1.8p-1f));
   CHECK(mesh.vertices[61].surface_index == 0);
   CHECK(mesh.vertices[62].position == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[62].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[62].texcoords == float2(0x1.e66666p-1f, 0x1p+0f));
   CHECK(mesh.vertices[62].surface_index == 0);
   CHECK(mesh.vertices[63].position == float3(0x1.4p+2f, 0x1.666666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[63].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[63].texcoords == float2(0x1.666666p-1f, 0x1p+0f));
   CHECK(mesh.vertices[63].surface_index == 0);
   CHECK(mesh.vertices[64].position == float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.vertices[64].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[64].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[64].surface_index == 3);
   CHECK(mesh.vertices[65].position == float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.vertices[65].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[65].texcoords == float2(0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[65].surface_index == 3);
   CHECK(mesh.vertices[66].position == float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.vertices[66].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[66].texcoords == float2(0x1p+0f, 0x1p+0f));
   CHECK(mesh.vertices[66].surface_index == 3);
   CHECK(mesh.vertices[67].position == float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.vertices[67].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[67].texcoords == float2(0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[67].surface_index == 3);
   CHECK(mesh.vertices[68].position == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[68].normal == float3(0x0p-1022f, 0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[68].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[68].surface_index == 4);
   CHECK(mesh.vertices[69].position == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.vertices[69].normal == float3(0x0p-1022f, 0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[69].texcoords == float2(0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[69].surface_index == 4);
   CHECK(mesh.vertices[70].position == float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.vertices[70].normal == float3(0x0p-1022f, 0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[70].texcoords == float2(0x1p+0f, 0x1p+0f));
   CHECK(mesh.vertices[70].surface_index == 4);
   CHECK(mesh.vertices[71].position == float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.vertices[71].normal == float3(0x0p-1022f, 0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[71].texcoords == float2(0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[71].surface_index == 4);

   REQUIRE(mesh.triangles.size() == 36);

   CHECK(mesh.triangles[0] == std::array<uint16, 3>{0, 1, 2});
   CHECK(mesh.triangles[1] == std::array<uint16, 3>{0, 2, 3});
   CHECK(mesh.triangles[2] == std::array<uint16, 3>{4, 5, 6});
   CHECK(mesh.triangles[3] == std::array<uint16, 3>{4, 6, 7});
   CHECK(mesh.triangles[4] == std::array<uint16, 3>{8, 9, 10});
   CHECK(mesh.triangles[5] == std::array<uint16, 3>{8, 10, 11});
   CHECK(mesh.triangles[6] == std::array<uint16, 3>{12, 13, 14});
   CHECK(mesh.triangles[7] == std::array<uint16, 3>{12, 14, 15});
   CHECK(mesh.triangles[8] == std::array<uint16, 3>{16, 17, 18});
   CHECK(mesh.triangles[9] == std::array<uint16, 3>{16, 18, 19});
   CHECK(mesh.triangles[10] == std::array<uint16, 3>{20, 21, 22});
   CHECK(mesh.triangles[11] == std::array<uint16, 3>{20, 22, 23});
   CHECK(mesh.triangles[12] == std::array<uint16, 3>{24, 25, 26});
   CHECK(mesh.triangles[13] == std::array<uint16, 3>{24, 26, 27});
   CHECK(mesh.triangles[14] == std::array<uint16, 3>{28, 29, 30});
   CHECK(mesh.triangles[15] == std::array<uint16, 3>{28, 30, 31});
   CHECK(mesh.triangles[16] == std::array<uint16, 3>{32, 33, 34});
   CHECK(mesh.triangles[17] == std::array<uint16, 3>{32, 34, 35});
   CHECK(mesh.triangles[18] == std::array<uint16, 3>{36, 37, 38});
   CHECK(mesh.triangles[19] == std::array<uint16, 3>{36, 38, 39});
   CHECK(mesh.triangles[20] == std::array<uint16, 3>{40, 41, 42});
   CHECK(mesh.triangles[21] == std::array<uint16, 3>{40, 42, 43});
   CHECK(mesh.triangles[22] == std::array<uint16, 3>{44, 45, 46});
   CHECK(mesh.triangles[23] == std::array<uint16, 3>{44, 46, 47});
   CHECK(mesh.triangles[24] == std::array<uint16, 3>{48, 49, 50});
   CHECK(mesh.triangles[25] == std::array<uint16, 3>{48, 50, 51});
   CHECK(mesh.triangles[26] == std::array<uint16, 3>{52, 53, 54});
   CHECK(mesh.triangles[27] == std::array<uint16, 3>{52, 54, 55});
   CHECK(mesh.triangles[28] == std::array<uint16, 3>{56, 57, 58});
   CHECK(mesh.triangles[29] == std::array<uint16, 3>{56, 58, 59});
   CHECK(mesh.triangles[30] == std::array<uint16, 3>{60, 61, 62});
   CHECK(mesh.triangles[31] == std::array<uint16, 3>{60, 62, 63});
   CHECK(mesh.triangles[32] == std::array<uint16, 3>{64, 65, 66});
   CHECK(mesh.triangles[33] == std::array<uint16, 3>{64, 66, 67});
   CHECK(mesh.triangles[34] == std::array<uint16, 3>{68, 69, 70});
   CHECK(mesh.triangles[35] == std::array<uint16, 3>{68, 70, 71});

   REQUIRE(mesh.occluders.size() == 18);

   CHECK(mesh.occluders[0] == std::array<uint16, 4>{0, 1, 2, 3});
   CHECK(mesh.occluders[1] == std::array<uint16, 4>{4, 5, 6, 7});
   CHECK(mesh.occluders[2] == std::array<uint16, 4>{8, 9, 10, 11});
   CHECK(mesh.occluders[3] == std::array<uint16, 4>{12, 13, 14, 15});
   CHECK(mesh.occluders[4] == std::array<uint16, 4>{16, 17, 18, 19});
   CHECK(mesh.occluders[5] == std::array<uint16, 4>{20, 21, 22, 23});
   CHECK(mesh.occluders[6] == std::array<uint16, 4>{24, 25, 26, 27});
   CHECK(mesh.occluders[7] == std::array<uint16, 4>{28, 29, 30, 31});
   CHECK(mesh.occluders[8] == std::array<uint16, 4>{32, 33, 34, 35});
   CHECK(mesh.occluders[9] == std::array<uint16, 4>{36, 37, 38, 39});
   CHECK(mesh.occluders[10] == std::array<uint16, 4>{40, 41, 42, 43});
   CHECK(mesh.occluders[11] == std::array<uint16, 4>{44, 45, 46, 47});
   CHECK(mesh.occluders[12] == std::array<uint16, 4>{48, 49, 50, 51});
   CHECK(mesh.occluders[13] == std::array<uint16, 4>{52, 53, 54, 55});
   CHECK(mesh.occluders[14] == std::array<uint16, 4>{56, 57, 58, 59});
   CHECK(mesh.occluders[15] == std::array<uint16, 4>{60, 61, 62, 63});
   CHECK(mesh.occluders[16] == std::array<uint16, 4>{64, 65, 66, 67});
   CHECK(mesh.occluders[17] == std::array<uint16, 4>{68, 69, 70, 71});

   REQUIRE(mesh.collision_vertices.size() == 42);

   CHECK(mesh.collision_vertices[0].position ==
         float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[0].surface_index == 2);
   CHECK(mesh.collision_vertices[1].position ==
         float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[1].surface_index == 2);
   CHECK(mesh.collision_vertices[2].position ==
         float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[2].surface_index == 2);
   CHECK(mesh.collision_vertices[3].position ==
         float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[3].surface_index == 2);
   CHECK(mesh.collision_vertices[4].position ==
         float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[4].surface_index == 0);
   CHECK(mesh.collision_vertices[5].position ==
         float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[5].surface_index == 0);
   CHECK(mesh.collision_vertices[6].position ==
         float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[6].surface_index == 0);
   CHECK(mesh.collision_vertices[7].position ==
         float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[7].surface_index == 1);
   CHECK(mesh.collision_vertices[8].position ==
         float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[8].surface_index == 1);
   CHECK(mesh.collision_vertices[9].position ==
         float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[9].surface_index == 1);
   CHECK(mesh.collision_vertices[10].position ==
         float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[10].surface_index == 1);
   CHECK(mesh.collision_vertices[11].position ==
         float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[11].surface_index == 1);
   CHECK(mesh.collision_vertices[12].position ==
         float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[12].surface_index == 1);
   CHECK(mesh.collision_vertices[13].position ==
         float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[13].surface_index == 1);
   CHECK(mesh.collision_vertices[14].position ==
         float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[14].surface_index == 0);
   CHECK(mesh.collision_vertices[15].position ==
         float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[15].surface_index == 0);
   CHECK(mesh.collision_vertices[16].position ==
         float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[16].surface_index == 0);
   CHECK(mesh.collision_vertices[17].position ==
         float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[17].surface_index == 0);
   CHECK(mesh.collision_vertices[18].position ==
         float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[18].surface_index == 2);
   CHECK(mesh.collision_vertices[19].position ==
         float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[19].surface_index == 2);
   CHECK(mesh.collision_vertices[20].position ==
         float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[20].surface_index == 2);
   CHECK(mesh.collision_vertices[21].position ==
         float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[21].surface_index == 2);
   CHECK(mesh.collision_vertices[22].position ==
         float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[22].surface_index == 3);
   CHECK(mesh.collision_vertices[23].position ==
         float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[23].surface_index == 3);
   CHECK(mesh.collision_vertices[24].position ==
         float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[24].surface_index == 3);
   CHECK(mesh.collision_vertices[25].position ==
         float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[25].surface_index == 3);
   CHECK(mesh.collision_vertices[26].position ==
         float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[26].surface_index == 4);
   CHECK(mesh.collision_vertices[27].position ==
         float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[27].surface_index == 4);
   CHECK(mesh.collision_vertices[28].position ==
         float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[28].surface_index == 4);
   CHECK(mesh.collision_vertices[29].position ==
         float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[29].surface_index == 4);
   CHECK(mesh.collision_vertices[30].position ==
         float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[30].surface_index == 5);
   CHECK(mesh.collision_vertices[31].position ==
         float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[31].surface_index == 5);
   CHECK(mesh.collision_vertices[32].position ==
         float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[32].surface_index == 5);
   CHECK(mesh.collision_vertices[33].position ==
         float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[33].surface_index == 5);
   CHECK(mesh.collision_vertices[34].position ==
         float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[34].surface_index == 0);
   CHECK(mesh.collision_vertices[35].position ==
         float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[35].surface_index == 0);
   CHECK(mesh.collision_vertices[36].position ==
         float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[36].surface_index == 0);
   CHECK(mesh.collision_vertices[37].position ==
         float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[37].surface_index == 0);
   CHECK(mesh.collision_vertices[38].position ==
         float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[38].surface_index == 1);
   CHECK(mesh.collision_vertices[39].position ==
         float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[39].surface_index == 1);
   CHECK(mesh.collision_vertices[40].position ==
         float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[40].surface_index == 1);
   CHECK(mesh.collision_vertices[41].position ==
         float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[41].surface_index == 1);

   REQUIRE(mesh.collision_triangles.size() == 20);

   CHECK(mesh.collision_triangles[0] == std::array<uint16, 3>{0, 1, 2});
   CHECK(mesh.collision_triangles[1] == std::array<uint16, 3>{0, 2, 3});
   CHECK(mesh.collision_triangles[2] == std::array<uint16, 3>{4, 5, 6});
   CHECK(mesh.collision_triangles[3] == std::array<uint16, 3>{7, 8, 9});
   CHECK(mesh.collision_triangles[4] == std::array<uint16, 3>{10, 11, 12});
   CHECK(mesh.collision_triangles[5] == std::array<uint16, 3>{10, 12, 13});
   CHECK(mesh.collision_triangles[6] == std::array<uint16, 3>{14, 15, 16});
   CHECK(mesh.collision_triangles[7] == std::array<uint16, 3>{14, 16, 17});
   CHECK(mesh.collision_triangles[8] == std::array<uint16, 3>{18, 19, 20});
   CHECK(mesh.collision_triangles[9] == std::array<uint16, 3>{18, 20, 21});
   CHECK(mesh.collision_triangles[10] == std::array<uint16, 3>{22, 23, 24});
   CHECK(mesh.collision_triangles[11] == std::array<uint16, 3>{22, 24, 25});
   CHECK(mesh.collision_triangles[12] == std::array<uint16, 3>{26, 27, 28});
   CHECK(mesh.collision_triangles[13] == std::array<uint16, 3>{26, 28, 29});
   CHECK(mesh.collision_triangles[14] == std::array<uint16, 3>{30, 31, 32});
   CHECK(mesh.collision_triangles[15] == std::array<uint16, 3>{30, 32, 33});
   CHECK(mesh.collision_triangles[16] == std::array<uint16, 3>{34, 35, 36});
   CHECK(mesh.collision_triangles[17] == std::array<uint16, 3>{34, 36, 37});
   CHECK(mesh.collision_triangles[18] == std::array<uint16, 3>{38, 39, 40});
   CHECK(mesh.collision_triangles[19] == std::array<uint16, 3>{38, 40, 41});

   REQUIRE(mesh.collision_occluders.size() == 9);

   CHECK(mesh.collision_occluders[0] == std::array<uint16, 4>{0, 1, 2, 3});
   CHECK(mesh.collision_occluders[1] == std::array<uint16, 4>{10, 11, 12, 13});
   CHECK(mesh.collision_occluders[2] == std::array<uint16, 4>{14, 15, 16, 17});
   CHECK(mesh.collision_occluders[3] == std::array<uint16, 4>{18, 19, 20, 21});
   CHECK(mesh.collision_occluders[4] == std::array<uint16, 4>{22, 23, 24, 25});
   CHECK(mesh.collision_occluders[5] == std::array<uint16, 4>{26, 27, 28, 29});
   CHECK(mesh.collision_occluders[6] == std::array<uint16, 4>{30, 31, 32, 33});
   CHECK(mesh.collision_occluders[7] == std::array<uint16, 4>{34, 35, 36, 37});
   CHECK(mesh.collision_occluders[8] == std::array<uint16, 4>{38, 39, 40, 41});

   REQUIRE(mesh.snap_points.size() == 6);

   CHECK(mesh.snap_points[0] == float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.snap_points[1] == float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.snap_points[2] == float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.snap_points[3] == float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.snap_points[4] == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.snap_points[5] == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));

   REQUIRE(mesh.snap_edges.size() == 7);

   CHECK(mesh.snap_edges[0] == std::array<uint16, 2>{0, 1});
   CHECK(mesh.snap_edges[1] == std::array<uint16, 2>{1, 2});
   CHECK(mesh.snap_edges[2] == std::array<uint16, 2>{2, 3});
   CHECK(mesh.snap_edges[3] == std::array<uint16, 2>{3, 0});
   CHECK(mesh.snap_edges[4] == std::array<uint16, 2>{2, 4});
   CHECK(mesh.snap_edges[5] == std::array<uint16, 2>{3, 5});
   CHECK(mesh.snap_edges[6] == std::array<uint16, 2>{4, 5});
}

TEST_CASE("world blocks generate_mesh ring", "[World][Blocks]")
{
   block_custom_mesh mesh = generate_mesh(block_custom_mesh_description_ring{
      .inner_radius = 16.0f,
      .outer_radius = 4.0f,
      .height = 4.0f,
      .segments = 16,
      .flat_shading = false,
      .texture_loops = 2.0f,
   });

   REQUIRE(mesh.vertices.size() == 136);

   CHECK(mesh.vertices[0].position == float3(0x1.8p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.vertices[0].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[0].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[0].surface_index == 0);
   CHECK(mesh.vertices[1].position == float3(0x1p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.vertices[1].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[1].texcoords == float2(0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[1].surface_index == 0);
   CHECK(mesh.vertices[2].position == float3(0x1.d906bcp+3f, 0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.vertices[2].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[2].texcoords == float2(0x1p+0f, 0x1p-3f));
   CHECK(mesh.vertices[2].surface_index == 0);
   CHECK(mesh.vertices[3].position == float3(0x1.62c50cp+4f, 0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.vertices[3].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[3].texcoords == float2(0x0p-1022f, 0x1p-3f));
   CHECK(mesh.vertices[3].surface_index == 0);
   CHECK(mesh.vertices[4].position == float3(0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.vertices[4].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[4].texcoords == float2(0x1p+0f, 0x1p-2f));
   CHECK(mesh.vertices[4].surface_index == 0);
   CHECK(mesh.vertices[5].position == float3(0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.vertices[5].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[5].texcoords == float2(0x0p-1022f, 0x1p-2f));
   CHECK(mesh.vertices[5].surface_index == 0);
   CHECK(mesh.vertices[6].position == float3(0x1.87de2ap+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.vertices[6].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[6].texcoords == float2(0x1p+0f, 0x1.8p-2f));
   CHECK(mesh.vertices[6].surface_index == 0);
   CHECK(mesh.vertices[7].position == float3(0x1.25e6ap+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.vertices[7].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[7].texcoords == float2(0x0p-1022f, 0x1.8p-2f));
   CHECK(mesh.vertices[7].surface_index == 0);
   CHECK(mesh.vertices[8].position == float3(-0x1.777a5cp-21f, 0x1p+2f, 0x1p+4f));
   CHECK(mesh.vertices[8].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[8].texcoords == float2(0x1p+0f, 0x1p-1f));
   CHECK(mesh.vertices[8].surface_index == 0);
   CHECK(mesh.vertices[9].position == float3(-0x1.199bc4p-20f, 0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.vertices[9].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[9].texcoords == float2(0x0p-1022f, 0x1p-1f));
   CHECK(mesh.vertices[9].surface_index == 0);
   CHECK(mesh.vertices[10].position == float3(-0x1.87de3p+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.vertices[10].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[10].texcoords == float2(0x1p+0f, 0x1.4p-1f));
   CHECK(mesh.vertices[10].surface_index == 0);
   CHECK(mesh.vertices[11].position == float3(-0x1.25e6a4p+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.vertices[11].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[11].texcoords == float2(0x0p-1022f, 0x1.4p-1f));
   CHECK(mesh.vertices[11].surface_index == 0);
   CHECK(mesh.vertices[12].position == float3(-0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.vertices[12].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[12].texcoords == float2(0x1p+0f, 0x1.8p-1f));
   CHECK(mesh.vertices[12].surface_index == 0);
   CHECK(mesh.vertices[13].position == float3(-0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.vertices[13].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[13].texcoords == float2(0x0p-1022f, 0x1.8p-1f));
   CHECK(mesh.vertices[13].surface_index == 0);
   CHECK(mesh.vertices[14].position == float3(-0x1.d906cp+3f, 0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.vertices[14].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[14].texcoords == float2(0x1p+0f, 0x1.cp-1f));
   CHECK(mesh.vertices[14].surface_index == 0);
   CHECK(mesh.vertices[15].position == float3(-0x1.62c51p+4f, 0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.vertices[15].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[15].texcoords == float2(0x0p-1022f, 0x1.cp-1f));
   CHECK(mesh.vertices[15].surface_index == 0);
   CHECK(mesh.vertices[16].position == float3(-0x1p+4f, 0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.vertices[16].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[16].texcoords == float2(0x1p+0f, 0x1p+0f));
   CHECK(mesh.vertices[16].surface_index == 0);
   CHECK(mesh.vertices[17].position == float3(-0x1.8p+4f, 0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.vertices[17].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[17].texcoords == float2(0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[17].surface_index == 0);
   CHECK(mesh.vertices[18].position ==
         float3(-0x1.d906bcp+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.vertices[18].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[18].texcoords == float2(0x1p+0f, 0x1.2p+0f));
   CHECK(mesh.vertices[18].surface_index == 0);
   CHECK(mesh.vertices[19].position == float3(-0x1.62c50cp+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.vertices[19].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[19].texcoords == float2(0x0p-1022f, 0x1.2p+0f));
   CHECK(mesh.vertices[19].surface_index == 0);
   CHECK(mesh.vertices[20].position ==
         float3(-0x1.6a09e2p+3f, 0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.vertices[20].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[20].texcoords == float2(0x1p+0f, 0x1.4p+0f));
   CHECK(mesh.vertices[20].surface_index == 0);
   CHECK(mesh.vertices[21].position == float3(-0x1.0f876ap+4f, 0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.vertices[21].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[21].texcoords == float2(0x0p-1022f, 0x1.4p+0f));
   CHECK(mesh.vertices[21].surface_index == 0);
   CHECK(mesh.vertices[22].position ==
         float3(-0x1.87de16p+2f, 0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.vertices[22].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[22].texcoords == float2(0x1p+0f, 0x1.6p+0f));
   CHECK(mesh.vertices[22].surface_index == 0);
   CHECK(mesh.vertices[23].position == float3(-0x1.25e69p+3f, 0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.vertices[23].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[23].texcoords == float2(0x0p-1022f, 0x1.6p+0f));
   CHECK(mesh.vertices[23].surface_index == 0);
   CHECK(mesh.vertices[24].position == float3(0x1.99bc5cp-23f, 0x1p+2f, -0x1p+4f));
   CHECK(mesh.vertices[24].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[24].texcoords == float2(0x1p+0f, 0x1.8p+0f));
   CHECK(mesh.vertices[24].surface_index == 0);
   CHECK(mesh.vertices[25].position == float3(0x1.334d44p-22f, 0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.vertices[25].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[25].texcoords == float2(0x0p-1022f, 0x1.8p+0f));
   CHECK(mesh.vertices[25].surface_index == 0);
   CHECK(mesh.vertices[26].position == float3(0x1.87de36p+2f, 0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.vertices[26].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[26].texcoords == float2(0x1p+0f, 0x1.ap+0f));
   CHECK(mesh.vertices[26].surface_index == 0);
   CHECK(mesh.vertices[27].position == float3(0x1.25e6a8p+3f, 0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.vertices[27].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[27].texcoords == float2(0x0p-1022f, 0x1.ap+0f));
   CHECK(mesh.vertices[27].surface_index == 0);
   CHECK(mesh.vertices[28].position == float3(0x1.6a09eep+3f, 0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.vertices[28].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[28].texcoords == float2(0x1p+0f, 0x1.cp+0f));
   CHECK(mesh.vertices[28].surface_index == 0);
   CHECK(mesh.vertices[29].position == float3(0x1.0f8772p+4f, 0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.vertices[29].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[29].texcoords == float2(0x0p-1022f, 0x1.cp+0f));
   CHECK(mesh.vertices[29].surface_index == 0);
   CHECK(mesh.vertices[30].position == float3(0x1.d906bep+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.vertices[30].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[30].texcoords == float2(0x1p+0f, 0x1.ep+0f));
   CHECK(mesh.vertices[30].surface_index == 0);
   CHECK(mesh.vertices[31].position == float3(0x1.62c50ep+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.vertices[31].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[31].texcoords == float2(0x0p-1022f, 0x1.ep+0f));
   CHECK(mesh.vertices[31].surface_index == 0);
   CHECK(mesh.vertices[32].position == float3(0x1p+4f, 0x1p+2f, 0x1.777a5cp-19f));
   CHECK(mesh.vertices[32].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[32].texcoords == float2(0x1p+0f, 0x1p+1f));
   CHECK(mesh.vertices[32].surface_index == 0);
   CHECK(mesh.vertices[33].position == float3(0x1.8p+4f, 0x1p+2f, 0x1.199bc4p-18f));
   CHECK(mesh.vertices[33].normal == float3(0x0p-1022f, 0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[33].texcoords == float2(0x0p-1022f, 0x1p+1f));
   CHECK(mesh.vertices[33].surface_index == 0);
   CHECK(mesh.vertices[34].position == float3(0x1p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.vertices[34].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[34].texcoords == float2(0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[34].surface_index == 1);
   CHECK(mesh.vertices[35].position == float3(0x1.8p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.vertices[35].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[35].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[35].surface_index == 1);
   CHECK(mesh.vertices[36].position == float3(0x1.62c50cp+4f, -0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.vertices[36].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[36].texcoords == float2(0x0p-1022f, 0x1p-3f));
   CHECK(mesh.vertices[36].surface_index == 1);
   CHECK(mesh.vertices[37].position == float3(0x1.d906bcp+3f, -0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.vertices[37].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[37].texcoords == float2(0x1p+0f, 0x1p-3f));
   CHECK(mesh.vertices[37].surface_index == 1);
   CHECK(mesh.vertices[38].position == float3(0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.vertices[38].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[38].texcoords == float2(0x0p-1022f, 0x1p-2f));
   CHECK(mesh.vertices[38].surface_index == 1);
   CHECK(mesh.vertices[39].position == float3(0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.vertices[39].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[39].texcoords == float2(0x1p+0f, 0x1p-2f));
   CHECK(mesh.vertices[39].surface_index == 1);
   CHECK(mesh.vertices[40].position == float3(0x1.25e6ap+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.vertices[40].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[40].texcoords == float2(0x0p-1022f, 0x1.8p-2f));
   CHECK(mesh.vertices[40].surface_index == 1);
   CHECK(mesh.vertices[41].position == float3(0x1.87de2ap+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.vertices[41].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[41].texcoords == float2(0x1p+0f, 0x1.8p-2f));
   CHECK(mesh.vertices[41].surface_index == 1);
   CHECK(mesh.vertices[42].position == float3(-0x1.199bc4p-20f, -0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.vertices[42].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[42].texcoords == float2(0x0p-1022f, 0x1p-1f));
   CHECK(mesh.vertices[42].surface_index == 1);
   CHECK(mesh.vertices[43].position == float3(-0x1.777a5cp-21f, -0x1p+2f, 0x1p+4f));
   CHECK(mesh.vertices[43].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[43].texcoords == float2(0x1p+0f, 0x1p-1f));
   CHECK(mesh.vertices[43].surface_index == 1);
   CHECK(mesh.vertices[44].position ==
         float3(-0x1.25e6a4p+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.vertices[44].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[44].texcoords == float2(0x0p-1022f, 0x1.4p-1f));
   CHECK(mesh.vertices[44].surface_index == 1);
   CHECK(mesh.vertices[45].position == float3(-0x1.87de3p+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.vertices[45].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[45].texcoords == float2(0x1p+0f, 0x1.4p-1f));
   CHECK(mesh.vertices[45].surface_index == 1);
   CHECK(mesh.vertices[46].position ==
         float3(-0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.vertices[46].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[46].texcoords == float2(0x0p-1022f, 0x1.8p-1f));
   CHECK(mesh.vertices[46].surface_index == 1);
   CHECK(mesh.vertices[47].position ==
         float3(-0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.vertices[47].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[47].texcoords == float2(0x1p+0f, 0x1.8p-1f));
   CHECK(mesh.vertices[47].surface_index == 1);
   CHECK(mesh.vertices[48].position == float3(-0x1.62c51p+4f, -0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.vertices[48].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[48].texcoords == float2(0x0p-1022f, 0x1.cp-1f));
   CHECK(mesh.vertices[48].surface_index == 1);
   CHECK(mesh.vertices[49].position == float3(-0x1.d906cp+3f, -0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.vertices[49].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[49].texcoords == float2(0x1p+0f, 0x1.cp-1f));
   CHECK(mesh.vertices[49].surface_index == 1);
   CHECK(mesh.vertices[50].position == float3(-0x1.8p+4f, -0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.vertices[50].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[50].texcoords == float2(0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[50].surface_index == 1);
   CHECK(mesh.vertices[51].position == float3(-0x1p+4f, -0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.vertices[51].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[51].texcoords == float2(0x1p+0f, 0x1p+0f));
   CHECK(mesh.vertices[51].surface_index == 1);
   CHECK(mesh.vertices[52].position ==
         float3(-0x1.62c50cp+4f, -0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.vertices[52].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[52].texcoords == float2(0x0p-1022f, 0x1.2p+0f));
   CHECK(mesh.vertices[52].surface_index == 1);
   CHECK(mesh.vertices[53].position ==
         float3(-0x1.d906bcp+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.vertices[53].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[53].texcoords == float2(0x1p+0f, 0x1.2p+0f));
   CHECK(mesh.vertices[53].surface_index == 1);
   CHECK(mesh.vertices[54].position ==
         float3(-0x1.0f876ap+4f, -0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.vertices[54].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[54].texcoords == float2(0x0p-1022f, 0x1.4p+0f));
   CHECK(mesh.vertices[54].surface_index == 1);
   CHECK(mesh.vertices[55].position ==
         float3(-0x1.6a09e2p+3f, -0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.vertices[55].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[55].texcoords == float2(0x1p+0f, 0x1.4p+0f));
   CHECK(mesh.vertices[55].surface_index == 1);
   CHECK(mesh.vertices[56].position ==
         float3(-0x1.25e69p+3f, -0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.vertices[56].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[56].texcoords == float2(0x0p-1022f, 0x1.6p+0f));
   CHECK(mesh.vertices[56].surface_index == 1);
   CHECK(mesh.vertices[57].position ==
         float3(-0x1.87de16p+2f, -0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.vertices[57].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[57].texcoords == float2(0x1p+0f, 0x1.6p+0f));
   CHECK(mesh.vertices[57].surface_index == 1);
   CHECK(mesh.vertices[58].position == float3(0x1.334d44p-22f, -0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.vertices[58].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[58].texcoords == float2(0x0p-1022f, 0x1.8p+0f));
   CHECK(mesh.vertices[58].surface_index == 1);
   CHECK(mesh.vertices[59].position == float3(0x1.99bc5cp-23f, -0x1p+2f, -0x1p+4f));
   CHECK(mesh.vertices[59].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[59].texcoords == float2(0x1p+0f, 0x1.8p+0f));
   CHECK(mesh.vertices[59].surface_index == 1);
   CHECK(mesh.vertices[60].position ==
         float3(0x1.25e6a8p+3f, -0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.vertices[60].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[60].texcoords == float2(0x0p-1022f, 0x1.ap+0f));
   CHECK(mesh.vertices[60].surface_index == 1);
   CHECK(mesh.vertices[61].position ==
         float3(0x1.87de36p+2f, -0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.vertices[61].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[61].texcoords == float2(0x1p+0f, 0x1.ap+0f));
   CHECK(mesh.vertices[61].surface_index == 1);
   CHECK(mesh.vertices[62].position ==
         float3(0x1.0f8772p+4f, -0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.vertices[62].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[62].texcoords == float2(0x0p-1022f, 0x1.cp+0f));
   CHECK(mesh.vertices[62].surface_index == 1);
   CHECK(mesh.vertices[63].position ==
         float3(0x1.6a09eep+3f, -0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.vertices[63].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[63].texcoords == float2(0x1p+0f, 0x1.cp+0f));
   CHECK(mesh.vertices[63].surface_index == 1);
   CHECK(mesh.vertices[64].position == float3(0x1.62c50ep+4f, -0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.vertices[64].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[64].texcoords == float2(0x0p-1022f, 0x1.ep+0f));
   CHECK(mesh.vertices[64].surface_index == 1);
   CHECK(mesh.vertices[65].position ==
         float3(0x1.d906bep+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.vertices[65].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[65].texcoords == float2(0x1p+0f, 0x1.ep+0f));
   CHECK(mesh.vertices[65].surface_index == 1);
   CHECK(mesh.vertices[66].position == float3(0x1.8p+4f, -0x1p+2f, 0x1.199bc4p-18f));
   CHECK(mesh.vertices[66].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[66].texcoords == float2(0x0p-1022f, 0x1p+1f));
   CHECK(mesh.vertices[66].surface_index == 1);
   CHECK(mesh.vertices[67].position == float3(0x1p+4f, -0x1p+2f, 0x1.777a5cp-19f));
   CHECK(mesh.vertices[67].normal == float3(0x0p-1022f, -0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[67].texcoords == float2(0x1p+0f, 0x1p+1f));
   CHECK(mesh.vertices[67].surface_index == 1);
   CHECK(mesh.vertices[68].position == float3(0x1.8p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.vertices[68].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[68].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[68].surface_index == 2);
   CHECK(mesh.vertices[69].position == float3(0x1.8p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.vertices[69].normal == float3(0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[69].texcoords == float2(0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[69].surface_index == 2);
   CHECK(mesh.vertices[70].position == float3(0x1.62c50cp+4f, 0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.vertices[70].normal == float3(0x1.d906bcp-1f, 0x0p-1022f, 0x1.87de2cp-2f));
   CHECK(mesh.vertices[70].texcoords == float2(0x1p-3f, 0x1p+0f));
   CHECK(mesh.vertices[70].surface_index == 2);
   CHECK(mesh.vertices[71].position == float3(0x1.62c50cp+4f, -0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.vertices[71].normal == float3(0x1.d906bcp-1f, 0x0p-1022f, 0x1.87de2cp-2f));
   CHECK(mesh.vertices[71].texcoords == float2(0x1p-3f, 0x0p-1022f));
   CHECK(mesh.vertices[71].surface_index == 2);
   CHECK(mesh.vertices[72].position == float3(0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.vertices[72].normal == float3(0x1.6a09e6p-1f, 0x0p-1022f, 0x1.6a09e6p-1f));
   CHECK(mesh.vertices[72].texcoords == float2(0x1p-2f, 0x1p+0f));
   CHECK(mesh.vertices[72].surface_index == 2);
   CHECK(mesh.vertices[73].position == float3(0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.vertices[73].normal == float3(0x1.6a09e6p-1f, 0x0p-1022f, 0x1.6a09e6p-1f));
   CHECK(mesh.vertices[73].texcoords == float2(0x1p-2f, 0x0p-1022f));
   CHECK(mesh.vertices[73].surface_index == 2);
   CHECK(mesh.vertices[74].position == float3(0x1.25e6ap+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.vertices[74].normal == float3(0x1.87de2ap-2f, 0x0p-1022f, 0x1.d906bcp-1f));
   CHECK(mesh.vertices[74].texcoords == float2(0x1.8p-2f, 0x1p+0f));
   CHECK(mesh.vertices[74].surface_index == 2);
   CHECK(mesh.vertices[75].position == float3(0x1.25e6ap+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.vertices[75].normal == float3(0x1.87de2ap-2f, 0x0p-1022f, 0x1.d906bcp-1f));
   CHECK(mesh.vertices[75].texcoords == float2(0x1.8p-2f, 0x0p-1022f));
   CHECK(mesh.vertices[75].surface_index == 2);
   CHECK(mesh.vertices[76].position == float3(-0x1.199bc4p-20f, 0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.vertices[76].normal == float3(-0x1.777a5cp-25f, 0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[76].texcoords == float2(0x1p-1f, 0x1p+0f));
   CHECK(mesh.vertices[76].surface_index == 2);
   CHECK(mesh.vertices[77].position == float3(-0x1.199bc4p-20f, -0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.vertices[77].normal == float3(-0x1.777a5cp-25f, 0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[77].texcoords == float2(0x1p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[77].surface_index == 2);
   CHECK(mesh.vertices[78].position == float3(-0x1.25e6a4p+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.vertices[78].normal == float3(-0x1.87de3p-2f, 0x0p-1022f, 0x1.d906bcp-1f));
   CHECK(mesh.vertices[78].texcoords == float2(0x1.4p-1f, 0x1p+0f));
   CHECK(mesh.vertices[78].surface_index == 2);
   CHECK(mesh.vertices[79].position ==
         float3(-0x1.25e6a4p+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.vertices[79].normal == float3(-0x1.87de3p-2f, 0x0p-1022f, 0x1.d906bcp-1f));
   CHECK(mesh.vertices[79].texcoords == float2(0x1.4p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[79].surface_index == 2);
   CHECK(mesh.vertices[80].position == float3(-0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.vertices[80].normal ==
         float3(-0x1.6a09e6p-1f, 0x0p-1022f, 0x1.6a09e6p-1f));
   CHECK(mesh.vertices[80].texcoords == float2(0x1.8p-1f, 0x1p+0f));
   CHECK(mesh.vertices[80].surface_index == 2);
   CHECK(mesh.vertices[81].position ==
         float3(-0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.vertices[81].normal ==
         float3(-0x1.6a09e6p-1f, 0x0p-1022f, 0x1.6a09e6p-1f));
   CHECK(mesh.vertices[81].texcoords == float2(0x1.8p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[81].surface_index == 2);
   CHECK(mesh.vertices[82].position == float3(-0x1.62c51p+4f, 0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.vertices[82].normal == float3(-0x1.d906cp-1f, 0x0p-1022f, 0x1.87de2p-2f));
   CHECK(mesh.vertices[82].texcoords == float2(0x1.cp-1f, 0x1p+0f));
   CHECK(mesh.vertices[82].surface_index == 2);
   CHECK(mesh.vertices[83].position == float3(-0x1.62c51p+4f, -0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.vertices[83].normal == float3(-0x1.d906cp-1f, 0x0p-1022f, 0x1.87de2p-2f));
   CHECK(mesh.vertices[83].texcoords == float2(0x1.cp-1f, 0x0p-1022f));
   CHECK(mesh.vertices[83].surface_index == 2);
   CHECK(mesh.vertices[84].position == float3(-0x1.8p+4f, 0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.vertices[84].normal == float3(-0x1p+0f, 0x0p-1022f, -0x1.777a5cp-24f));
   CHECK(mesh.vertices[84].texcoords == float2(0x1p+0f, 0x1p+0f));
   CHECK(mesh.vertices[84].surface_index == 2);
   CHECK(mesh.vertices[85].position == float3(-0x1.8p+4f, -0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.vertices[85].normal == float3(-0x1p+0f, 0x0p-1022f, -0x1.777a5cp-24f));
   CHECK(mesh.vertices[85].texcoords == float2(0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[85].surface_index == 2);
   CHECK(mesh.vertices[86].position == float3(-0x1.62c50cp+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.vertices[86].normal ==
         float3(-0x1.d906bcp-1f, 0x0p-1022f, -0x1.87de2ap-2f));
   CHECK(mesh.vertices[86].texcoords == float2(0x1.2p+0f, 0x1p+0f));
   CHECK(mesh.vertices[86].surface_index == 2);
   CHECK(mesh.vertices[87].position ==
         float3(-0x1.62c50cp+4f, -0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.vertices[87].normal ==
         float3(-0x1.d906bcp-1f, 0x0p-1022f, -0x1.87de2ap-2f));
   CHECK(mesh.vertices[87].texcoords == float2(0x1.2p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[87].surface_index == 2);
   CHECK(mesh.vertices[88].position == float3(-0x1.0f876ap+4f, 0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.vertices[88].normal ==
         float3(-0x1.6a09e2p-1f, 0x0p-1022f, -0x1.6a09eap-1f));
   CHECK(mesh.vertices[88].texcoords == float2(0x1.4p+0f, 0x1p+0f));
   CHECK(mesh.vertices[88].surface_index == 2);
   CHECK(mesh.vertices[89].position ==
         float3(-0x1.0f876ap+4f, -0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.vertices[89].normal ==
         float3(-0x1.6a09e2p-1f, 0x0p-1022f, -0x1.6a09eap-1f));
   CHECK(mesh.vertices[89].texcoords == float2(0x1.4p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[89].surface_index == 2);
   CHECK(mesh.vertices[90].position == float3(-0x1.25e69p+3f, 0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.vertices[90].normal ==
         float3(-0x1.87de16p-2f, 0x0p-1022f, -0x1.d906c2p-1f));
   CHECK(mesh.vertices[90].texcoords == float2(0x1.6p+0f, 0x1p+0f));
   CHECK(mesh.vertices[90].surface_index == 2);
   CHECK(mesh.vertices[91].position ==
         float3(-0x1.25e69p+3f, -0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.vertices[91].normal ==
         float3(-0x1.87de16p-2f, 0x0p-1022f, -0x1.d906c2p-1f));
   CHECK(mesh.vertices[91].texcoords == float2(0x1.6p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[91].surface_index == 2);
   CHECK(mesh.vertices[92].position == float3(0x1.334d44p-22f, 0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.vertices[92].normal == float3(0x1.99bc5cp-27f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[92].texcoords == float2(0x1.8p+0f, 0x1p+0f));
   CHECK(mesh.vertices[92].surface_index == 2);
   CHECK(mesh.vertices[93].position == float3(0x1.334d44p-22f, -0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.vertices[93].normal == float3(0x1.99bc5cp-27f, 0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[93].texcoords == float2(0x1.8p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[93].surface_index == 2);
   CHECK(mesh.vertices[94].position == float3(0x1.25e6a8p+3f, 0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.vertices[94].normal ==
         float3(0x1.87de36p-2f, 0x0p-1022f, -0x1.d906bap-1f));
   CHECK(mesh.vertices[94].texcoords == float2(0x1.ap+0f, 0x1p+0f));
   CHECK(mesh.vertices[94].surface_index == 2);
   CHECK(mesh.vertices[95].position ==
         float3(0x1.25e6a8p+3f, -0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.vertices[95].normal ==
         float3(0x1.87de36p-2f, 0x0p-1022f, -0x1.d906bap-1f));
   CHECK(mesh.vertices[95].texcoords == float2(0x1.ap+0f, 0x0p-1022f));
   CHECK(mesh.vertices[95].surface_index == 2);
   CHECK(mesh.vertices[96].position == float3(0x1.0f8772p+4f, 0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.vertices[96].normal ==
         float3(0x1.6a09eep-1f, 0x0p-1022f, -0x1.6a09dep-1f));
   CHECK(mesh.vertices[96].texcoords == float2(0x1.cp+0f, 0x1p+0f));
   CHECK(mesh.vertices[96].surface_index == 2);
   CHECK(mesh.vertices[97].position ==
         float3(0x1.0f8772p+4f, -0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.vertices[97].normal ==
         float3(0x1.6a09eep-1f, 0x0p-1022f, -0x1.6a09dep-1f));
   CHECK(mesh.vertices[97].texcoords == float2(0x1.cp+0f, 0x0p-1022f));
   CHECK(mesh.vertices[97].surface_index == 2);
   CHECK(mesh.vertices[98].position == float3(0x1.62c50ep+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.vertices[98].normal ==
         float3(0x1.d906bep-1f, 0x0p-1022f, -0x1.87de2ap-2f));
   CHECK(mesh.vertices[98].texcoords == float2(0x1.ep+0f, 0x1p+0f));
   CHECK(mesh.vertices[98].surface_index == 2);
   CHECK(mesh.vertices[99].position == float3(0x1.62c50ep+4f, -0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.vertices[99].normal ==
         float3(0x1.d906bep-1f, 0x0p-1022f, -0x1.87de2ap-2f));
   CHECK(mesh.vertices[99].texcoords == float2(0x1.ep+0f, 0x0p-1022f));
   CHECK(mesh.vertices[99].surface_index == 2);
   CHECK(mesh.vertices[100].position == float3(0x1.8p+4f, 0x1p+2f, 0x1.199bc4p-18f));
   CHECK(mesh.vertices[100].normal == float3(0x1p+0f, 0x0p-1022f, 0x1.777a5cp-23f));
   CHECK(mesh.vertices[100].texcoords == float2(0x1p+1f, 0x1p+0f));
   CHECK(mesh.vertices[100].surface_index == 2);
   CHECK(mesh.vertices[101].position == float3(0x1.8p+4f, -0x1p+2f, 0x1.199bc4p-18f));
   CHECK(mesh.vertices[101].normal == float3(0x1p+0f, 0x0p-1022f, 0x1.777a5cp-23f));
   CHECK(mesh.vertices[101].texcoords == float2(0x1p+1f, 0x0p-1022f));
   CHECK(mesh.vertices[101].surface_index == 2);
   CHECK(mesh.vertices[102].position == float3(0x1p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.vertices[102].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[102].texcoords == float2(0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[102].surface_index == 3);
   CHECK(mesh.vertices[103].position == float3(0x1p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.vertices[103].normal == float3(-0x1p+0f, 0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[103].texcoords == float2(0x0p-1022f, 0x0p-1022f));
   CHECK(mesh.vertices[103].surface_index == 3);
   CHECK(mesh.vertices[104].position ==
         float3(0x1.d906bcp+3f, -0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.vertices[104].normal ==
         float3(-0x1.d906bcp-1f, -0x0p-1022f, -0x1.87de2cp-2f));
   CHECK(mesh.vertices[104].texcoords == float2(0x1p-3f, 0x0p-1022f));
   CHECK(mesh.vertices[104].surface_index == 3);
   CHECK(mesh.vertices[105].position == float3(0x1.d906bcp+3f, 0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.vertices[105].normal ==
         float3(-0x1.d906bcp-1f, -0x0p-1022f, -0x1.87de2cp-2f));
   CHECK(mesh.vertices[105].texcoords == float2(0x1p-3f, 0x1p+0f));
   CHECK(mesh.vertices[105].surface_index == 3);
   CHECK(mesh.vertices[106].position ==
         float3(0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.vertices[106].normal ==
         float3(-0x1.6a09e6p-1f, -0x0p-1022f, -0x1.6a09e6p-1f));
   CHECK(mesh.vertices[106].texcoords == float2(0x1p-2f, 0x0p-1022f));
   CHECK(mesh.vertices[106].surface_index == 3);
   CHECK(mesh.vertices[107].position == float3(0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.vertices[107].normal ==
         float3(-0x1.6a09e6p-1f, -0x0p-1022f, -0x1.6a09e6p-1f));
   CHECK(mesh.vertices[107].texcoords == float2(0x1p-2f, 0x1p+0f));
   CHECK(mesh.vertices[107].surface_index == 3);
   CHECK(mesh.vertices[108].position ==
         float3(0x1.87de2ap+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.vertices[108].normal ==
         float3(-0x1.87de2ap-2f, -0x0p-1022f, -0x1.d906bcp-1f));
   CHECK(mesh.vertices[108].texcoords == float2(0x1.8p-2f, 0x0p-1022f));
   CHECK(mesh.vertices[108].surface_index == 3);
   CHECK(mesh.vertices[109].position == float3(0x1.87de2ap+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.vertices[109].normal ==
         float3(-0x1.87de2ap-2f, -0x0p-1022f, -0x1.d906bcp-1f));
   CHECK(mesh.vertices[109].texcoords == float2(0x1.8p-2f, 0x1p+0f));
   CHECK(mesh.vertices[109].surface_index == 3);
   CHECK(mesh.vertices[110].position == float3(-0x1.777a5cp-21f, -0x1p+2f, 0x1p+4f));
   CHECK(mesh.vertices[110].normal == float3(0x1.777a5cp-25f, -0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[110].texcoords == float2(0x1p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[110].surface_index == 3);
   CHECK(mesh.vertices[111].position == float3(-0x1.777a5cp-21f, 0x1p+2f, 0x1p+4f));
   CHECK(mesh.vertices[111].normal == float3(0x1.777a5cp-25f, -0x0p-1022f, -0x1p+0f));
   CHECK(mesh.vertices[111].texcoords == float2(0x1p-1f, 0x1p+0f));
   CHECK(mesh.vertices[111].surface_index == 3);
   CHECK(mesh.vertices[112].position ==
         float3(-0x1.87de3p+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.vertices[112].normal ==
         float3(0x1.87de3p-2f, -0x0p-1022f, -0x1.d906bcp-1f));
   CHECK(mesh.vertices[112].texcoords == float2(0x1.4p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[112].surface_index == 3);
   CHECK(mesh.vertices[113].position == float3(-0x1.87de3p+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.vertices[113].normal ==
         float3(0x1.87de3p-2f, -0x0p-1022f, -0x1.d906bcp-1f));
   CHECK(mesh.vertices[113].texcoords == float2(0x1.4p-1f, 0x1p+0f));
   CHECK(mesh.vertices[113].surface_index == 3);
   CHECK(mesh.vertices[114].position ==
         float3(-0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.vertices[114].normal ==
         float3(0x1.6a09e6p-1f, -0x0p-1022f, -0x1.6a09e6p-1f));
   CHECK(mesh.vertices[114].texcoords == float2(0x1.8p-1f, 0x0p-1022f));
   CHECK(mesh.vertices[114].surface_index == 3);
   CHECK(mesh.vertices[115].position ==
         float3(-0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.vertices[115].normal ==
         float3(0x1.6a09e6p-1f, -0x0p-1022f, -0x1.6a09e6p-1f));
   CHECK(mesh.vertices[115].texcoords == float2(0x1.8p-1f, 0x1p+0f));
   CHECK(mesh.vertices[115].surface_index == 3);
   CHECK(mesh.vertices[116].position == float3(-0x1.d906cp+3f, -0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.vertices[116].normal ==
         float3(0x1.d906cp-1f, -0x0p-1022f, -0x1.87de2p-2f));
   CHECK(mesh.vertices[116].texcoords == float2(0x1.cp-1f, 0x0p-1022f));
   CHECK(mesh.vertices[116].surface_index == 3);
   CHECK(mesh.vertices[117].position == float3(-0x1.d906cp+3f, 0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.vertices[117].normal ==
         float3(0x1.d906cp-1f, -0x0p-1022f, -0x1.87de2p-2f));
   CHECK(mesh.vertices[117].texcoords == float2(0x1.cp-1f, 0x1p+0f));
   CHECK(mesh.vertices[117].surface_index == 3);
   CHECK(mesh.vertices[118].position == float3(-0x1p+4f, -0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.vertices[118].normal == float3(0x1p+0f, -0x0p-1022f, 0x1.777a5cp-24f));
   CHECK(mesh.vertices[118].texcoords == float2(0x1p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[118].surface_index == 3);
   CHECK(mesh.vertices[119].position == float3(-0x1p+4f, 0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.vertices[119].normal == float3(0x1p+0f, -0x0p-1022f, 0x1.777a5cp-24f));
   CHECK(mesh.vertices[119].texcoords == float2(0x1p+0f, 0x1p+0f));
   CHECK(mesh.vertices[119].surface_index == 3);
   CHECK(mesh.vertices[120].position ==
         float3(-0x1.d906bcp+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.vertices[120].normal ==
         float3(0x1.d906bcp-1f, -0x0p-1022f, 0x1.87de2ap-2f));
   CHECK(mesh.vertices[120].texcoords == float2(0x1.2p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[120].surface_index == 3);
   CHECK(mesh.vertices[121].position ==
         float3(-0x1.d906bcp+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.vertices[121].normal ==
         float3(0x1.d906bcp-1f, -0x0p-1022f, 0x1.87de2ap-2f));
   CHECK(mesh.vertices[121].texcoords == float2(0x1.2p+0f, 0x1p+0f));
   CHECK(mesh.vertices[121].surface_index == 3);
   CHECK(mesh.vertices[122].position ==
         float3(-0x1.6a09e2p+3f, -0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.vertices[122].normal ==
         float3(0x1.6a09e2p-1f, -0x0p-1022f, 0x1.6a09eap-1f));
   CHECK(mesh.vertices[122].texcoords == float2(0x1.4p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[122].surface_index == 3);
   CHECK(mesh.vertices[123].position ==
         float3(-0x1.6a09e2p+3f, 0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.vertices[123].normal ==
         float3(0x1.6a09e2p-1f, -0x0p-1022f, 0x1.6a09eap-1f));
   CHECK(mesh.vertices[123].texcoords == float2(0x1.4p+0f, 0x1p+0f));
   CHECK(mesh.vertices[123].surface_index == 3);
   CHECK(mesh.vertices[124].position ==
         float3(-0x1.87de16p+2f, -0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.vertices[124].normal ==
         float3(0x1.87de16p-2f, -0x0p-1022f, 0x1.d906c2p-1f));
   CHECK(mesh.vertices[124].texcoords == float2(0x1.6p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[124].surface_index == 3);
   CHECK(mesh.vertices[125].position ==
         float3(-0x1.87de16p+2f, 0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.vertices[125].normal ==
         float3(0x1.87de16p-2f, -0x0p-1022f, 0x1.d906c2p-1f));
   CHECK(mesh.vertices[125].texcoords == float2(0x1.6p+0f, 0x1p+0f));
   CHECK(mesh.vertices[125].surface_index == 3);
   CHECK(mesh.vertices[126].position == float3(0x1.99bc5cp-23f, -0x1p+2f, -0x1p+4f));
   CHECK(mesh.vertices[126].normal == float3(-0x1.99bc5cp-27f, -0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[126].texcoords == float2(0x1.8p+0f, 0x0p-1022f));
   CHECK(mesh.vertices[126].surface_index == 3);
   CHECK(mesh.vertices[127].position == float3(0x1.99bc5cp-23f, 0x1p+2f, -0x1p+4f));
   CHECK(mesh.vertices[127].normal == float3(-0x1.99bc5cp-27f, -0x0p-1022f, 0x1p+0f));
   CHECK(mesh.vertices[127].texcoords == float2(0x1.8p+0f, 0x1p+0f));
   CHECK(mesh.vertices[127].surface_index == 3);
   CHECK(mesh.vertices[128].position ==
         float3(0x1.87de36p+2f, -0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.vertices[128].normal ==
         float3(-0x1.87de36p-2f, -0x0p-1022f, 0x1.d906bap-1f));
   CHECK(mesh.vertices[128].texcoords == float2(0x1.ap+0f, 0x0p-1022f));
   CHECK(mesh.vertices[128].surface_index == 3);
   CHECK(mesh.vertices[129].position ==
         float3(0x1.87de36p+2f, 0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.vertices[129].normal ==
         float3(-0x1.87de36p-2f, -0x0p-1022f, 0x1.d906bap-1f));
   CHECK(mesh.vertices[129].texcoords == float2(0x1.ap+0f, 0x1p+0f));
   CHECK(mesh.vertices[129].surface_index == 3);
   CHECK(mesh.vertices[130].position ==
         float3(0x1.6a09eep+3f, -0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.vertices[130].normal ==
         float3(-0x1.6a09eep-1f, -0x0p-1022f, 0x1.6a09dep-1f));
   CHECK(mesh.vertices[130].texcoords == float2(0x1.cp+0f, 0x0p-1022f));
   CHECK(mesh.vertices[130].surface_index == 3);
   CHECK(mesh.vertices[131].position ==
         float3(0x1.6a09eep+3f, 0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.vertices[131].normal ==
         float3(-0x1.6a09eep-1f, -0x0p-1022f, 0x1.6a09dep-1f));
   CHECK(mesh.vertices[131].texcoords == float2(0x1.cp+0f, 0x1p+0f));
   CHECK(mesh.vertices[131].surface_index == 3);
   CHECK(mesh.vertices[132].position ==
         float3(0x1.d906bep+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.vertices[132].normal ==
         float3(-0x1.d906bep-1f, -0x0p-1022f, 0x1.87de2ap-2f));
   CHECK(mesh.vertices[132].texcoords == float2(0x1.ep+0f, 0x0p-1022f));
   CHECK(mesh.vertices[132].surface_index == 3);
   CHECK(mesh.vertices[133].position ==
         float3(0x1.d906bep+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.vertices[133].normal ==
         float3(-0x1.d906bep-1f, -0x0p-1022f, 0x1.87de2ap-2f));
   CHECK(mesh.vertices[133].texcoords == float2(0x1.ep+0f, 0x1p+0f));
   CHECK(mesh.vertices[133].surface_index == 3);
   CHECK(mesh.vertices[134].position == float3(0x1p+4f, -0x1p+2f, 0x1.777a5cp-19f));
   CHECK(mesh.vertices[134].normal == float3(-0x1p+0f, -0x0p-1022f, -0x1.777a5cp-23f));
   CHECK(mesh.vertices[134].texcoords == float2(0x1p+1f, 0x0p-1022f));
   CHECK(mesh.vertices[134].surface_index == 3);
   CHECK(mesh.vertices[135].position == float3(0x1p+4f, 0x1p+2f, 0x1.777a5cp-19f));
   CHECK(mesh.vertices[135].normal == float3(-0x1p+0f, -0x0p-1022f, -0x1.777a5cp-23f));
   CHECK(mesh.vertices[135].texcoords == float2(0x1p+1f, 0x1p+0f));
   CHECK(mesh.vertices[135].surface_index == 3);

   REQUIRE(mesh.triangles.size() == 128);

   CHECK(mesh.triangles[0] == std::array<uint16, 3>{2, 3, 0});
   CHECK(mesh.triangles[1] == std::array<uint16, 3>{2, 0, 1});
   CHECK(mesh.triangles[2] == std::array<uint16, 3>{4, 5, 3});
   CHECK(mesh.triangles[3] == std::array<uint16, 3>{4, 3, 2});
   CHECK(mesh.triangles[4] == std::array<uint16, 3>{6, 7, 5});
   CHECK(mesh.triangles[5] == std::array<uint16, 3>{6, 5, 4});
   CHECK(mesh.triangles[6] == std::array<uint16, 3>{8, 9, 7});
   CHECK(mesh.triangles[7] == std::array<uint16, 3>{8, 7, 6});
   CHECK(mesh.triangles[8] == std::array<uint16, 3>{10, 11, 9});
   CHECK(mesh.triangles[9] == std::array<uint16, 3>{10, 9, 8});
   CHECK(mesh.triangles[10] == std::array<uint16, 3>{12, 13, 11});
   CHECK(mesh.triangles[11] == std::array<uint16, 3>{12, 11, 10});
   CHECK(mesh.triangles[12] == std::array<uint16, 3>{14, 15, 13});
   CHECK(mesh.triangles[13] == std::array<uint16, 3>{14, 13, 12});
   CHECK(mesh.triangles[14] == std::array<uint16, 3>{16, 17, 15});
   CHECK(mesh.triangles[15] == std::array<uint16, 3>{16, 15, 14});
   CHECK(mesh.triangles[16] == std::array<uint16, 3>{18, 19, 17});
   CHECK(mesh.triangles[17] == std::array<uint16, 3>{18, 17, 16});
   CHECK(mesh.triangles[18] == std::array<uint16, 3>{20, 21, 19});
   CHECK(mesh.triangles[19] == std::array<uint16, 3>{20, 19, 18});
   CHECK(mesh.triangles[20] == std::array<uint16, 3>{22, 23, 21});
   CHECK(mesh.triangles[21] == std::array<uint16, 3>{22, 21, 20});
   CHECK(mesh.triangles[22] == std::array<uint16, 3>{24, 25, 23});
   CHECK(mesh.triangles[23] == std::array<uint16, 3>{24, 23, 22});
   CHECK(mesh.triangles[24] == std::array<uint16, 3>{26, 27, 25});
   CHECK(mesh.triangles[25] == std::array<uint16, 3>{26, 25, 24});
   CHECK(mesh.triangles[26] == std::array<uint16, 3>{28, 29, 27});
   CHECK(mesh.triangles[27] == std::array<uint16, 3>{28, 27, 26});
   CHECK(mesh.triangles[28] == std::array<uint16, 3>{30, 31, 29});
   CHECK(mesh.triangles[29] == std::array<uint16, 3>{30, 29, 28});
   CHECK(mesh.triangles[30] == std::array<uint16, 3>{32, 33, 31});
   CHECK(mesh.triangles[31] == std::array<uint16, 3>{32, 31, 30});
   CHECK(mesh.triangles[32] == std::array<uint16, 3>{34, 35, 36});
   CHECK(mesh.triangles[33] == std::array<uint16, 3>{34, 36, 37});
   CHECK(mesh.triangles[34] == std::array<uint16, 3>{37, 36, 38});
   CHECK(mesh.triangles[35] == std::array<uint16, 3>{37, 38, 39});
   CHECK(mesh.triangles[36] == std::array<uint16, 3>{39, 38, 40});
   CHECK(mesh.triangles[37] == std::array<uint16, 3>{39, 40, 41});
   CHECK(mesh.triangles[38] == std::array<uint16, 3>{41, 40, 42});
   CHECK(mesh.triangles[39] == std::array<uint16, 3>{41, 42, 43});
   CHECK(mesh.triangles[40] == std::array<uint16, 3>{43, 42, 44});
   CHECK(mesh.triangles[41] == std::array<uint16, 3>{43, 44, 45});
   CHECK(mesh.triangles[42] == std::array<uint16, 3>{45, 44, 46});
   CHECK(mesh.triangles[43] == std::array<uint16, 3>{45, 46, 47});
   CHECK(mesh.triangles[44] == std::array<uint16, 3>{47, 46, 48});
   CHECK(mesh.triangles[45] == std::array<uint16, 3>{47, 48, 49});
   CHECK(mesh.triangles[46] == std::array<uint16, 3>{49, 48, 50});
   CHECK(mesh.triangles[47] == std::array<uint16, 3>{49, 50, 51});
   CHECK(mesh.triangles[48] == std::array<uint16, 3>{51, 50, 52});
   CHECK(mesh.triangles[49] == std::array<uint16, 3>{51, 52, 53});
   CHECK(mesh.triangles[50] == std::array<uint16, 3>{53, 52, 54});
   CHECK(mesh.triangles[51] == std::array<uint16, 3>{53, 54, 55});
   CHECK(mesh.triangles[52] == std::array<uint16, 3>{55, 54, 56});
   CHECK(mesh.triangles[53] == std::array<uint16, 3>{55, 56, 57});
   CHECK(mesh.triangles[54] == std::array<uint16, 3>{57, 56, 58});
   CHECK(mesh.triangles[55] == std::array<uint16, 3>{57, 58, 59});
   CHECK(mesh.triangles[56] == std::array<uint16, 3>{59, 58, 60});
   CHECK(mesh.triangles[57] == std::array<uint16, 3>{59, 60, 61});
   CHECK(mesh.triangles[58] == std::array<uint16, 3>{61, 60, 62});
   CHECK(mesh.triangles[59] == std::array<uint16, 3>{61, 62, 63});
   CHECK(mesh.triangles[60] == std::array<uint16, 3>{63, 62, 64});
   CHECK(mesh.triangles[61] == std::array<uint16, 3>{63, 64, 65});
   CHECK(mesh.triangles[62] == std::array<uint16, 3>{65, 64, 66});
   CHECK(mesh.triangles[63] == std::array<uint16, 3>{65, 66, 67});
   CHECK(mesh.triangles[64] == std::array<uint16, 3>{70, 71, 68});
   CHECK(mesh.triangles[65] == std::array<uint16, 3>{70, 68, 69});
   CHECK(mesh.triangles[66] == std::array<uint16, 3>{72, 73, 71});
   CHECK(mesh.triangles[67] == std::array<uint16, 3>{72, 71, 70});
   CHECK(mesh.triangles[68] == std::array<uint16, 3>{74, 75, 73});
   CHECK(mesh.triangles[69] == std::array<uint16, 3>{74, 73, 72});
   CHECK(mesh.triangles[70] == std::array<uint16, 3>{76, 77, 75});
   CHECK(mesh.triangles[71] == std::array<uint16, 3>{76, 75, 74});
   CHECK(mesh.triangles[72] == std::array<uint16, 3>{78, 79, 77});
   CHECK(mesh.triangles[73] == std::array<uint16, 3>{78, 77, 76});
   CHECK(mesh.triangles[74] == std::array<uint16, 3>{80, 81, 79});
   CHECK(mesh.triangles[75] == std::array<uint16, 3>{80, 79, 78});
   CHECK(mesh.triangles[76] == std::array<uint16, 3>{82, 83, 81});
   CHECK(mesh.triangles[77] == std::array<uint16, 3>{82, 81, 80});
   CHECK(mesh.triangles[78] == std::array<uint16, 3>{84, 85, 83});
   CHECK(mesh.triangles[79] == std::array<uint16, 3>{84, 83, 82});
   CHECK(mesh.triangles[80] == std::array<uint16, 3>{86, 87, 85});
   CHECK(mesh.triangles[81] == std::array<uint16, 3>{86, 85, 84});
   CHECK(mesh.triangles[82] == std::array<uint16, 3>{88, 89, 87});
   CHECK(mesh.triangles[83] == std::array<uint16, 3>{88, 87, 86});
   CHECK(mesh.triangles[84] == std::array<uint16, 3>{90, 91, 89});
   CHECK(mesh.triangles[85] == std::array<uint16, 3>{90, 89, 88});
   CHECK(mesh.triangles[86] == std::array<uint16, 3>{92, 93, 91});
   CHECK(mesh.triangles[87] == std::array<uint16, 3>{92, 91, 90});
   CHECK(mesh.triangles[88] == std::array<uint16, 3>{94, 95, 93});
   CHECK(mesh.triangles[89] == std::array<uint16, 3>{94, 93, 92});
   CHECK(mesh.triangles[90] == std::array<uint16, 3>{96, 97, 95});
   CHECK(mesh.triangles[91] == std::array<uint16, 3>{96, 95, 94});
   CHECK(mesh.triangles[92] == std::array<uint16, 3>{98, 99, 97});
   CHECK(mesh.triangles[93] == std::array<uint16, 3>{98, 97, 96});
   CHECK(mesh.triangles[94] == std::array<uint16, 3>{100, 101, 99});
   CHECK(mesh.triangles[95] == std::array<uint16, 3>{100, 99, 98});
   CHECK(mesh.triangles[96] == std::array<uint16, 3>{102, 103, 104});
   CHECK(mesh.triangles[97] == std::array<uint16, 3>{102, 104, 105});
   CHECK(mesh.triangles[98] == std::array<uint16, 3>{105, 104, 106});
   CHECK(mesh.triangles[99] == std::array<uint16, 3>{105, 106, 107});
   CHECK(mesh.triangles[100] == std::array<uint16, 3>{107, 106, 108});
   CHECK(mesh.triangles[101] == std::array<uint16, 3>{107, 108, 109});
   CHECK(mesh.triangles[102] == std::array<uint16, 3>{109, 108, 110});
   CHECK(mesh.triangles[103] == std::array<uint16, 3>{109, 110, 111});
   CHECK(mesh.triangles[104] == std::array<uint16, 3>{111, 110, 112});
   CHECK(mesh.triangles[105] == std::array<uint16, 3>{111, 112, 113});
   CHECK(mesh.triangles[106] == std::array<uint16, 3>{113, 112, 114});
   CHECK(mesh.triangles[107] == std::array<uint16, 3>{113, 114, 115});
   CHECK(mesh.triangles[108] == std::array<uint16, 3>{115, 114, 116});
   CHECK(mesh.triangles[109] == std::array<uint16, 3>{115, 116, 117});
   CHECK(mesh.triangles[110] == std::array<uint16, 3>{117, 116, 118});
   CHECK(mesh.triangles[111] == std::array<uint16, 3>{117, 118, 119});
   CHECK(mesh.triangles[112] == std::array<uint16, 3>{119, 118, 120});
   CHECK(mesh.triangles[113] == std::array<uint16, 3>{119, 120, 121});
   CHECK(mesh.triangles[114] == std::array<uint16, 3>{121, 120, 122});
   CHECK(mesh.triangles[115] == std::array<uint16, 3>{121, 122, 123});
   CHECK(mesh.triangles[116] == std::array<uint16, 3>{123, 122, 124});
   CHECK(mesh.triangles[117] == std::array<uint16, 3>{123, 124, 125});
   CHECK(mesh.triangles[118] == std::array<uint16, 3>{125, 124, 126});
   CHECK(mesh.triangles[119] == std::array<uint16, 3>{125, 126, 127});
   CHECK(mesh.triangles[120] == std::array<uint16, 3>{127, 126, 128});
   CHECK(mesh.triangles[121] == std::array<uint16, 3>{127, 128, 129});
   CHECK(mesh.triangles[122] == std::array<uint16, 3>{129, 128, 130});
   CHECK(mesh.triangles[123] == std::array<uint16, 3>{129, 130, 131});
   CHECK(mesh.triangles[124] == std::array<uint16, 3>{131, 130, 132});
   CHECK(mesh.triangles[125] == std::array<uint16, 3>{131, 132, 133});
   CHECK(mesh.triangles[126] == std::array<uint16, 3>{133, 132, 134});
   CHECK(mesh.triangles[127] == std::array<uint16, 3>{133, 134, 135});

   REQUIRE(mesh.occluders.size() == 64);

   CHECK(mesh.occluders[0] == std::array<uint16, 4>{2, 3, 0, 1});
   CHECK(mesh.occluders[1] == std::array<uint16, 4>{4, 5, 3, 2});
   CHECK(mesh.occluders[2] == std::array<uint16, 4>{6, 7, 5, 4});
   CHECK(mesh.occluders[3] == std::array<uint16, 4>{8, 9, 7, 6});
   CHECK(mesh.occluders[4] == std::array<uint16, 4>{10, 11, 9, 8});
   CHECK(mesh.occluders[5] == std::array<uint16, 4>{12, 13, 11, 10});
   CHECK(mesh.occluders[6] == std::array<uint16, 4>{14, 15, 13, 12});
   CHECK(mesh.occluders[7] == std::array<uint16, 4>{16, 17, 15, 14});
   CHECK(mesh.occluders[8] == std::array<uint16, 4>{18, 19, 17, 16});
   CHECK(mesh.occluders[9] == std::array<uint16, 4>{20, 21, 19, 18});
   CHECK(mesh.occluders[10] == std::array<uint16, 4>{22, 23, 21, 20});
   CHECK(mesh.occluders[11] == std::array<uint16, 4>{24, 25, 23, 22});
   CHECK(mesh.occluders[12] == std::array<uint16, 4>{26, 27, 25, 24});
   CHECK(mesh.occluders[13] == std::array<uint16, 4>{28, 29, 27, 26});
   CHECK(mesh.occluders[14] == std::array<uint16, 4>{30, 31, 29, 28});
   CHECK(mesh.occluders[15] == std::array<uint16, 4>{32, 33, 31, 30});
   CHECK(mesh.occluders[16] == std::array<uint16, 4>{34, 35, 36, 37});
   CHECK(mesh.occluders[17] == std::array<uint16, 4>{37, 36, 38, 39});
   CHECK(mesh.occluders[18] == std::array<uint16, 4>{39, 38, 40, 41});
   CHECK(mesh.occluders[19] == std::array<uint16, 4>{41, 40, 42, 43});
   CHECK(mesh.occluders[20] == std::array<uint16, 4>{43, 42, 44, 45});
   CHECK(mesh.occluders[21] == std::array<uint16, 4>{45, 44, 46, 47});
   CHECK(mesh.occluders[22] == std::array<uint16, 4>{47, 46, 48, 49});
   CHECK(mesh.occluders[23] == std::array<uint16, 4>{49, 48, 50, 51});
   CHECK(mesh.occluders[24] == std::array<uint16, 4>{51, 50, 52, 53});
   CHECK(mesh.occluders[25] == std::array<uint16, 4>{53, 52, 54, 55});
   CHECK(mesh.occluders[26] == std::array<uint16, 4>{55, 54, 56, 57});
   CHECK(mesh.occluders[27] == std::array<uint16, 4>{57, 56, 58, 59});
   CHECK(mesh.occluders[28] == std::array<uint16, 4>{59, 58, 60, 61});
   CHECK(mesh.occluders[29] == std::array<uint16, 4>{61, 60, 62, 63});
   CHECK(mesh.occluders[30] == std::array<uint16, 4>{63, 62, 64, 65});
   CHECK(mesh.occluders[31] == std::array<uint16, 4>{65, 64, 66, 67});
   CHECK(mesh.occluders[32] == std::array<uint16, 4>{70, 71, 68, 69});
   CHECK(mesh.occluders[33] == std::array<uint16, 4>{72, 73, 71, 70});
   CHECK(mesh.occluders[34] == std::array<uint16, 4>{74, 75, 73, 72});
   CHECK(mesh.occluders[35] == std::array<uint16, 4>{76, 77, 75, 74});
   CHECK(mesh.occluders[36] == std::array<uint16, 4>{78, 79, 77, 76});
   CHECK(mesh.occluders[37] == std::array<uint16, 4>{80, 81, 79, 78});
   CHECK(mesh.occluders[38] == std::array<uint16, 4>{82, 83, 81, 80});
   CHECK(mesh.occluders[39] == std::array<uint16, 4>{84, 85, 83, 82});
   CHECK(mesh.occluders[40] == std::array<uint16, 4>{86, 87, 85, 84});
   CHECK(mesh.occluders[41] == std::array<uint16, 4>{88, 89, 87, 86});
   CHECK(mesh.occluders[42] == std::array<uint16, 4>{90, 91, 89, 88});
   CHECK(mesh.occluders[43] == std::array<uint16, 4>{92, 93, 91, 90});
   CHECK(mesh.occluders[44] == std::array<uint16, 4>{94, 95, 93, 92});
   CHECK(mesh.occluders[45] == std::array<uint16, 4>{96, 97, 95, 94});
   CHECK(mesh.occluders[46] == std::array<uint16, 4>{98, 99, 97, 96});
   CHECK(mesh.occluders[47] == std::array<uint16, 4>{100, 101, 99, 98});
   CHECK(mesh.occluders[48] == std::array<uint16, 4>{102, 103, 104, 105});
   CHECK(mesh.occluders[49] == std::array<uint16, 4>{105, 104, 106, 107});
   CHECK(mesh.occluders[50] == std::array<uint16, 4>{107, 106, 108, 109});
   CHECK(mesh.occluders[51] == std::array<uint16, 4>{109, 108, 110, 111});
   CHECK(mesh.occluders[52] == std::array<uint16, 4>{111, 110, 112, 113});
   CHECK(mesh.occluders[53] == std::array<uint16, 4>{113, 112, 114, 115});
   CHECK(mesh.occluders[54] == std::array<uint16, 4>{115, 114, 116, 117});
   CHECK(mesh.occluders[55] == std::array<uint16, 4>{117, 116, 118, 119});
   CHECK(mesh.occluders[56] == std::array<uint16, 4>{119, 118, 120, 121});
   CHECK(mesh.occluders[57] == std::array<uint16, 4>{121, 120, 122, 123});
   CHECK(mesh.occluders[58] == std::array<uint16, 4>{123, 122, 124, 125});
   CHECK(mesh.occluders[59] == std::array<uint16, 4>{125, 124, 126, 127});
   CHECK(mesh.occluders[60] == std::array<uint16, 4>{127, 126, 128, 129});
   CHECK(mesh.occluders[61] == std::array<uint16, 4>{129, 128, 130, 131});
   CHECK(mesh.occluders[62] == std::array<uint16, 4>{131, 130, 132, 133});
   CHECK(mesh.occluders[63] == std::array<uint16, 4>{133, 132, 134, 135});

   REQUIRE(mesh.collision_vertices.size() == 128);

   CHECK(mesh.collision_vertices[0].position == float3(0x1.8p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.collision_vertices[0].surface_index == 0);
   CHECK(mesh.collision_vertices[1].position == float3(0x1p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.collision_vertices[1].surface_index == 0);
   CHECK(mesh.collision_vertices[2].position ==
         float3(0x1.d906bcp+3f, 0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.collision_vertices[2].surface_index == 0);
   CHECK(mesh.collision_vertices[3].position ==
         float3(0x1.62c50cp+4f, 0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[3].surface_index == 0);
   CHECK(mesh.collision_vertices[4].position ==
         float3(0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.collision_vertices[4].surface_index == 0);
   CHECK(mesh.collision_vertices[5].position ==
         float3(0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.collision_vertices[5].surface_index == 0);
   CHECK(mesh.collision_vertices[6].position ==
         float3(0x1.87de2ap+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.collision_vertices[6].surface_index == 0);
   CHECK(mesh.collision_vertices[7].position ==
         float3(0x1.25e6ap+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[7].surface_index == 0);
   CHECK(mesh.collision_vertices[8].position ==
         float3(-0x1.777a5cp-21f, 0x1p+2f, 0x1p+4f));
   CHECK(mesh.collision_vertices[8].surface_index == 0);
   CHECK(mesh.collision_vertices[9].position ==
         float3(-0x1.199bc4p-20f, 0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.collision_vertices[9].surface_index == 0);
   CHECK(mesh.collision_vertices[10].position ==
         float3(-0x1.87de3p+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.collision_vertices[10].surface_index == 0);
   CHECK(mesh.collision_vertices[11].position ==
         float3(-0x1.25e6a4p+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[11].surface_index == 0);
   CHECK(mesh.collision_vertices[12].position ==
         float3(-0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.collision_vertices[12].surface_index == 0);
   CHECK(mesh.collision_vertices[13].position ==
         float3(-0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.collision_vertices[13].surface_index == 0);
   CHECK(mesh.collision_vertices[14].position ==
         float3(-0x1.d906cp+3f, 0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.collision_vertices[14].surface_index == 0);
   CHECK(mesh.collision_vertices[15].position ==
         float3(-0x1.62c51p+4f, 0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.collision_vertices[15].surface_index == 0);
   CHECK(mesh.collision_vertices[16].position ==
         float3(-0x1p+4f, 0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.collision_vertices[16].surface_index == 0);
   CHECK(mesh.collision_vertices[17].position ==
         float3(-0x1.8p+4f, 0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.collision_vertices[17].surface_index == 0);
   CHECK(mesh.collision_vertices[18].position ==
         float3(-0x1.d906bcp+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.collision_vertices[18].surface_index == 0);
   CHECK(mesh.collision_vertices[19].position ==
         float3(-0x1.62c50cp+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[19].surface_index == 0);
   CHECK(mesh.collision_vertices[20].position ==
         float3(-0x1.6a09e2p+3f, 0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.collision_vertices[20].surface_index == 0);
   CHECK(mesh.collision_vertices[21].position ==
         float3(-0x1.0f876ap+4f, 0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.collision_vertices[21].surface_index == 0);
   CHECK(mesh.collision_vertices[22].position ==
         float3(-0x1.87de16p+2f, 0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.collision_vertices[22].surface_index == 0);
   CHECK(mesh.collision_vertices[23].position ==
         float3(-0x1.25e69p+3f, 0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.collision_vertices[23].surface_index == 0);
   CHECK(mesh.collision_vertices[24].position ==
         float3(0x1.99bc5cp-23f, 0x1p+2f, -0x1p+4f));
   CHECK(mesh.collision_vertices[24].surface_index == 0);
   CHECK(mesh.collision_vertices[25].position ==
         float3(0x1.334d44p-22f, 0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.collision_vertices[25].surface_index == 0);
   CHECK(mesh.collision_vertices[26].position ==
         float3(0x1.87de36p+2f, 0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.collision_vertices[26].surface_index == 0);
   CHECK(mesh.collision_vertices[27].position ==
         float3(0x1.25e6a8p+3f, 0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[27].surface_index == 0);
   CHECK(mesh.collision_vertices[28].position ==
         float3(0x1.6a09eep+3f, 0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.collision_vertices[28].surface_index == 0);
   CHECK(mesh.collision_vertices[29].position ==
         float3(0x1.0f8772p+4f, 0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.collision_vertices[29].surface_index == 0);
   CHECK(mesh.collision_vertices[30].position ==
         float3(0x1.d906bep+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.collision_vertices[30].surface_index == 0);
   CHECK(mesh.collision_vertices[31].position ==
         float3(0x1.62c50ep+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[31].surface_index == 0);
   CHECK(mesh.collision_vertices[32].position == float3(0x1p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.collision_vertices[32].surface_index == 1);
   CHECK(mesh.collision_vertices[33].position ==
         float3(0x1.8p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.collision_vertices[33].surface_index == 1);
   CHECK(mesh.collision_vertices[34].position ==
         float3(0x1.62c50cp+4f, -0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[34].surface_index == 1);
   CHECK(mesh.collision_vertices[35].position ==
         float3(0x1.d906bcp+3f, -0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.collision_vertices[35].surface_index == 1);
   CHECK(mesh.collision_vertices[36].position ==
         float3(0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.collision_vertices[36].surface_index == 1);
   CHECK(mesh.collision_vertices[37].position ==
         float3(0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.collision_vertices[37].surface_index == 1);
   CHECK(mesh.collision_vertices[38].position ==
         float3(0x1.25e6ap+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[38].surface_index == 1);
   CHECK(mesh.collision_vertices[39].position ==
         float3(0x1.87de2ap+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.collision_vertices[39].surface_index == 1);
   CHECK(mesh.collision_vertices[40].position ==
         float3(-0x1.199bc4p-20f, -0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.collision_vertices[40].surface_index == 1);
   CHECK(mesh.collision_vertices[41].position ==
         float3(-0x1.777a5cp-21f, -0x1p+2f, 0x1p+4f));
   CHECK(mesh.collision_vertices[41].surface_index == 1);
   CHECK(mesh.collision_vertices[42].position ==
         float3(-0x1.25e6a4p+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[42].surface_index == 1);
   CHECK(mesh.collision_vertices[43].position ==
         float3(-0x1.87de3p+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.collision_vertices[43].surface_index == 1);
   CHECK(mesh.collision_vertices[44].position ==
         float3(-0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.collision_vertices[44].surface_index == 1);
   CHECK(mesh.collision_vertices[45].position ==
         float3(-0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.collision_vertices[45].surface_index == 1);
   CHECK(mesh.collision_vertices[46].position ==
         float3(-0x1.62c51p+4f, -0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.collision_vertices[46].surface_index == 1);
   CHECK(mesh.collision_vertices[47].position ==
         float3(-0x1.d906cp+3f, -0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.collision_vertices[47].surface_index == 1);
   CHECK(mesh.collision_vertices[48].position ==
         float3(-0x1.8p+4f, -0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.collision_vertices[48].surface_index == 1);
   CHECK(mesh.collision_vertices[49].position ==
         float3(-0x1p+4f, -0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.collision_vertices[49].surface_index == 1);
   CHECK(mesh.collision_vertices[50].position ==
         float3(-0x1.62c50cp+4f, -0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[50].surface_index == 1);
   CHECK(mesh.collision_vertices[51].position ==
         float3(-0x1.d906bcp+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.collision_vertices[51].surface_index == 1);
   CHECK(mesh.collision_vertices[52].position ==
         float3(-0x1.0f876ap+4f, -0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.collision_vertices[52].surface_index == 1);
   CHECK(mesh.collision_vertices[53].position ==
         float3(-0x1.6a09e2p+3f, -0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.collision_vertices[53].surface_index == 1);
   CHECK(mesh.collision_vertices[54].position ==
         float3(-0x1.25e69p+3f, -0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.collision_vertices[54].surface_index == 1);
   CHECK(mesh.collision_vertices[55].position ==
         float3(-0x1.87de16p+2f, -0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.collision_vertices[55].surface_index == 1);
   CHECK(mesh.collision_vertices[56].position ==
         float3(0x1.334d44p-22f, -0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.collision_vertices[56].surface_index == 1);
   CHECK(mesh.collision_vertices[57].position ==
         float3(0x1.99bc5cp-23f, -0x1p+2f, -0x1p+4f));
   CHECK(mesh.collision_vertices[57].surface_index == 1);
   CHECK(mesh.collision_vertices[58].position ==
         float3(0x1.25e6a8p+3f, -0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[58].surface_index == 1);
   CHECK(mesh.collision_vertices[59].position ==
         float3(0x1.87de36p+2f, -0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.collision_vertices[59].surface_index == 1);
   CHECK(mesh.collision_vertices[60].position ==
         float3(0x1.0f8772p+4f, -0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.collision_vertices[60].surface_index == 1);
   CHECK(mesh.collision_vertices[61].position ==
         float3(0x1.6a09eep+3f, -0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.collision_vertices[61].surface_index == 1);
   CHECK(mesh.collision_vertices[62].position ==
         float3(0x1.62c50ep+4f, -0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[62].surface_index == 1);
   CHECK(mesh.collision_vertices[63].position ==
         float3(0x1.d906bep+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.collision_vertices[63].surface_index == 1);
   CHECK(mesh.collision_vertices[64].position ==
         float3(0x1.8p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.collision_vertices[64].surface_index == 2);
   CHECK(mesh.collision_vertices[65].position == float3(0x1.8p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.collision_vertices[65].surface_index == 2);
   CHECK(mesh.collision_vertices[66].position ==
         float3(0x1.62c50cp+4f, 0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[66].surface_index == 2);
   CHECK(mesh.collision_vertices[67].position ==
         float3(0x1.62c50cp+4f, -0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[67].surface_index == 2);
   CHECK(mesh.collision_vertices[68].position ==
         float3(0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.collision_vertices[68].surface_index == 2);
   CHECK(mesh.collision_vertices[69].position ==
         float3(0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.collision_vertices[69].surface_index == 2);
   CHECK(mesh.collision_vertices[70].position ==
         float3(0x1.25e6ap+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[70].surface_index == 2);
   CHECK(mesh.collision_vertices[71].position ==
         float3(0x1.25e6ap+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[71].surface_index == 2);
   CHECK(mesh.collision_vertices[72].position ==
         float3(-0x1.199bc4p-20f, 0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.collision_vertices[72].surface_index == 2);
   CHECK(mesh.collision_vertices[73].position ==
         float3(-0x1.199bc4p-20f, -0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.collision_vertices[73].surface_index == 2);
   CHECK(mesh.collision_vertices[74].position ==
         float3(-0x1.25e6a4p+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[74].surface_index == 2);
   CHECK(mesh.collision_vertices[75].position ==
         float3(-0x1.25e6a4p+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[75].surface_index == 2);
   CHECK(mesh.collision_vertices[76].position ==
         float3(-0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.collision_vertices[76].surface_index == 2);
   CHECK(mesh.collision_vertices[77].position ==
         float3(-0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.collision_vertices[77].surface_index == 2);
   CHECK(mesh.collision_vertices[78].position ==
         float3(-0x1.62c51p+4f, 0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.collision_vertices[78].surface_index == 2);
   CHECK(mesh.collision_vertices[79].position ==
         float3(-0x1.62c51p+4f, -0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.collision_vertices[79].surface_index == 2);
   CHECK(mesh.collision_vertices[80].position ==
         float3(-0x1.8p+4f, 0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.collision_vertices[80].surface_index == 2);
   CHECK(mesh.collision_vertices[81].position ==
         float3(-0x1.8p+4f, -0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.collision_vertices[81].surface_index == 2);
   CHECK(mesh.collision_vertices[82].position ==
         float3(-0x1.62c50cp+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[82].surface_index == 2);
   CHECK(mesh.collision_vertices[83].position ==
         float3(-0x1.62c50cp+4f, -0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[83].surface_index == 2);
   CHECK(mesh.collision_vertices[84].position ==
         float3(-0x1.0f876ap+4f, 0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.collision_vertices[84].surface_index == 2);
   CHECK(mesh.collision_vertices[85].position ==
         float3(-0x1.0f876ap+4f, -0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.collision_vertices[85].surface_index == 2);
   CHECK(mesh.collision_vertices[86].position ==
         float3(-0x1.25e69p+3f, 0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.collision_vertices[86].surface_index == 2);
   CHECK(mesh.collision_vertices[87].position ==
         float3(-0x1.25e69p+3f, -0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.collision_vertices[87].surface_index == 2);
   CHECK(mesh.collision_vertices[88].position ==
         float3(0x1.334d44p-22f, 0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.collision_vertices[88].surface_index == 2);
   CHECK(mesh.collision_vertices[89].position ==
         float3(0x1.334d44p-22f, -0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.collision_vertices[89].surface_index == 2);
   CHECK(mesh.collision_vertices[90].position ==
         float3(0x1.25e6a8p+3f, 0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[90].surface_index == 2);
   CHECK(mesh.collision_vertices[91].position ==
         float3(0x1.25e6a8p+3f, -0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.collision_vertices[91].surface_index == 2);
   CHECK(mesh.collision_vertices[92].position ==
         float3(0x1.0f8772p+4f, 0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.collision_vertices[92].surface_index == 2);
   CHECK(mesh.collision_vertices[93].position ==
         float3(0x1.0f8772p+4f, -0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.collision_vertices[93].surface_index == 2);
   CHECK(mesh.collision_vertices[94].position ==
         float3(0x1.62c50ep+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[94].surface_index == 2);
   CHECK(mesh.collision_vertices[95].position ==
         float3(0x1.62c50ep+4f, -0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.collision_vertices[95].surface_index == 2);
   CHECK(mesh.collision_vertices[96].position == float3(0x1p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.collision_vertices[96].surface_index == 3);
   CHECK(mesh.collision_vertices[97].position == float3(0x1p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.collision_vertices[97].surface_index == 3);
   CHECK(mesh.collision_vertices[98].position ==
         float3(0x1.d906bcp+3f, -0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.collision_vertices[98].surface_index == 3);
   CHECK(mesh.collision_vertices[99].position ==
         float3(0x1.d906bcp+3f, 0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.collision_vertices[99].surface_index == 3);
   CHECK(mesh.collision_vertices[100].position ==
         float3(0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.collision_vertices[100].surface_index == 3);
   CHECK(mesh.collision_vertices[101].position ==
         float3(0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.collision_vertices[101].surface_index == 3);
   CHECK(mesh.collision_vertices[102].position ==
         float3(0x1.87de2ap+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.collision_vertices[102].surface_index == 3);
   CHECK(mesh.collision_vertices[103].position ==
         float3(0x1.87de2ap+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.collision_vertices[103].surface_index == 3);
   CHECK(mesh.collision_vertices[104].position ==
         float3(-0x1.777a5cp-21f, -0x1p+2f, 0x1p+4f));
   CHECK(mesh.collision_vertices[104].surface_index == 3);
   CHECK(mesh.collision_vertices[105].position ==
         float3(-0x1.777a5cp-21f, 0x1p+2f, 0x1p+4f));
   CHECK(mesh.collision_vertices[105].surface_index == 3);
   CHECK(mesh.collision_vertices[106].position ==
         float3(-0x1.87de3p+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.collision_vertices[106].surface_index == 3);
   CHECK(mesh.collision_vertices[107].position ==
         float3(-0x1.87de3p+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.collision_vertices[107].surface_index == 3);
   CHECK(mesh.collision_vertices[108].position ==
         float3(-0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.collision_vertices[108].surface_index == 3);
   CHECK(mesh.collision_vertices[109].position ==
         float3(-0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.collision_vertices[109].surface_index == 3);
   CHECK(mesh.collision_vertices[110].position ==
         float3(-0x1.d906cp+3f, -0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.collision_vertices[110].surface_index == 3);
   CHECK(mesh.collision_vertices[111].position ==
         float3(-0x1.d906cp+3f, 0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.collision_vertices[111].surface_index == 3);
   CHECK(mesh.collision_vertices[112].position ==
         float3(-0x1p+4f, -0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.collision_vertices[112].surface_index == 3);
   CHECK(mesh.collision_vertices[113].position ==
         float3(-0x1p+4f, 0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.collision_vertices[113].surface_index == 3);
   CHECK(mesh.collision_vertices[114].position ==
         float3(-0x1.d906bcp+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.collision_vertices[114].surface_index == 3);
   CHECK(mesh.collision_vertices[115].position ==
         float3(-0x1.d906bcp+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.collision_vertices[115].surface_index == 3);
   CHECK(mesh.collision_vertices[116].position ==
         float3(-0x1.6a09e2p+3f, -0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.collision_vertices[116].surface_index == 3);
   CHECK(mesh.collision_vertices[117].position ==
         float3(-0x1.6a09e2p+3f, 0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.collision_vertices[117].surface_index == 3);
   CHECK(mesh.collision_vertices[118].position ==
         float3(-0x1.87de16p+2f, -0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.collision_vertices[118].surface_index == 3);
   CHECK(mesh.collision_vertices[119].position ==
         float3(-0x1.87de16p+2f, 0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.collision_vertices[119].surface_index == 3);
   CHECK(mesh.collision_vertices[120].position ==
         float3(0x1.99bc5cp-23f, -0x1p+2f, -0x1p+4f));
   CHECK(mesh.collision_vertices[120].surface_index == 3);
   CHECK(mesh.collision_vertices[121].position ==
         float3(0x1.99bc5cp-23f, 0x1p+2f, -0x1p+4f));
   CHECK(mesh.collision_vertices[121].surface_index == 3);
   CHECK(mesh.collision_vertices[122].position ==
         float3(0x1.87de36p+2f, -0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.collision_vertices[122].surface_index == 3);
   CHECK(mesh.collision_vertices[123].position ==
         float3(0x1.87de36p+2f, 0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.collision_vertices[123].surface_index == 3);
   CHECK(mesh.collision_vertices[124].position ==
         float3(0x1.6a09eep+3f, -0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.collision_vertices[124].surface_index == 3);
   CHECK(mesh.collision_vertices[125].position ==
         float3(0x1.6a09eep+3f, 0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.collision_vertices[125].surface_index == 3);
   CHECK(mesh.collision_vertices[126].position ==
         float3(0x1.d906bep+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.collision_vertices[126].surface_index == 3);
   CHECK(mesh.collision_vertices[127].position ==
         float3(0x1.d906bep+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.collision_vertices[127].surface_index == 3);

   REQUIRE(mesh.collision_triangles.size() == 128);

   CHECK(mesh.collision_triangles[0] == std::array<uint16, 3>{2, 3, 0});
   CHECK(mesh.collision_triangles[1] == std::array<uint16, 3>{2, 0, 1});
   CHECK(mesh.collision_triangles[2] == std::array<uint16, 3>{4, 5, 3});
   CHECK(mesh.collision_triangles[3] == std::array<uint16, 3>{4, 3, 2});
   CHECK(mesh.collision_triangles[4] == std::array<uint16, 3>{6, 7, 5});
   CHECK(mesh.collision_triangles[5] == std::array<uint16, 3>{6, 5, 4});
   CHECK(mesh.collision_triangles[6] == std::array<uint16, 3>{8, 9, 7});
   CHECK(mesh.collision_triangles[7] == std::array<uint16, 3>{8, 7, 6});
   CHECK(mesh.collision_triangles[8] == std::array<uint16, 3>{10, 11, 9});
   CHECK(mesh.collision_triangles[9] == std::array<uint16, 3>{10, 9, 8});
   CHECK(mesh.collision_triangles[10] == std::array<uint16, 3>{12, 13, 11});
   CHECK(mesh.collision_triangles[11] == std::array<uint16, 3>{12, 11, 10});
   CHECK(mesh.collision_triangles[12] == std::array<uint16, 3>{14, 15, 13});
   CHECK(mesh.collision_triangles[13] == std::array<uint16, 3>{14, 13, 12});
   CHECK(mesh.collision_triangles[14] == std::array<uint16, 3>{16, 17, 15});
   CHECK(mesh.collision_triangles[15] == std::array<uint16, 3>{16, 15, 14});
   CHECK(mesh.collision_triangles[16] == std::array<uint16, 3>{18, 19, 17});
   CHECK(mesh.collision_triangles[17] == std::array<uint16, 3>{18, 17, 16});
   CHECK(mesh.collision_triangles[18] == std::array<uint16, 3>{20, 21, 19});
   CHECK(mesh.collision_triangles[19] == std::array<uint16, 3>{20, 19, 18});
   CHECK(mesh.collision_triangles[20] == std::array<uint16, 3>{22, 23, 21});
   CHECK(mesh.collision_triangles[21] == std::array<uint16, 3>{22, 21, 20});
   CHECK(mesh.collision_triangles[22] == std::array<uint16, 3>{24, 25, 23});
   CHECK(mesh.collision_triangles[23] == std::array<uint16, 3>{24, 23, 22});
   CHECK(mesh.collision_triangles[24] == std::array<uint16, 3>{26, 27, 25});
   CHECK(mesh.collision_triangles[25] == std::array<uint16, 3>{26, 25, 24});
   CHECK(mesh.collision_triangles[26] == std::array<uint16, 3>{28, 29, 27});
   CHECK(mesh.collision_triangles[27] == std::array<uint16, 3>{28, 27, 26});
   CHECK(mesh.collision_triangles[28] == std::array<uint16, 3>{30, 31, 29});
   CHECK(mesh.collision_triangles[29] == std::array<uint16, 3>{30, 29, 28});
   CHECK(mesh.collision_triangles[30] == std::array<uint16, 3>{1, 0, 31});
   CHECK(mesh.collision_triangles[31] == std::array<uint16, 3>{1, 31, 30});
   CHECK(mesh.collision_triangles[32] == std::array<uint16, 3>{32, 33, 34});
   CHECK(mesh.collision_triangles[33] == std::array<uint16, 3>{32, 34, 35});
   CHECK(mesh.collision_triangles[34] == std::array<uint16, 3>{35, 34, 36});
   CHECK(mesh.collision_triangles[35] == std::array<uint16, 3>{35, 36, 37});
   CHECK(mesh.collision_triangles[36] == std::array<uint16, 3>{37, 36, 38});
   CHECK(mesh.collision_triangles[37] == std::array<uint16, 3>{37, 38, 39});
   CHECK(mesh.collision_triangles[38] == std::array<uint16, 3>{39, 38, 40});
   CHECK(mesh.collision_triangles[39] == std::array<uint16, 3>{39, 40, 41});
   CHECK(mesh.collision_triangles[40] == std::array<uint16, 3>{41, 40, 42});
   CHECK(mesh.collision_triangles[41] == std::array<uint16, 3>{41, 42, 43});
   CHECK(mesh.collision_triangles[42] == std::array<uint16, 3>{43, 42, 44});
   CHECK(mesh.collision_triangles[43] == std::array<uint16, 3>{43, 44, 45});
   CHECK(mesh.collision_triangles[44] == std::array<uint16, 3>{45, 44, 46});
   CHECK(mesh.collision_triangles[45] == std::array<uint16, 3>{45, 46, 47});
   CHECK(mesh.collision_triangles[46] == std::array<uint16, 3>{47, 46, 48});
   CHECK(mesh.collision_triangles[47] == std::array<uint16, 3>{47, 48, 49});
   CHECK(mesh.collision_triangles[48] == std::array<uint16, 3>{49, 48, 50});
   CHECK(mesh.collision_triangles[49] == std::array<uint16, 3>{49, 50, 51});
   CHECK(mesh.collision_triangles[50] == std::array<uint16, 3>{51, 50, 52});
   CHECK(mesh.collision_triangles[51] == std::array<uint16, 3>{51, 52, 53});
   CHECK(mesh.collision_triangles[52] == std::array<uint16, 3>{53, 52, 54});
   CHECK(mesh.collision_triangles[53] == std::array<uint16, 3>{53, 54, 55});
   CHECK(mesh.collision_triangles[54] == std::array<uint16, 3>{55, 54, 56});
   CHECK(mesh.collision_triangles[55] == std::array<uint16, 3>{55, 56, 57});
   CHECK(mesh.collision_triangles[56] == std::array<uint16, 3>{57, 56, 58});
   CHECK(mesh.collision_triangles[57] == std::array<uint16, 3>{57, 58, 59});
   CHECK(mesh.collision_triangles[58] == std::array<uint16, 3>{59, 58, 60});
   CHECK(mesh.collision_triangles[59] == std::array<uint16, 3>{59, 60, 61});
   CHECK(mesh.collision_triangles[60] == std::array<uint16, 3>{61, 60, 62});
   CHECK(mesh.collision_triangles[61] == std::array<uint16, 3>{61, 62, 63});
   CHECK(mesh.collision_triangles[62] == std::array<uint16, 3>{63, 62, 33});
   CHECK(mesh.collision_triangles[63] == std::array<uint16, 3>{63, 33, 32});
   CHECK(mesh.collision_triangles[64] == std::array<uint16, 3>{66, 67, 64});
   CHECK(mesh.collision_triangles[65] == std::array<uint16, 3>{66, 64, 65});
   CHECK(mesh.collision_triangles[66] == std::array<uint16, 3>{68, 69, 67});
   CHECK(mesh.collision_triangles[67] == std::array<uint16, 3>{68, 67, 66});
   CHECK(mesh.collision_triangles[68] == std::array<uint16, 3>{70, 71, 69});
   CHECK(mesh.collision_triangles[69] == std::array<uint16, 3>{70, 69, 68});
   CHECK(mesh.collision_triangles[70] == std::array<uint16, 3>{72, 73, 71});
   CHECK(mesh.collision_triangles[71] == std::array<uint16, 3>{72, 71, 70});
   CHECK(mesh.collision_triangles[72] == std::array<uint16, 3>{74, 75, 73});
   CHECK(mesh.collision_triangles[73] == std::array<uint16, 3>{74, 73, 72});
   CHECK(mesh.collision_triangles[74] == std::array<uint16, 3>{76, 77, 75});
   CHECK(mesh.collision_triangles[75] == std::array<uint16, 3>{76, 75, 74});
   CHECK(mesh.collision_triangles[76] == std::array<uint16, 3>{78, 79, 77});
   CHECK(mesh.collision_triangles[77] == std::array<uint16, 3>{78, 77, 76});
   CHECK(mesh.collision_triangles[78] == std::array<uint16, 3>{80, 81, 79});
   CHECK(mesh.collision_triangles[79] == std::array<uint16, 3>{80, 79, 78});
   CHECK(mesh.collision_triangles[80] == std::array<uint16, 3>{82, 83, 81});
   CHECK(mesh.collision_triangles[81] == std::array<uint16, 3>{82, 81, 80});
   CHECK(mesh.collision_triangles[82] == std::array<uint16, 3>{84, 85, 83});
   CHECK(mesh.collision_triangles[83] == std::array<uint16, 3>{84, 83, 82});
   CHECK(mesh.collision_triangles[84] == std::array<uint16, 3>{86, 87, 85});
   CHECK(mesh.collision_triangles[85] == std::array<uint16, 3>{86, 85, 84});
   CHECK(mesh.collision_triangles[86] == std::array<uint16, 3>{88, 89, 87});
   CHECK(mesh.collision_triangles[87] == std::array<uint16, 3>{88, 87, 86});
   CHECK(mesh.collision_triangles[88] == std::array<uint16, 3>{90, 91, 89});
   CHECK(mesh.collision_triangles[89] == std::array<uint16, 3>{90, 89, 88});
   CHECK(mesh.collision_triangles[90] == std::array<uint16, 3>{92, 93, 91});
   CHECK(mesh.collision_triangles[91] == std::array<uint16, 3>{92, 91, 90});
   CHECK(mesh.collision_triangles[92] == std::array<uint16, 3>{94, 95, 93});
   CHECK(mesh.collision_triangles[93] == std::array<uint16, 3>{94, 93, 92});
   CHECK(mesh.collision_triangles[94] == std::array<uint16, 3>{65, 64, 95});
   CHECK(mesh.collision_triangles[95] == std::array<uint16, 3>{65, 95, 94});
   CHECK(mesh.collision_triangles[96] == std::array<uint16, 3>{96, 97, 98});
   CHECK(mesh.collision_triangles[97] == std::array<uint16, 3>{96, 98, 99});
   CHECK(mesh.collision_triangles[98] == std::array<uint16, 3>{99, 98, 100});
   CHECK(mesh.collision_triangles[99] == std::array<uint16, 3>{99, 100, 101});
   CHECK(mesh.collision_triangles[100] == std::array<uint16, 3>{101, 100, 102});
   CHECK(mesh.collision_triangles[101] == std::array<uint16, 3>{101, 102, 103});
   CHECK(mesh.collision_triangles[102] == std::array<uint16, 3>{103, 102, 104});
   CHECK(mesh.collision_triangles[103] == std::array<uint16, 3>{103, 104, 105});
   CHECK(mesh.collision_triangles[104] == std::array<uint16, 3>{105, 104, 106});
   CHECK(mesh.collision_triangles[105] == std::array<uint16, 3>{105, 106, 107});
   CHECK(mesh.collision_triangles[106] == std::array<uint16, 3>{107, 106, 108});
   CHECK(mesh.collision_triangles[107] == std::array<uint16, 3>{107, 108, 109});
   CHECK(mesh.collision_triangles[108] == std::array<uint16, 3>{109, 108, 110});
   CHECK(mesh.collision_triangles[109] == std::array<uint16, 3>{109, 110, 111});
   CHECK(mesh.collision_triangles[110] == std::array<uint16, 3>{111, 110, 112});
   CHECK(mesh.collision_triangles[111] == std::array<uint16, 3>{111, 112, 113});
   CHECK(mesh.collision_triangles[112] == std::array<uint16, 3>{113, 112, 114});
   CHECK(mesh.collision_triangles[113] == std::array<uint16, 3>{113, 114, 115});
   CHECK(mesh.collision_triangles[114] == std::array<uint16, 3>{115, 114, 116});
   CHECK(mesh.collision_triangles[115] == std::array<uint16, 3>{115, 116, 117});
   CHECK(mesh.collision_triangles[116] == std::array<uint16, 3>{117, 116, 118});
   CHECK(mesh.collision_triangles[117] == std::array<uint16, 3>{117, 118, 119});
   CHECK(mesh.collision_triangles[118] == std::array<uint16, 3>{119, 118, 120});
   CHECK(mesh.collision_triangles[119] == std::array<uint16, 3>{119, 120, 121});
   CHECK(mesh.collision_triangles[120] == std::array<uint16, 3>{121, 120, 122});
   CHECK(mesh.collision_triangles[121] == std::array<uint16, 3>{121, 122, 123});
   CHECK(mesh.collision_triangles[122] == std::array<uint16, 3>{123, 122, 124});
   CHECK(mesh.collision_triangles[123] == std::array<uint16, 3>{123, 124, 125});
   CHECK(mesh.collision_triangles[124] == std::array<uint16, 3>{125, 124, 126});
   CHECK(mesh.collision_triangles[125] == std::array<uint16, 3>{125, 126, 127});
   CHECK(mesh.collision_triangles[126] == std::array<uint16, 3>{127, 126, 97});
   CHECK(mesh.collision_triangles[127] == std::array<uint16, 3>{127, 97, 96});

   REQUIRE(mesh.collision_occluders.size() == 64);

   CHECK(mesh.collision_occluders[0] == std::array<uint16, 4>{2, 3, 0, 1});
   CHECK(mesh.collision_occluders[1] == std::array<uint16, 4>{4, 5, 3, 2});
   CHECK(mesh.collision_occluders[2] == std::array<uint16, 4>{6, 7, 5, 4});
   CHECK(mesh.collision_occluders[3] == std::array<uint16, 4>{8, 9, 7, 6});
   CHECK(mesh.collision_occluders[4] == std::array<uint16, 4>{10, 11, 9, 8});
   CHECK(mesh.collision_occluders[5] == std::array<uint16, 4>{12, 13, 11, 10});
   CHECK(mesh.collision_occluders[6] == std::array<uint16, 4>{14, 15, 13, 12});
   CHECK(mesh.collision_occluders[7] == std::array<uint16, 4>{16, 17, 15, 14});
   CHECK(mesh.collision_occluders[8] == std::array<uint16, 4>{18, 19, 17, 16});
   CHECK(mesh.collision_occluders[9] == std::array<uint16, 4>{20, 21, 19, 18});
   CHECK(mesh.collision_occluders[10] == std::array<uint16, 4>{22, 23, 21, 20});
   CHECK(mesh.collision_occluders[11] == std::array<uint16, 4>{24, 25, 23, 22});
   CHECK(mesh.collision_occluders[12] == std::array<uint16, 4>{26, 27, 25, 24});
   CHECK(mesh.collision_occluders[13] == std::array<uint16, 4>{28, 29, 27, 26});
   CHECK(mesh.collision_occluders[14] == std::array<uint16, 4>{30, 31, 29, 28});
   CHECK(mesh.collision_occluders[15] == std::array<uint16, 4>{1, 0, 31, 30});
   CHECK(mesh.collision_occluders[16] == std::array<uint16, 4>{32, 33, 34, 35});
   CHECK(mesh.collision_occluders[17] == std::array<uint16, 4>{35, 34, 36, 37});
   CHECK(mesh.collision_occluders[18] == std::array<uint16, 4>{37, 36, 38, 39});
   CHECK(mesh.collision_occluders[19] == std::array<uint16, 4>{39, 38, 40, 41});
   CHECK(mesh.collision_occluders[20] == std::array<uint16, 4>{41, 40, 42, 43});
   CHECK(mesh.collision_occluders[21] == std::array<uint16, 4>{43, 42, 44, 45});
   CHECK(mesh.collision_occluders[22] == std::array<uint16, 4>{45, 44, 46, 47});
   CHECK(mesh.collision_occluders[23] == std::array<uint16, 4>{47, 46, 48, 49});
   CHECK(mesh.collision_occluders[24] == std::array<uint16, 4>{49, 48, 50, 51});
   CHECK(mesh.collision_occluders[25] == std::array<uint16, 4>{51, 50, 52, 53});
   CHECK(mesh.collision_occluders[26] == std::array<uint16, 4>{53, 52, 54, 55});
   CHECK(mesh.collision_occluders[27] == std::array<uint16, 4>{55, 54, 56, 57});
   CHECK(mesh.collision_occluders[28] == std::array<uint16, 4>{57, 56, 58, 59});
   CHECK(mesh.collision_occluders[29] == std::array<uint16, 4>{59, 58, 60, 61});
   CHECK(mesh.collision_occluders[30] == std::array<uint16, 4>{61, 60, 62, 63});
   CHECK(mesh.collision_occluders[31] == std::array<uint16, 4>{63, 62, 33, 32});
   CHECK(mesh.collision_occluders[32] == std::array<uint16, 4>{66, 67, 64, 65});
   CHECK(mesh.collision_occluders[33] == std::array<uint16, 4>{68, 69, 67, 66});
   CHECK(mesh.collision_occluders[34] == std::array<uint16, 4>{70, 71, 69, 68});
   CHECK(mesh.collision_occluders[35] == std::array<uint16, 4>{72, 73, 71, 70});
   CHECK(mesh.collision_occluders[36] == std::array<uint16, 4>{74, 75, 73, 72});
   CHECK(mesh.collision_occluders[37] == std::array<uint16, 4>{76, 77, 75, 74});
   CHECK(mesh.collision_occluders[38] == std::array<uint16, 4>{78, 79, 77, 76});
   CHECK(mesh.collision_occluders[39] == std::array<uint16, 4>{80, 81, 79, 78});
   CHECK(mesh.collision_occluders[40] == std::array<uint16, 4>{82, 83, 81, 80});
   CHECK(mesh.collision_occluders[41] == std::array<uint16, 4>{84, 85, 83, 82});
   CHECK(mesh.collision_occluders[42] == std::array<uint16, 4>{86, 87, 85, 84});
   CHECK(mesh.collision_occluders[43] == std::array<uint16, 4>{88, 89, 87, 86});
   CHECK(mesh.collision_occluders[44] == std::array<uint16, 4>{90, 91, 89, 88});
   CHECK(mesh.collision_occluders[45] == std::array<uint16, 4>{92, 93, 91, 90});
   CHECK(mesh.collision_occluders[46] == std::array<uint16, 4>{94, 95, 93, 92});
   CHECK(mesh.collision_occluders[47] == std::array<uint16, 4>{65, 64, 95, 94});
   CHECK(mesh.collision_occluders[48] == std::array<uint16, 4>{96, 97, 98, 99});
   CHECK(mesh.collision_occluders[49] == std::array<uint16, 4>{99, 98, 100, 101});
   CHECK(mesh.collision_occluders[50] == std::array<uint16, 4>{101, 100, 102, 103});
   CHECK(mesh.collision_occluders[51] == std::array<uint16, 4>{103, 102, 104, 105});
   CHECK(mesh.collision_occluders[52] == std::array<uint16, 4>{105, 104, 106, 107});
   CHECK(mesh.collision_occluders[53] == std::array<uint16, 4>{107, 106, 108, 109});
   CHECK(mesh.collision_occluders[54] == std::array<uint16, 4>{109, 108, 110, 111});
   CHECK(mesh.collision_occluders[55] == std::array<uint16, 4>{111, 110, 112, 113});
   CHECK(mesh.collision_occluders[56] == std::array<uint16, 4>{113, 112, 114, 115});
   CHECK(mesh.collision_occluders[57] == std::array<uint16, 4>{115, 114, 116, 117});
   CHECK(mesh.collision_occluders[58] == std::array<uint16, 4>{117, 116, 118, 119});
   CHECK(mesh.collision_occluders[59] == std::array<uint16, 4>{119, 118, 120, 121});
   CHECK(mesh.collision_occluders[60] == std::array<uint16, 4>{121, 120, 122, 123});
   CHECK(mesh.collision_occluders[61] == std::array<uint16, 4>{123, 122, 124, 125});
   CHECK(mesh.collision_occluders[62] == std::array<uint16, 4>{125, 124, 126, 127});
   CHECK(mesh.collision_occluders[63] == std::array<uint16, 4>{127, 126, 97, 96});

   REQUIRE(mesh.snap_points.size() == 64);

   CHECK(mesh.snap_points[0] == float3(0x1p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.snap_points[1] == float3(0x1.8p+4f, 0x1p+2f, 0x0p-1022f));
   CHECK(mesh.snap_points[2] == float3(0x1p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.snap_points[3] == float3(0x1.8p+4f, -0x1p+2f, 0x0p-1022f));
   CHECK(mesh.snap_points[4] == float3(0x1.d906bcp+3f, 0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.snap_points[5] == float3(0x1.62c50cp+4f, 0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.snap_points[6] == float3(0x1.d906bcp+3f, -0x1p+2f, 0x1.87de2cp+2f));
   CHECK(mesh.snap_points[7] == float3(0x1.62c50cp+4f, -0x1p+2f, 0x1.25e6ap+3f));
   CHECK(mesh.snap_points[8] == float3(0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.snap_points[9] == float3(0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.snap_points[10] == float3(0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.snap_points[11] == float3(0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.snap_points[12] == float3(0x1.87de2ap+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.snap_points[13] == float3(0x1.25e6ap+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.snap_points[14] == float3(0x1.87de2ap+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.snap_points[15] == float3(0x1.25e6ap+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.snap_points[16] == float3(-0x1.777a5cp-21f, 0x1p+2f, 0x1p+4f));
   CHECK(mesh.snap_points[17] == float3(-0x1.199bc4p-20f, 0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.snap_points[18] == float3(-0x1.777a5cp-21f, -0x1p+2f, 0x1p+4f));
   CHECK(mesh.snap_points[19] == float3(-0x1.199bc4p-20f, -0x1p+2f, 0x1.8p+4f));
   CHECK(mesh.snap_points[20] == float3(-0x1.87de3p+2f, 0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.snap_points[21] == float3(-0x1.25e6a4p+3f, 0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.snap_points[22] == float3(-0x1.87de3p+2f, -0x1p+2f, 0x1.d906bcp+3f));
   CHECK(mesh.snap_points[23] == float3(-0x1.25e6a4p+3f, -0x1p+2f, 0x1.62c50cp+4f));
   CHECK(mesh.snap_points[24] == float3(-0x1.6a09e6p+3f, 0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.snap_points[25] == float3(-0x1.0f876cp+4f, 0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.snap_points[26] == float3(-0x1.6a09e6p+3f, -0x1p+2f, 0x1.6a09e6p+3f));
   CHECK(mesh.snap_points[27] == float3(-0x1.0f876cp+4f, -0x1p+2f, 0x1.0f876cp+4f));
   CHECK(mesh.snap_points[28] == float3(-0x1.d906cp+3f, 0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.snap_points[29] == float3(-0x1.62c51p+4f, 0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.snap_points[30] == float3(-0x1.d906cp+3f, -0x1p+2f, 0x1.87de2p+2f));
   CHECK(mesh.snap_points[31] == float3(-0x1.62c51p+4f, -0x1p+2f, 0x1.25e698p+3f));
   CHECK(mesh.snap_points[32] == float3(-0x1p+4f, 0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.snap_points[33] == float3(-0x1.8p+4f, 0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.snap_points[34] == float3(-0x1p+4f, -0x1p+2f, -0x1.777a5cp-20f));
   CHECK(mesh.snap_points[35] == float3(-0x1.8p+4f, -0x1p+2f, -0x1.199bc4p-19f));
   CHECK(mesh.snap_points[36] == float3(-0x1.d906bcp+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.snap_points[37] == float3(-0x1.62c50cp+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.snap_points[38] == float3(-0x1.d906bcp+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.snap_points[39] == float3(-0x1.62c50cp+4f, -0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.snap_points[40] == float3(-0x1.6a09e2p+3f, 0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.snap_points[41] == float3(-0x1.0f876ap+4f, 0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.snap_points[42] == float3(-0x1.6a09e2p+3f, -0x1p+2f, -0x1.6a09eap+3f));
   CHECK(mesh.snap_points[43] == float3(-0x1.0f876ap+4f, -0x1p+2f, -0x1.0f877p+4f));
   CHECK(mesh.snap_points[44] == float3(-0x1.87de16p+2f, 0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.snap_points[45] == float3(-0x1.25e69p+3f, 0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.snap_points[46] == float3(-0x1.87de16p+2f, -0x1p+2f, -0x1.d906c2p+3f));
   CHECK(mesh.snap_points[47] == float3(-0x1.25e69p+3f, -0x1p+2f, -0x1.62c512p+4f));
   CHECK(mesh.snap_points[48] == float3(0x1.99bc5cp-23f, 0x1p+2f, -0x1p+4f));
   CHECK(mesh.snap_points[49] == float3(0x1.334d44p-22f, 0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.snap_points[50] == float3(0x1.99bc5cp-23f, -0x1p+2f, -0x1p+4f));
   CHECK(mesh.snap_points[51] == float3(0x1.334d44p-22f, -0x1p+2f, -0x1.8p+4f));
   CHECK(mesh.snap_points[52] == float3(0x1.87de36p+2f, 0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.snap_points[53] == float3(0x1.25e6a8p+3f, 0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.snap_points[54] == float3(0x1.87de36p+2f, -0x1p+2f, -0x1.d906bap+3f));
   CHECK(mesh.snap_points[55] == float3(0x1.25e6a8p+3f, -0x1p+2f, -0x1.62c50cp+4f));
   CHECK(mesh.snap_points[56] == float3(0x1.6a09eep+3f, 0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.snap_points[57] == float3(0x1.0f8772p+4f, 0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.snap_points[58] == float3(0x1.6a09eep+3f, -0x1p+2f, -0x1.6a09dep+3f));
   CHECK(mesh.snap_points[59] == float3(0x1.0f8772p+4f, -0x1p+2f, -0x1.0f8766p+4f));
   CHECK(mesh.snap_points[60] == float3(0x1.d906bep+3f, 0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.snap_points[61] == float3(0x1.62c50ep+4f, 0x1p+2f, -0x1.25e6ap+3f));
   CHECK(mesh.snap_points[62] == float3(0x1.d906bep+3f, -0x1p+2f, -0x1.87de2ap+2f));
   CHECK(mesh.snap_points[63] == float3(0x1.62c50ep+4f, -0x1p+2f, -0x1.25e6ap+3f));

   REQUIRE(mesh.snap_edges.size() == 128);

   CHECK(mesh.snap_edges[0] == std::array<uint16, 2>{0, 1});
   CHECK(mesh.snap_edges[1] == std::array<uint16, 2>{2, 3});
   CHECK(mesh.snap_edges[2] == std::array<uint16, 2>{0, 2});
   CHECK(mesh.snap_edges[3] == std::array<uint16, 2>{1, 3});
   CHECK(mesh.snap_edges[4] == std::array<uint16, 2>{0, 4});
   CHECK(mesh.snap_edges[5] == std::array<uint16, 2>{1, 5});
   CHECK(mesh.snap_edges[6] == std::array<uint16, 2>{2, 6});
   CHECK(mesh.snap_edges[7] == std::array<uint16, 2>{3, 7});
   CHECK(mesh.snap_edges[8] == std::array<uint16, 2>{4, 5});
   CHECK(mesh.snap_edges[9] == std::array<uint16, 2>{6, 7});
   CHECK(mesh.snap_edges[10] == std::array<uint16, 2>{4, 6});
   CHECK(mesh.snap_edges[11] == std::array<uint16, 2>{5, 7});
   CHECK(mesh.snap_edges[12] == std::array<uint16, 2>{4, 8});
   CHECK(mesh.snap_edges[13] == std::array<uint16, 2>{5, 9});
   CHECK(mesh.snap_edges[14] == std::array<uint16, 2>{6, 10});
   CHECK(mesh.snap_edges[15] == std::array<uint16, 2>{7, 11});
   CHECK(mesh.snap_edges[16] == std::array<uint16, 2>{8, 9});
   CHECK(mesh.snap_edges[17] == std::array<uint16, 2>{10, 11});
   CHECK(mesh.snap_edges[18] == std::array<uint16, 2>{8, 10});
   CHECK(mesh.snap_edges[19] == std::array<uint16, 2>{9, 11});
   CHECK(mesh.snap_edges[20] == std::array<uint16, 2>{8, 12});
   CHECK(mesh.snap_edges[21] == std::array<uint16, 2>{9, 13});
   CHECK(mesh.snap_edges[22] == std::array<uint16, 2>{10, 14});
   CHECK(mesh.snap_edges[23] == std::array<uint16, 2>{11, 15});
   CHECK(mesh.snap_edges[24] == std::array<uint16, 2>{12, 13});
   CHECK(mesh.snap_edges[25] == std::array<uint16, 2>{14, 15});
   CHECK(mesh.snap_edges[26] == std::array<uint16, 2>{12, 14});
   CHECK(mesh.snap_edges[27] == std::array<uint16, 2>{13, 15});
   CHECK(mesh.snap_edges[28] == std::array<uint16, 2>{12, 16});
   CHECK(mesh.snap_edges[29] == std::array<uint16, 2>{13, 17});
   CHECK(mesh.snap_edges[30] == std::array<uint16, 2>{14, 18});
   CHECK(mesh.snap_edges[31] == std::array<uint16, 2>{15, 19});
   CHECK(mesh.snap_edges[32] == std::array<uint16, 2>{16, 17});
   CHECK(mesh.snap_edges[33] == std::array<uint16, 2>{18, 19});
   CHECK(mesh.snap_edges[34] == std::array<uint16, 2>{16, 18});
   CHECK(mesh.snap_edges[35] == std::array<uint16, 2>{17, 19});
   CHECK(mesh.snap_edges[36] == std::array<uint16, 2>{16, 20});
   CHECK(mesh.snap_edges[37] == std::array<uint16, 2>{17, 21});
   CHECK(mesh.snap_edges[38] == std::array<uint16, 2>{18, 22});
   CHECK(mesh.snap_edges[39] == std::array<uint16, 2>{19, 23});
   CHECK(mesh.snap_edges[40] == std::array<uint16, 2>{20, 21});
   CHECK(mesh.snap_edges[41] == std::array<uint16, 2>{22, 23});
   CHECK(mesh.snap_edges[42] == std::array<uint16, 2>{20, 22});
   CHECK(mesh.snap_edges[43] == std::array<uint16, 2>{21, 23});
   CHECK(mesh.snap_edges[44] == std::array<uint16, 2>{20, 24});
   CHECK(mesh.snap_edges[45] == std::array<uint16, 2>{21, 25});
   CHECK(mesh.snap_edges[46] == std::array<uint16, 2>{22, 26});
   CHECK(mesh.snap_edges[47] == std::array<uint16, 2>{23, 27});
   CHECK(mesh.snap_edges[48] == std::array<uint16, 2>{24, 25});
   CHECK(mesh.snap_edges[49] == std::array<uint16, 2>{26, 27});
   CHECK(mesh.snap_edges[50] == std::array<uint16, 2>{24, 26});
   CHECK(mesh.snap_edges[51] == std::array<uint16, 2>{25, 27});
   CHECK(mesh.snap_edges[52] == std::array<uint16, 2>{24, 28});
   CHECK(mesh.snap_edges[53] == std::array<uint16, 2>{25, 29});
   CHECK(mesh.snap_edges[54] == std::array<uint16, 2>{26, 30});
   CHECK(mesh.snap_edges[55] == std::array<uint16, 2>{27, 31});
   CHECK(mesh.snap_edges[56] == std::array<uint16, 2>{28, 29});
   CHECK(mesh.snap_edges[57] == std::array<uint16, 2>{30, 31});
   CHECK(mesh.snap_edges[58] == std::array<uint16, 2>{28, 30});
   CHECK(mesh.snap_edges[59] == std::array<uint16, 2>{29, 31});
   CHECK(mesh.snap_edges[60] == std::array<uint16, 2>{28, 32});
   CHECK(mesh.snap_edges[61] == std::array<uint16, 2>{29, 33});
   CHECK(mesh.snap_edges[62] == std::array<uint16, 2>{30, 34});
   CHECK(mesh.snap_edges[63] == std::array<uint16, 2>{31, 35});
   CHECK(mesh.snap_edges[64] == std::array<uint16, 2>{32, 33});
   CHECK(mesh.snap_edges[65] == std::array<uint16, 2>{34, 35});
   CHECK(mesh.snap_edges[66] == std::array<uint16, 2>{32, 34});
   CHECK(mesh.snap_edges[67] == std::array<uint16, 2>{33, 35});
   CHECK(mesh.snap_edges[68] == std::array<uint16, 2>{32, 36});
   CHECK(mesh.snap_edges[69] == std::array<uint16, 2>{33, 37});
   CHECK(mesh.snap_edges[70] == std::array<uint16, 2>{34, 38});
   CHECK(mesh.snap_edges[71] == std::array<uint16, 2>{35, 39});
   CHECK(mesh.snap_edges[72] == std::array<uint16, 2>{36, 37});
   CHECK(mesh.snap_edges[73] == std::array<uint16, 2>{38, 39});
   CHECK(mesh.snap_edges[74] == std::array<uint16, 2>{36, 38});
   CHECK(mesh.snap_edges[75] == std::array<uint16, 2>{37, 39});
   CHECK(mesh.snap_edges[76] == std::array<uint16, 2>{36, 40});
   CHECK(mesh.snap_edges[77] == std::array<uint16, 2>{37, 41});
   CHECK(mesh.snap_edges[78] == std::array<uint16, 2>{38, 42});
   CHECK(mesh.snap_edges[79] == std::array<uint16, 2>{39, 43});
   CHECK(mesh.snap_edges[80] == std::array<uint16, 2>{40, 41});
   CHECK(mesh.snap_edges[81] == std::array<uint16, 2>{42, 43});
   CHECK(mesh.snap_edges[82] == std::array<uint16, 2>{40, 42});
   CHECK(mesh.snap_edges[83] == std::array<uint16, 2>{41, 43});
   CHECK(mesh.snap_edges[84] == std::array<uint16, 2>{40, 44});
   CHECK(mesh.snap_edges[85] == std::array<uint16, 2>{41, 45});
   CHECK(mesh.snap_edges[86] == std::array<uint16, 2>{42, 46});
   CHECK(mesh.snap_edges[87] == std::array<uint16, 2>{43, 47});
   CHECK(mesh.snap_edges[88] == std::array<uint16, 2>{44, 45});
   CHECK(mesh.snap_edges[89] == std::array<uint16, 2>{46, 47});
   CHECK(mesh.snap_edges[90] == std::array<uint16, 2>{44, 46});
   CHECK(mesh.snap_edges[91] == std::array<uint16, 2>{45, 47});
   CHECK(mesh.snap_edges[92] == std::array<uint16, 2>{44, 48});
   CHECK(mesh.snap_edges[93] == std::array<uint16, 2>{45, 49});
   CHECK(mesh.snap_edges[94] == std::array<uint16, 2>{46, 50});
   CHECK(mesh.snap_edges[95] == std::array<uint16, 2>{47, 51});
   CHECK(mesh.snap_edges[96] == std::array<uint16, 2>{48, 49});
   CHECK(mesh.snap_edges[97] == std::array<uint16, 2>{50, 51});
   CHECK(mesh.snap_edges[98] == std::array<uint16, 2>{48, 50});
   CHECK(mesh.snap_edges[99] == std::array<uint16, 2>{49, 51});
   CHECK(mesh.snap_edges[100] == std::array<uint16, 2>{48, 52});
   CHECK(mesh.snap_edges[101] == std::array<uint16, 2>{49, 53});
   CHECK(mesh.snap_edges[102] == std::array<uint16, 2>{50, 54});
   CHECK(mesh.snap_edges[103] == std::array<uint16, 2>{51, 55});
   CHECK(mesh.snap_edges[104] == std::array<uint16, 2>{52, 53});
   CHECK(mesh.snap_edges[105] == std::array<uint16, 2>{54, 55});
   CHECK(mesh.snap_edges[106] == std::array<uint16, 2>{52, 54});
   CHECK(mesh.snap_edges[107] == std::array<uint16, 2>{53, 55});
   CHECK(mesh.snap_edges[108] == std::array<uint16, 2>{52, 56});
   CHECK(mesh.snap_edges[109] == std::array<uint16, 2>{53, 57});
   CHECK(mesh.snap_edges[110] == std::array<uint16, 2>{54, 58});
   CHECK(mesh.snap_edges[111] == std::array<uint16, 2>{55, 59});
   CHECK(mesh.snap_edges[112] == std::array<uint16, 2>{56, 57});
   CHECK(mesh.snap_edges[113] == std::array<uint16, 2>{58, 59});
   CHECK(mesh.snap_edges[114] == std::array<uint16, 2>{56, 58});
   CHECK(mesh.snap_edges[115] == std::array<uint16, 2>{57, 59});
   CHECK(mesh.snap_edges[116] == std::array<uint16, 2>{56, 60});
   CHECK(mesh.snap_edges[117] == std::array<uint16, 2>{57, 61});
   CHECK(mesh.snap_edges[118] == std::array<uint16, 2>{58, 62});
   CHECK(mesh.snap_edges[119] == std::array<uint16, 2>{59, 63});
   CHECK(mesh.snap_edges[120] == std::array<uint16, 2>{60, 61});
   CHECK(mesh.snap_edges[121] == std::array<uint16, 2>{62, 63});
   CHECK(mesh.snap_edges[122] == std::array<uint16, 2>{60, 62});
   CHECK(mesh.snap_edges[123] == std::array<uint16, 2>{61, 63});
   CHECK(mesh.snap_edges[124] == std::array<uint16, 2>{60, 0});
   CHECK(mesh.snap_edges[125] == std::array<uint16, 2>{61, 1});
   CHECK(mesh.snap_edges[126] == std::array<uint16, 2>{62, 2});
   CHECK(mesh.snap_edges[127] == std::array<uint16, 2>{63, 3});
}
}
