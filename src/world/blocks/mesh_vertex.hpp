#pragma once

#include "types.hpp"

namespace we::world {

struct block_vertex {
   float3 position;
   float3 normal;
   float2 texcoords;
   uint32 surface_index;
};

}