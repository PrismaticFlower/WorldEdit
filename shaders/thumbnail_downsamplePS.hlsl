#include "resource_heaps.hlsli"
#include "samplers.hlsli"

struct apply_inputs {
   uint thumbnail_index;
};

ConstantBuffer<apply_inputs> input : register(b0);

static Texture2D thumbnail = Texture2DHeap[input.thumbnail_index];

float4 main(float2 texcoords : TEXCOORDS) : SV_TARGET
{
   float4 color;
   
   color = thumbnail.Sample(sampler_bilinear_clamp, texcoords, int2(-1, -1)) * 0.25;
   color += thumbnail.Sample(sampler_bilinear_clamp, texcoords, int2(-1, 1)) * 0.25;
   color += thumbnail.Sample(sampler_bilinear_clamp, texcoords, int2(1, -1)) * 0.25;
   color += thumbnail.Sample(sampler_bilinear_clamp, texcoords, int2(1, 1)) * 0.25;
   
   return color;
}