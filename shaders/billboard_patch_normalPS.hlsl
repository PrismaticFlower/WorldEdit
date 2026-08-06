
#include "frame_constants.hlsli"
#include "fog.hlsli"
#include "lights_common.hlsli"
#include "resource_heaps.hlsli"
#include "samplers.hlsli"

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

float4 main(input_vertex input) : SV_TARGET
{
   Texture2D diffuse_map = Texture2DHeap[texture_index];

   const float3 positionWS = input.positionWS;
   float3 normalWS = normalize(input.normalWS);

   float3 diffuse_color = diffuse_map.Sample(sampler_anisotropic_wrap, input.texcoords).rgb;
   
   diffuse_color *= input.darkening;

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.viewWS = normalize(cb_frame.view_positionWS - positionWS);
   lighting_inputs.diffuse_color = diffuse_color;
   lighting_inputs.specular_color = 0.0;
   lighting_inputs.positionSS = input.positionPS.xy;
   lighting_inputs.receive_static_light = true;

   float3 lighting = calculate_lighting(lighting_inputs);

   return apply_fog(float4(lighting, 1.0), input.fog);
}