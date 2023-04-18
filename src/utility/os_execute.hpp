#pragma once

#include <string>

namespace we::utility {

/// @brief Use ExpandEnvironmentStrings on the provided string.
/// @param string The string.
/// @return The string with nay environment variables expanded or an empty string.
auto expand_environment_strings(const std::string& string) noexcept -> std::string;

/// @brief Execute a command line using ExpandEnvironmentStrings and CreateProcess. It will not wait for the process to complete before returning.
/// @param command_line The command line.
/// @return True on successfully creating the process, false on failure.
bool os_execute_async(const std::string& command_line) noexcept;

}
