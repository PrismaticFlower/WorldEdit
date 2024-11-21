#include "gizmos.hpp"

#include "math/iq_intersectors.hpp"
#include "math/matrix_funcs.hpp"
#include "math/plane_funcs.hpp"
#include "math/quaternion_funcs.hpp"

#include <string>
#include <vector>

namespace we {

namespace {

enum class position_widget : uint8 {
   none,
   x,
   y,
   z,

   yz,
   xz,
   xy,

   xyz
};
enum class position_state : uint8 { idle, hovered, active, locked };

struct gizmo_id {
   std::string name;
   int64 instance = INT64_MIN;

   bool operator==(const gizmo_id&) const noexcept = default;
};

struct gizmo_position_state {
   gizmo_id id;
   quaternion rotation;

   position_state state = position_state::idle;
   position_widget active_widget = position_widget::none;

   float3 active_positionWS;
   float3 activation_offsetGS;

   float alignment = 1.0f;

   bool submitted_last_frame = true;
};

struct gizmo_movement_state {
   gizmo_id id;
   quaternion rotation;
   float3 positionWS;

   bool submitted_last_frame = true;
};

constexpr float3 x_color = {0.7835378f, 0.051269457f, 0.051269457f};
constexpr float3 y_color = {0.051269457f, 0.116970666f, 0.7835378f};
constexpr float3 z_color = {0.029556835f, 0.7835378f, 0.029556835f};

constexpr float3 x_color_hover = {1.0f, 0.2158605f, 0.2158605f};
constexpr float3 y_color_hover = {0.2158605f, 0.3515326f, 1.0f};
constexpr float3 z_color_hover = {0.27889428f, 1.0f, 0.27889428f};

constexpr uint32 x_color_u32 = 0xff'e5'40'40;
constexpr uint32 y_color_u32 = 0xff'40'60'e5;
constexpr uint32 z_color_u32 = 0xff'30'e5'30;

constexpr uint32 x_color_hover_u32 = 0xff'ff'80'80;
constexpr uint32 y_color_hover_u32 = 0xff'80'a0'ff;
constexpr uint32 z_color_hover_u32 = 0xff'90'ff'90;

constexpr float xyz_hover_alpha = 0.25f;

constexpr float position_gizmo_length = 0.1f;
constexpr float position_gizmo_hit_pad = 0.0075f;
constexpr float position_gizmo_plane_cull_angle = 0.15f;
constexpr float position_gizmo_plane_outline_width = 0.2f;
constexpr float position_gizmo_plane_inner_alpha = 0.125f;

auto align_value(const float value, const float alignment) noexcept -> float
{
   return std::round(value / alignment) * alignment;
}

auto xyz_intersects(const float3& ray_originGS, const float3& ray_directionGS,
                    const float3& camera_upGS, const float3& camera_rightGS,
                    const float length) noexcept -> float
{
   const float half_length = length * 0.5f;

   const std::array<float3, 2> xyz_lineGS = {
      camera_rightGS * half_length,
      -camera_rightGS * half_length,
   };

   const std::array<float3, 4> xyz_quadGS{
      xyz_lineGS[0] + camera_upGS * half_length,
      xyz_lineGS[1] + camera_upGS * half_length,
      xyz_lineGS[1] - camera_upGS * half_length,
      xyz_lineGS[0] - camera_upGS * half_length,
   };

   float hit = triIntersect(ray_originGS, ray_directionGS, xyz_quadGS[0],
                            xyz_quadGS[1], xyz_quadGS[2])
                  .x;

   if (hit >= 0.0f) return hit;

   hit = triIntersect(ray_originGS, ray_directionGS, xyz_quadGS[3],
                      xyz_quadGS[0], xyz_quadGS[2])
            .x;

   return hit;
}

}

struct gizmos::impl {
   bool want_capture_mouse() const noexcept
   {
      return _capture_mouse;
   }

   bool want_capture_keyboard() const noexcept
   {
      return _capture_keyboard;
   }

