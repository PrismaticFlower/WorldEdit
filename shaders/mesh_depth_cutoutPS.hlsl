
#include "bindless.hlsli"

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMAL;
   float3 tangentWS : TANGENT;
   float3 bitangentWS : BITANGENT;
   float2 texcoords : TEXCOORD;
};

struct material_input {
   uint flags;
   uint diffuse_map;
   uint normal_map;
};

ConstantBuffer<material_input> material : register(b0, space1);
SamplerState texture_sampler : register(s0);

void main(input_vertex input_vertex)
{
   Texture2D<float4> diffuse_map = GetTexture2D(material.diffuse_map);

   float4 diffuse_color = diffuse_map.Sample(texture_sampler, input_vertex.texcoords);

   if (diffuse_color.a < 0.5) discard;
}