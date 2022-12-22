
#include "float16_packing.hpp"

#include <bit>

#include <immintrin.h>

namespace we::utility {

static_assert(alignof(float4) == 16);

[[msvc::forceinline]] auto pack_float16(const float v) noexcept -> uint16
{
   __m128 vec = _mm_set_ss(v);
   __m128i packed = _mm_cvtps_ph(vec, _MM_FROUND_TO_NEAREST_INT);

   return static_cast<uint16>(_mm_extract_epi16(packed, 0));
}

[[msvc::forceinline]] auto pack_float16(const float2& v) noexcept
   -> std::array<uint16, 2>
{
   float4 aligned_v{v.x, v.y, 0.0f, 0.0f};

   __m128 vec = _mm_load_ps(&aligned_v.x);
   __m128i packed = _mm_cvtps_ph(vec, _MM_FROUND_TO_NEAREST_INT);

   return {static_cast<uint16>(_mm_extract_epi16(packed, 0)),
           static_cast<uint16>(_mm_extract_epi16(packed, 1))};
}

[[msvc::forceinline]] auto pack_float16(const float3& v) noexcept
   -> std::array<uint16, 3>
{
   float4 aligned_v{v.x, v.y, v.z, 0.0f};

   __m128 vec = _mm_load_ps(&aligned_v.x);
   __m128i packed = _mm_cvtps_ph(vec, _MM_FROUND_TO_NEAREST_INT);

   return {static_cast<uint16>(_mm_extract_epi16(packed, 0)),
           static_cast<uint16>(_mm_extract_epi16(packed, 1)),
           static_cast<uint16>(_mm_extract_epi16(packed, 2))};
}

[[msvc::forceinline]] auto pack_float16(const float4& v) noexcept
   -> std::array<uint16, 4>
{
   __m128 vec = _mm_load_ps(&v.x);
   __m128i packed = _mm_cvtps_ph(vec, _MM_FROUND_TO_NEAREST_INT);

   return {static_cast<uint16>(_mm_extract_epi16(packed, 0)),
           static_cast<uint16>(_mm_extract_epi16(packed, 1)),
           static_cast<uint16>(_mm_extract_epi16(packed, 2)),
           static_cast<uint16>(_mm_extract_epi16(packed, 3))};
}

[[msvc::forceinline]] auto unpack_float16(const uint16 v) noexcept -> float
{
   __m128i packed = _mm_cvtsi32_si128(v);
   __m128 vec = _mm_cvtph_ps(packed);

   return _mm_cvtss_f32(vec);
}

[[msvc::forceinline]] auto unpack_float16(const std::array<uint16, 2>& v) noexcept -> float2
{
   __m128i packed = _mm_cvtsi32_si128(std::bit_cast<int32>(v));
   __m128 vec = _mm_cvtph_ps(packed);

   return {_mm_cvtss_f32(_mm_permute_ps(vec, _MM_SHUFFLE(0, 0, 0, 0))),
           _mm_cvtss_f32(_mm_permute_ps(vec, _MM_SHUFFLE(1, 1, 1, 1)))};
}

[[msvc::forceinline]] auto unpack_float16(const std::array<uint16, 3>& v) noexcept -> float3
{
   float4 unpacked = unpack_float16({v[0], v[1], v[2], 0});

   return {unpacked.x, unpacked.y, unpacked.z};
}

[[msvc::forceinline]] auto unpack_float16(const std::array<uint16, 4>& v) noexcept -> float4
{
   __m128i packed = _mm_cvtsi64_si128(std::bit_cast<int64>(v));
   __m128 vec = _mm_cvtph_ps(packed);

   return {_mm_cvtss_f32(_mm_permute_ps(vec, _MM_SHUFFLE(0, 0, 0, 0))),
           _mm_cvtss_f32(_mm_permute_ps(vec, _MM_SHUFFLE(1, 1, 1, 1))),
           _mm_cvtss_f32(_mm_permute_ps(vec, _MM_SHUFFLE(2, 2, 2, 2))),
           _mm_cvtss_f32(_mm_permute_ps(vec, _MM_SHUFFLE(3, 3, 3, 3)))};
}
}
