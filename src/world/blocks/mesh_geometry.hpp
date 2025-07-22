#include "mesh_vertex.hpp"

#include <array>

namespace we::world {

extern const std::array<block_vertex, 24> block_cube_vertices;
extern const std::array<std::array<uint16, 3>, 12> block_cube_triangles;
extern const std::array<std::array<uint16, 4>, 6> block_cube_occluders;
extern const std::array<float3, 8> block_cube_points;
extern const std::array<std::array<uint16, 2>, 12> block_cube_edges;

extern const std::array<block_vertex, 18> block_ramp_vertices;
extern const std::array<std::array<uint16, 3>, 8> block_ramp_triangles;
extern const std::array<std::array<uint16, 4>, 3> block_ramp_occluders;
extern const std::array<float3, 6> block_ramp_points;
extern const std::array<std::array<uint16, 2>, 9> block_ramp_edges;

extern const std::array<block_vertex, 328> block_hemisphere_vertices;
extern const std::array<std::array<uint16, 3>, 510> block_hemisphere_triangles;
extern const std::array<std::array<uint16, 4>, 0> block_hemisphere_occluders;
extern const std::array<float3, 1> block_hemisphere_points;
extern const std::array<std::array<uint16, 2>, 0> block_hemisphere_edges;

extern const std::array<block_vertex, 16> block_pyramid_vertices;
extern const std::array<std::array<uint16, 3>, 6> block_pyramid_triangles;
extern const std::array<std::array<uint16, 4>, 1> block_pyramid_occluders;
extern const std::array<float3, 5> block_pyramid_points;
extern const std::array<std::array<uint16, 2>, 8> block_pyramid_edges;

extern const std::array<std::array<uint16, 3>, 2> block_quad_triangles;
extern const std::array<std::array<uint16, 3>, 2> block_quad_alternate_triangles;
extern const std::array<float2, 4> block_quad_vertex_texcoords;

};