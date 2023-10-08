#pragma once

#include "../active_elements.hpp"
#include "../object_class_library.hpp"
#include "../world.hpp"

#include <optional>
#include <span>

namespace we::world {

template<typename T>
struct raycast_result {
   float distance = 0.0f;
   id<T> id;
};

template<>
struct raycast_result<object> {
   float distance = 0.0f;
   float3 normalWS;
   object_id id;
};

template<>
struct raycast_result<path> {
   float distance = 0.0f;
   path_id id;
   std::size_t node_index = 0;
};

template<>
struct raycast_result<sector> {
   float distance = 0.0f;
   float3 normalWS;
   sector_id id;
};

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const object> objects,
             const object_class_library& object_classes,
             std::optional<object_id> ignore_object = std::nullopt) noexcept
   -> std::optional<raycast_result<object>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const light> lights) noexcept
   -> std::optional<raycast_result<light>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const path> paths,
             const float node_size) noexcept -> std::optional<raycast_result<path>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const region> regions) noexcept
   -> std::optional<raycast_result<region>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const sector> sectors) noexcept
   -> std::optional<raycast_result<sector>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const portal> portals) noexcept
   -> std::optional<raycast_result<portal>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             const active_layers active_layers, std::span<const hintnode> hintnodes) noexcept
   -> std::optional<raycast_result<hintnode>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const barrier> barriers, const float barrier_height) noexcept
   -> std::optional<raycast_result<barrier>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const planning_hub> hubs, const float hub_height) noexcept
   -> std::optional<raycast_result<planning_hub>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const planning_connection> connections,
             std::span<const planning_hub> hubs, const float connection_height) noexcept
   -> std::optional<raycast_result<planning_connection>>;

auto raycast(const float3 ray_origin, const float3 ray_direction,
             std::span<const boundary> boundaries, const float boundary_height) noexcept
   -> std::optional<raycast_result<boundary>>;

}