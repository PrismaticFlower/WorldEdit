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

   const float2 position0NDC = (position0PS.xy / position0PS.w);
   const float2 position1NDC = (position1PS.xy / position1PS.w);

   const float2 line_normalNDC = normalize(position1NDC - position0NDC).yx * float2(-1.0, 1.0);

   output_vertex output;

   const float2 line_offset = (cb_frame.line_width + 1.0) / cb_frame.viewport_size * 2.0;

   float4 positionPS = 0.0;
   float distance_sign = 1.0;

   switch (line_vertex_id) {
   case 0:
      positionPS = float4((position0NDC - line_normalNDC * line_offset) * position0PS.w, position0PS.zw);
      distance_sign = -1.0;
      break;
   case 1:
      positionPS = float4((position0NDC + line_normalNDC * line_offset) * position0PS.w, position0PS.zw);
      distance_sign = 1.0;
      break;
   case 2:
      positionPS = float4((position1NDC + line_normalNDC * line_offset) * position1PS.w, position1PS.zw);
      distance_sign = 1.0;
      break;
   case 3:
      positionPS = float4((position1NDC + line_normalNDC * line_offset) * position1PS.w, position1PS.zw);
      distance_sign = 1.0;
      break;
   case 4:
      positionPS = float4((position0NDC - line_normalNDC * line_offset) * position0PS.w, position0PS.zw);
      distance_sign = -1.0;
      break;
   case 5:
      positionPS = float4((position1NDC - line_normalNDC * line_offset) * position1PS.w, position1PS.zw);
      distance_sign = -1.0;
      break;
   }

   output.line_distance = (cb_frame.line_width + 1.0) * distance_sign;
   output.color = unpack_bgra_srgb(ln.color).rgb;
   output.positionPS = positionPS;

   return output;
}