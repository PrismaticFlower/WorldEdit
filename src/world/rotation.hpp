#pragma once

#include "types.hpp"

namespace we::world {

struct rotation {
   rotation() = default;

   rotation(const quaternion& from) noexcept;

   rotation(const float3& from_euler) noexcept;

   quaternion quat;
   float3 euler;

   operator const quaternion&() const noexcept
   {
      return quat;
   }
};

}