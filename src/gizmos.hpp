#pragma once

#include "graphics/camera.hpp"
#include "types.hpp"
#include "utility/implementation_storage.hpp"

#include <string_view>
#include <vector>

namespace we {

struct gizmo_position_desc {
   std::string_view name;
   int64 instance;
   float alignment = 1.0f;
   quaternion gizmo_rotation;
};

struct gizmo_draw_shape {
   float4x4 transform;
   float3 color;
};

struct gizmo_draw_line {
   float3 position_start;
   float3 position_end;
   uint32 color;
};

struct gizmo_draw_lists {
   std::vector<gizmo_draw_shape> cones;
   std::vector<gizmo_draw_shape> cylinders;
   std::vector<gizmo_draw_shape> boxes;
   std::vector<gizmo_draw_line> lines;
};

struct gizmo_button_input {
   bool left_mouse_down = false;
   bool ctrl_down = false;
};

struct gizmos {
   gizmos();

   ~gizmos();

   gizmos(const gizmos&) = delete;
   gizmos(gizmos&&) = delete;

   bool want_capture_mouse() const noexcept;

   bool want_capture_keyboard() const noexcept;

   void update(const graphics::camera_ray cursor_ray,
               const gizmo_button_input button_input,
               const graphics::camera& camera, const float gizmo_scale) noexcept;

   auto get_draw_lists() const noexcept -> const gizmo_draw_lists&;

   bool gizmo_position(const gizmo_position_desc& desc, float3& positionWS) noexcept;

   bool can_close_last_edit() const noexcept;

   void deactivate() noexcept;

private:
   struct impl;

   implementation_storage<impl, 1024> impl; // TODO: Resize
};

}