   void update(const graphics::camera_ray cursor_rayWS,
               const gizmo_button_input button_input,
               const graphics::camera& camera, const float gizmo_scale) noexcept
   {
      draw_lists.cones.clear();
      draw_lists.lines.clear();
      draw_lists.pixel_lines.clear();
      draw_lists.quads.clear();

      _cursor_rayWS = cursor_rayWS;
      _input = button_input;
      _capture_mouse = _want_mouse_input;
      _capture_keyboard = _want_keyboard_input;
      _want_mouse_input = false;
      _want_keyboard_input = false;
      _last_gizmo_deactivated = false;

      _gizmo_scale = gizmo_scale * camera.projection_matrix()[0].x;
      _camera_positionWS = camera.position();
      _camera_forwardWS = camera.forward();
      _camera_upWS = camera.up();
      _camera_rightWS = camera.right();
      _axis_line_length = camera.far_clip();

      _closest_gizmo = _next_frame_closest_gizmo;
      _closest_gizmo_id.name = _next_frame_closest_gizmo_id.name;
      _closest_gizmo_id.instance = _next_frame_closest_gizmo_id.instance;

      _next_frame_closest_gizmo = FLT_MAX;
      _next_frame_closest_gizmo_id.name.clear();
      _next_frame_closest_gizmo_id.instance = INT64_MIN;

      garbage_collect();
   }

