#pragma once

#include "../path.hpp"

#include <span>

namespace we::world {

auto get_path_properties(const path_type type) -> std::span<const char* const>;

auto get_path_node_properties(const path_type type) -> std::span<const char* const>;

}
