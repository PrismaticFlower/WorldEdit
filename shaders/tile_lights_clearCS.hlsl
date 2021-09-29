
#define TILE_LIGHT_WORDS 8

struct clear_inputs {
   uint2 tile_counts;
   uint4 clear_value[TILE_LIGHT_WORDS / 4];
};

ConstantBuffer<clear_inputs> inputs : register(b0);
RWStructuredBuffer<uint4[TILE_LIGHT_WORDS / 4]> light_tile_indices : register(u0);

// clang-format off

[numthreads(32, 32, 1)]
void main(uint3 dispatch_id : SV_DispatchThreadID) {
   // clang-format on

   if (any(dispatch_id.xy >= inputs.tile_counts)) return;

   const uint tile_index = dispatch_id.x + dispatch_id.y * inputs.tile_counts.x;

   light_tile_indices[tile_index] = inputs.clear_value;
}
