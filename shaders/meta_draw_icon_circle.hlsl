#include "bindings.hlsli"
#include "frame_constants.hlsli"

struct meta_draw_icon {
   float3 positionWS;
   float  radius;
   float3 color;
   uint   pad;
};

StructuredBuffer<meta_draw_icon> icons : register(META_DRAW_INSTANCE_DATA_REGISTER);

struct output_vertex {
   nointerpolation float3 color : COLOR;
   float2 positionUS : POSITIONUS;
   float4 positionPS : SV_Position;
};

output_vertex mainVS(uint vertex_id : SV_VertexID)
{
   uint icon_id = vertex_id / 6;
   uint icon_vertex_id = vertex_id % 6;

   meta_draw_icon icon = icons[icon_id];

   float2 positionUS = 0.0;

   switch (icon_vertex_id) {
   case 0:
      positionUS = float2(-1.0, -1.0);
      break;
   case 1:
      positionUS = float2(1.0, -1.0);
      break;
   case 2:
      positionUS = float2(1.0, 1.0);
      break;
   case 3:
      positionUS = float2(-1.0, -1.0);
      break;
   case 4:
      positionUS = float2(1.0, 1.0);
      break;
   case 5:
      positionUS = float2(-1.0, 1.0);
      break;
   }

   output_vertex output;

   output.color = icon.color;

   const float radius = icon.radius;

   float3 positionLS = mul((float3x3)cb_frame.world_from_view, float3(positionUS, 0.0) * radius);
   float3 positionWS = icon.positionWS + positionLS;

   output.positionUS = positionUS;
   output.positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));

   return output;
}

float4 mainPS(output_vertex input) : SV_TARGET
{
   if (dot(input.positionUS, input.positionUS) > 1.0) discard;

   return float4(input.color, 1.0);
}