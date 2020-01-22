#pragma once

#include "world.hpp"

#include <output_stream.hpp>

#include <filesystem>

namespace sk::world {

auto load_world(const std::filesystem::path& path, output_stream& output) -> world;

}