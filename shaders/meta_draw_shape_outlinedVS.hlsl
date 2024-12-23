#include "bindings.hlsli"
#include "frame_constants.hlsli"

struct meta_draw_object {
   float4x4 transform;

   float4 color;
   float4 outlined_color;
};

StructuredBuffer<meta_draw_object> instance_descs : register(META_DRAW_INSTANCE_DATA_REGISTER);

struct input_vertex {
   float3 positionOS : POSITION;
   uint instance_id : SV_InstanceID;
};

struct output_vertex {
   float4 positionPS : SV_Position;
   float4 color : COLOR;
   float4 outline_color : OUTLINE_COLOR;
   nointerpolation float4 flat_positionPS : FLAT_POSITIONPS;
};

output_vertex main(input_vertex input)
{
   meta_draw_object instance = instance_descs[input.instance_id];

   output_vertex output;

   const float3 positionWS = mul(instance.transform, float4(input.positionOS, 1.0)).xyz;
   const float4 positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));

   output.positionPS = positionPS;
   output.color = instance.color;
   output.outline_color = instance.outlined_color;
   output.flat_positionPS = positionPS;

   return output;
}