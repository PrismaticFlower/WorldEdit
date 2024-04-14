#include "scalar_funcs.hpp"

#include <immintrin.h>

namespace we {

float sqrt(float v) noexcept
{
   _mm_store_ss(&v, _mm_sqrt_ss(_mm_load_ss(&v)));

   return v;
}

float rsqrt(float v) noexcept
{
   return 1.0f / sqrt(v);
}

float fast_rsqrt(float v) noexcept
{
   _mm_store_ss(&v, _mm_rsqrt_ss(_mm_load_ss(&v)));

   return v;
}

}