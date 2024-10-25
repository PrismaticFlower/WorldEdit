#pragma once

#include "io/path.hpp"
#include "scene.hpp"
#include "types.hpp"

#include <cstddef>
#include <span>

namespace we::assets::msh {

auto read_scene(const std::span<const std::byte> bytes) -> scene;

auto read_scene(const io::path& path) -> scene;

auto read_scene_options(const io::path& path) -> options;

}
