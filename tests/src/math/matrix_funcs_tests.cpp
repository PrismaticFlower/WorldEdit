#include "pch.h"

#include "math/matrix_funcs.hpp"

namespace we::tests {

static_assert(float4x4{
                 {0.5f, 0.0f, 0.0f, 0.0f},
                 {0.0f, 0.2f, 0.0f, 0.0f},
                 {0.0f, 0.0f, 1.0f, 0.0f},
                 {2.0f, 0.0f, -4.0f, 0.0f},
              } * float3{10.0f, 10.0f, 10.0f} ==
              float3{7.0f, 2.0f, 6.0f});

static_assert(transpose(float4x4{
                 {1.0f, 2.0f, 3.0f, 4.0f},
                 {5.0f, 6.0f, 7.0f, 8.0f},
                 {9.0f, 10.0f, 11.0f, 12.0f},
                 {13.0f, 14.0f, 15.0f, 16.0f},
              }) == float4x4{{1.0f, 5.0f, 9.0f, 13.0f},
                             {2.0f, 6.0f, 10.0f, 14.0f},
                             {3.0f, 7.0f, 11.0f, 15.0f},
                             {4.0f, 8.0f, 12.0f, 16.0f}});

static_assert((float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                        {0.0f, 1.0f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 1.0f, 0.0f},
                        {1.0f, 2.0f, 3.0f, 1.0f}} *
               float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                        {0.0f, 1.0f, 0.0f, 0.0f},
                        {0.0f, 0.0f, 1.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f, 1.0f}}) ==
              float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                       {0.0f, 1.0f, 0.0f, 0.0f},
                       {0.0f, 0.0f, 1.0f, 0.0f},
                       {1.0f, 2.0f, 3.0f, 1.0f}});

TEST_CASE("math matrix function tests", "[Math][MatrixFuncs]") {}

}
