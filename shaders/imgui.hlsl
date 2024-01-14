
#include "resource_heaps.hlsli"
#include "samplers.hlsli"
#include "srgb.hlsli"

struct input_vertex {
   float2 position : POSITION;
   float2 texcoords : TEXCOORDS;
   float4 color : COLOR;
};

struct output_vertex {
   float4 position : SV_Position;
   float2 texcoords : TEXCOORDS;
   float4 color : COLOR;
};

// clang-format off

cbuffer TextureConstants : register(b0)
{
   uint texture_index;
};

cbuffer ViewportConstants : register(b1)
{
   float2 inv_viewport_size;
};

// clang-format on

output_vertex mainVS(input_vertex input)
{
   output_vertex output;

   output.position = float4((input.position * inv_viewport_size - 0.5) * float2(2.0, -2.0), 0.0, 1.0);
   output.texcoords = input.texcoords;
   output.color = float4(srgb_to_linear(input.color.rgb), input.color.a);

   return output;
}

float4 mainPS(output_vertex input) : SV_Target0
{
   Texture2D tex = Texture2DHeap[texture_index];

   return tex.Sample(sampler_bilinear_wrap, input.texcoords) * input.color;
}