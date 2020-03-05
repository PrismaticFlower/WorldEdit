#pragma once

#include "camera.hpp"

#include <cmath>

namespace sk::graphics {

void camera::update() noexcept
{
   _world_matrix = float4x4{_rotation};
   _world_matrix[3] = {_position, 1.0f};

   quaternion rotation_inverse = glm::conjugate(_rotation);
   _view_matrix = float4x4{rotation_inverse};
   _view_matrix[3] = {rotation_inverse * -_position, 1.0f};

   _projection_matrix = {1.0f, 0.0f, 0.0f, 0.0f, //
                         0.0f, 1.0f, 0.0f, 0.0f, //
                         0.0f, 0.0f, 1.0f, 0.0f, //
                         0.0f, 0.0f, 0.0f, 1.0f};

   const auto near_clip = 0.01f;
   const auto far_clip = 1000.0f;

   _projection_matrix[0][0] = 1.0f / std::tan(_fov * 0.5f);
   _projection_matrix[1][1] = _projection_matrix[0][0] * _aspect_ratio;

   _projection_matrix[2][2] = near_clip / (far_clip - near_clip);
   _projection_matrix[3][2] = far_clip * _projection_matrix[2][2];
   _projection_matrix[2][3] = -1.0f;

   _view_projection_matrix = _projection_matrix * _view_matrix;
}

}
