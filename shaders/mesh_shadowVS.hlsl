#include "bindings.hlsli"

struct camera_matrices {
   float4x4 projection_from_world;
};

struct object_constants {
   float4x4 world_from_object;
};

ConstantBuffer<camera_matrices> camera : register(FRAME_CB_REGISTER);
ConstantBuffer<object_constants> object : register(OBJECT_CB_REGISTER);

struct input_vertex {
   float3 positionOS : POSITION;
};

struct output_vertex {
   float4 positionPS : SV_Position;
};

output_vertex main(input_vertex input)
{
   output_vertex output;

   output.positionPS =
      mul(camera.projection_from_world, mul(object.world_from_object, float4(input.positionOS, 1.0)));
   output.positionPS.z = max(output.positionPS.z, 0.0);

   return output;
}