#include "scalar_funcs.hpp"

#ifdef _M_X64
#include <immintrin.h>
#else
#include <math.h>
#endif

namespace we {

float sqrt(float v) noexcept
{
#ifdef _M_X64
   _mm_store_ss(&v, _mm_sqrt_ss(_mm_load_ss(&v)));

   return v;
#else
   return sqrtf(v);
#endif
}

float rsqrt(float v) noexcept
{
   return 1.0f / sqrt(v);
}

float fast_rsqrt(float v) noexcept
{
#ifdef _M_X64
   _mm_store_ss(&v, _mm_rsqrt_ss(_mm_load_ss(&v)));

   return v;
#else
   return 1.0f / sqrtf(v);
#endif
}
}