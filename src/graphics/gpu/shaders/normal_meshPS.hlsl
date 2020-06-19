
#define MAX_LIGHTS 1023

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
   uint padding;
};

struct light_constant_buffer {
   uint light_count;
   uint3 padding0;

   light_description lights[MAX_LIGHTS];

   float4 padding1[3];
};

struct light_region_description {
   float4x4 world_to_region;
   float3 position;
   uint type;
   float3 size;
   uint padding;
};

struct input_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMALWS;
   float2 texcoords : TEXCOORD;
};

ConstantBuffer<light_constant_buffer> light_constants : register(b0);
StructuredBuffer<light_region_description> light_region_descriptions : register(t0);

Texture2D<float4> diffuse_map : register(t0, space1);

SamplerState trilinear_sampler : register(s0);

const static float region_fade_distance_sq = 0.1 * 0.1;

float4 main(input_vertex input_vertex) : SV_TARGET
{
   const float3 normalWS = normalize(input_vertex.normalWS);
   const float3 positionWS = input_vertex.positionWS;

   float3 total_light = 0.0;

   for (uint i = 0; i < light_constants.light_count; ++i) {
      light_description light = light_constants.lights[i];

      switch (light.type) {
      case light_type::directional: {
         float region_fade = 1.0;

         if (light.region_type == directional_region_type::none) {
            region_fade = 1.0;
         }
         else {
            light_region_description region_desc =
               light_region_descriptions.Load(light.directional_region_index);

            const float3 positionRS =
               mul(region_desc.world_to_region, float4(positionWS, 1.0)).xyz;

            switch (light.region_type) {
            case directional_region_type::none: {
               region_fade = 1.0;
               break;
            }
            case directional_region_type::box: {
               const float3 region_to_position =
                  max(abs(positionRS) - region_desc.size, 0.0);
               const float region_distance_sq =
                  dot(region_to_position, region_to_position);

               region_fade =
                  1.0 - saturate(region_distance_sq / region_fade_distance_sq);

               break;
            }
            case directional_region_type::sphere: {
               const float region_distance =
                  max(length(positionRS) - region_desc.size.x, 0.0);
               const float region_distance_sq = region_distance * region_distance;

               region_fade =
                  1.0 - saturate(region_distance_sq / region_fade_distance_sq);

               break;
            }
            case directional_region_type::cylinder: {
               const float radius = region_desc.size.x;
               const float height = region_desc.size.y;

               const float cap_distance = max(abs(positionRS.y) - height, 0.0);
               const float edge_distance =
                  max(length(float2(positionRS.x, positionRS.z)) - radius, 0.0);
               const float region_distance = max(cap_distance, edge_distance);
               const float region_distance_sq = region_distance * region_distance;

               region_fade =
                  1.0 - saturate(region_distance_sq / region_fade_distance_sq);

               break;
            }
            }
         }

         const float falloff = saturate(dot(normalWS, light.directionWS));

         total_light += falloff * region_fade * light.color;
         break;
      }
      case light_type::point_: {
         const float3 light_directionWS = normalize(light.positionWS - positionWS);
         const float light_distance = distance(light.positionWS, positionWS);

         const float attenuation = saturate(
            1.0 - ((light_distance * light_distance) / (light.range * light.range)));

         const float falloff = saturate(dot(normalWS, light_directionWS));

         total_light += falloff * attenuation * light.color;
         break;
      }
      case light_type::spot: {
         const float3 light_directionWS = normalize(light.positionWS - positionWS);
         const float light_distance = distance(light.positionWS, positionWS);

         const float attenuation = saturate(
            1.0 - ((light_distance * light_distance) / (light.range * light.range)));

         const float theta = saturate(dot(light_directionWS, light.directionWS));
         const float cone_falloff =
            saturate((theta - light.spot_outer_param) * light.spot_inner_param);

         const float falloff = saturate(dot(normalWS, light_directionWS));

         total_light += falloff * cone_falloff * attenuation * light.color;
         break;
      }
      }
   }

   const float3 diffuse_color =
      diffuse_map.Sample(trilinear_sampler, input_vertex.texcoords).rgb;

   return float4(total_light * diffuse_color, 1.0f);
}