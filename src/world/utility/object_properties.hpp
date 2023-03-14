#pragma once

#include "../object.hpp"
#include "assets/odf/definition.hpp"

#include <span>

namespace we::world {

auto make_object_instance_properties(const assets::odf::definition& definition,
                                     std::span<const instance_property> existing_properties)
   -> std::vector<const instance_property>;

}
