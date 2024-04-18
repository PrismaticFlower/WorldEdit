#pragma once

#include "math/vector_funcs.hpp"

#include <utility>

namespace we {

// The MIT License
// Copyright © Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to
// do so, subject to the following conditions: The above copyright notice and this
// permission notice shall be included in all copies or substantial portions of
// the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

inline auto line_distance_lnorm(float2 p, float2 a, float2 b) noexcept -> float
{
   float2 pa = p - a, ba = b - a;

   float s = (ba.x * ba.y > 0.0f) ? 1.0f : -1.0f;
   float h = std::min(std::max((pa.y + s * pa.x) / (ba.y + s * ba.x), 0.0f), 1.0f);

   float2 q = abs(pa - h * ba);

   return std::max(q.x, q.y);
};

}