#include "bindings.hlsli"
#include "frame_constants.hlsli"
#include "line_helpers.hlsli"
#include "srgb.hlsli"

struct meta_draw_line {
   float3 position0WS;
   uint color;
   float3 position1WS;
   uint pad;
};

StructuredBuffer<meta_draw_line> lines : register(META_DRAW_INSTANCE_DATA_REGISTER);

struct output_vertex {
   noperspective float line_distance : LINE_DISTANCE;
   nointerpolation float3 color : COLOR;
   float4 positionPS : SV_Position;
};

output_vertex main(uint vertex_id : SV_VertexID)
{
   uint line_id = vertex_id / 6;
   uint line_vertex_id = vertex_id % 6;

   meta_draw_line ln = lines[line_id];

   const float4 position0PS = mul(cb_frame.view_projection_matrix, float4(ln.position0WS, 1.0));
   const float4 position1PS = mul(cb_frame.view_projection_matrix, float4(ln.position1WS, 1.0));

   const float2 position0RT = to_rendertarget_position(position0PS);
   const float2 position1RT = to_rendertarget_position(position1PS);

   const float2 line_normalRT = normalize(position1RT - position0RT).yx * float2(-1.0, 1.0);

   output_vertex output;

   const float line_offset = floor(cb_frame.line_width + 0.5);

   float4 positionPS = 0.0;
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
      positionPS = from_rendertarget_position((position1RT + line_normalRT * line_offset), position1PS);
      distance_sign = 1.0;
      break;
   case 4:
      positionPS = from_rendertarget_position((position0RT - line_normalRT * line_offset), position0PS);
      distance_sign = -1.0;
      break;
   case 5:
      positionPS = from_rendertarget_position((position1RT - line_normalRT * line_offset), position1PS);
      distance_sign = -1.0;
      break;
   }

   output.line_distance = line_offset * distance_sign;
   output.color = unpack_bgra_srgb(ln.color).rgb;
   output.positionPS = positionPS;

   return output;
}