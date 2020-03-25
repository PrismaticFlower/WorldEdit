#pragma once

#include "definition.hpp"

#include <memory>

namespace sk::assets::odf {

auto default_object_class_definition() noexcept -> const std::shared_ptr<definition>&;

}