   bool gizmo_position(const gizmo_position_desc& desc, float3& positionWS) noexcept
   {
      gizmo_position_state& gizmo = add_position_gizmo(desc.name, desc.instance);

      gizmo.rotation = normalize(desc.gizmo_rotation);
      gizmo.alignment = desc.alignment;

      const float gizmo_camera_scale = distance(_camera_positionWS, positionWS);
      const float gizmo_length =
         position_gizmo_length * _gizmo_scale * gizmo_camera_scale;
      const float gizmo_hit_pad =
         position_gizmo_hit_pad * _gizmo_scale * gizmo_camera_scale;

      const float plane_selector_length = gizmo_length * 0.2f;
      const float plane_selector_padding =
         gizmo_length * 0.5f - plane_selector_length * 0.5f;

      const float3 ray_originWS = _cursor_rayWS.origin;
      const float3 ray_directionWS = _cursor_rayWS.direction;
      const float3 view_directionGS =
         normalize(conjugate(gizmo.rotation) * (_camera_positionWS - positionWS));

      const bool yz_plane_visible =
         fabs(dot(view_directionGS, float3{1.0f, 0.0f, 0.0f})) >
         position_gizmo_plane_cull_angle;
      const bool xz_plane_visible =
         fabs(dot(view_directionGS, float3{0.0f, 1.0f, 0.0f})) >
         position_gizmo_plane_cull_angle;
      const bool xy_plane_visible =
         fabs(dot(view_directionGS, float3{0.0f, 0.0f, 1.0f})) >
         position_gizmo_plane_cull_angle;

      if (not _input.left_mouse_down) {
         _last_gizmo_deactivated = gizmo.state == position_state::active;
         gizmo.state = position_state::idle;
      }

      if (gizmo.state == position_state::idle or gizmo.state == position_state::hovered) {
         const float3 ray_originGS =
            conjugate(gizmo.rotation) * (ray_originWS - positionWS);
         const float3 ray_directionGS = conjugate(gizmo.rotation) * ray_directionWS;

         const float half_length = gizmo_length * 0.5f;
         const float padding = gizmo_hit_pad;

         const float x_hit =
            boxIntersection(ray_originGS - float3{half_length, 0.0f, 0.0f},
                            ray_directionGS, {half_length, padding, padding});
         const float y_hit =
            boxIntersection(ray_originGS - float3{0.0f, half_length, 0.0f},
                            ray_directionGS, {padding, half_length, padding});
         const float z_hit =
            boxIntersection(ray_originGS - float3{0.0f, 0.0f, half_length},
                            ray_directionGS, {padding, padding, half_length});

         const float plane_selector_half_length = plane_selector_length * 0.5f;
         const float plane_selector_centre =
            plane_selector_padding + plane_selector_length * 0.5f;

         const float yz_hit =
            yz_plane_visible
               ? boxIntersection(ray_originGS - float3{0.0f, plane_selector_centre,
                                                       plane_selector_centre},
                                 ray_directionGS,
                                 {0.0f, plane_selector_half_length,
                                  plane_selector_half_length})
               : -1.0f;
         const float xz_hit =
            xz_plane_visible
               ? boxIntersection(ray_originGS - float3{plane_selector_centre, 0.0f,
                                                       plane_selector_centre},
                                 ray_directionGS,
                                 {plane_selector_half_length, 0.0f,
                                  plane_selector_half_length})
               : -1.0f;
         const float xy_hit =
            xy_plane_visible
               ? boxIntersection(ray_originGS - float3{plane_selector_centre,
                                                       plane_selector_centre, 0.0f},
                                 ray_directionGS,
                                 {plane_selector_half_length,
                                  plane_selector_half_length, 0.0f})
               : -1.0f;

         const float xyz_hit =
            xyz_intersects(ray_originGS, ray_directionGS,
                           conjugate(gizmo.rotation) * _camera_upWS,
                           conjugate(gizmo.rotation) * _camera_rightWS,
                           plane_selector_length);

         gizmo.active_widget = position_widget::none;
         gizmo.state = position_state::idle;

         float nearest = std::numeric_limits<float>::max();

         if (x_hit > 0.0f and x_hit < nearest) {
            nearest = x_hit;
            gizmo.state = position_state::hovered;
            gizmo.active_widget = position_widget::x;
         }

         if (y_hit > 0.0f and y_hit < nearest) {
            nearest = y_hit;
            gizmo.state = position_state::hovered;
            gizmo.active_widget = position_widget::y;
         }

         if (z_hit > 0.0f and z_hit < nearest) {
            nearest = z_hit;
            gizmo.state = position_state::hovered;
            gizmo.active_widget = position_widget::z;
         }

         if (yz_hit > 0.0f and yz_hit < nearest) {
            nearest = yz_hit;
            gizmo.state = position_state::hovered;
            gizmo.active_widget = position_widget::yz;
         }

         if (xz_hit > 0.0f and xz_hit < nearest) {
            nearest = xz_hit;
            gizmo.state = position_state::hovered;
            gizmo.active_widget = position_widget::xz;
         }

         if (xy_hit > 0.0f and xy_hit < nearest) {
            nearest = xy_hit;
            gizmo.state = position_state::hovered;
            gizmo.active_widget = position_widget::xy;
         }

         if (xyz_hit > 0.0f) {
            nearest = xyz_hit;
            gizmo.state = position_state::hovered;
            gizmo.active_widget = position_widget::xyz;
         }

         if (gizmo.state == position_state::hovered) {
            if (nearest >= _closest_gizmo and gizmo.id != _closest_gizmo_id) {
               gizmo.state = position_state::idle;
               gizmo.active_widget = position_widget::none;
            }

            if (nearest < _next_frame_closest_gizmo) {
               _next_frame_closest_gizmo = nearest;
               _next_frame_closest_gizmo_id = gizmo.id;
            }
         }
      }

      if (_input.left_mouse_down) {
         if (gizmo.state == position_state::idle) {
            gizmo.state = position_state::locked;
         }
         else if (gizmo.state == position_state::hovered) {
            gizmo.state = position_state::active;
            gizmo.active_positionWS = positionWS;
            gizmo.activation_offsetGS = {0.0f, 0.0f, 0.0f};

            const float3 ray_originGS = conjugate(gizmo.rotation) * ray_originWS;
            const float3 ray_directionGS = conjugate(gizmo.rotation) * ray_directionWS;
            const float3 positionGS = conjugate(gizmo.rotation) * positionWS;

            float3 plane_normalGS = {};

            switch (gizmo.active_widget) {
            case position_widget::x:
            case position_widget::y:
            case position_widget::z: {
               const float3 eye_directionGS =
                  conjugate(gizmo.rotation) * (positionWS - _camera_positionWS);
               float3 axisGS = {};

               if (gizmo.active_widget == position_widget::x) {
                  axisGS = float3{1.0f, 0.0f, 0.0f};
               }
               else if (gizmo.active_widget == position_widget::y) {
                  axisGS = float3{0.0f, 1.0f, 0.0f};
               }
               else if (gizmo.active_widget == position_widget::z) {
                  axisGS = float3{0.0f, 0.0f, 1.0f};
               }

               const float3 plane_tangentGS = cross(axisGS, eye_directionGS);
               plane_normalGS = cross(axisGS, plane_tangentGS);
            } break;
            case position_widget::yz: {
               plane_normalGS = {1.0f, 0.0f, 0.0f};
            } break;
            case position_widget::xz: {
               plane_normalGS = {0.0f, 1.0f, 0.0f};
            } break;
            case position_widget::xy: {
               plane_normalGS = {0.0f, 0.0f, 1.0f};
            } break;
            case position_widget::xyz: {
               plane_normalGS = conjugate(gizmo.rotation) * _camera_forwardWS;
            } break;
            }

            const float4 planeGS = make_plane_from_point(positionGS, plane_normalGS);

            if (const float hit = intersect_plane(ray_originGS, ray_directionGS, planeGS);
                hit > 0.0f) {
               const float3 hit_positionGS = ray_originGS + ray_directionGS * hit;

               if (gizmo.active_widget == position_widget::x) {
                  gizmo.activation_offsetGS.x = positionGS.x - hit_positionGS.x;
               }
               else if (gizmo.active_widget == position_widget::y) {
                  gizmo.activation_offsetGS.y = positionGS.y - hit_positionGS.y;
               }
               else if (gizmo.active_widget == position_widget::z) {
                  gizmo.activation_offsetGS.z = positionGS.z - hit_positionGS.z;
               }
               else if (gizmo.active_widget == position_widget::yz) {
                  gizmo.activation_offsetGS.y = positionGS.y - hit_positionGS.y;
                  gizmo.activation_offsetGS.z = positionGS.z - hit_positionGS.z;
               }
               else if (gizmo.active_widget == position_widget::xz) {
                  gizmo.activation_offsetGS.x = positionGS.x - hit_positionGS.x;
                  gizmo.activation_offsetGS.z = positionGS.z - hit_positionGS.z;
               }
               else if (gizmo.active_widget == position_widget::xy) {
                  gizmo.activation_offsetGS.x = positionGS.x - hit_positionGS.x;
                  gizmo.activation_offsetGS.y = positionGS.y - hit_positionGS.y;
               }
               else if (gizmo.active_widget == position_widget::xyz) {
                  gizmo.activation_offsetGS = positionGS - hit_positionGS;
               }
            }
         }
      }

      if (gizmo.state == position_state::active) {
         const float3 active_positionWS = gizmo.active_positionWS;
         const float3 ray_originGS = conjugate(gizmo.rotation) * ray_originWS;
         const float3 ray_directionGS = conjugate(gizmo.rotation) * ray_directionWS;
         const float3 positionGS = conjugate(gizmo.rotation) * active_positionWS;

         float3 plane_normalGS = {};

         switch (gizmo.active_widget) {
         case position_widget::x:
         case position_widget::y:
         case position_widget::z: {
            const float3 eye_directionGS =
               conjugate(gizmo.rotation) * (active_positionWS - _camera_positionWS);
            float3 axisGS = {};

            if (gizmo.active_widget == position_widget::x) {
               axisGS = float3{1.0f, 0.0f, 0.0f};
            }
            else if (gizmo.active_widget == position_widget::y) {
               axisGS = float3{0.0f, 1.0f, 0.0f};
            }
            else if (gizmo.active_widget == position_widget::z) {
               axisGS = float3{0.0f, 0.0f, 1.0f};
            }

            const float3 plane_tangentGS = cross(axisGS, eye_directionGS);
            plane_normalGS = cross(axisGS, plane_tangentGS);
         } break;
         case position_widget::yz: {
            plane_normalGS = {1.0f, 0.0f, 0.0f};
         } break;
         case position_widget::xz: {
            plane_normalGS = {0.0f, 1.0f, 0.0f};
         } break;
         case position_widget::xy: {
            plane_normalGS = {0.0f, 0.0f, 1.0f};
         } break;
         case position_widget::xyz: {
            plane_normalGS = conjugate(gizmo.rotation) * _camera_forwardWS;
         } break;
         }

         const float4 planeGS = make_plane_from_point(positionGS, plane_normalGS);

         if (const float hit = intersect_plane(ray_originGS, ray_directionGS, planeGS);
             hit > 0.0f) {
            const float3 hit_positionGS =
               ray_originGS + ray_directionGS * hit + gizmo.activation_offsetGS;
            float3 new_positionGS = positionGS;

            const float alignment = gizmo.alignment;
            const bool align = _input.ctrl_down;

            if (gizmo.active_widget == position_widget::x) {
               new_positionGS.x = align ? align_value(hit_positionGS.x, alignment)
                                        : hit_positionGS.x;
            }
            else if (gizmo.active_widget == position_widget::y) {
               new_positionGS.y = align ? align_value(hit_positionGS.y, alignment)
                                        : hit_positionGS.y;
            }
            else if (gizmo.active_widget == position_widget::z) {
               new_positionGS.z = align ? align_value(hit_positionGS.z, alignment)
                                        : hit_positionGS.z;
            }
            else if (gizmo.active_widget == position_widget::yz) {
               new_positionGS.y = align ? align_value(hit_positionGS.y, alignment)
                                        : hit_positionGS.y;
               new_positionGS.z = align ? align_value(hit_positionGS.z, alignment)
                                        : hit_positionGS.z;
            }
            else if (gizmo.active_widget == position_widget::xz) {
               new_positionGS.x = align ? align_value(hit_positionGS.x, alignment)
                                        : hit_positionGS.x;
               new_positionGS.z = align ? align_value(hit_positionGS.z, alignment)
                                        : hit_positionGS.z;
            }
            else if (gizmo.active_widget == position_widget::xy) {
               new_positionGS.x = align ? align_value(hit_positionGS.x, alignment)
                                        : hit_positionGS.x;
               new_positionGS.y = align ? align_value(hit_positionGS.y, alignment)
                                        : hit_positionGS.y;
            }
            else if (gizmo.active_widget == position_widget::xyz) {
               new_positionGS.x = align ? align_value(hit_positionGS.x, alignment)
                                        : hit_positionGS.x;
               new_positionGS.y = align ? align_value(hit_positionGS.y, alignment)
                                        : hit_positionGS.y;
               new_positionGS.z = align ? align_value(hit_positionGS.z, alignment)
                                        : hit_positionGS.z;
            }

            positionWS = gizmo.rotation * new_positionGS;
         }
      }

      {
         const bool is_hover = gizmo.state == position_state::hovered;
         const bool is_active = gizmo.state == position_state::active;
         const bool x_active = gizmo.active_widget == position_widget::x;
         const bool y_active = gizmo.active_widget == position_widget::y;
         const bool z_active = gizmo.active_widget == position_widget::z;
         const bool yz_active = gizmo.active_widget == position_widget::yz;
         const bool xz_active = gizmo.active_widget == position_widget::xz;
         const bool xy_active = gizmo.active_widget == position_widget::xy;
         const bool xyz_active = gizmo.active_widget == position_widget::xyz;

         const float3 x_axisWS = gizmo.rotation * float3{1.0f, 0.0f, 0.0f};
         const float3 y_axisWS = gizmo.rotation * float3{0.0f, 1.0f, 0.0f};
         const float3 z_axisWS = gizmo.rotation * float3{0.0f, 0.0f, 1.0f};

         if (is_active) {
            const float3 active_positionWS = gizmo.active_positionWS;

            if (x_active or xz_active or xy_active) {
               draw_lists.pixel_lines.emplace_back(active_positionWS -
                                                      x_axisWS * _axis_line_length,
                                                   active_positionWS +
                                                      x_axisWS * _axis_line_length,
                                                   x_color_u32);
            }

            if (y_active or yz_active or xy_active) {
               draw_lists.pixel_lines.emplace_back(active_positionWS -
                                                      y_axisWS * _axis_line_length,
                                                   active_positionWS +
                                                      y_axisWS * _axis_line_length,
                                                   y_color_u32);
            }

            if (z_active or yz_active or xz_active) {
               draw_lists.pixel_lines.emplace_back(active_positionWS -
                                                      z_axisWS * _axis_line_length,
                                                   active_positionWS +
                                                      z_axisWS * _axis_line_length,
                                                   z_color_u32);
            }
         }

         const float cone_length = gizmo_hit_pad * 2.0f;
         const float line_length = gizmo_length - cone_length;

         const float3 x_line_endWS = positionWS + x_axisWS * line_length;
         const float3 y_line_endWS = positionWS + y_axisWS * line_length;
         const float3 z_line_endWS = positionWS + z_axisWS * line_length;

         draw_lists.cones.emplace_back(x_line_endWS,
                                       x_line_endWS + x_axisWS * cone_length,
                                       gizmo_hit_pad,
                                       is_hover and x_active ? x_color_hover : x_color);
         draw_lists.cones.emplace_back(y_line_endWS,
                                       y_line_endWS + y_axisWS * cone_length,
                                       gizmo_hit_pad,
                                       is_hover and y_active ? y_color_hover : y_color);
         draw_lists.cones.emplace_back(z_line_endWS,
                                       z_line_endWS + z_axisWS * cone_length,
                                       gizmo_hit_pad,
                                       is_hover and z_active ? z_color_hover : z_color);

         draw_lists.lines.emplace_back(positionWS, x_line_endWS, gizmo_hit_pad * 0.25f,
                                       is_hover and x_active ? x_color_hover : x_color);
         draw_lists.lines.emplace_back(positionWS, y_line_endWS, gizmo_hit_pad * 0.25f,
                                       is_hover and y_active ? y_color_hover : y_color);
         draw_lists.lines.emplace_back(positionWS, z_line_endWS, gizmo_hit_pad * 0.25f,
                                       is_hover and z_active ? z_color_hover : z_color);

         const float outline_width = position_gizmo_plane_outline_width;
         const float inner_alpha = position_gizmo_plane_inner_alpha;

         if (yz_plane_visible) {
            const float3 yz_minGS = {0.0f, plane_selector_padding,
                                     plane_selector_padding};
            const float3 yz_maxGS =
               yz_minGS + float3{0.0f, plane_selector_length, plane_selector_length};

            const std::array<float3, 4> yz_quadGS = {
               float3{0.0f, yz_minGS.y, yz_minGS.z},
               float3{0.0f, yz_minGS.y, yz_maxGS.z},
               float3{0.0f, yz_maxGS.y, yz_maxGS.z},
               float3{0.0f, yz_maxGS.y, yz_minGS.z},
            };

            const std::array<float3, 4> yz_quadWS = {
               gizmo.rotation * yz_quadGS[0] + positionWS,
               gizmo.rotation * yz_quadGS[1] + positionWS,
               gizmo.rotation * yz_quadGS[2] + positionWS,
               gizmo.rotation * yz_quadGS[3] + positionWS,
            };

            draw_lists.quads.emplace_back(yz_quadWS,
                                          is_hover and yz_active ? x_color_hover : x_color,
                                          outline_width, inner_alpha);
         }

         if (xz_plane_visible) {
            const float3 xz_minGS = {plane_selector_padding, 0.0f,
                                     plane_selector_padding};
            const float3 xz_maxGS =
               xz_minGS + float3{plane_selector_length, 0.0f, plane_selector_length};

            const std::array<float3, 4> xz_quadGS = {
               float3{xz_minGS.x, 0.0f, xz_minGS.z},
               float3{xz_minGS.x, 0.0f, xz_maxGS.z},
               float3{xz_maxGS.x, 0.0f, xz_maxGS.z},
               float3{xz_maxGS.x, 0.0f, xz_minGS.z},
            };

            const std::array<float3, 4> xz_quadWS = {
               gizmo.rotation * xz_quadGS[0] + positionWS,
               gizmo.rotation * xz_quadGS[1] + positionWS,
               gizmo.rotation * xz_quadGS[2] + positionWS,
               gizmo.rotation * xz_quadGS[3] + positionWS,
            };

            draw_lists.quads.emplace_back(xz_quadWS,
                                          is_hover and xz_active ? y_color_hover : y_color,
                                          outline_width, inner_alpha);
         }

         if (xy_plane_visible) {
            const float3 xy_minGS = {plane_selector_padding,
                                     plane_selector_padding, 0.0f};
            const float3 xy_maxGS =
               xy_minGS + float3{plane_selector_length, plane_selector_length, 0.0f};

            const std::array<float3, 4> xy_quadGS = {
               float3{xy_minGS.x, xy_minGS.y, 0.0f},
               float3{xy_maxGS.x, xy_minGS.y, 0.0f},
               float3{xy_maxGS.x, xy_maxGS.y, 0.0f},
               float3{xy_minGS.x, xy_maxGS.y, 0.0f},
            };

            const std::array<float3, 4> xy_quadWS = {
               gizmo.rotation * xy_quadGS[0] + positionWS,
               gizmo.rotation * xy_quadGS[1] + positionWS,
               gizmo.rotation * xy_quadGS[2] + positionWS,
               gizmo.rotation * xy_quadGS[3] + positionWS,
            };

            draw_lists.quads.emplace_back(xy_quadWS,
                                          is_hover and xy_active ? z_color_hover : z_color,
                                          outline_width, inner_alpha);
         }

#
         const std::array<float3, 2> xyz_lineWS = {
            positionWS + _camera_rightWS * plane_selector_length * 0.5f,
            positionWS - _camera_rightWS * plane_selector_length * 0.5f,
         };

         const std::array<float3, 4> xyz_quadWS = {
            xyz_lineWS[0] + _camera_upWS * plane_selector_length * 0.5f,
            xyz_lineWS[1] + _camera_upWS * plane_selector_length * 0.5f,
            xyz_lineWS[1] - _camera_upWS * plane_selector_length * 0.5f,
            xyz_lineWS[0] - _camera_upWS * plane_selector_length * 0.5f,
         };

         draw_lists.quads.emplace_back(xyz_quadWS, float3{1.0f, 1.0f, 1.0f},
                                       outline_width,
                                       is_hover and xyz_active ? xyz_hover_alpha
                                                               : inner_alpha);
      }

      _want_mouse_input |= (gizmo.state == position_state::hovered or
                            gizmo.state == position_state::active);
      _want_keyboard_input |= gizmo.state == position_state::active;

      return gizmo.state == position_state::active;
   }

