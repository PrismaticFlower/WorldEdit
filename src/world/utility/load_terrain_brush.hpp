#pragma once

#include "container/dynamic_array_2d.hpp"
#include "io/path.hpp"
#include "types.hpp"

#include <stdexcept>

namespace we::world {

struct brush_load_error : std::runtime_error {
   using std::runtime_error::runtime_error;
};

auto load_brush(const io::path& file_path) -> container::dynamic_array_2d<uint8>;

}
