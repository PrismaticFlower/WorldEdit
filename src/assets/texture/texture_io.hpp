#pragma once

#include "texture.hpp"

#include <filesystem>

namespace sk::assets::texture {

auto load_texture(const std::filesystem::path& path) -> texture;

}