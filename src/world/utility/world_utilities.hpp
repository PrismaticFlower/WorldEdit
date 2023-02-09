#pragma once

#include "../world.hpp"

#include <span>
#include <string>
#include <string_view>

namespace we::world {

template<typename Type>
inline auto select_entities(world& world) -> std::vector<Type>&
{
   if constexpr (std::is_same_v<Type, object>) return world.objects;
   if constexpr (std::is_same_v<Type, light>) return world.lights;
   if constexpr (std::is_same_v<Type, path>) return world.paths;
   if constexpr (std::is_same_v<Type, region>) return world.regions;
   if constexpr (std::is_same_v<Type, sector>) return world.sectors;
   if constexpr (std::is_same_v<Type, portal>) return world.portals;
   if constexpr (std::is_same_v<Type, hintnode>) return world.hintnodes;
   if constexpr (std::is_same_v<Type, barrier>) return world.barriers;
   if constexpr (std::is_same_v<Type, planning_hub>) return world.planning_hubs;
   if constexpr (std::is_same_v<Type, planning_connection>)
      return world.planning_connections;
   if constexpr (std::is_same_v<Type, boundary>) return world.boundaries;
}

template<typename Type>
inline auto find_entity(std::vector<Type>& entities, const id<Type> id) -> Type*
{
   for (auto& entity : entities) {
      if (entity.id == id) return &entity;
   }

   return nullptr;
}

template<typename Type>
inline auto find_entity(world& world, const id<Type> id) -> Type*
{
   return find_entity(select_entities<Type>(world), id);
}

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

auto create_unique_name(const std::vector<object>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<light>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<path>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<region>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<sector>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<portal>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<hintnode>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<barrier>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<planning_hub>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<planning_connection>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::vector<boundary>& entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_light_region_name(const std::vector<light>& lights,
                                     const std::vector<region>& regions,
                                     const std::string_view reference_name)
   -> std::string;

/// @brief Check if a light is directional. (It's type is directional or one of the directional_region_* types)
/// @param light
/// @return
bool is_directional_light(const light& light) noexcept;

/// @brief Check if a light has a region.
/// @param light
/// @return
bool is_region_light(const light& light) noexcept;

}
