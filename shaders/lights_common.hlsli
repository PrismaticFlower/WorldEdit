#pragma once

#include "bindings.hlsli"
#include "samplers.hlsli"

#define MAX_LIGHTS 256
#define TILE_LIGHT_WORDS 8
#define LIGHT_REGISTER_SPACE space3

namespace light_type {
const static uint directional = 0;
const static uint point_ = 1;
const static uint spot = 2;
};

namespace directional_region_type {
const static uint none = 0;
const static uint box = 1;
const static uint sphere = 2;
const static uint cylinder = 3;
};

struct light_description {
   float3 directionWS;
   uint type;
   float3 positionWS;
   float range;
   float3 color;
   float spot_outer_param;
   float spot_inner_param;
   uint region_type;
   uint directional_region_index;
   uint shadow_caster;
};

struct light_constant_buffer {
   uint light_tiles_width;
   uint light_tiles_index;
   uint light_region_list_index;
   uint shadow_map_index;
   float3 sky_ambient_color;
   uint padding1;
   float3 ground_ambient_color;
   uint padding2;
   float4x4 shadow_cascade_transforms[4];
   float2 shadow_map_resolution;
   float2 inv_shadow_map_resolution;

   light_description lights[MAX_LIGHTS];
};

struct light_region_description {
   float4x4 world_to_region;
   float3 position;
   uint type;
   float3 size;
   uint padding;
};

struct calculate_light_inputs {
   float3 positionWS;
   float3 normalWS;
   float3 viewWS;
   float3 diffuse_color;
   float3 specular_color;
   uint2 positionSS;
};

struct light_info {
   float3 light_directionWS;
   float falloff;
};

const static float region_fade_distance_sq = 0.1 * 0.1;
const static int shadow_cascade_count = 4;
const static uint light_tile_size = 8;
const static uint light_tile_word_bits = 32;

ConstantBuffer<light_constant_buffer> light_constants : register(LIGHTS_CB_REGISTER);
static StructuredBuffer<uint[TILE_LIGHT_WORDS]> light_tiles =
   ResourceDescriptorHeap[light_constants.light_tiles_index];
static StructuredBuffer<light_region_description> light_region_list =
   ResourceDescriptorHeap[light_constants.light_region_list_index];
const static float shadow_map_bias = 0.001;

float shadow_cascade_signed_distance(float3 positionLS)
{
   float3 centered_positionLS = (positionLS - 0.5);
   float3 distance = abs(centered_positionLS) - 0.5;

   return max(max(distance.x, distance.y), distance.z);
}

uint select_shadow_map_cascade(float3 positionWS)
{
   uint cascade_index = (shadow_cascade_count - 1);

   [unroll] for (int i = (shadow_cascade_count - 1); i >= 0; --i)
   {
      float3 positionLS = mul(light_constants.shadow_cascade_transforms[i], float4(positionWS, 1.0)).xyz;

      float cascade_signed_distance = shadow_cascade_signed_distance(positionLS);

      if (shadow_cascade_signed_distance(positionLS) <= 0.0) cascade_index = i;
   }

   return cascade_index;
}

float sample_shadow_map(Texture2DArray<float> shadow_map, float3 positionLS, uint cascade_index,
                        float2 inv_shadow_map_resolution)
{
   // Sampling pattern from MiniEngine https://github.com/microsoft/DirectX-Graphics-Samples/blob/0aa79bad78992da0b6a8279ddb9002c1753cb849/MiniEngine/Model/Shaders/Lighting.hlsli#L70

   const float light_depth = positionLS.z - shadow_map_bias;

   const float dilation = 2.0;

   float d1 = dilation * inv_shadow_map_resolution.x * 0.125;
   float d2 = dilation * inv_shadow_map_resolution.x * 0.875;
   float d3 = dilation * inv_shadow_map_resolution.x * 0.625;
   float d4 = dilation * inv_shadow_map_resolution.x * 0.375;

   float shadow =
      (2.0 * shadow_map.SampleCmpLevelZero(sampler_shadow, float3(positionLS.xy, cascade_index), light_depth) +
       shadow_map.SampleCmpLevelZero(sampler_shadow,
                                     float3(positionLS.xy + float2(-d2, d1), cascade_index), light_depth) +
       shadow_map.SampleCmpLevelZero(sampler_shadow,
                                     float3(positionLS.xy + float2(-d1, -d2), cascade_index), light_depth) +
       shadow_map.SampleCmpLevelZero(sampler_shadow,
                                     float3(positionLS.xy + float2(d2, -d1), cascade_index), light_depth) +
       shadow_map.SampleCmpLevelZero(sampler_shadow,
                                     float3(positionLS.xy + float2(d1, d2), cascade_index), light_depth) +
       shadow_map.SampleCmpLevelZero(sampler_shadow,
                                     float3(positionLS.xy + float2(-d4, d3), cascade_index), light_depth) +
       shadow_map.SampleCmpLevelZero(sampler_shadow,
                                     float3(positionLS.xy + float2(-d3, -d4), cascade_index), light_depth) +
       shadow_map.SampleCmpLevelZero(sampler_shadow,
                                     float3(positionLS.xy + float2(d4, -d3), cascade_index), light_depth) +
       shadow_map.SampleCmpLevelZero(sampler_shadow, float3(positionLS.xy + float2(d3, d4), cascade_index),
                                     light_depth)) /
      10.0;

   return shadow;
}

