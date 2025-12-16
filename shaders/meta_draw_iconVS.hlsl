#include "bindings.hlsli"
#include "frame_constants.hlsli"

struct meta_draw_icon {
   float3 positionWS;
   float radius;
   float3 color;
   uint   pad;
};

StructuredBuffer<meta_draw_icon> icons : register(META_DRAW_INSTANCE_DATA_REGISTER);

struct output_vertex {
   float4 color : COLOR;
   float4 positionPS : SV_Position;
};

output_vertex main(float2 positionUS : POSITION, uint icon_id  : SV_InstanceID)
{
   meta_draw_icon icon = icons[icon_id];

   output_vertex output;

   output.color = float4(icon.color, 1.0);

   const float radius = icon.radius;

   float3 positionLS = mul((float3x3)cb_frame.world_from_view, float3(positionUS, 0.0) * radius);
   float3 positionWS = icon.positionWS + positionLS;

   output.positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));

   return output;
}