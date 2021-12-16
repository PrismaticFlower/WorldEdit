#pragma once

#include "output_stream.hpp"
#include "world.hpp"

#include <filesystem>

namespace we::world {

void save_world(const std::filesystem::path& path, const world& world);

}
