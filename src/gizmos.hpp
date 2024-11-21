#pragma once

#include "graphics/camera.hpp"
#include "types.hpp"
#include "utility/implementation_storage.hpp"

#include <array>
#include <string_view>
#include <vector>

namespace we {

struct gizmo_position_desc {
   std::string_view name;
   int64 instance;
   float alignment = 1.0f;
   quaternion gizmo_rotation;
};

struct gizmo_movement_desc {
   std::string_view name;
   int64 instance;
   float alignment = 1.0f;
   quaternion gizmo_rotation;
   float3 gizmo_positionWS;

   bool gizmo_space_output = false;
};

struct gizmo_draw_cone {
   float3 position_start;
   float3 position_end;
   float base_radius;

   float3 color;
};

struct gizmo_draw_pixel_line {
   float3 position_start;
   float3 position_end;
   uint32 color;
};

struct gizmo_draw_line {
   float3 position_start;
   float3 position_end;
   float half_width;

   float3 color;
};

struct gizmo_draw_quad {
   std::array<float3, 4> cornersWS;

   float3 color;
   float outline_width;
   float inner_alpha;
};

struct gizmo_draw_lists {
   std::vector<gizmo_draw_cone> cones;
   std::vector<gizmo_draw_pixel_line> pixel_lines;
   std::vector<gizmo_draw_line> lines;
   std::vector<gizmo_draw_quad> quads;
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

   bool gizmo_movement(const gizmo_movement_desc& desc, float3& out_movement) noexcept;

   bool can_close_last_edit() const noexcept;

   void deactivate() noexcept;

private:
   struct impl;

   implementation_storage<impl, 1024> impl; // TODO: Resize
};

}