
#include "gizmo.hpp"
#include "math/intersectors.hpp"
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

void gizmo::update(const graphics::camera_ray cursor_ray, const bool is_mouse_down) noexcept
{
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
         const float length = _translate_gizmo_hit_length;

         const float x_hit = boxIntersection(offset_ray_origin, ray_direction,
                                             {_translate_gizmo_size, length, length});
         const float y_hit = boxIntersection(offset_ray_origin, ray_direction,
                                             {length, _translate_gizmo_size, length});
         const float z_hit = boxIntersection(offset_ray_origin, ray_direction,
                                             {length, length, _translate_gizmo_size});

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
               get_translate_position(cursor_ray, _gizmo_position);
            _translate.translating = true;
         }
      }

      if (_translate.translating and is_mouse_down and
          _translate.active_axis != axis::none) {
         const float3 position =
            get_translate_position(cursor_ray, _translate.start_cursor_position);
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
         const float radius = _rotate_gizmo_radius + _rotate_gizmo_hit_pad;
         const float height = _rotate_gizmo_hit_height;

         const float x_hit =
            iCylinder(ray_origin, ray_direction,
                      _gizmo_position + float3{1.0f, 0.0f, 0.0f} * height,
                      _gizmo_position - float3{1.0f, 0.0f, 0.0f} * height, radius)
               .x;
         const float y_hit =
            iCylinder(ray_origin, ray_direction,
                      _gizmo_position + float3{0.0f, 1.0f, 0.0f} * height,
                      _gizmo_position - float3{0.0f, 1.0f, 0.0f} * height, radius)
               .x;
         const float z_hit =
            iCylinder(ray_origin, ray_direction,
                      _gizmo_position + float3{0.0f, 0.0f, 1.0f} * height,
                      _gizmo_position - float3{0.0f, 0.0f, 1.0f} * height, radius)
               .x;

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
      const float size = _translate_gizmo_size;

      const bool x_hover = _translate.active_axis == axis::x;
      const bool y_hover = _translate.active_axis == axis::y;
      const bool z_hover = _translate.active_axis == axis::z;

      if (_translate.translating) {
         switch (_translate.active_axis) {
         case axis::x: {
            tool_visualizers.lines.emplace_back(_translate.start_position -
                                                   float3{1024.0f, 0.0f, 0.0f},
                                                _translate.start_position +
                                                   float3{1024.0f, 0.0f, 0.0f},
                                                x_color);
         } break;
         case axis::y: {
            tool_visualizers.lines.emplace_back(_translate.start_position -
                                                   float3{0.0f, 1024.0f, 0.0f},
                                                _translate.start_position +
                                                   float3{0.0f, 1024.0f, 0.0f},
                                                y_color);
         } break;
         case axis::z: {
            tool_visualizers.lines.emplace_back(_translate.start_position -
                                                   float3{0.0f, 0.0f, 1024.0f},
                                                _translate.start_position +
                                                   float3{0.0f, 0.0f, 1024.0f},
                                                z_color);
         } break;
         }
      }

      tool_visualizers.lines.emplace_back(_gizmo_position,
                                          _gizmo_position + float3{size, 0.0f, 0.0f},
                                          x_hover ? x_color_hover : x_color);
      tool_visualizers.lines.emplace_back(_gizmo_position,
                                          _gizmo_position + float3{0.0f, size, 0.0f},
                                          y_hover ? y_color_hover : y_color);
      tool_visualizers.lines.emplace_back(_gizmo_position,
                                          _gizmo_position + float3{0.0f, 0.0f, size},
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

bool gizmo::show_translate(const float3 gizmo_position, float3& movement) noexcept
{
   _gizmo_position = gizmo_position;
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

auto gizmo::get_translate_position(const graphics::camera_ray ray,
                                   const float3 fallback) const noexcept -> float3
{
   const float4 x_plane =
      make_plane(_translate.start_position, float3{1.0f, 0.0f, 0.0f});
   const float4 y_plane =
      make_plane(_translate.start_position, float3{0.0f, 1.0f, 0.0f});
   const float4 z_plane =
      make_plane(_translate.start_position, float3{0.0f, 0.0f, 1.0f});

   float distance = std::numeric_limits<float>::max();

   if (_translate.active_axis == axis::x) {
      if (const float y_plane_hit = plaIntersect(ray.origin, ray.direction, y_plane);
          y_plane_hit > 0.0f) {
         distance = std::min(y_plane_hit, distance);
      }

      if (distance == std::numeric_limits<float>::max()) {
         if (const float z_plane_hit = plaIntersect(ray.origin, ray.direction, z_plane);
             z_plane_hit > 0.0f) {
            distance = std::min(z_plane_hit, distance);
         }
      }
   }
   else if (_translate.active_axis == axis::y) {
      if (const float x_plane_hit = plaIntersect(ray.origin, ray.direction, x_plane);
          x_plane_hit > 0.0f) {
         distance = std::min(x_plane_hit, distance);
      }

      if (const float z_plane_hit = plaIntersect(ray.origin, ray.direction, z_plane);
          z_plane_hit > 0.0f) {
         distance = std::min(z_plane_hit, distance);
      }
   }
   else {
      if (const float y_plane_hit = plaIntersect(ray.origin, ray.direction, y_plane);
          y_plane_hit > 0.0f) {
         distance = std::min(y_plane_hit, distance);
      }

      if (distance == std::numeric_limits<float>::max()) {
         if (const float x_plane_hit = plaIntersect(ray.origin, ray.direction, x_plane);
             x_plane_hit > 0.0f) {
            distance = std::min(x_plane_hit, distance);
         }
      }
   }

   if (distance != std::numeric_limits<float>::max()) {
      return ray.origin + ray.direction * distance;
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