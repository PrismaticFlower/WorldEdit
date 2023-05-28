#include "frame_constants.hlsli"
#include "water_constants.hlsli"

struct output_vertex
{
   float3 positionWS : POSITIONWS;
   float4 positionPS : SV_Position;
};

struct patch_bbox
{
   float2 min;
   float2 max;
};

StructuredBuffer<patch_bbox> patches : register(WATER_PATCH_DATA_REGISTER);

output_vertex main(uint vertex_index : SV_VertexID)
{
   uint patch_index = vertex_index / 6;
   
   patch_bbox patch = patches[patch_index];
   
   float3 min_positionWS = {patch.min.x, 0.0, patch.min.y};
   float3 max_positionWS = {patch.max.x, 0.0, patch.max.y};
   
   float3 positionWS = 0.0;
   
   switch (vertex_index % 6)
   {
      case 0:
         positionWS = float3(min_positionWS.x, cb_water.height, min_positionWS.z);
         break;
      case 1:
         positionWS = float3(max_positionWS.x, cb_water.height, min_positionWS.z);
         break;
      case 2:
         positionWS = float3(min_positionWS.x, cb_water.height, max_positionWS.z);
         break;
      case 3:
         positionWS = float3(max_positionWS.x, cb_water.height, max_positionWS.z);
         break;
      case 4:
         positionWS = float3(max_positionWS.x, cb_water.height, min_positionWS.z);
         break;
      case 5:
         positionWS = float3(min_positionWS.x, cb_water.height, max_positionWS.z);
         break;
   }
   
   output_vertex output;

   output.positionWS = positionWS;
   output.positionPS = mul(cb_frame.view_projection_matrix, float4(positionWS, 1.0));

   return output;
}