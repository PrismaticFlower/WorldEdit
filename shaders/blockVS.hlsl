#include "bindings.hlsli"
#include "frame_constants.hlsli"

#define MAX_SURFACES 6

enum texture_mode {
   texture_mode_world_space_auto,

   texture_mode_world_space_zy,
   texture_mode_world_space_xz,
   texture_mode_world_space_xy,

   texture_mode_tangent_space_xyz,

   texture_mode_unwrapped
};
enum texture_rotation { texture_rotation_d0, texture_rotation_d90, texture_rotation_d180, texture_rotation_d270 };
enum quad_split { quad_split_regular, quad_split_alternate };

struct surface_info {
   uint material_index : 8; 
   uint texture_mode : 3;
   uint scaleX : 4; 
   uint scaleY : 4;
   uint rotation : 2;
   uint offsetX : 13;
   uint offsetY : 13;
   uint quad_split : 1;
};

struct block_instance_description {
   float4x4 world_from_object;
   float3x3 adjugate_world_from_object;
   surface_info surfaces[MAX_SURFACES];
   uint3 padding;
};

struct block_quad_description {
   float3 positionWS[4];
   float3 normalWS[2];
   surface_info surface;
};

StructuredBuffer<uint> instance_index : register(BLOCK_INSTANCE_INDEX_REGISTER);
StructuredBuffer<block_instance_description> instance_descs : register(BLOCK_INSTANCE_DATA_REGISTER);
StructuredBuffer<block_quad_description> quad_descs : register(BLOCK_INSTANCE_DATA_REGISTER);

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
   const float3 normalWS = normalize(mul(instance.adjugate_world_from_object, input.normalOS));
   const float4 positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));

   float2 texcoords;
   
   switch (surface.texture_mode) {
   case texture_mode_world_space_auto: {
      const float3 normal_absWS = abs(normalWS);

      if (normal_absWS.x < normal_absWS.y && normal_absWS.z < normal_absWS.y) {
         texcoords = positionWS.xz;
      }
      else if (normal_absWS.x < normal_absWS.z) {
         texcoords = positionWS.xy;
      }
      else {
         texcoords = positionWS.zy;
      }
   } break;
   case texture_mode_world_space_zy:
      texcoords = positionWS.zy;
      break;
   case texture_mode_world_space_xz:
      texcoords = positionWS.xz;
      break;
   case texture_mode_world_space_xy:
      texcoords = positionWS.xy;
      break;
   case texture_mode_tangent_space_xyz: {
      const float3 tangentWS = normalize(mul(instance.adjugate_world_from_object, input.tangentOS));
      const float3 bitangentWS = normalize(input.bitangent_sign * cross(normalWS, tangentWS));

      const float3x3 texture_from_world = {tangentWS, bitangentWS, normalWS};

      texcoords = mul(texture_from_world, positionWS).xy;
   } break;
   case texture_mode_unwrapped:
      texcoords = input.texcoords;
      break;
   };

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
   output.normalWS = normalWS;
   output.texcoords = texcoords;
   output.material_index = surface.material_index;
   output.positionPS = positionPS;

   return output;
}

output_vertex main_quad(uint invocation_id : SV_VertexID)
{
   uint instance_id = invocation_id / 6;
   
   block_quad_description quad = quad_descs[instance_index[instance_id]];
   surface_info surface = quad.surface;
   
   uint instance_vertex_id = invocation_id % 6;
   uint tri_id = instance_vertex_id / 3;
   uint vertex_id = instance_vertex_id % 3;

   switch (surface.quad_split) {
   case quad_split_regular: {
      // Remap vertex index to form two triangles with indices {0, 1, 2}, {0, 2, 3}
      if (tri_id == 1 && vertex_id > 0) vertex_id += 1;
   } break;
   case quad_split_alternate: {
      // Remap vertex index to form two triangles with indices {0, 1, 3}, {1, 2, 3}
      if (tri_id == 0) {
         if (vertex_id == 2) vertex_id = 3;
      }
      else {
         vertex_id += 1;
      }
   } break;
   }

enum quad_split { quad_split_regular, quad_split_alternate };

   const float3 positionWS = quad.positionWS[vertex_id];
   const float3 normalWS = quad.normalWS[tri_id];
   const float4 positionPS = mul(cb_frame.projection_from_world, float4(positionWS, 1.0));
   
   float2 texcoords;
   
   switch (surface.texture_mode) {
   case texture_mode_world_space_auto: {
      const float3 normal_absWS = abs(normalWS);

      if (normal_absWS.x < normal_absWS.y && normal_absWS.z < normal_absWS.y) {
         texcoords = positionWS.xz;
      }
      else if (normal_absWS.x < normal_absWS.z) {
         texcoords = positionWS.xy;
      }
      else {
         texcoords = positionWS.zy;
      }
   } break;
   case texture_mode_world_space_zy:
      texcoords = positionWS.zy;
      break;
   case texture_mode_world_space_xz:
      texcoords = positionWS.xz;
      break;
   case texture_mode_world_space_xy:
      texcoords = positionWS.xy;
      break;
   case texture_mode_tangent_space_xyz:
   case texture_mode_unwrapped: {
      switch (vertex_id) {
         case 0:
            texcoords = float2(0.0, 0.0);
         break;
         case 1:
            texcoords = float2(1.0, 0.0);
         break;
         case 2:
            texcoords = float2(1.0, 1.0);
         break;
         case 3:
            texcoords = float2(0.0, 1.0);
         break;
      }
   } break;
   };

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

   output_vertex output;

   output.positionWS = positionWS;
   output.normalWS = normalWS;
   output.texcoords = texcoords;
   output.material_index = 0;
   output.positionPS = positionPS;

   return output;
}