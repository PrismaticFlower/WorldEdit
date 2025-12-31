#pragma once

#include "../tool_context.hpp"

#include <vector>

namespace we::munge {

/// @brief Munge .odf files into .class and .class.req files.
/// @param context Tool context for the munge.
void execute_odf_munge(const tool_context& context) noexcept;

/// @brief Munge a single .odf file into a .class file and .class.req file.
/// @param input_file_path The path to the texture file.
/// @param context The munge context.
void execute_odf_munge(const io::path& input_file_path, const tool_context& context);

}