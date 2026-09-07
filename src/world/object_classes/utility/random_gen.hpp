#pragma once

#include "types.hpp"

#include <bit>

namespace we::world {

struct random_gen {
   auto operator()() noexcept -> int32
   {
      const uint32 v = state * 0x19660d + 0x3c6ef35f;
      state = v * 0x19660d + 0x3c6ef35f;

      return v >> 0x10 | state & 0xffff0000;
   }

   constexpr auto get_float() noexcept -> float
   {
      const uint32 v = state * 0x19660d + 0x3c6ef35f;
      state = v * 0x19660d + 0x3c6ef35f;

      return std::bit_cast<float>((v >> 0x10 | state & 0xffff0000) >> 9 | 0x3f800000) -
             1.0f;
   }

   uint32 state = 0x94153a94;
};

}