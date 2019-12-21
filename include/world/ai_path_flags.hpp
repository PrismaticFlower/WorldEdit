#pragma once

#include <utility/enum_bitflags.hpp>

namespace sk::world {

enum class ai_path_flags {
   soldier = 0b1,
   hover = 0b10,
   small = 0b100,
   medium = 0b1000,
   huge = 0b10000,
   flyer = 0b100000
};

constexpr bool marked_as_enum_bitflag(const ai_path_flags)
{
   return true;
}

}