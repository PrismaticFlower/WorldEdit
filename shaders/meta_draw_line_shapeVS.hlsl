#include "bindings.hlsli"
#include "frame_constants.hlsli"
#include "line_helpers.hlsli"

struct meta_draw_instance {
   float3x4 world_from_local;
   float4 color;
};

struct meta_draw_line {
   float3 position0LS;
   float3 position1LS;
};

StructuredBuffer<meta_draw_instance> instances : register(META_DRAW_INSTANCE_DATA_REGISTER);
StructuredBuffer<meta_draw_line> lines : register(META_DRAW_LINE_SHAPE_DATA_REGISTER);

struct output_vertex {
   noperspective float line_distance : LINE_DISTANCE;
   float4 color : COLOR;
   float4 positionPS : SV_Position;
};

output_vertex main(uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
   // The index of the line is stored in the bottom 14 bits and the index 
   // of the vertex for the line's quad is stored in the top two.
   uint line_id = vertex_id & 0x3fff;
   uint line_vertex_id = (vertex_id & 0xc000) >> 14;

   meta_draw_instance instance = instances[instance_id];
   meta_draw_line ln = lines[line_id];

   const float3 position0WS = mul(instance.world_from_local, float4(ln.position0LS, 1.0));
   const float3 position1WS = mul(instance.world_from_local, float4(ln.position1LS, 1.0));

   const float4 position0PS = mul(cb_frame.projection_from_world, float4(position0WS, 1.0));
   const float4 position1PS = mul(cb_frame.projection_from_world, float4(position1WS, 1.0));

   const float2 position0RT = to_rendertarget_position(position0PS);
   const float2 position1RT = to_rendertarget_position(position1PS);

   const float2 line_normalRT = normalize(position1RT - position0RT).yx * float2(-1.0, 1.0);

   output_vertex output;

   const float line_offset = floor(cb_frame.line_width + 0.5);

   float4 positionPS = 0.0;
   float4 color = 0.0;
   float distance_sign = 1.0;

   switch (line_vertex_id) {
   case 0:
      positionPS = from_rendertarget_position((position0RT - line_normalRT * line_offset), position0PS);
      distance_sign = -1.0;
      break;
   case 1:
      positionPS = from_rendertarget_position((position0RT + line_normalRT * line_offset), position0PS);
      distance_sign = 1.0;
      break;
   case 2:
      positionPS = from_rendertarget_position((position1RT + line_normalRT * line_offset), position1PS);
      distance_sign = 1.0;
      break;
   case 3:
      positionPS = from_rendertarget_position((position1RT - line_normalRT * line_offset), position1PS);
      distance_sign = -1.0;
      break;
   }

   output.line_distance = line_offset * distance_sign;
   output.color = instance.color;
   output.positionPS = positionPS;

   return output;
}