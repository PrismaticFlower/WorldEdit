#pragma once

#include "key_node.hpp"

#include <string_view>

namespace we::assets::config {

struct read_options {
   /// @brief Support basic escape sequences in strings (\', \", \?, \\, \a, \b, \f, \n, \r, \t, \v)
   bool support_escape_sequences = false;
};

auto read_config(std::string_view str, const read_options options = {}) -> node;

}
