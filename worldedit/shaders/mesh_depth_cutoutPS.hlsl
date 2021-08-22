
struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMAL;
   float3 tangentWS : TANGENT;
   float3 bitangentWS : BITANGENT;
   float2 texcoords : TEXCOORD;
};

Texture2D<float4> diffuse_map : register(t0, space1);
SamplerState texture_sampler : register(s0);

void main(input_vertex input_vertex)
{
   float4 diffuse_color = diffuse_map.Sample(texture_sampler, input_vertex.texcoords);

   if (diffuse_color.a < 0.5) discard;
}