float sample_cascaded_shadow_map(float3 positionWS, float3 normalWS)
{
   uint cascade_index = select_shadow_map_cascade(positionWS);

   float3 positionLS =
      mul(light_constants.shadow_cascade_transforms[cascade_index], float4(positionWS, 1.0)).xyz;

   float shadow = sample_shadow_map(ResourceDescriptorHeap[light_constants.shadow_map_index], positionLS,
                                    cascade_index, light_constants.inv_shadow_map_resolution);

   return shadow;
}

float3 calc_ambient_light(float3 normalWS)
{
   const float factor = normalWS.y * -0.5 + 0.5;

   float3 color;

   color = light_constants.sky_ambient_color * -factor + light_constants.sky_ambient_color;
   color = light_constants.ground_ambient_color * factor + color;

   return color;
}

light_info get_light_info(light_description light, calculate_light_inputs input)
{
   const float3 normalWS = input.normalWS;
   const float3 positionWS = input.positionWS;

   light_info info;

   info.light_directionWS = 0.0;
   info.falloff = 0.0;

   switch (light.type) {
   case light_type::directional: {
      float region_fade_or_shadow = 1.0;

      if (light.region_type == directional_region_type::none) {
         if (light.shadow_caster) {
            region_fade_or_shadow = sample_cascaded_shadow_map(positionWS, normalWS);
         }
      }
      else {
         light_region_description region_desc = light_region_list.Load(light.directional_region_index);

         const float3 positionRS = mul(float4(positionWS, 1.0), region_desc.world_to_region).xyz;

         switch (light.region_type) {
         case directional_region_type::none: {
            region_fade_or_shadow = 1.0;
            break;
         }
         case directional_region_type::box: {
            const float3 region_to_position = max(abs(positionRS) - region_desc.size, 0.0);
            const float region_distance_sq = dot(region_to_position, region_to_position);

            region_fade_or_shadow = 1.0 - saturate(region_distance_sq / region_fade_distance_sq);

            break;
         }
         case directional_region_type::sphere: {
            const float region_distance = max(length(positionRS) - region_desc.size.x, 0.0);
            const float region_distance_sq = region_distance * region_distance;

            region_fade_or_shadow = 1.0 - saturate(region_distance_sq / region_fade_distance_sq);

            break;
         }
         case directional_region_type::cylinder: {
            const float radius = region_desc.size.x;
            const float height = region_desc.size.y;

            const float cap_distance = max(abs(positionRS.y) - height, 0.0);
            const float edge_distance = max(length(float2(positionRS.x, positionRS.z)) - radius, 0.0);
            const float region_distance = max(cap_distance, edge_distance);
            const float region_distance_sq = region_distance * region_distance;

            region_fade_or_shadow = 1.0 - saturate(region_distance_sq / region_fade_distance_sq);

            break;
         }
         }
      }

      info.light_directionWS = light.directionWS;
      info.falloff = region_fade_or_shadow;

      break;
   }
   case light_type::point_: {
      const float3 light_directionWS = normalize(light.positionWS - positionWS);
      const float light_distance = distance(light.positionWS, positionWS);

      const float attenuation =
         saturate(1.0 - ((light_distance * light_distance) / (light.range * light.range)));

      info.light_directionWS = light_directionWS;
      info.falloff = attenuation;

      break;
   }
   case light_type::spot: {
      const float3 light_directionWS = normalize(light.positionWS - positionWS);
      const float light_distance = distance(light.positionWS, positionWS);

      const float attenuation =
         saturate(1.0 - ((light_distance * light_distance) / (light.range * light.range)));

      const float theta = saturate(dot(light_directionWS, light.directionWS));
      const float cone_falloff = saturate((theta - light.spot_outer_param) * light.spot_inner_param);

      info.light_directionWS = light_directionWS;
      info.falloff = attenuation;

      break;
   }
   }

   return info;
}

float3 calculate_light(calculate_light_inputs input, light_description light, light_info light_info)
{
   const float3 diffuse = saturate(dot(input.normalWS, light_info.light_directionWS)) * input.diffuse_color;

   const float3 half_vectorWS = normalize(light_info.light_directionWS + input.viewWS);
   const float NdotH = saturate(dot(input.normalWS, half_vectorWS));
   const float3 specular = pow(NdotH, 64.0) * input.specular_color;

   return (diffuse + specular) * light.color * light_info.falloff;
}

float3 calculate_lighting(calculate_light_inputs input)
{
   float3 total_light = calc_ambient_light(input.normalWS) * input.diffuse_color;

   const uint2 tile_position = input.positionSS / light_tile_size;
   const uint tile_index = tile_position.x + tile_position.y * light_constants.light_tiles_width;

   for (uint i = 0; i < TILE_LIGHT_WORDS; ++i) {
      uint light_mask = WaveActiveBitOr(light_tiles[tile_index][i]);

      while (light_mask != 0) {
         uint active_bit_index = firstbitlow(light_mask); // get active bit

         light_mask &= ~(1u << active_bit_index); // clear bit from mask

         // process light

         uint light_index = (i * light_tile_word_bits) + active_bit_index;

         light_description light = light_constants.lights[light_index];
         light_info light_info = get_light_info(light, input);

         total_light += calculate_light(input, light, light_info);
      }
   }

   return total_light;
}
