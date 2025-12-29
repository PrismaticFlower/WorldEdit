#pragma once

#include "../tool_context.hpp"

#include <vector>

namespace we::assets {
struct option;
}

namespace we::munge {

/// @brief Munge .tga files into .texture files
/// @param context Tool context for the munge.
void execute_texture_munge(const tool_context& context) noexcept;

/// @brief Munge a single .tga file into a .texture file.
/// @param input_file_path The path to the texture file.
/// @param directory_options The options from the directories tga.option file.
/// @param context The munge context.
void execute_texture_munge(const io::path& input_file_path,
                           const std::vector<assets::option>& directory_options,
                           const tool_context& context);

}