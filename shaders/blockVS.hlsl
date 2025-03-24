#include "bindings.hlsli"
#include "frame_constants.hlsli"

#define MAX_SURFACES 6

enum texture_rotation { texture_rotation_d0, texture_rotation_d90, texture_rotation_d180, texture_rotation_d270 };

struct surface_info {
   uint material_index : 8; 
   uint scaleX : 4; 
   uint scaleY : 4;
   uint rotation : 2;
   uint offsetX : 13;
   uint offsetY : 13;
};

struct block_instance_description {
   float4x4 world_from_object;
   float3x3 adjugate_world_from_object;
   surface_info surfaces[MAX_SURFACES];
   uint3 padding;
};

StructuredBuffer<uint> instance_index : register(BLOCK_INSTANCE_INDEX_REGISTER);
StructuredBuffer<block_instance_description> instance_descs : register(BLOCK_INSTANCE_DATA_REGISTER);

struct input_vertex {
   float3 positionOS : POSITION;
   float3 tangentOS : TANGENT;
   float bitangent_sign : BITANGENT_SIGN;
   float3 normalOS : NORMAL;
   float2 texcoords : TEXCOORD;
   uint surface_index : SURFACE;
   uint instance_id : SV_InstanceID;
};

struct output_vertex {
   float3 positionWS : POSITIONWS;
   float3 tangentWS : TANGENTWS;
   float3 bitangentWS : BITANGENTWS;
   float3 normalWS : NORMALWS;
   float2 texcoords : TEXCOORD;
   nointerpolation uint material_index : MATERIAL;
   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   block_instance_description instance = instance_descs[instance_index[input.instance_id]];
   surface_info surface = instance.surfaces[min(input.surface_index, MAX_SURFACES - 1)];

   output_vertex output;

   const float3 positionWS = mul(instance.world_from_object, float4(input.positionOS, 1.0)).xyz;
   const float3 tangentWS = normalize(mul(instance.adjugate_world_from_object, input.tangentOS));
   const float3 normalWS = normalize(mul(instance.adjugate_world_from_object, input.normalOS));
   const float4 positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));

   const float3 bitangentWS = normalize(input.bitangent_sign * cross(normalWS, tangentWS));

   const float3x3 texture_from_world = {tangentWS, bitangentWS, normalWS};

   float2 texcoords = input.texcoords;
   
   texcoords = mul(texture_from_world, positionWS).xy;
   texcoords *= exp2(float2((int)surface.scaleX - 7, (int)surface.scaleY - 7));

   if (surface.rotation == texture_rotation_d90) {
      texcoords = float2(-texcoords.y, texcoords.x);
   }
   else if (surface.rotation == texture_rotation_d180) {
      texcoords = -texcoords;
   }
   else if (surface.rotation == texture_rotation_d270) {
      texcoords = float2(texcoords.y, -texcoords.x);
   }

   texcoords += (float2(surface.offsetX, surface.offsetY) * (1.0 / 8192.0));

   output.positionWS = positionWS;
   output.tangentWS = tangentWS;
   output.bitangentWS = bitangentWS;
   output.normalWS = normalWS;
   output.texcoords = texcoords;
   output.material_index = surface.material_index;
   output.positionPS = positionPS;

   return output;
}