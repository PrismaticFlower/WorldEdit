struct camera_matrices {
   float4x4 view_projection_matrix;
};

struct object_constants {
   float4x4 world_matrix;
};

ConstantBuffer<object_constants> object : register(b0);
ConstantBuffer<camera_matrices> camera : register(b1);

struct input_vertex {
   float3 positionOS : POSITION;
};

struct output_vertex {
   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   output.positionPS = mul(camera.view_projection_matrix,
                           mul(object.world_matrix, float4(input.positionOS, 1.0)));

   return output;
}