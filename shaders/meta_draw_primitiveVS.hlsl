#include "frame_constants.hlsli"

float decompress_srgb(const float v)
{
   return (v < 0.04045) ? v / 12.92 : pow(abs((v + 0.055)) / 1.055, 2.4);
}

float4 decompress_srgb(const float4 color)
{
   return float4(decompress_srgb(color.x), decompress_srgb(color.y), decompress_srgb(color.z), color.w);
}

struct input_vertex {
   float3 positionWS : POSITION;
   float4 color : COLOR;
};

struct output_vertex {
   float4 color : COLOR;
   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   const float3 positionWS = input.positionWS;
   const float4 positionPS = mul(cb_frame.view_projection_matrix, float4(positionWS, 1.0));

   output.positionPS = positionPS;
   output.color = decompress_srgb(input.color);

   return output;
}