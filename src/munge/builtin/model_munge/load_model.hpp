#pragma once

#include "model.hpp"

#include "../../feedback.hpp"

#include "io/path.hpp"

namespace we::assets {
struct option;
}

namespace we::munge {

/// @brief Load .msh file and process it into a model structure.
/// @param path Path to the .msh file.
/// @param directory_options The options from the directories msh.option file.
/// @param feedback Feedback for reporting warnings.
/// @return The loaded model.
auto load_model(const io::path& path,
                const std::vector<assets::option>& directory_options,
                munge_feedback& feedback) -> model_container;

}