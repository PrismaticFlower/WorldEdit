#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <boost/container/small_vector.hpp>

namespace we::assets {

struct option {
   std::string name;
   boost::container::small_vector<std::string, 8> arguments;
};

using options = std::vector<option>;

auto parse_options(std::string_view str) -> options;

}
