
#include "gizmo.hpp"
#include "math/intersectors.hpp"
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

auto make_plane(float3 position, float3 normal) noexcept -> float4
{
   const float w = -dot(position, normal);

   return float4{normal, w};
}

auto make_circle_point(const float angle, const float radius) -> float2
{
   return {radius * std::cos(angle), radius * std::sin(angle)};
}

auto rotate_gizmo_intersect(const float3 ray_origin, const float3 ray_direction,
                            const float3 position, const float3 normal,
                            const float nohit_radius, const float hit_radius) -> float
{
   if (diskIntersect(ray_origin, ray_direction, position, normal, nohit_radius) > 0.0f) {
      return -1.0f;
   }

   return diskIntersect(ray_origin, ray_direction, position, normal, hit_radius);
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
   case mode::translate: {
      return _translate.active_axis != axis::none;
   } break;
   case mode::rotate: {
      return _rotate.active_axis != axis::none;
   } break;
   }

   std::unreachable();
}

void gizmo::update(const graphics::camera_ray cursor_ray, const bool is_mouse_down,
                   const graphics::camera& camera, const float gizmo_scale) noexcept
{
   const float camera_scale = distance(camera.position(), _gizmo_position) *
                              camera.projection_matrix()[0].x;

   _translate_gizmo_length = translate_gizmo_length * camera_scale * gizmo_scale;
   _translate_gizmo_hit_pad = translate_gizmo_hit_pad * camera_scale * gizmo_scale;
   _rotate_gizmo_radius = rotate_gizmo_radius * camera_scale * gizmo_scale;
   _rotate_gizmo_hit_pad = rotate_gizmo_hit_pad * camera_scale * gizmo_scale;

   const float3 ray_origin = cursor_ray.origin;
   const float3 offset_ray_origin = ray_origin - _gizmo_position;
   const float3 ray_direction = cursor_ray.direction;

   if (not std::exchange(_used_last_tick, false)) {
      _mode = mode::inactive;
   }

   switch (_mode) {
   case mode::inactive: {
      _translate = {};
   } break;
   case mode::translate: {
      if (not _translate.translating) {
         const float3 local_ray_origin = conjugate(_gizmo_rotation) * offset_ray_origin;
         const float3 local_ray_direction = conjugate(_gizmo_rotation) * ray_direction;

         const float half_length = _translate_gizmo_length / 2.0f;
         const float padding = _translate_gizmo_hit_pad;

         const float x_hit =
            boxIntersection(local_ray_origin - float3{half_length, 0.0f, 0.0f},
                            local_ray_direction, {half_length, padding, padding});
         const float y_hit =
            boxIntersection(local_ray_origin - float3{0.0f, half_length, 0.0f},
                            local_ray_direction, {padding, half_length, padding});
         const float z_hit =
            boxIntersection(local_ray_origin - float3{0.0f, 0.0f, half_length},
                            local_ray_direction, {padding, padding, half_length});

         _translate.active_axis = axis::none;

         float nearest = std::numeric_limits<float>::max();

         if (x_hit > 0.0f and x_hit < nearest) {
            nearest = x_hit;
            _translate.active_axis = axis::x;
         }
         if (y_hit > 0.0f and y_hit < nearest) {
            nearest = y_hit;
            _translate.active_axis = axis::y;
         }
         if (z_hit > 0.0f and z_hit < nearest) {
            nearest = z_hit;
            _translate.active_axis = axis::z;
         }

         if (_translate.active_axis == axis::none and is_mouse_down) {
            _translate.mouse_down_over_gizmo = false;
         }
         else if (is_mouse_down and not _translate.mouse_down_over_gizmo) {
            _translate.start_position = _gizmo_position;
            _translate.start_cursor_position =
               get_translate_position(cursor_ray, camera.position(), _gizmo_position);
            _translate.translating = true;
         }
      }

      if (_translate.translating and is_mouse_down and
          _translate.active_axis != axis::none) {
         const float3 position =
            get_translate_position(cursor_ray, camera.position(),
                                   _translate.start_cursor_position);
         const float3 movement = position - _translate.start_cursor_position;

         if (_translate.active_axis == axis::x) {
            _translate.movement_x = movement.x;
         }
         else if (_translate.active_axis == axis::y) {
            _translate.movement_y = movement.y;
         }
         else if (_translate.active_axis == axis::z) {
            _translate.movement_z = movement.z;
         }
      }
      else if (_translate.translating) {
         _translate = {};
      }
   } break;
   case mode::rotate: {
      if (not _rotate.rotating) {
         const float nohit_radius = _rotate_gizmo_radius - _rotate_gizmo_hit_pad;
         const float hit_radius = _rotate_gizmo_radius + _rotate_gizmo_hit_pad;

         const float x_hit =
            rotate_gizmo_intersect(ray_origin, ray_direction, _gizmo_position,
                                   float3{1.0f, 0.0f, 0.0f}, nohit_radius, hit_radius);
         const float y_hit =
            rotate_gizmo_intersect(ray_origin, ray_direction, _gizmo_position,
                                   float3{0.0f, 1.0f, 0.0f}, nohit_radius, hit_radius);
         const float z_hit =
            rotate_gizmo_intersect(ray_origin, ray_direction, _gizmo_position,
                                   float3{0.0f, 0.0f, 1.0f}, nohit_radius, hit_radius);

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
            _rotate.mouse_down_over_gizmo = false;
         }
         else if (is_mouse_down and not _rotate.mouse_down_over_gizmo) {
            const float3 unclamped_cursor_position =
               get_rotate_position(cursor_ray, _rotate.current_cursor_position);
            const float3 cursor_direction =
               normalize(unclamped_cursor_position - _gizmo_position);

            _rotate.current_cursor_position =
               _gizmo_position + cursor_direction * _rotate_gizmo_radius;
            _rotate.rotating = true;
         }
      }

      if (_rotate.rotating and is_mouse_down and _rotate.active_axis != axis::none) {
         const float3 unclamped_cursor_position =
            get_rotate_position(cursor_ray, _rotate.current_cursor_position);
         const float3 cursor_direction =
            normalize(unclamped_cursor_position - _gizmo_position);

         const float3 cursor_position =
            _gizmo_position + cursor_direction * _rotate_gizmo_radius;
         const float3 current_cursor_position = _rotate.current_cursor_position;

         if (_rotate.active_axis == axis::x) {
            const float start_angle =
               std::atan2(current_cursor_position.y - _gizmo_position.y,
                          current_cursor_position.z - _gizmo_position.z);
            const float current_angle =
               std::atan2(cursor_position.y - _gizmo_position.y,
                          cursor_position.z - _gizmo_position.z);

            _rotate.angle_x = (current_angle - start_angle);
         }
         else if (_rotate.active_axis == axis::y) {
            const float start_angle =
               std::atan2(current_cursor_position.x - _gizmo_position.x,
                          current_cursor_position.z - _gizmo_position.z);
            const float current_angle =
               std::atan2(cursor_position.x - _gizmo_position.x,
                          cursor_position.z - _gizmo_position.z);

            _rotate.angle_y = current_angle - start_angle;
         }
         else if (_rotate.active_axis == axis::z) {
            const float start_angle =
               std::atan2(current_cursor_position.x - _gizmo_position.x,
                          current_cursor_position.y - _gizmo_position.y);
            const float current_angle =
               std::atan2(cursor_position.x - _gizmo_position.x,
                          cursor_position.y - _gizmo_position.y);

            _rotate.angle_z = current_angle - start_angle;
         }

         _rotate.current_cursor_position = cursor_position;
      }
      else if (_rotate.rotating) {
         _rotate = {};
      }
   } break;
   }
}