   bool gizmo_movement(const gizmo_movement_desc& desc, float3& out_movement) noexcept
   {
      gizmo_movement_state& gizmo = add_movement_gizmo(desc.name, desc.instance);

      if (add_position_gizmo(desc.name, desc.instance).state != position_state::active) {
         gizmo.rotation = normalize(desc.gizmo_rotation);
         gizmo.positionWS = desc.gizmo_positionWS;
      }

      const float3 start_positionWS = gizmo.positionWS;

      const bool moved = gizmo_position({.name = desc.name,
                                         .instance = desc.instance,
                                         .alignment = desc.alignment,
                                         .gizmo_rotation = desc.gizmo_rotation},
                                        gizmo.positionWS);

      if (moved) {
         if (desc.gizmo_space_output) {
            out_movement = conjugate(gizmo.rotation) * gizmo.positionWS -
                           conjugate(gizmo.rotation) * start_positionWS;
         }
         else {
            out_movement = gizmo.positionWS - start_positionWS;
         }
      }
      else {
         out_movement = {0.0f, 0.0f, 0.0f};
      }

      return moved;
   }

   bool can_close_last_edit() const noexcept
   {
      return _last_gizmo_deactivated;
   }

   void deactivate() noexcept
   {
      abort();
   }

   gizmo_draw_lists draw_lists;

private:
   graphics::camera_ray _cursor_rayWS;
   gizmo_button_input _input;
   bool _want_mouse_input = false;
   bool _want_keyboard_input = false;
   bool _capture_mouse = false;
   bool _capture_keyboard = false;
   bool _last_gizmo_deactivated = false;

