#pragma once

#include "options.hpp"

#include "io/path.hpp"

#include <vector>

namespace we::assets {
struct option;
}

namespace we::munge {

auto load_texture_options(const io::path& texture_path,
                          const std::vector<assets::option>& folder_options)
   -> texture_munge_options;

}