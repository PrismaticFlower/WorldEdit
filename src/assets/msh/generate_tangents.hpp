#include "types.hpp"

#include <array>
#include <span>
#include <vector>

namespace we::assets::msh {

struct generate_tangents_input {
   std::span<float3> positions;
   std::span<float3> normals;
   std::span<uint32> colors;
   std::span<float2> texcoords;

   std::span<std::array<uint16, 3>> triangles;
};

struct generate_tangents_output {
   std::vector<float3> positions;
   std::vector<float3> normals;
   std::vector<float3> tangents;
   std::vector<float3> bitangents;
   std::vector<uint32> colors;
   std::vector<float2> texcoords;

   std::vector<std::array<uint16, 3>> triangles;
};

auto generate_tangents(generate_tangents_input input) -> generate_tangents_output;

}
