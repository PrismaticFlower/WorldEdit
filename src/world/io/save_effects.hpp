#pragma once

#include "../effects.hpp"
#include "io/path.hpp"
#include "output_stream.hpp"

namespace we::world {

void save_effects(const io::path& path, const effects& effects);

}
