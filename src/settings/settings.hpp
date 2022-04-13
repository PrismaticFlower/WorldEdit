#pragma once

#include "graphics.hpp"

namespace we::settings {

class settings final : public graphics {
public:
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

   auto sector_color() const noexcept -> float4 override
   {
      return {0.6f, 0.3f, 1.0f, 0.3f};
   }

   auto portal_color() const noexcept -> float4 override
   {
      return {0.3f, 0.9f, 0.0f, 0.3f};
   }

   auto hintnode_color() const noexcept -> float3 override
   {
      return {1.0f, 0.15f, 0.15f};
   }

   auto hintnode_outline_color() const noexcept -> float3 override
   {
      return {0.8f, 0.04f, 0.04f};
   }

   auto boundary_color() const noexcept -> float4 override
   {
      return {1.0f, 0.0f, 0.0f, 0.2f};
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

   auto selected_color() const noexcept -> float3 override
   {
      return {1.0f, 1.0f, 1.0f};
   }

   auto barrier_height() const noexcept -> float override
   {
      return 64.0f;
   }

   auto boundary_height() const noexcept -> float override
   {
      return 64.0f;
   }

   auto line_width() const noexcept -> float override
   {
      return 2.5f;
   }
};

}
