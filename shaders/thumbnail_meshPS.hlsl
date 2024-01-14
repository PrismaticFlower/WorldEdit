#include "bindings.hlsli"
#include "material_normal.hlsli"
#include "resource_heaps.hlsli"
#include "samplers.hlsli"

struct camera_constants {
   float3 positionOS;
};

ConstantBuffer<camera_constants> cb_camera : register(THUMBNAIL_CAMERA_CB_REGISTER);

const static float3 light_directionOS = normalize(float3(0.0, 1.0, 1.0));
const static float3 light_color = float3(0.6, 0.6, 0.6);

#ifdef THUMBNAIL_ALPHA_CUTOUT
const static bool alpha_cutout = true;
#else
const static bool alpha_cutout = false;
#endif

struct input_vertex {
   float3 positionOS : POSITION;
   float3 normalOS : NORMAL;
   float3 tangentOS : TANGENT;
   float3 bitangentOS : BITANGENT;
   float2 texcoords : TEXCOORD;
   float4 color : COLOR;

   float4 positionSS : SV_Position;
};

float3 transform_normalOS(const input_vertex input, const float3 normalTS)
{
   return normalize(mul(normalTS, float3x3(input.tangentOS, input.bitangentOS, input.normalOS)));
}

float3 calculate_light(float3 normalOS, float3 viewOS, float3 diffuse_color, float3 specular_color)
{
   const float3 diffuse = saturate(dot(normalOS, light_directionOS)) * diffuse_color;

   const float3 half_vectorOS = normalize(light_directionOS + viewOS);
   const float NdotH = saturate(dot(normalOS, half_vectorOS));
   const float3 specular = pow(NdotH, 64.0) * specular_color;

   return (diffuse + specular) * light_color + (diffuse_color * 0.1);
}

float4 main(input_vertex input) : SV_TARGET
{
   Texture2D diffuse_map = Texture2DHeap[material.diffuse_map_index];

   const float2 texcoords = input.texcoords;
   
   float4 normal_map_sample = float4(0.5, 0.5, 1.0, 1.0);

   if (material.flags & flags::has_normal_map) {
      Texture2D normal_map = Texture2DHeap[material.normal_map_index];

      if (material.flags & flags::tile_normal_map) {
         normal_map_sample =
            normal_map.Sample(sampler_anisotropic_wrap, texcoords * material.detail_scale);
      }
      else {
         normal_map_sample = normal_map.Sample(sampler_anisotropic_wrap, texcoords);
      }
   }

   const float3 normalTS = normal_map_sample.xyz * 2.0 - 1.0;
   const float3 normalOS = transform_normalOS(input, normalTS);

   const float3 positionOS = input.positionOS;
   const float3 viewOS = normalize(cb_camera.positionOS - normalOS);
   float4 diffuse_color = diffuse_map.Sample(sampler_anisotropic_wrap, texcoords);

   if (alpha_cutout) {
      if (diffuse_color.a < 0.5) {
         discard;
      }
   }
   
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

   if (material.flags & flags::transparent)
      diffuse_color.rgb *= diffuse_color.a;

   if (material.flags & flags::unlit)
      return diffuse_color;

   float specular_visibility = 1.0;

   if (material.flags & flags::specular_visibility_in_diffuse_map) {
      specular_visibility = diffuse_color.a;
   }
   else {
      specular_visibility = normal_map_sample.a;
   }
   
   float3 lighting;
  
   if (static_lighting) {
      lighting = input.color.rgb * diffuse_color.rgb;
   }
   else {
      lighting = calculate_light(normalOS, viewOS, diffuse_color.rgb, material.specular_color * specular_visibility);
   }

   if (material.flags & flags::has_env_map) {
      TextureCube env_map = TextureCubeHeap[material.env_map_index];

      const float3 reflectionWS = normalize(reflect(-viewOS, normalOS));

      lighting += env_map.Sample(sampler_anisotropic_wrap, reflectionWS).rgb * material.env_color *
                  specular_visibility;
   }

   float alpha = 1.0;
   
   if (material.flags & flags::transparent)
      alpha = diffuse_color.a;

   return float4(lighting, alpha);
}