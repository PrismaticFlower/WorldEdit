#include "bindings.hlsli"
#include "frame_constants.hlsli"
#include "line_helpers.hlsli"

struct wireframe_constants {
   float3 color;
};

ConstantBuffer<wireframe_constants> cb_wireframe_constants : register(META_MESH_CB_REGISTER);

struct input_vertex {
   float4 positionRT : SV_Position;
   nointerpolation float4 positionPS : FLAT_POSITIONPS;
};

[earlydepthstencil] float4 main(input_vertex input_vertex) : SV_TARGET
{

   const float4 positionPS0 = GetAttributeAtVertex(input_vertex.positionPS, 0);
   const float4 positionPS1 = GetAttributeAtVertex(input_vertex.positionPS, 1);
   const float4 positionPS2 = GetAttributeAtVertex(input_vertex.positionPS, 2);

   const float2 positionRT0 = to_rendertarget_position(positionPS0);
   const float2 positionRT1 = to_rendertarget_position(positionPS1);
   const float2 positionRT2 = to_rendertarget_position(positionPS2);

   const float distance0 = line_distance(input_vertex.positionRT.xy, positionRT0, positionRT1);
   const float distance1 = line_distance(input_vertex.positionRT.xy, positionRT1, positionRT2);
   const float distance2 = line_distance(input_vertex.positionRT.xy, positionRT2, positionRT0);

   const float min_distance = min(min(distance0, distance1), distance2);
   const float alpha = line_alpha(min_distance);

   if (alpha == 0.0) discard;

   return float4(cb_wireframe_constants.color, line_alpha(min_distance));
}
