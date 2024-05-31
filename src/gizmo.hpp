#pragma once

#include "graphics/camera.hpp"
#include "key.hpp"
#include "types.hpp"
#include "world/tool_visualizers.hpp"

#include <optional>

namespace we {

struct gizmo {
   bool want_capture_mouse() const noexcept;

   void update(const graphics::camera_ray cursor_ray, const bool is_mouse_down,
               const graphics::camera& camera) noexcept;

   void update_scale(const graphics::camera& camera, const float gizmo_scale) noexcept;

   void draw(world::tool_visualizers& tool_visualizers) noexcept;

   void draw_active_axis_line(const quaternion rotation, const float3 positionWS,
                              world::tool_visualizers& tool_visualizers);

   bool show_translate(const float3 gizmo_position,
                       const quaternion gizmo_rotation, float3& movement) noexcept;

   bool show_rotate(const float3 gizmo_position,
                    const quaternion gizmo_rotation, float3& rotation) noexcept;

   bool can_close_last_edit() const noexcept;

   void deactivate() noexcept;

private:
   auto get_translate_position(const graphics::camera_ray world_ray,
                               const float3 camera_position,
                               const float3 fallback) const noexcept -> float3;

   auto get_rotate_position(const graphics::camera_ray rayGS,
                            const float3 fallbackGS) const noexcept -> float3;

   enum class mode : uint8 { inactive, translate, rotate };
   enum class axis : uint8 { none, x, y, z };

   mode _last_mode = mode::inactive;
   mode _mode = mode::inactive;

   bool _used_last_tick = false;

   float3 _gizmo_position;
   quaternion _gizmo_rotation;

   float4x4 _gizmo_to_world;
   float4x4 _world_to_gizmo;

   struct translate_state {
      float3 start_position = {0.0f, 0.0f, 0.0f};
      float3 current_cursor_position = {0.0f, 0.0f, 0.0f};
      std::optional<float> movement_x;
      std::optional<float> movement_y;
      std::optional<float> movement_z;

      axis active_axis = axis::none;
      bool mouse_down_outside_gizmo = false;
      bool translating = false;
   } _translate;

   struct rotate_state {
      float3 current_cursor_positionGS = {0.0f, 0.0f, 0.0f};
      std::optional<float> angle_x;
      std::optional<float> angle_y;
      std::optional<float> angle_z;

      axis active_axis = axis::none;
      bool mouse_down_outside_gizmo = false;
      bool rotating = false;
   } _rotate;

   bool _translating_last_frame = false;
   bool _rotating_last_frame = false;

   float _translate_gizmo_length = 4.0f;
   float _translate_gizmo_hit_pad = 0.25f;
   float _rotate_gizmo_radius = 4.0f;
   float _rotate_gizmo_hit_pad = 0.25f;
};

}