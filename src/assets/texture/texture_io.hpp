#pragma once

#include "texture.hpp"

#include <filesystem>

namespace we::assets::texture {

auto load_texture(const std::filesystem::path& path) -> texture;

}
