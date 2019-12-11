#pragma once

#include "terrain.hpp"

#include <cstddef>

#include <gsl/gsl>

namespace sk::assets::terrain {

auto read_terrain(const gsl::span<const std::byte> bytes) -> terrain;

}
