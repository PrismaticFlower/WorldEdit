#include "bindings.hlsli"
#include "frame_constants.hlsli"

struct meta_draw_object {
   float4x4 transform;
};

StructuredBuffer<meta_draw_object> instance_descs : register(META_DRAW_INSTANCE_DATA_REGISTER);

struct input_vertex {
   float3 positionOS : POSITION;
   uint instance_id : SV_InstanceID;
};

float4 main(input_vertex input) : SV_Position
{
   meta_draw_object instance = instance_descs[input.instance_id];
   
   const float3 positionWS = mul(instance.transform, float4(input.positionOS, 1.0)).xyz;
   
   return mul(cb_frame.projection_from_world, float4(positionWS, 1.0));
}