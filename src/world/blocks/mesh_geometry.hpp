#include "types.hpp"

#include <array>

namespace we::world {

struct block_vertex {
   float3 position;
   float3 tangent;
   float bitangent_sign;
   float3 normal;
   float2 texcoords;
   uint32 surface_index;
};

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

};