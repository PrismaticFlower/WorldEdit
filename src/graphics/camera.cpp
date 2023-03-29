#include "camera.hpp"

#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <cmath>

#include "imgui/imgui.h"

namespace we::graphics {

auto camera::right() const noexcept -> float3
{
   return float3{_world_matrix[0].x, _world_matrix[0].y, _world_matrix[0].z};
}

auto camera::left() const noexcept -> float3
{
   return -float3{_world_matrix[0].x, _world_matrix[0].y, _world_matrix[0].z};
}

auto camera::up() const noexcept -> float3
{
   return float3{_world_matrix[1].x, _world_matrix[1].y, _world_matrix[1].z};
}

auto camera::down() const noexcept -> float3
{
   return -float3{_world_matrix[1].x, _world_matrix[1].y, _world_matrix[1].z};
}

auto camera::forward() const noexcept -> float3
{
   return -float3{_world_matrix[2].x, _world_matrix[2].y, _world_matrix[2].z};
}

auto camera::back() const noexcept -> float3
{
   return float3{_world_matrix[2].x, _world_matrix[2].y, _world_matrix[2].z};
}

auto camera::world_matrix() const noexcept -> const float4x4&
{
   return _world_matrix;
}

auto camera::view_matrix() const noexcept -> const float4x4&
{
   return _view_matrix;
}

auto camera::projection_matrix() const noexcept -> const float4x4&
{
   return _projection_matrix;
}

auto camera::view_projection_matrix() const noexcept -> const float4x4&
{
   return _view_projection_matrix;
}

auto camera::inv_view_projection_matrix() const noexcept -> const float4x4&
{
   return _inv_view_projection_matrix;
}

auto camera::position() const noexcept -> float3
{
   return _position;
}

void camera::position(const float3 new_position) noexcept
{
   _position = new_position;

   update();
}

auto camera::aspect_ratio() const noexcept -> float
{
   return _aspect_ratio;
}

void camera::aspect_ratio(const float new_ratio) noexcept
{
   _aspect_ratio = new_ratio;

   update();
}

auto camera::fov() const noexcept -> float
{
   return _fov;
}

void camera::fov(const float new_fov) noexcept
{
   _fov = new_fov;

   update();
}

void camera::near_clip(const float new_near_clip) noexcept
{
   _near_clip = new_near_clip;

   update();
}

auto camera::near_clip() const noexcept -> float
{
   return _near_clip;
}

void camera::far_clip(const float new_far_clip) noexcept
{
   _far_clip = new_far_clip;

   update();
}

auto camera::far_clip() const noexcept -> float
{
   return _far_clip;
}

auto camera::pitch() -> float
{
   return _pitch;
}

void camera::pitch(const float new_pitch)
{
   _pitch = new_pitch;

   update();
}

auto camera::yaw() -> float
{
   return _yaw;
}

void camera::yaw(const float new_yaw)
{
   _yaw = new_yaw;

   update();
}

auto camera::view_width() const noexcept -> float
{
   return _view_width;
}

void camera::view_width(const float new_view_width) noexcept
{
   _view_width = new_view_width;

   update();
}

auto camera::zoom() const noexcept -> float
{
   return _zoom;
}

void camera::zoom(const float new_zoom) noexcept
{
   _zoom = new_zoom;

   update();
}

auto camera::projection() const noexcept -> camera_projection
{
   return _projection;
}

void camera::projection(const camera_projection new_projection) noexcept
{
   _projection = new_projection;

   update();
}

void camera::update() noexcept
{
   float pitch = 0.0f;
   float yaw = 0.0f;

   if (_projection == camera_projection::perspective) {
      pitch = _pitch;
      yaw = _yaw;
   }
   else {
      constexpr float rotation_step = 0.17453292f;

      pitch = std::floor(_pitch / rotation_step) * rotation_step;
      yaw = std::floor(_yaw / rotation_step) * rotation_step;
   }

   float4x4 rotation = make_rotation_matrix_from_euler({pitch, yaw, 0.0f});

   _world_matrix = rotation;
   _world_matrix[3] = {_position, 1.0f};

   float4x4 rotation_inverse = transpose(rotation);
   _view_matrix = rotation_inverse;
   _view_matrix[3] = {rotation_inverse * -_position, 1.0f};

   if (_projection == camera_projection::perspective) {
      _projection_matrix = {{0.0f, 0.0f, 0.0f, 0.0f}, //
                            {0.0f, 0.0f, 0.0f, 0.0f}, //
                            {0.0f, 0.0f, 0.0f, 0.0f}, //
                            {0.0f, 0.0f, 0.0f, 0.0f}};

      const float fov = _fov / _zoom;
      const float near_clip = _near_clip;
      const float far_clip = _far_clip;

      _projection_matrix[0].x = 1.0f / std::tan(fov * 0.5f);
      _projection_matrix[1].y = _projection_matrix[0].x * _aspect_ratio;

      _projection_matrix[2].z = far_clip / (near_clip - far_clip);
      _projection_matrix[3].z = near_clip * _projection_matrix[2].z;
      _projection_matrix[2].w = -1.0f;
   }
   else {
      _projection_matrix = {{1.0f, 0.0f, 0.0f, 0.0f}, //
                            {0.0f, 1.0f, 0.0f, 0.0f}, //
                            {0.0f, 0.0f, 1.0f, 0.0f}, //
                            {0.0f, 0.0f, 0.0f, 1.0f}};

      const float near_clip = -_far_clip;
      const float far_clip = _far_clip;

      const float view_width = _view_width / _zoom;
      const float view_height = view_width / _aspect_ratio;
      const float z_range = 1.0f / (near_clip - far_clip);

      _projection_matrix[0].x = 2.0f / view_width;
      _projection_matrix[1].y = 2.0f / view_height;
      _projection_matrix[2].z = z_range;
      _projection_matrix[3].z = z_range * near_clip;
   }

   _view_projection_matrix = _projection_matrix * _view_matrix;

   _inv_view_projection_matrix = inverse(_view_projection_matrix);
}

auto make_camera_ray(const camera& camera, const float2 cursor_position,
                     const float2 window_size) noexcept -> camera_ray
{
   const float2 ndc_pos =
      ((cursor_position + 0.5f) / window_size * 2.0f - 1.0f) * float2{1.0f, -1.0f};

   if (camera.projection() == camera_projection::perspective) {

      float4 corner = float4{ndc_pos.x, ndc_pos.y, 1.0f, 1.0f};

      corner = camera.inv_view_projection_matrix() * corner;
      corner /= corner.w;

      float3 ray_direction =
         normalize(float3{corner.x, corner.y, corner.z} - camera.position());

      return {.origin = camera.position(), .direction = ray_direction};
   }
   else {
      const float4 origin = camera.inv_view_projection_matrix() *
                            float4{ndc_pos.x, ndc_pos.y, 0.0f, 1.0f};

      return {.origin = float3{origin.x, origin.y, origin.z},
              .direction = transpose(camera.view_matrix()) * float3{0.0f, 0.0f, -1.0f}};
   }
}
}
