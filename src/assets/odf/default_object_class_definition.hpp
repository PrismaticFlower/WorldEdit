#pragma once

#include "definition.hpp"

#include <memory>

namespace we::assets::odf {

auto default_object_class_definition() noexcept
   -> const std::shared_ptr<const definition>&;

}
