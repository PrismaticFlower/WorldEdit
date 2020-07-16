#pragma once

#include "scene.hpp"
#include "types.hpp"

#include <cstddef>
#include <filesystem>
#include <span>

namespace sk::assets::msh {

auto read_scene(const std::span<const std::byte> bytes) -> scene;

auto read_scene(const std::filesystem::path& path) -> scene;

auto read_scene_options(const std::filesystem::path& path) -> options;

}
