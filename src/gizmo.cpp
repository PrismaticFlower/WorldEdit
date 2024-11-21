
#include "gizmo.hpp"
#include "math/iq_intersectors.hpp"
#include "math/plane_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <cmath>
#include <limits>
#include <numbers>
#include <utility>

namespace we {

namespace {

constexpr uint32 x_color = 0xff'e5'40'40;
constexpr uint32 y_color = 0xff'40'60'e5;
constexpr uint32 z_color = 0xff'30'e5'30;

constexpr uint32 x_color_hover = 0xff'ff'80'80;
constexpr uint32 y_color_hover = 0xff'80'a0'ff;
constexpr uint32 z_color_hover = 0xff'90'ff'90;

constexpr uint32 circle_divisions = 64;

auto make_circle_point(const float angle, const float radius) -> float2
{
   return {radius * std::cos(angle), radius * std::sin(angle)};
}

auto rotate_gizmo_intersect(const float3 ray_origin, const float3 ray_direction,
                            const float3 normal, const float nohit_radius,
                            const float hit_radius) -> float
{
   if (diskIntersect(ray_origin, ray_direction, float3{0.0f, 0.0f, 0.0f},
                     normal, nohit_radius) > 0.0f) {
      return -1.0f;
   }

   return diskIntersect(ray_origin, ray_direction, float3{0.0f, 0.0f, 0.0f},
                        normal, hit_radius);
}

constexpr float translate_gizmo_length = 0.1f;
constexpr float translate_gizmo_hit_pad = 0.00625f;
constexpr float rotate_gizmo_radius = 0.1f;
constexpr float rotate_gizmo_hit_pad = 0.01f;

}

bool gizmo::want_capture_mouse() const noexcept
{
   switch (_mode) {
   case mode::inactive: {
      return false;
   } break;
   case mode::rotate: {
      return _rotate.active_axis != axis::none;
   } break;
   }

   std::unreachable();
}

void gizmo::update(const graphics::camera_ray cursor_ray,
                   const bool is_mouse_down, const graphics::camera& camera) noexcept
{
   const float3 ray_origin = cursor_ray.origin;
   const float3 offset_ray_origin = ray_origin - _gizmo_position;
   const float3 ray_direction = cursor_ray.direction;

   if (not std::exchange(_used_last_tick, false)) {
      _mode = mode::inactive;
   }

   _rotating_last_frame = _rotate.rotating;

   switch (_mode) {
   case mode::inactive: {
      _rotate = {};
   } break;
   case mode::rotate: {
      graphics::camera_ray rayGS{
         .origin = _world_to_gizmo * cursor_ray.origin,
         .direction = normalize(float3x3{_world_to_gizmo} * cursor_ray.direction),
      };
      const float3 camera_positionGS = _world_to_gizmo * camera.position();

      if (not _rotate.rotating) {
         const float nohit_radius = _rotate_gizmo_radius - _rotate_gizmo_hit_pad;
         const float hit_radius = _rotate_gizmo_radius + _rotate_gizmo_hit_pad;

         const float x_hit = rotate_gizmo_intersect(rayGS.origin, rayGS.direction,
                                                    float3{1.0f, 0.0f, 0.0f},
                                                    nohit_radius, hit_radius);
         const float y_hit = rotate_gizmo_intersect(rayGS.origin, rayGS.direction,
                                                    float3{0.0f, 1.0f, 0.0f},
                                                    nohit_radius, hit_radius);
         const float z_hit = rotate_gizmo_intersect(rayGS.origin, rayGS.direction,
                                                    float3{0.0f, 0.0f, 1.0f},
                                                    nohit_radius, hit_radius);

         _rotate.active_axis = axis::none;

         float nearest = std::numeric_limits<float>::max();

         if (x_hit > 0.0f and x_hit < nearest) {
            nearest = x_hit;
            _rotate.active_axis = axis::x;
         }
         if (y_hit > 0.0f and y_hit < nearest) {
            nearest = y_hit;
            _rotate.active_axis = axis::y;
         }
         if (z_hit > 0.0f and z_hit < nearest) {
            nearest = z_hit;
            _rotate.active_axis = axis::z;
         }

         if (_rotate.active_axis == axis::none and is_mouse_down) {
            _rotate.mouse_down_outside_gizmo = true;
         }
         else if (is_mouse_down and not _rotate.mouse_down_outside_gizmo) {
            const float3 unclamped_cursor_positionGS =
               get_rotate_position(rayGS, _rotate.current_cursor_positionGS);
            const float3 cursor_directionGS = normalize(unclamped_cursor_positionGS);

            _rotate.current_cursor_positionGS =
               cursor_directionGS * _rotate_gizmo_radius;
            _rotate.rotating = true;
         }
         else if (not is_mouse_down) {
            _rotate.mouse_down_outside_gizmo = false;
         }

         if (_rotate.mouse_down_outside_gizmo) {
            _rotate.active_axis = axis::none;
         }
      }

      if (_rotate.rotating and is_mouse_down and _rotate.active_axis != axis::none) {
         const float3 unclamped_cursor_positionGS =
            get_rotate_position(rayGS, _rotate.current_cursor_positionGS);
         const float3 cursor_directionGS = normalize(unclamped_cursor_positionGS);

         const float3 cursor_positionGS = cursor_directionGS * _rotate_gizmo_radius;
         const float3 current_cursor_positionGS = _rotate.current_cursor_positionGS;

         if (_rotate.active_axis == axis::x) {
            const float start_angle = std::atan2(current_cursor_positionGS.y,
                                                 current_cursor_positionGS.z);
            const float current_angle =
               std::atan2(cursor_positionGS.y, cursor_positionGS.z);

            _rotate.angle_x = (current_angle - start_angle);
         }
         else if (_rotate.active_axis == axis::y) {
            const float start_angle = std::atan2(current_cursor_positionGS.x,
                                                 current_cursor_positionGS.z);
            const float current_angle =
               std::atan2(cursor_positionGS.x, cursor_positionGS.z);

            _rotate.angle_y = current_angle - start_angle;
         }
         else if (_rotate.active_axis == axis::z) {
            const float start_angle = std::atan2(current_cursor_positionGS.x,
                                                 current_cursor_positionGS.y);
            const float current_angle =
               std::atan2(cursor_positionGS.x, cursor_positionGS.y);

            _rotate.angle_z = current_angle - start_angle;
         }

         _rotate.current_cursor_positionGS = cursor_positionGS;
      }
      else if (_rotate.rotating) {
         _rotate = {};
      }
   } break;
   }
}

void gizmo::update_scale(const graphics::camera& camera, const float gizmo_scale) noexcept
{
   const float camera_scale = distance(camera.position(), _gizmo_position) *
                              camera.projection_matrix()[0].x;

   _rotate_gizmo_radius = rotate_gizmo_radius * camera_scale * gizmo_scale;
   _rotate_gizmo_hit_pad = rotate_gizmo_hit_pad * camera_scale * gizmo_scale;
   _axis_line_length = camera.far_clip();
}

void gizmo::draw(world::tool_visualizers& tool_visualizers) noexcept
{
   if (_mode == mode::rotate) {
      constexpr float pi_2 = std::numbers::pi_v<float> * 2.0f;
      constexpr float divisor = circle_divisions;
      const float radius = _rotate_gizmo_radius;

      const bool x_hover = _rotate.active_axis == axis::x;
      const bool y_hover = _rotate.active_axis == axis::y;
      const bool z_hover = _rotate.active_axis == axis::z;

      if (_rotate.rotating) {
         const float3 directionGS = normalize(_rotate.current_cursor_positionGS);

         tool_visualizers.add_line_overlay(_gizmo_to_world * float3{0.0f, 0.0f, 0.0f},
                                           _gizmo_to_world * (directionGS * radius),
                                           0xff'ff'ff'ff);
      }

      for (uint32 i = 0; i <= circle_divisions; ++i) {
         const float2 start =
            make_circle_point((static_cast<float>(i) / divisor) * pi_2, radius);
         const float2 end =
            make_circle_point((static_cast<float>((i + 1) % circle_divisions) / divisor) * pi_2,
                              radius);

         tool_visualizers.add_line_overlay(_gizmo_to_world *
                                              float3{0.0f, start.x, start.y},
                                           _gizmo_to_world * float3{0.0f, end.x, end.y},
                                           x_hover ? x_color_hover : x_color);
      }

      for (uint32 i = 0; i <= circle_divisions; ++i) {
         const float2 start =
            make_circle_point((static_cast<float>(i) / divisor) * pi_2, radius);
         const float2 end =
            make_circle_point((static_cast<float>((i + 1) % circle_divisions) / divisor) * pi_2,
                              radius);

         tool_visualizers.add_line_overlay(_gizmo_to_world *
                                              float3{start.x, 0.0f, start.y},
                                           _gizmo_to_world * float3{end.x, 0.0f, end.y},
                                           y_hover ? y_color_hover : y_color);
      }

      for (uint32 i = 0; i <= circle_divisions; ++i) {
         const float2 start =
            make_circle_point((static_cast<float>(i) / divisor) * pi_2, radius);
         const float2 end =
            make_circle_point((static_cast<float>((i + 1) % circle_divisions) / divisor) * pi_2,
                              radius);

         tool_visualizers.add_line_overlay(_gizmo_to_world *
                                              float3{start.x, start.y, 0.0f},
                                           _gizmo_to_world * float3{end.x, end.y, 0.0f},
                                           z_hover ? z_color_hover : z_color);
      }
   }
}

bool gizmo::show_rotate(const float3 gizmo_position,
                        const quaternion gizmo_rotation, float3& rotation) noexcept
{
   if (not _rotate.rotating) {
      _gizmo_position = gizmo_position;
      _gizmo_rotation = gizmo_rotation;

      _gizmo_to_world = to_matrix(_gizmo_rotation);
      _gizmo_to_world[3] = {gizmo_position, 1.0f};

      _world_to_gizmo = to_matrix(conjugate(_gizmo_rotation));
      _world_to_gizmo[3] = {conjugate(_gizmo_rotation) * -gizmo_position, 1.0f};
   }

   _used_last_tick = true;

   if (std::exchange(_mode, mode::rotate) != mode::rotate) {
      return false;
   }

   const float3 start_rotation = rotation;

   if (std::optional<float> angle_x = std::exchange(_rotate.angle_x, std::nullopt);
       angle_x) {
      rotation.x += *angle_x;
   }
   if (std::optional<float> angle_y = std::exchange(_rotate.angle_y, std::nullopt);
       angle_y) {
      rotation.y += *angle_y;
   }
   if (std::optional<float> angle_z = std::exchange(_rotate.angle_z, std::nullopt);
       angle_z) {
      rotation.z += *angle_z;
   }

   return start_rotation != rotation;
}

bool gizmo::can_close_last_edit() const noexcept
{
   if (_rotating_last_frame and not _rotate.rotating) return true;

   return false;
}

void gizmo::deactivate() noexcept
{
   _mode = mode::inactive;
   _used_last_tick = false;
}

auto gizmo::get_rotate_position(const graphics::camera_ray rayGS,
                                const float3 fallbackGS) const noexcept -> float3
{
   const float4 x_planeGS = make_plane_from_point(float3{}, float3{1.0f, 0.0f, 0.0f});
   const float4 y_planeGS = make_plane_from_point(float3{}, float3{0.0f, 1.0f, 0.0f});
   const float4 z_planeGS = make_plane_from_point(float3{}, float3{0.0f, 0.0f, 1.0f});

   float distance = std::numeric_limits<float>::max();

   if (_rotate.active_axis == axis::x) {
      if (const float x_plane_hit =
             intersect_plane(rayGS.origin, rayGS.direction, x_planeGS);
          x_plane_hit > 0.0f) {
         distance = std::min(x_plane_hit, distance);
      }
   }
   else if (_rotate.active_axis == axis::y) {
      if (const float y_plane_hit =
             intersect_plane(rayGS.origin, rayGS.direction, y_planeGS);
          y_plane_hit > 0.0f) {
         distance = std::min(y_plane_hit, distance);
      }
   }
   else {
      if (const float z_plane_hit =
             intersect_plane(rayGS.origin, rayGS.direction, z_planeGS);
          z_plane_hit > 0.0f) {
         distance = std::min(z_plane_hit, distance);
      }
   }

   if (distance != std::numeric_limits<float>::max()) {
      return rayGS.origin + rayGS.direction * distance;
   }

   return fallbackGS;
}

}