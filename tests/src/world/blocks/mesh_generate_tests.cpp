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
      fmt::format("CHECK(mesh.collision_vertices[{}] == "
                  "float3({:a}f, {:a}f, {:a}f));\n",
                  i, mesh.collision_vertices[i].x, mesh.collision_vertices[i].y,
                  mesh.collision_vertices[i].z);
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
   world::block_custom_mesh mesh = world::generate_mesh(
      world::block_custom_mesh_stairway_desc{.size = {10.0f, 1.0f, 10.0f},
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

   CHECK(mesh.collision_vertices[0] == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[1] == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[2] == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[3] == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[4] == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[5] == float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[6] == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[7] == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[8] == float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[9] == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[10] == float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[11] == float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[12] == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[13] == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[14] == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[15] == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[16] == float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[17] == float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[18] == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[19] == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+1f));
   CHECK(mesh.collision_vertices[20] == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[21] == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[22] == float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[23] == float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[24] == float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[25] == float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[26] == float3(0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[27] == float3(-0x1.4p+2f, 0x1.e66666p-1f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[28] == float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[29] == float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[30] == float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[31] == float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[32] == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[33] == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[34] == float3(0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[35] == float3(0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[36] == float3(0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[37] == float3(0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[38] == float3(-0x1.4p+2f, 0x0p-1022f, -0x1.4p+2f));
   CHECK(mesh.collision_vertices[39] == float3(-0x1.4p+2f, 0x0p-1022f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[40] == float3(-0x1.4p+2f, 0x1.99999ap-3f, 0x1.4p+2f));
   CHECK(mesh.collision_vertices[41] == float3(-0x1.4p+2f, 0x1.99999ap-3f, -0x1.4p+2f));

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

}
