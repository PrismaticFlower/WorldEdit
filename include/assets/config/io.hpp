#pragma once

#include "key_node.hpp"

#include <string_view>

namespace sk::assets::config {

auto read_config(std::string_view str) -> node;

}