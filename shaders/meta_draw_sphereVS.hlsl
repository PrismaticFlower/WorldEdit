#include "bindings.hlsli"
#include "frame_constants.hlsli"

struct meta_draw_sphere {
   float3 positionWS;
   float radius;

   float4 color;
};

StructuredBuffer<meta_draw_sphere> instance_descs : register(META_DRAW_INSTANCE_DATA_REGISTER);

struct input_vertex {
   float3 positionOS : POSITION;
   uint instance_id : SV_InstanceID;
};

struct output_vertex {
   float4 color : COLOR;
   float4 positionPS : SV_Position;
   float4 flat_positionPS : FLAT_POSITIONPS;
};

output_vertex main(input_vertex input)
{
   meta_draw_sphere instance = instance_descs[input.instance_id];

   output_vertex output;

   const float3 positionWS = input.positionOS * instance.radius + instance.positionWS;
   const float4 positionPS = mul(cb_frame.view_projection_matrix, float4(positionWS, 1.0));

   output.positionPS = positionPS;
   output.flat_positionPS = positionPS;
   output.color = instance.color;

   return output;
}