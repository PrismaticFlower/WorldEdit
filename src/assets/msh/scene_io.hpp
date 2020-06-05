#pragma once

#include "scene.hpp"
#include "types.hpp"

#include <cstddef>
#include <span>

namespace sk::assets::msh {

auto read_scene_from_bytes(const std::span<const std::byte> bytes) -> scene;

}
