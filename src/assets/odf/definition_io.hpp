#pragma once

#include "definition.hpp"

namespace we::assets::odf {

auto read_definition(std::vector<char> string_storage,
                     std::string_view platform_filter) -> definition;

}
