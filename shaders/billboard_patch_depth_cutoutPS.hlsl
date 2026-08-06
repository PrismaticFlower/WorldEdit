#include "resource_heaps.hlsli"
#include "samplers.hlsli"
#include "bindings.hlsli"

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS  : NORMALWS;
   float2 texcoords : TEXCOORD;
   float  fog : FOG;
   float  darkening : DARKNESS;

   float4 positionPS : SV_Position;
};

cbuffer TextureIndex : register(LEAF_PATCH_TEXTURE_CB_REGISTER)
{
   uint texture_index;
};

void main(input_vertex input)
{
   Texture2D diffuse_map = Texture2DHeap[texture_index];

   if (diffuse_map.Sample(sampler_anisotropic_wrap, input.texcoords).a < 0.5) discard;
}