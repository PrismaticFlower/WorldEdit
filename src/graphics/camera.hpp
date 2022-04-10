#pragma once

#include "types.hpp"

#include <glm/gtc/matrix_access.hpp>

namespace we::graphics {

struct camera_ray {
   float3 origin;
   float3 direction;
};

class camera {
public:
   auto right() const noexcept -> float3
   {
      return _world_matrix[0];
   }

   auto left() const noexcept -> float3
   {
      return -_world_matrix[0];
   }

   auto up() const noexcept -> float3
   {
      return _world_matrix[1];
   }

   auto down() const noexcept -> float3
   {
      return -_world_matrix[1];
   }

   auto forward() const noexcept -> float3
   {
      return -_world_matrix[2];
   }

   auto back() const noexcept -> float3
   {
      return _world_matrix[2];
   }

   auto world_matrix() const noexcept -> const float4x4&
   {
      return _world_matrix;
   }

   auto view_matrix() const noexcept -> const float4x4&
   {
      return _view_matrix;
   }

   auto projection_matrix() const noexcept -> const float4x4&
   {
      return _projection_matrix;
   }

   auto view_projection_matrix() const noexcept -> const float4x4&
   {
      return _view_projection_matrix;
   }

   auto inv_view_projection_matrix() const noexcept -> const float4x4&
   {
      return _inv_view_projection_matrix;
   }

   auto near_clip() const noexcept
   {
      return _near_clip;
   }

   auto far_clip() const noexcept
   {
      return _far_clip;
   }

protected:
   float4x4 _world_matrix;
   float4x4 _view_matrix;
   float4x4 _projection_matrix;
   float4x4 _view_projection_matrix;
   float4x4 _inv_view_projection_matrix;

   float _near_clip = 1.0f;
   float _far_clip = 500.0f;
};

class perspective_camera : public camera {
public:
   auto position() const noexcept -> float3
   {
      return _position;
   }

   void position(const float3 new_position) noexcept
   {
      _position = new_position;

      update();
   }

   auto rotation() const noexcept -> quaternion
   {
      return _rotation;
   }

   void rotation(const quaternion new_rotation) noexcept
   {
      _rotation = new_rotation;

      update();
   }

   auto aspect_ratio() const noexcept -> float
   {
      return _aspect_ratio;
   }

   void aspect_ratio(const float new_ratio) noexcept
   {
      _aspect_ratio = new_ratio;

      update();
   }

   auto fov() const noexcept -> float
   {
      return _fov;
   }

   void fov(const float new_fov) noexcept
   {
      _fov = new_fov;

      update();
   }

   void near_clip(const float new_near_clip) noexcept
   {
      _near_clip = new_near_clip;

      update();
   }

   void far_clip(const float new_far_clip) noexcept
   {
      _far_clip = new_far_clip;

      update();
   }

private:
   void update() noexcept;

   float3 _position{0.0f, 0.0f, 0.0f};
   quaternion _rotation{1.0f, 0.0f, 0.0f, 0.0f};

   float _aspect_ratio = 1.0f;
   float _fov = 0.7853981635f;
};

class shadow_orthographic_camera : public camera {
public:
   void look_at(const float3 eye, const float3 at, const float3 up) noexcept
   {
      _view_matrix = glm::lookAtLH(eye, at, up);
      _world_matrix = glm::inverse(_view_matrix);

      update();
   }

   auto bounds(const float3 min, const float3 max) noexcept
   {
      _min = min;
      _max = max;

      update();
   }

   auto min() const noexcept -> float3
   {
      return _min;
   }

   void min(const float3 new_min) noexcept
   {
      _min = new_min;

      update();
   }

   auto max() const noexcept -> float3
   {
      return _max;
   }

   void max(const float3 new_max) noexcept
   {
      _max = new_max;

      update();
   }

private:
   void update() noexcept;

   float3 _position{0.0f, 0.0f, 0.0f};
   quaternion _rotation{1.0f, 0.0f, 0.0f, 0.0f};

   float3 _min = {-64.0f, -64.0f, 0.0f};
   float3 _max = {64.0f, 64.0f, 1000.f};
};

template<typename T>
class basic_controllable_camera : public T {
public:
   auto pitch() -> float
   {
      return _pitch;
   }

   void pitch(const float new_pitch)
   {
      _pitch = new_pitch;

      rotation(quaternion{float3{_pitch, _yaw, 0.0f}});
   }

   auto yaw() -> float
   {
      return _yaw;
   }

   void yaw(const float new_yaw)
   {
      _yaw = new_yaw;

      rotation(quaternion{float3{_pitch, _yaw, 0.0f}});
   }

private:
   using T::rotation;

   float _pitch = 0.0f;
   float _yaw = 0.0f;
};

using controllable_perspective_camera = basic_controllable_camera<perspective_camera>;

auto make_camera_ray(const controllable_perspective_camera& camera,
                     const float2 cursor_position,
                     const float2 window_size) noexcept -> camera_ray;

}
