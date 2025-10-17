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
   int64 instance = 0;
   float alignment = 1.0f;
   quaternion gizmo_rotation;
};

struct gizmo_movement_desc {
   std::string_view name;
   int64 instance = 0;
   float alignment = 1.0f;
   quaternion gizmo_rotation;
   float3 gizmo_positionWS;

   bool gizmo_space_output = false;
};

struct gizmo_rotation_desc {
   std::string_view name;
   int64 instance = 0;
   quaternion gizmo_rotation;
   float3 gizmo_positionWS;
};

struct gizmo_size_desc {
   std::string_view name;
   int64 instance = 0;
   float alignment = 1.0f;
   quaternion gizmo_rotation;

   bool show_y_axis = true;
   bool show_z_axis = true;
};

struct gizmo_cone_size_desc {
   std::string_view name;
   int64 instance = 0;
   float alignment = 1.0f;
   quaternion gizmo_rotation;
   float3 gizmo_positionWS;
};

struct gizmo_ring_size_desc {
   std::string_view name;
   int64 instance = 0;
   float alignment = 1.0f;
   quaternion gizmo_rotation;
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

struct gizmo_draw_rotation_widget {
   float3 positionWS;

   float3 x_axisWS;
   float3 y_axisWS;
   float3 z_axisWS;

   float inner_radius;
   float outer_radius;

   float3 x_color;
   float3 y_color;
   float3 z_color;

   bool x_visible = true;
   bool y_visible = true;
   bool z_visible = true;
};

struct gizmo_draw_lists {
   std::vector<gizmo_draw_cone> cones;
   std::vector<gizmo_draw_pixel_line> pixel_lines;
   std::vector<gizmo_draw_line> lines;
   std::vector<gizmo_draw_quad> quads;
   std::vector<gizmo_draw_rotation_widget> rotation_widgets;
};

struct gizmo_button_input {
   bool left_mouse_down = false;
   bool ctrl_down = false;
   bool alt_down = false;
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

   bool gizmo_rotation(const gizmo_rotation_desc& desc, float3& rotation) noexcept;

   bool gizmo_size(const gizmo_size_desc& desc, float3& positionWS, float3& size) noexcept;

   bool gizmo_cone_size(const gizmo_cone_size_desc& desc, float& length,
                        float& base_radius) noexcept;

   bool gizmo_ring_size(const gizmo_ring_size_desc& desc, float3& positionWS,
                        float& inner_radius, float& outer_radius, float& height) noexcept;

   bool can_close_last_edit() const noexcept;

   /// @brief Check if the user started interacting this frame with the last gizmo while Alt was held down. This can be used to implement Alt-Click to duplicate patterns.
   /// @return True if Alt was held when the gizmo was activated and it was activated this frame, false otherwise.
   bool is_activated_with_duplication() const noexcept;

   /// @brief Check if last gizmo had Alt held down when activated. This can be used to mark edits after the duplication as transparent (allowing a smoother user experience).
   /// @return True if Alt was held when the gizmo was activated, false otherwise.
   bool was_duplication_triggered() const noexcept;

private:
   struct impl;

   implementation_storage<impl, 560> impl;
};
}