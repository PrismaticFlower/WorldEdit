
#define LIGHT_TILE_REGISTER_SPACE space4
#define MAX_LIGHTS 1024

struct light_index {
   uint light_index[16];

   uint get(uint i)
   {
      return (i & 1 ? (light_index[i / 2] >> 16) : (light_index[i / 2])) & 0xffff;
   }

   void set(uint i, uint value)
   {
      if (i & 1) {
         light_index[i / 2] |= (value << 16);
      }
      else {
         light_index[i / 2] |= value;
      }
   }
};

struct culling_inputs {
   uint2 tile_counts;
   uint light_count;

   float2 tiles_startVS;
   float2 tile_lengthVS;

   float4 light_bounding_spheresVS[MAX_LIGHTS];
};

ConstantBuffer<culling_inputs> cull_inputs : register(b0, LIGHT_TILE_REGISTER_SPACE);
RWTexture2D<uint> light_tiles : register(u1, LIGHT_TILE_REGISTER_SPACE);
RWStructuredBuffer<light_index> light_tile_indices : register(u2, LIGHT_TILE_REGISTER_SPACE);

const static uint max_tile_lights = 32;

struct tile_bounds {
   float2 centreVS;
   float2 sizeVS;
};

tile_bounds make_bounds(float2 tile_index)
{
   tile_index.y = -tile_index.y;

   tile_bounds bounds;

   const float2 sizeVS = cull_inputs.tile_lengthVS * 0.5;

   bounds.centreVS = cull_inputs.tiles_startVS + cull_inputs.tile_lengthVS * tile_index;
   bounds.sizeVS = sizeVS;

   return bounds;
}

bool intersects(tile_bounds bounds, float4 bounding_sphereVS)
{
   const float2 positionTS = bounds.centreVS - bounding_sphereVS.xy;
   const float2 positionDS = abs(positionTS) - bounds.sizeVS;
   const float bounds_distance = length(max(positionDS, 0.0)) + min(max(positionDS.x, positionDS.y), 0.0);

   return bounds_distance <= bounding_sphereVS.w;
}

// clang-format off

[numthreads(8, 8, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID) {
   // clang-format on

   if (any(dispatch_id.xy >= cull_inputs.tile_counts)) return;

   tile_bounds bounds = make_bounds(dispatch_id.xy);
   uint tile_light_count = 0;

   [loop] for (uint i = 0; i < cull_inputs.light_count; ++i)
   {
      if (!intersects(bounds, cull_inputs.light_bounding_spheresVS[i])) continue;

      const uint tile_light_index = tile_light_count;

      tile_light_count += 1;

      if (tile_light_count <= max_tile_lights) {
         const uint tile_index = dispatch_id.x + dispatch_id.y * cull_inputs.tile_counts.y;

         light_tile_indices[tile_index].set(tile_light_index, i);
      }
   }

   light_tiles[dispatch_id.xy] = min(tile_light_count, max_tile_lights);
}
