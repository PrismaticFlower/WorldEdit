
#include "lights_common.hlsli"

namespace material {
const static bool alpha_cutout = MATERIAL_ALPHA_CUTOUT;
const static bool transparent = MATERIAL_TRANSPARENT;
}

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMALWS;
   float2 texcoords : TEXCOORD;
};

Texture2D<float4> diffuse_map : register(t0, space1);
Texture2D<float4> normal_map : register(t1, space1);

SamplerState trilinear_sampler : register(s0);

// TODO: Generate tangent vectors on CPU.
float3 TEMP_get_normalWS(const float3 normalWS, const float3 positionWS,
                         const float2 texcoords, const float3 normalTS)
{
   const float3 pos_dx = ddx(positionWS);
   const float3 pos_dy = ddy(positionWS);
   const float2 tex_dx = ddx(texcoords);
   const float2 tex_dy = ddy(texcoords);

   if ((tex_dx.x * tex_dy.y - tex_dy.x * tex_dx.y) == 0.0) {
      return normalWS;
   }

   const float3 unorm_tangentWS = (tex_dy.y * pos_dx - tex_dx.y * pos_dy) *
                                  rcp(tex_dx.x * tex_dy.y - tex_dy.x * tex_dx.y);

   const float3 tangentWS =
      normalize(unorm_tangentWS - normalWS * dot(normalWS, unorm_tangentWS));
   const float3 bitangentWS = cross(normalWS, tangentWS);

   return normalize(mul(normalTS, float3x3(tangentWS, bitangentWS, normalWS)));
}

float4 main(input_vertex input_vertex) : SV_TARGET
{
   const float3 normalTS =
      normal_map.Sample(trilinear_sampler, input_vertex.texcoords) * 2.0 - 1.0;
   const float3 normalWS =
      TEMP_get_normalWS(input_vertex.normalWS, input_vertex.positionWS,
                        input_vertex.texcoords, normalTS);

   const float3 positionWS = input_vertex.positionWS;
   float4 diffuse_color =
      diffuse_map.Sample(trilinear_sampler, input_vertex.texcoords);

   if (material::alpha_cutout && diffuse_color.a < 0.5) discard;
   if (material::transparent) diffuse_color.rgb *= diffuse_color.a;

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.diffuse_color = diffuse_color;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, material::transparent ? diffuse_color.a : 1.0);
}