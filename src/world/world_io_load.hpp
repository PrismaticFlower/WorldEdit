#pragma once

#include "output_stream.hpp"
#include "world.hpp"

#include <filesystem>

namespace we::world {

auto load_world(const std::filesystem::path& path, output_stream& output) -> world;

}
