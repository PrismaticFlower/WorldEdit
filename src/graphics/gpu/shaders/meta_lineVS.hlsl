struct global_matrices {
   float4x4 view_projection_matrix;
};

ConstantBuffer<global_matrices> cb_global_matrices : register(b0);

struct input_vertex {
   float3 positionWS : POSITION;
};

float4 main(input_vertex input) : SV_POSITION
{
   return mul(cb_global_matrices.view_projection_matrix, float4(input.positionWS, 1.0));
}