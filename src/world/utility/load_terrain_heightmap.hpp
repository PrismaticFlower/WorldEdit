#pragma once

#include "container/dynamic_array_2d.hpp"
#include "types.hpp"

#include <filesystem>
#include <stdexcept>
#include <variant>

namespace we::world {

struct heightmap_load_error : std::runtime_error {
   using std::runtime_error::runtime_error;
};

auto load_heightmap(const std::filesystem::path& file_path)
   -> std::variant<container::dynamic_array_2d<uint8>, container::dynamic_array_2d<uint16>>;

}
