#pragma once

#include "model.hpp"

#include "io/path.hpp"

namespace we::munge {

void write_model(const io::path& output_file_path, const model_container& container);

}