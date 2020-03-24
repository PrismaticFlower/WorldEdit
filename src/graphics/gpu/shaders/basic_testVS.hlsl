struct global_matrices {
   float4x4 view_projection_matrix;
};

struct object_constants {
   float4x4 world_matrix;
};

ConstantBuffer<global_matrices> cb_global_matrices : register(b0);
ConstantBuffer<object_constants> cb_object_constants : register(b1);

struct input_vertex {
   float3 positionOS : POSITION;
   float3 normalOS : NORMAL;
   float2 texcoords : TEXCOORD;
};

struct output_vertex {
   float3 positionWS : POSITIONWS;
   float3 normalWS : NORMALWS;
   float2 texcoords : TEXCOORD;

   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   const float3 positionWS =
      mul(cb_object_constants.world_matrix, float4(input.positionOS, 1.0)).xyz;

   output.positionWS = positionWS;
   output.normalWS = mul((float3x3)cb_object_constants.world_matrix, input.normalOS);
   output.texcoords = input.texcoords;
   output.positionPS =
      mul(cb_global_matrices.view_projection_matrix, float4(positionWS, 1.0));

   return output;
}