#pragma once

#include "types.hpp"

namespace sk::graphics {

class camera {
public:
   void update() noexcept;

   auto position() const noexcept -> float3
   {
      return _position;
   }

   void position(const float3 new_position) noexcept
   {
      _position = new_position;
   }

   auto rotation() const noexcept -> quaternion
   {
      return _rotation;
   }

   void rotation(const quaternion new_rotation) noexcept
   {
      _rotation = new_rotation;
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

private:
   float3 _position{0.0f, 0.0f, 0.0f};
   quaternion _rotation{1.0f, 0.0f, 0.0f, 0.0f};

   matrix4x4 _view_matrix;
   matrix4x4 _projection_matrix;
   matrix4x4 _view_projection_matrix;
};

}
