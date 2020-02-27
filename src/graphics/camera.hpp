#pragma once

#include "types.hpp"

#include <glm/gtc/matrix_access.hpp>

namespace sk::graphics {

class camera {
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

   auto right() const noexcept -> float3
   {
      return -_world_matrix[0];
   }

   auto left() const noexcept -> float3
   {
      return _world_matrix[0];
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

   auto view_matrix() const noexcept -> const matrix4x4&
   {
      return _view_matrix;
   }

   auto projection_matrix() const noexcept -> const matrix4x4&
   {
      return _projection_matrix;
   }

   auto view_projection_matrix() const noexcept -> const matrix4x4&
   {
      return _view_projection_matrix;
   }

protected:
   auto rotation() const noexcept -> quaternion
   {
      return _rotation;
   }

   void rotation(const quaternion new_rotation) noexcept
   {
      _rotation = new_rotation;

      update();
   }

private:
   void update() noexcept;

   float3 _position{0.0f, 0.0f, 0.0f};
   quaternion _rotation{1.0f, 0.0f, 0.0f, 0.0f};

   float _aspect_ratio = 1.0f;
   float _fov = 0.7853981635f;

   float _pitch = 0.0f;
   float _yaw = 0.0f;

   matrix4x4 _world_matrix;
   matrix4x4 _view_matrix;
   matrix4x4 _projection_matrix;
   matrix4x4 _view_projection_matrix;
};

}
