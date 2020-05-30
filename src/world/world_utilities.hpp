#pragma once

#include "world.hpp"

#include <span>
#include <string_view>

namespace sk::world {

template<typename Type>
inline auto find_entity(const std::vector<Type>& entities, const std::string_view name)
   -> const Type*
{
   for (auto& entity : entities) {
      if (entity.name == name) return &entity;
   }

   return nullptr;
}

inline auto find_region(const world& world, const std::string_view name) -> const region*
{
   return find_entity(world.regions, name);
}

inline auto find_region_by_description(const world& world, const std::string_view description)
   -> const region*
{
   for (auto& region : world.regions) {
      if (region.description == description) return &region;
   }

   return nullptr;
}

}