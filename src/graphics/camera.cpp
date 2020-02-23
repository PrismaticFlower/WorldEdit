#pragma once

#include "camera.hpp"

#include <cmath>

namespace sk::graphics {

void camera::update() noexcept
{
   _projection_matrix = {1.0f, 0.0f, 0.0f, 0.0f, //
                         0.0f, 1.0f, 0.0f, 0.0f, //
                         0.0f, 0.0f, 1.0f, 0.0f, //
                         0.0f, 0.0f, 0.0f, 1.0f};

   const auto fov = 0.7853981635f;
   const auto aspect_ratio = 1.0f / 1.0f;
   const auto near_clip = 0.01f;
   const auto far_clip = 100.0f;

   _projection_matrix[0][0] = 1.0f / std::tan(fov * 0.5f);
   _projection_matrix[1][1] = _projection_matrix[0][0] * aspect_ratio;

   _projection_matrix[2][2] = near_clip / (far_clip - near_clip);
   _projection_matrix[3][2] = far_clip * _projection_matrix[2][2];
   _projection_matrix[2][3] = -1.0f;
}

}
