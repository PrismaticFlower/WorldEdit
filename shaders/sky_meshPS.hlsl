
#include "bindings.hlsli"
#include "frame_constants.hlsli"
#include "material_normal.hlsli"
#include "resource_heaps.hlsli"
#include "samplers.hlsli"
#include "sky_mesh_constants.hlsli"

struct input_vertex {
   float2 texcoords : TEXCOORD;
   float4 color : COLOR;
};

float4 main(input_vertex input) : SV_TARGET
{
   Texture2D diffuse_map = Texture2DHeap[material.diffuse_map_index];

   float2 texcoords = input.texcoords;

   if (material.flags & flags::scrolling) {
      texcoords -= material.scrolling_amount * cb_frame.texture_scroll_duration;
   }

   float4 diffuse_color = diffuse_map.Sample(sampler_anisotropic_wrap, texcoords);

   if (cb_mesh_constants.alpha_cutout && diffuse_color.a < 0.5) discard;

   if (material.flags & flags::has_detail_map) {
      Texture2D detail_map = Texture2DHeap[material.detail_map_index];

      diffuse_color.rgb *=
         (detail_map.Sample(sampler_anisotropic_wrap, texcoords * material.detail_scale).rgb * 2.0);
   }

   const bool static_lighting = material.flags & flags::static_lighting;

   if (static_lighting) {
      diffuse_color.a *= input.color.a;
   }
   else {
      diffuse_color *= input.color;
   }

   if (material.flags & flags::transparent) {
      diffuse_color.rgb *= diffuse_color.a;
   }
   else {
      diffuse_color.a = 1.0;
   }

   return diffuse_color;
}