void gizmo::draw(world::tool_visualizers& tool_visualizers) noexcept
{
   if (_mode == mode::translate) {
      const float length = _translate_gizmo_length;

      const bool x_hover = _translate.active_axis == axis::x;
      const bool y_hover = _translate.active_axis == axis::y;
      const bool z_hover = _translate.active_axis == axis::z;

      const float3 x_axis = _gizmo_rotation * float3{1.0f, 0.0f, 0.0f};
      const float3 y_axis = _gizmo_rotation * float3{0.0f, 1.0f, 0.0f};
      const float3 z_axis = _gizmo_rotation * float3{0.0f, 0.0f, 1.0f};

      if (_translate.translating) {
         switch (_translate.active_axis) {
         case axis::x: {
            tool_visualizers.lines.emplace_back(_gizmo_position - x_axis * 1024.0f,
                                                _gizmo_position + x_axis * 1024.0f,
                                                x_color);
         } break;
         case axis::y: {
            tool_visualizers.lines.emplace_back(_gizmo_position - y_axis * 1024.0f,
                                                _gizmo_position + y_axis * 1024.0f,
                                                y_color);
         } break;
         case axis::z: {
            tool_visualizers.lines.emplace_back(_gizmo_position - z_axis * 1024.0f,
                                                _gizmo_position + z_axis * 1024.0f,
                                                z_color);
         } break;
         }
      }

      tool_visualizers.lines.emplace_back(_gizmo_position,
                                          _gizmo_position + x_axis * length,
                                          x_hover ? x_color_hover : x_color);
      tool_visualizers.lines.emplace_back(_gizmo_position,
                                          _gizmo_position + y_axis * length,
                                          y_hover ? y_color_hover : y_color);
      tool_visualizers.lines.emplace_back(_gizmo_position,
                                          _gizmo_position + z_axis * length,
                                          z_hover ? z_color_hover : z_color);
   }
   else if (_mode == mode::rotate) {
      constexpr float pi_2 = std::numbers::pi_v<float> * 2.0f;
      constexpr float divisor = circle_divisions;
      const float radius = _rotate_gizmo_radius;

      const bool x_hover = _rotate.active_axis == axis::x;
      const bool y_hover = _rotate.active_axis == axis::y;
      const bool z_hover = _rotate.active_axis == axis::z;

      tool_visualizers.lines.reserve(tool_visualizers.lines.size() +
                                     circle_divisions * 4);

      if (_rotate.rotating) {
         const float3 direction =
            normalize(_rotate.current_cursor_position - _gizmo_position);

         tool_visualizers.lines.emplace_back(_gizmo_position,
                                             _gizmo_position + direction * radius,
                                             0xff'ff'ff'ff);
      }

      for (uint32 i = 0; i <= circle_divisions; ++i) {
         const float2 start =
            make_circle_point((static_cast<float>(i) / divisor) * pi_2, radius);
         const float2 end =
            make_circle_point((static_cast<float>((i + 1) % circle_divisions) / divisor) * pi_2,
                              radius);

         tool_visualizers.lines.emplace_back(_gizmo_position +
                                                float3{0.0f, start.x, start.y},
                                             _gizmo_position +
                                                float3{0.0f, end.x, end.y},
                                             x_hover ? x_color_hover : x_color);
      }

      for (uint32 i = 0; i <= circle_divisions; ++i) {
         const float2 start =
            make_circle_point((static_cast<float>(i) / divisor) * pi_2, radius);
         const float2 end =
            make_circle_point((static_cast<float>((i + 1) % circle_divisions) / divisor) * pi_2,
                              radius);

         tool_visualizers.lines.emplace_back(_gizmo_position +
                                                float3{start.x, 0.0f, start.y},
                                             _gizmo_position +
                                                float3{end.x, 0.0f, end.y},
                                             y_hover ? y_color_hover : y_color);
      }

      for (uint32 i = 0; i <= circle_divisions; ++i) {
         const float2 start =
            make_circle_point((static_cast<float>(i) / divisor) * pi_2, radius);
         const float2 end =
            make_circle_point((static_cast<float>((i + 1) % circle_divisions) / divisor) * pi_2,
                              radius);

         tool_visualizers.lines.emplace_back(_gizmo_position +
                                                float3{start.x, start.y, 0.0f},
                                             _gizmo_position + float3{end.x, end.y, 0.0f},
                                             z_hover ? z_color_hover : z_color);
      }
   }
}

bool gizmo::show_translate(const float3 gizmo_position,
                           const quaternion gizmo_rotation, float3& movement) noexcept
{
   _gizmo_position = gizmo_position;
   _gizmo_rotation = normalize(gizmo_rotation);
   _used_last_tick = true;

   if (not _translate.start_movement) _translate.start_movement = movement;

   if (std::exchange(_mode, mode::translate) != mode::translate) {
      return false;
   }

   const float3 start_movement = movement;

   if (std::optional<float> movement_x =
          std::exchange(_translate.movement_x, std::nullopt);
       movement_x) {
      movement.x = (*movement_x + _translate.start_movement->x);
   }
   if (std::optional<float> movement_y =
          std::exchange(_translate.movement_y, std::nullopt);
       movement_y) {
      movement.y = (*movement_y + _translate.start_movement->y);
   }
   if (std::optional<float> movement_z =
          std::exchange(_translate.movement_z, std::nullopt);
       movement_z) {
      movement.z = (*movement_z + _translate.start_movement->z);
   }

   return start_movement != movement;
}

bool gizmo::show_rotate(const float3 gizmo_position, float3& rotation) noexcept
{
   _gizmo_position = gizmo_position;
   _gizmo_rotation = {};
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

void gizmo::deactivate() noexcept
{
   _mode = mode::inactive;
   _used_last_tick = false;
}

auto gizmo::get_translate_position(graphics::camera_ray world_ray,
                                   const float3 camera_position,
                                   const float3 fallback) const noexcept -> float3
{
   graphics::camera_ray ray;

   ray.origin = conjugate(_gizmo_rotation) * (world_ray.origin);
   ray.direction = conjugate(_gizmo_rotation) * world_ray.direction;

   const float3 eye_direction =
      conjugate(_gizmo_rotation) * (_translate.start_position - camera_position);
   const float3 axis = [&] {
      if (_translate.active_axis == axis::x) {
         return float3{1.0f, 0.0f, 0.0f};
      }
      else if (_translate.active_axis == axis::y) {
         return float3{0.0f, 1.0f, 0.0f};
      }
      else if (_translate.active_axis == axis::z) {
         return float3{0.0f, 0.0f, 1.0f};
      }

      return float3{};
   }();

   const float3 plane_tangent = cross(axis, eye_direction);
   const float3 plane_normal = cross(axis, plane_tangent);

   const float4 plane =
      make_plane(conjugate(_gizmo_rotation) * _translate.start_position, plane_normal);

   if (const float hit = plaIntersect(ray.origin, ray.direction, plane); hit > 0.0f) {
      return ray.origin + ray.direction * hit;
   }

   return fallback;
}

auto gizmo::get_rotate_position(const graphics::camera_ray ray,
                                const float3 fallback) const noexcept -> float3
{
   const float4 x_plane = make_plane(_gizmo_position, float3{1.0f, 0.0f, 0.0f});
   const float4 y_plane = make_plane(_gizmo_position, float3{0.0f, 1.0f, 0.0f});
   const float4 z_plane = make_plane(_gizmo_position, float3{0.0f, 0.0f, 1.0f});

   float distance = std::numeric_limits<float>::max();

   if (_rotate.active_axis == axis::x) {
      if (const float x_plane_hit = plaIntersect(ray.origin, ray.direction, x_plane);
          x_plane_hit > 0.0f) {
         distance = std::min(x_plane_hit, distance);
      }
   }
   else if (_rotate.active_axis == axis::y) {
      if (const float y_plane_hit = plaIntersect(ray.origin, ray.direction, y_plane);
          y_plane_hit > 0.0f) {
         distance = std::min(y_plane_hit, distance);
      }
   }
   else {
      if (const float z_plane_hit = plaIntersect(ray.origin, ray.direction, z_plane);
          z_plane_hit > 0.0f) {
         distance = std::min(z_plane_hit, distance);
      }
   }

   if (distance != std::numeric_limits<float>::max()) {
      return ray.origin + ray.direction * distance;
   }

   return fallback;
}

}