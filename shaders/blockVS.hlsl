#include "bindings.hlsli"
#include "frame_constants.hlsli"

struct block_instance_transform {
   float4x4 world_from_object;
   float3x3 adjugate_world_from_object;
};

StructuredBuffer<uint> instance_index : register(BLOCK_INSTANCE_INDEX_REGISTER);
StructuredBuffer<block_instance_transform> instance_descs : register(BLOCK_INSTANCE_DATA_REGISTER);

struct input_vertex {
   float3 positionOS : POSITION;
   float3 normalOS : NORMAL;
   float2 texcoords : TEXCOORD;
   uint instance_id : SV_InstanceID;
};

struct output_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMALWS;
   float2 texcoords : TEXCOORD;
   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   block_instance_transform instance = instance_descs[instance_index[input.instance_id]];

   output_vertex output;

   const float3 positionWS = mul(instance.world_from_object, float4(input.positionOS, 1.0)).xyz;
   const float3 normalWS = normalize(mul(instance.adjugate_world_from_object, input.normalOS));
   const float4 positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));

   output.positionWS = positionWS;
   output.normalWS = normalWS;
   output.texcoords = input.texcoords;
   output.positionPS = positionPS;

   return output;
}