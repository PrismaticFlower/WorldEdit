#pragma once

#include <filesystem>

namespace we::utility {

void try_show_in_explorer(const std::filesystem::path& file) noexcept;

}
