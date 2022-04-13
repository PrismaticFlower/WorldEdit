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

   virtual auto sector_color() const noexcept -> float4 = 0;

   virtual auto portal_color() const noexcept -> float4 = 0;

   virtual auto hintnode_color() const noexcept -> float3 = 0;

   virtual auto hintnode_outline_color() const noexcept -> float3 = 0;

   virtual auto boundary_color() const noexcept -> float4 = 0;

   virtual auto light_volume_alpha() const noexcept -> float = 0;

   virtual auto hover_color() const noexcept -> float3 = 0;

   virtual auto selected_color() const noexcept -> float3 = 0;

   virtual auto barrier_height() const noexcept -> float = 0;

   virtual auto boundary_height() const noexcept -> float = 0;

   virtual auto line_width() const noexcept -> float = 0;
};

}
