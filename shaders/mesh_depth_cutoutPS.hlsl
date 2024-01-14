
#include "material_normal.hlsli"
#include "resource_heaps.hlsli"
#include "samplers.hlsli"

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMAL;
   float3 tangentWS : TANGENT;
   float3 bitangentWS : BITANGENT;
   float2 texcoords : TEXCOORD;
};

void main(input_vertex input_vertex)
{
   Texture2D diffuse_map = Texture2DHeap[material.diffuse_map_index];

   float4 diffuse_color = diffuse_map.Sample(sampler_anisotropic_wrap, input_vertex.texcoords);

   if (diffuse_color.a < 0.5) discard;
}