#include "bindings.hlsli"
#include "frame_constants.hlsli"

cbuffer block_description : register(BLOCK_CUSTOM_MESH_CB_REGISTER) {
   uint surface_index;
   float alpha;
   float4x4 world_from_local;
};

const static uint show_all_surfaces = 0xffffffffu;
const static float NaN = asfloat(0x7fc00000);

struct input_vertex {
   float3 positionLS : POSITION;
   float3 normalLS : NORMAL;
   float2 texcoords : TEXCOORD;
   uint surface_index : SURFACE;
};

struct output_vertex {
   float4 color : COLOR;
   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   const float3 positionWS = mul(world_from_local, float4(input.positionLS, 1.0)).xyz;
   float4 positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));

   if (surface_index != show_all_surfaces && surface_index != input.surface_index) {
      positionPS = NaN;
   }

   output.color = float4(1.0, 1.0, 1.0, alpha);
   output.positionPS = positionPS;

   return output;
}