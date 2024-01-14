#include "resource_heaps.hlsli"
#include "samplers.hlsli"

struct apply_inputs {
   uint env_map_index;
};

ConstantBuffer<apply_inputs> input : register(b0);

static TextureCube env_map = TextureCubeHeap[input.env_map_index];

float4 main(float2 location : LOCATION, uint face_index : SV_RenderTargetArrayIndex) : SV_TARGET
{
   const float2 positionNDC = (location * 2.0 - 1.0) * float2(1.0, -1.0);
   
   float3 position = 0.0;
   
   if (face_index == 0) { // X+
      position.zy = positionNDC * float2(-1.0, 1.0);
      position.x = 1.0;
   }
   else if (face_index == 1) { //X-
      position.zy = positionNDC;
      position.x = -1.0;
   }
   else if (face_index == 2) { // Y+
      position.xz = positionNDC * float2(1.0, -1.0);
      position.y = 1.0;
   }
   else if (face_index == 3) { // Y-
      position.xz = positionNDC;
      position.y = -1.0;
   }
   else if (face_index == 4) { // Z+
      position.xy = positionNDC;
      position.z = 1.0;
   }
   else if (face_index == 5) { // Z-
      position.xy = positionNDC * float2(-1.0, 1.0);
      position.z = -1.0;
   }
   
   return env_map.Sample(sampler_bilinear_wrap, position * float3(1.0, 1.0, -1.0));
}