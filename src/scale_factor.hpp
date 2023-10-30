#pragma once

#include <math.h>

namespace we {

struct scale_factor {
   float value = 1.0f;
};

inline auto operator*(const float size, const scale_factor factor) -> float
{
   return floorf(size * factor.value);
}

}
