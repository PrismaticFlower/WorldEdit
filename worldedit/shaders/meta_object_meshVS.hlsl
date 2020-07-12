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
};

struct output_vertex {
   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   const float3 positionWS =
      mul(cb_object_constants.world_matrix, float4(input.positionOS, 1.0)).xyz;
   const float4 positionPS =
      mul(cb_global_matrices.view_projection_matrix, float4(positionWS, 1.0));

   output.positionPS = positionPS;

   return output;
}