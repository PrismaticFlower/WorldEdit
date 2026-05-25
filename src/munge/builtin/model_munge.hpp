#pragma once

#include "../tool_context.hpp"

#include <vector>

namespace we::assets {
struct option;
}

namespace we::munge {

/// @brief Munge .msh files into .model files
/// @param context Tool context for the munge.
void execute_model_munge(const tool_context& context) noexcept;

/// @brief Munge a single .msh file into a .model file.
/// @param input_file_path The path to the model file.
/// @param directory_options The options from the directories msh.option file.
/// @param context The munge context.
void execute_model_munge(const io::path& input_file_path,
                         const std::vector<assets::option>& directory_options,
                         const tool_context& context);

}