#pragma once

#include "graphics.hpp"

namespace we::settings {

class settings final : public graphics {
   auto path_node_color() const noexcept -> float3 override
   {
      return {0.15f, 1.0f, 0.3f};
   }

   auto path_node_outline_color() const noexcept -> float3 override
   {
      return {0.1125f, 0.6f, 0.2225f};
   }

   auto path_node_connection_color() const noexcept -> float3 override
   {
      return {0.1f, 0.1f, 0.75f};
   }

   auto path_node_orientation_color() const noexcept -> float3 override
   {
      return {1.0f, 1.0f, 0.1f};
   }

   auto region_color() const noexcept -> float4 override
   {
      return {0.25f, 0.4f, 1.0f, 0.3f};
   }

   auto barrier_color() const noexcept -> float4 override
   {
      return {1.0f, 0.1f, 0.5f, 0.3f};
   }

   auto aabb_color() const noexcept -> float4 override
   {
      return {0.0f, 1.0f, 0.0f, 0.3f};
   }

   auto light_volume_alpha() const noexcept -> float override
   {
      return 0.05f;
   }

   auto hover_color() const noexcept -> float3 override
   {
      return {1.0f, 1.0f, 0.07f};
   }

   auto line_width() const noexcept -> float override
   {
      return 2.5f;
   }
};

}
