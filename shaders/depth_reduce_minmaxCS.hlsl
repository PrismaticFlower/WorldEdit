
struct input_constant_buffer {
   uint depth_srv;
   uint2 resolution;
};

ConstantBuffer<input_constant_buffer> input : register(b0);

static Texture2D<float> depth_texture = ResourceDescriptorHeap[input.depth_srv];

RWByteAddressBuffer output : register(u0);

const static float clear_depth = 1.0f;

groupshared uint shared_min_depth;
groupshared uint shared_max_depth;

// clang-format off

[numthreads(8, 8, 1)]
void main(uint2 DTid : SV_DispatchThreadID, uint group_index : SV_GroupIndex) { // clang-format on
   if (group_index == 0) {
      shared_min_depth = asuint(1.0);
      shared_max_depth = 0.0;
   }

   GroupMemoryBarrierWithGroupSync();

   if (DTid.x >= input.resolution.x || DTid.y >= input.resolution.y) return;

   const float depth = depth_texture[DTid.xy];

   if (depth != clear_depth) {
      const float wave_min_depth = WaveActiveMin(depth);
      const float wave_max_depth = WaveActiveMax(depth);

      if (WaveIsFirstLane()) {
         InterlockedMin(shared_min_depth, asuint(wave_min_depth));
         InterlockedMax(shared_max_depth, asuint(wave_max_depth));
      }
   }

   GroupMemoryBarrierWithGroupSync();

   if (group_index == 0) {
      output.InterlockedMin(0, shared_min_depth);
      output.InterlockedMax(4, shared_max_depth);
   }
}