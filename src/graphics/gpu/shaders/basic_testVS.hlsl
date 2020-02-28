struct global_matrices {
   float4x4 view_projection_matrix;
};

struct object_constants {
   float4x4 world_matrix;
};

ConstantBuffer<global_matrices> cb_global_matrices : register(b0);
ConstantBuffer<object_constants> cb_object_constants : register(b1);

float4 main(float3 positionOS : POSITION) : SV_POSITION
{
   const float3 positionWS =
      mul(cb_object_constants.world_matrix, float4(positionOS, 1.0)).xyz;

   return mul(cb_global_matrices.view_projection_matrix, float4(positionWS, 1.0));
}