#pragma once

#include "scene.hpp"

#include "../option_file.hpp"

#include "types.hpp"

#include "io/path.hpp"

#include <cstddef>
#include <span>

namespace we::assets::msh {

auto read_scene(const std::span<const std::byte> bytes) -> scene;

auto load_scene(const io::path& path, const options& directory_options) -> scene;

void save_scene(const io::path& path, const scene& scene);

}
