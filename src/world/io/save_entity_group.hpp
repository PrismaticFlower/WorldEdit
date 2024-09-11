#pragma once

#include "../entity_group.hpp"

#include <filesystem>

namespace we::world {

void save_entity_group(const std::filesystem::path& path, const entity_group& group);

}
