#include "samplers.hlsli"

const static float3 corners[8] = {
   float3(1.0, 1.0, 1.0),
   float3(-1.0, 1.0, -1.0),
   float3(-1.0, 1.0, 1.0),
   float3(1.0, 1.0, -1.0),
   
   float3(1.0, -1.0, 1.0),
   float3(-1.0, -1.0, -1.0),
   float3(-1.0, -1.0, 1.0),
   float3(1.0, -1.0, -1.0)
};

const static float3 positions[6][4] = {
   {corners[0], corners[3], corners[4], corners[7]}, // X+
   {corners[1], corners[2], corners[5], corners[6]}, // X-
   {corners[0], corners[1], corners[2], corners[3]}, // Y+
   {corners[4], corners[5], corners[6], corners[7]}, // Y-
   {corners[0], corners[2], corners[4], corners[6]}, // Z+
   {corners[1], corners[3], corners[5], corners[7]}, // Z-
};

struct apply_inputs {
   uint env_map_index;
};

ConstantBuffer<apply_inputs> input : register(b0);

static TextureCube env_map = ResourceDescriptorHeap[input.env_map_index];

float4 main(float2 location : LOCATION, uint face_index : SV_RenderTargetArrayIndex) : SV_TARGET
{
   const float3 position = lerp(lerp(positions[face_index][0], positions[face_index][0], location.x), lerp(positions[face_index][2], positions[face_index][3], location.x), location.y);
   
   return env_map.Sample(sampler_bilinear_wrap, position);
}