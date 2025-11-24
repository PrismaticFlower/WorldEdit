#pragma once

#include "../tool_context.hpp"

namespace we::munge {

/// @brief Munge .blk files into .blocks files
/// @param context Tool context for the munge.
void execute_blocks_munge(const tool_context& context) noexcept;

}