#pragma once

#include "settings.hpp"

#include <string_view>

namespace we::settings {

auto load(const std::string_view path) -> settings;

void save(const std::string_view path, const settings& settings) noexcept;

/// @brief Calls save() for the referenced settings on destruction.
struct saver {
   ~saver();

   const std::string_view path;
   const settings& settings;
   we::settings::settings start_settings;
};

}