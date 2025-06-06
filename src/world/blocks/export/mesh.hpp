#pragma once

#include "../../blocks.hpp"

#include "types.hpp"

#include "math/bounding_box.hpp"

#include <array>
#include <bit>
#include <vector>

namespace we::world {

struct block_world_vertex {
   float3 positionWS;
   float3 normalWS;
   float2 texcoords;

   bool operator==(const block_world_vertex& other) const noexcept
   {
      return std::memcmp(&other, this, sizeof(block_world_vertex)) == 0;
   }

   template<typename H>
   friend H AbslHashValue(H h, const block_world_vertex& vertex)
   {
      return H::combine(std::move(h),
                        std::bit_cast<std::array<uint8, sizeof(block_world_vertex)>>(
                           vertex));
   }
};

struct block_world_triangle {
   block_id block_id;

   std::array<block_world_vertex, 3> vertices;
   uint8 material_index = 0;
};

struct block_world_collision_triangle {
   block_id block_id;

   std::array<float3, 3> verticesWS;
   uint8 material_index = 0;
};

struct block_world_occluder {
   block_id block_id;

   std::array<float3, 4> verticesWS;

   float4 planeWS;
   float area;
};

struct block_world_mesh {
   std::vector<block_world_triangle> triangles;
   std::vector<block_world_collision_triangle> collision_triangles;

   math::bounding_box bboxWS;
};

}