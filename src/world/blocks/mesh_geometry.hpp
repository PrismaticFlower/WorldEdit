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

};