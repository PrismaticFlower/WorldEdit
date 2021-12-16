#pragma once

#include "utility/enum_bitflags.hpp"

#include <string>

namespace we::world {

enum class layer_flags { none = 0, inactive = 1, hidden = 2 };

constexpr bool marked_as_enum_bitflag(layer_flags) noexcept
{
   return true;
}

struct layer_description {
   std::string name;

   layer_flags flags = layer_flags::none;

   bool operator==(const layer_description&) const noexcept = default;
};

}
