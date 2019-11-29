#pragma once

#include "definition.hpp"

namespace sk::assets::odf {

auto read_definition(std::string_view str) -> definition;

}