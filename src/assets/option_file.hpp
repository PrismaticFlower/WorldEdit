#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <absl/container/inlined_vector.h>

namespace we::assets {

struct option {
   std::string name;
   absl::InlinedVector<std::string, 8> arguments;
};

using options = std::vector<option>;

auto parse_options(std::string_view str) -> options;

}
