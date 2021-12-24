#pragma once

#include "types.hpp"

#include <array>

namespace we::settings {

/// @brief Exposes read only access to settings for the renderer. All methods of this interface are thread safe.
class graphics {
public:
   virtual auto path_node_color() const noexcept -> float3 = 0;

   virtual auto path_node_outline_color() const noexcept -> float3 = 0;

   virtual auto path_node_connection_color() const noexcept -> float3 = 0;

   virtual auto path_node_orientation_color() const noexcept -> float3 = 0;

   virtual auto region_color() const noexcept -> float4 = 0;

   virtual auto barrier_color() const noexcept -> float4 = 0;

   virtual auto aabb_color() const noexcept -> float4 = 0;

   virtual auto light_volume_alpha() const noexcept -> float = 0;
};

}
