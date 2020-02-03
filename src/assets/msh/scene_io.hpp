#pragma once

#include "scene.hpp"
#include "types.hpp"

#include <cstddef>

#include <gsl/gsl>

namespace sk::assets::msh {

auto read_scene_from_bytes(const gsl::span<const std::byte> bytes) -> scene;

}
