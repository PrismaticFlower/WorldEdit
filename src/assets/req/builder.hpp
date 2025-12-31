#pragma once

#include <string>
#include <vector>

namespace we::assets::req {

/// @brief Add an entry to a requirement list if it isn't already present. This uses a linear search and is designed for use during munging.
/// @param list The list to add to.
/// @param entry The entry to add.
void add_to(std::vector<std::string>& list, std::string_view entry) noexcept;

}
