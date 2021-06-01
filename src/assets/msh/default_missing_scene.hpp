#pragma once

#include "flat_model.hpp"

#include <memory>

namespace sk::assets::msh {

auto default_missing_scene() noexcept -> const std::shared_ptr<const flat_model>&;

}
