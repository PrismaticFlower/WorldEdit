
#define TILE_LIGHT_WORDS 8

cbuffer clear_inputs : register(b0) {
   uint2 tile_counts;
};

const static uint4 clear_value[TILE_LIGHT_WORDS / 4] = {uint4(0, 0, 0, 0), uint4(0, 0, 0, 0)};

RWStructuredBuffer<uint4[TILE_LIGHT_WORDS / 4]> light_tile_indices : register(u0);

[numthreads(32, 32, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID) {
   if (any(dispatch_id.xy >= tile_counts)) return;

   const uint tile_index = dispatch_id.x + dispatch_id.y * tile_counts.x;

   light_tile_indices[tile_index] = clear_value;
}
