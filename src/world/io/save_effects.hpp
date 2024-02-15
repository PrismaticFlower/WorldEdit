#pragma once

#include "../effects.hpp"
#include "output_stream.hpp"

#include <filesystem>

namespace we::world {

void save_effects(const std::filesystem::path& path, const effects& effects);

}
