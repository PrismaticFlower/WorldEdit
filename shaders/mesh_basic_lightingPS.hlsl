
#include "frame_constants.hlsli"
#include "lights_common.hlsli"

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMAL;
   float3 tangentWS : TANGENT;
   float3 bitangentWS : BITANGENT;
   float2 texcoords : TEXCOORD;

   float4 positionSS : SV_Position;
};

const static float3 surface_color = 0.75;

float4 main(input_vertex input_vertex) : SV_TARGET
{
   const float3 normalWS = normalize(input_vertex.normalWS);
   const float3 positionWS = input_vertex.positionWS;
   const float3 viewWS = normalize(cb_frame.view_positionWS - positionWS);

   calculate_light_inputs lighting_inputs;

   lighting_inputs.positionWS = positionWS;
   lighting_inputs.normalWS = normalWS;
   lighting_inputs.viewWS = viewWS;
   lighting_inputs.diffuse_color = surface_color;
   lighting_inputs.specular_color = 0.0;
   lighting_inputs.positionSS = input_vertex.positionSS.xy;

   const float3 lighting = calculate_lighting(lighting_inputs);

   return float4(lighting, 1.0f);
}