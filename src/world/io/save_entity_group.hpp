#pragma once

#include "../entity_group.hpp"

#include <filesystem>

namespace we::world {

void save_entity_group(const std::filesystem::path& path, const entity_group& group);

auto save_entity_group_to_string(const entity_group& group) noexcept -> std::string;

}
