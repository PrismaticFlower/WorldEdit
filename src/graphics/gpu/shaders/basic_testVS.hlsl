cbuffer Matrices : register(b0)
{
   float4x4 view_projection_matrix;
};

float4 main(float3 pos : POSITION) : SV_POSITION
{
   return mul(view_projection_matrix, float4(pos * 0.5, 1.0));
}