   std::vector<gizmo_position_state> _position_gizmos;
   std::vector<gizmo_movement_state> _movement_gizmos;

   float _gizmo_scale = 1.0f;
   float3 _camera_positionWS = {0.0f, 0.0f, 0.0f};
   float3 _camera_forwardWS = {0.0f, 0.0f, 1.0f};
   float3 _camera_upWS = {0.0f, 0.0f, 1.0f};
   float3 _camera_rightWS = {0.0f, 0.0f, 1.0f};
   float _axis_line_length = 128.0f;

   float _closest_gizmo = 0.0f;
   gizmo_id _closest_gizmo_id;

   float _next_frame_closest_gizmo = FLT_MAX;
   gizmo_id _next_frame_closest_gizmo_id;

   auto add_position_gizmo(const std::string_view name, const int64 instance) noexcept
      -> gizmo_position_state&
   {
      for (gizmo_position_state& existing : _position_gizmos) {
         if (existing.id.name == name and existing.id.instance == instance) {
            existing.submitted_last_frame = true;

            return existing;
         }
      }

      return _position_gizmos.emplace_back(gizmo_position_state{
         .id = {.name = std::string{name}, .instance = instance}});
   }

   auto add_movement_gizmo(const std::string_view name, const int64 instance) noexcept
      -> gizmo_movement_state&
   {
      for (gizmo_movement_state& existing : _movement_gizmos) {
         if (existing.id.name == name and existing.id.instance == instance) {
            existing.submitted_last_frame = true;

            return existing;
         }
      }

      return _movement_gizmos.emplace_back(gizmo_movement_state{
         .id = {.name = std::string{name}, .instance = instance}});
   }

