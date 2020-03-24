
#include "pch.h"

#include "math/align.hpp"

namespace sk::math::tests {

static_assert(align_up(0, 16) == 0);

static_assert(align_up(-32, 16) == -16);

static_assert(align_up(12, 16) == 16);
static_assert(align_up(33, 16) == 48);

static_assert(align_up(16, 16) == 16);
static_assert(align_up(32, 16) == 32);

}
