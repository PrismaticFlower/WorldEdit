
#include "frame_constants.hlsli"
#include "fog.hlsli"
#include "samplers.hlsli"
#include "resource_heaps.hlsli"

struct input_vertex {
   float2 texcoords : TEXCOORD;
   float4 color : COLOR;
   float  fog : FOG;

   float4 positionPS : SV_Position;
};

cbuffer TextureIndex : register(LEAF_PATCH_TEXTURE_CB_REGISTER)
{
   uint texture_index;
};

float4 main(input_vertex input) : SV_TARGET
{
   Texture2D color_map = Texture2DHeap[texture_index];

   float4 color = color_map.Sample(sampler_anisotropic_wrap, input.texcoords);
   
   color *= input.color;

   return apply_fog(color, input.fog);
}