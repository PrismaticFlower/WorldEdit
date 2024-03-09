
#include "frame_constants.hlsli"
#include "terrain_common.hlsli"

float patch(float2 position)
{
   const float spacing = 2.0;
   const float size = 0.25;
   const float aa_scale = 1.5;

   position = position - spacing * round(position / spacing);

   float2 distance2 = abs(position) - size;
   float distance = max(distance2.x, distance2.y);

   float width = 0.5 * fwidth(distance) * aa_scale;

   return 1.0 - smoothstep(-width, width, distance);
}


[earlydepthstencil]
float4 main(input_vertex input) : SV_Target0
{
   float2 position = (input.positionWS.xz + terrain_constants.half_world_size)  + float2(0, terrain_constants.grid_size);

   int2 foliage_coords =  int2(position / terrain_constants.grid_size) / 2;

   foliage_coords.y -= 1;

   uint foliage_mask = foliage_map[int2(foliage_coords.x, foliage_coords.y)].r;

   if (foliage_mask == 0) discard;

   const float foliage_transparency = terrain_constants.foliage_transparency;

   float4 color = float4(0.0, 0.0, 0.0, 0.0);

   position *= terrain_constants.inv_grid_size;

   if (foliage_mask & 0b1) {
      float alpha = patch(position - 1.0 + float2(0.25, 0.25));

      color.rgb = lerp(color.rgb, terrain_constants.foliage_color_0, alpha);
      color.a = max(color.a, foliage_transparency * alpha);
   }
   
   if (foliage_mask & 0b10) {
      float alpha = patch(position - 1.0 + float2(0.25, -0.25));

      color.rgb = lerp(color.rgb, terrain_constants.foliage_color_1, alpha);
      color.a = max(color.a, foliage_transparency * alpha);
   }

   if (foliage_mask & 0b100) {
      float alpha = patch(position - 1.0 + float2(-0.25, 0.25));

      color.rgb = lerp(color.rgb, terrain_constants.foliage_color_2, alpha);
      color.a = max(color.a, foliage_transparency * alpha);
   }

   if (foliage_mask & 0b1000) {
      float alpha = patch(position - 1.0 + float2(-0.25, -0.25));

      color.rgb = lerp(color.rgb, terrain_constants.foliage_color_3, alpha);
      color.a = max(color.a, foliage_transparency * alpha);
   }

   if (color.a <= 1.0 / 512.0) discard;

   return color;
}
