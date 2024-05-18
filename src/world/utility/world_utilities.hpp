#pragma once

#include "../world.hpp"
#include "utility/string_icompare.hpp"

#include <span>
#include <string>
#include <string_view>

namespace we::world {

template<typename Type>
inline auto select_entities(world& world) -> pinned_vector<Type>&
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
   if constexpr (std::is_same_v<Type, measurement>) return world.measurements;
}

template<typename Type, typename Type_id>
inline auto find_entity(std::span<Type> entities, const Type_id id) -> Type*
{
   if (auto it = std::lower_bound(entities.begin(), entities.end(), id,
                                  [](const Type& entity, const Type_id id) {
                                     return entity.id < id;
                                  });
       it != entities.end()) {
      if (it->id == id) return &(*it);
   }

   return nullptr;
}

template<typename Type>
inline auto find_entity(pinned_vector<Type>& entities,
                        const id<std::type_identity_t<Type>> id) -> Type*
{
   return find_entity(std::span<Type>{entities.data(), entities.size()}, id);
}

template<typename Type>
inline auto find_entity(const pinned_vector<Type>& entities,
                        const id<std::type_identity_t<Type>> id) -> const Type*
{
   return find_entity(std::span<const Type>{entities.data(), entities.size()}, id);
}

template<typename Type>
inline auto find_entity(world& world, const id<std::type_identity_t<Type>> id)
   -> Type*
{
   return find_entity(select_entities<Type>(world), id);
}

template<typename Type>
inline auto find_entity(const pinned_vector<Type>& entities,
                        const std::string_view name) -> const Type*
{
   for (auto& entity : entities) {
      if (string::iequals(entity.name, name)) return &entity;
   }

   return nullptr;
}

template<typename Type>
inline auto find_entity(pinned_vector<Type>& entities, const std::string_view name)
   -> Type*
{
   for (auto& entity : entities) {
      if (string::iequals(entity.name, name)) return &entity;
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

auto get_hub_index(const std::span<const planning_hub> hubs, planning_hub_id id)
   -> uint32;

auto create_unique_name(const std::span<const object> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const light> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const path> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const region> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const sector> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const portal> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const hintnode> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const barrier> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const planning_hub> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const planning_connection> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const boundary> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const animation> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_name(const std::span<const animation_group> entities,
                        const std::string_view reference_name) -> std::string;

auto create_unique_light_region_name(const std::span<const light> lights,
                                     const std::span<const region> regions,
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

struct clostest_node_result {
   /// @brief The index of the closest node.
   std::size_t index = 0;
   /// @brief If the next closest node is forward or not.
   bool next_is_forward = false;
};

/// @brief Find the index of the node closest to the point in the path.
/// @param point The point to find the node closest to.
/// @param path The path to search through.
/// @return The closest node's index or zero if the path is empty.
auto find_closest_node(const float3& point, const path& path) noexcept
   -> clostest_node_result;

/// @brief Find the index of the edge closest to the point in the sector.
/// @param point The point to find the node closest to.
/// @param sector The sector to search through.
/// @return The closest edge's index or zero if the sector has no edges.
auto find_closest_edge(const float2& point, const sector& sector) noexcept
   -> std::size_t;

}