   void garbage_collect() noexcept
   {
      for (auto it = _position_gizmos.begin(); it != _position_gizmos.end();) {
         gizmo_position_state& gizmo = *it;

         if (not gizmo.submitted_last_frame) {
            it = _position_gizmos.erase(it);
         }
         else {
            gizmo.submitted_last_frame = false;

            it += 1;
         }
      }

      for (auto it = _movement_gizmos.begin(); it != _movement_gizmos.end();) {
         gizmo_movement_state& gizmo = *it;

         if (not gizmo.submitted_last_frame) {
            it = _movement_gizmos.erase(it);
         }
         else {
            gizmo.submitted_last_frame = false;

            it += 1;
         }
      }
   }
};

gizmos::gizmos() = default;

gizmos::~gizmos() = default;

bool gizmos::want_capture_mouse() const noexcept
{
   return impl->want_capture_mouse();
}

bool gizmos::want_capture_keyboard() const noexcept
{
   return impl->want_capture_keyboard();
}

void gizmos::update(const graphics::camera_ray cursor_ray,
                    const gizmo_button_input button_input,
                    const graphics::camera& camera, const float gizmo_scale) noexcept
{
   return impl->update(cursor_ray, button_input, camera, gizmo_scale);
}

auto gizmos::get_draw_lists() const noexcept -> const gizmo_draw_lists&
{
   return impl->draw_lists;
}

bool gizmos::gizmo_position(const gizmo_position_desc& desc, float3& positionWS) noexcept
{
   return impl->gizmo_position(desc, positionWS);
}

bool gizmos::gizmo_movement(const gizmo_movement_desc& desc, float3& out_movement) noexcept
{
   return impl->gizmo_movement(desc, out_movement);
}

bool gizmos::can_close_last_edit() const noexcept
{
   return impl->can_close_last_edit();
}

void gizmos::deactivate() noexcept
{
   return impl->deactivate();
}

}