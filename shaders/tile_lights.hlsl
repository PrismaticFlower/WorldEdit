
#define TILE_LIGHT_WORDS 8

struct tiling_inputs {
   uint2 tile_counts;

   float4x4 view_projection_matrix;
};

struct instance_data {
   row_major float3x4 transform;
   uint light_index;
   uint3 padding;
};

ConstantBuffer<tiling_inputs> tile_inputs : register(b0);
StructuredBuffer<instance_data> instances_data : register(t0);
RWStructuredBuffer<uint[TILE_LIGHT_WORDS]> light_tiles : register(u0);

struct input_vs {
   float3 positionOS : POSITION;
   uint instance : SV_InstanceID;
};

struct output_vs {
   nointerpolation uint light_word : LIGHTWORD;
   nointerpolation uint light_bit : LIGHTBIT;
   float4 position : SV_Position;
};

// clang-format off

output_vs mainVS(input_vs input)
{
   instance_data instance = instances_data[input.instance];

   float3 positionWS = mul(instance.transform, float4(input.positionOS, 1.0));

   output_vs output;

   output.position = mul(tile_inputs.view_projection_matrix, float4(positionWS, 1.0));
   output.light_word = instance.light_index / 32u;
   output.light_bit = 1u << (instance.light_index % 32u);

   return output;
}

[earlydepthstencil] 
void mainPS(output_vs input)
{
   const uint2 tile_coord = input.position.xy;
   const uint tile_index = tile_coord.x + tile_coord.y * tile_inputs.tile_counts.x;

   InterlockedOr(light_tiles[tile_index][input.light_word], input.light_bit);
}
