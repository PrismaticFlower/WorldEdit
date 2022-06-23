#pragma once

#include "types.hpp"

namespace we::graphics {
class shadow_ortho_camera {
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

   shadow_ortho_camera() noexcept
   {
      update();
   }

   void look_at(const float3 eye, const float3 at, const float3 up) noexcept
   {
      _view_matrix = glm::lookAtLH(eye, at, up);
      _world_matrix = glm::inverse(_view_matrix);

      update();
   }

   void set_projection(const float min_x, const float min_y, const float max_x,
                       const float max_y, const float min_z, const float max_z)
   {
      _projection_matrix = {1.0f, 0.0f, 0.0f, 0.0f, //
                            0.0f, 1.0f, 0.0f, 0.0f, //
                            0.0f, 0.0f, 1.0f, 0.0f, //
                            0.0f, 0.0f, 0.0f, 1.0f};

      _near_clip = min_z;
      _far_clip = max_z;

      const auto inv_x_range = 1.0f / (min_x - max_x);
      const auto inv_y_range = 1.0f / (max_y - min_y);
      const auto inv_z_range = 1.0f / (max_z - min_z);

      _projection_matrix[0][0] = inv_x_range + inv_x_range;
      _projection_matrix[1][1] = inv_y_range + inv_y_range;
      _projection_matrix[2][2] = inv_z_range;

      _projection_matrix[3][0] = -(max_x + min_x) * inv_x_range;
      _projection_matrix[3][1] = -(max_y + min_y) * inv_y_range;
      _projection_matrix[3][2] = -min_z * inv_z_range;

      update();
   }

   void set_stabilization(const float2 stabilization)
   {
      _stabilization = stabilization;

      update();
   }

   auto texture_matrix() const noexcept -> const float4x4&
   {
      return _texture_matrix;
   }

private:
   void update() noexcept
   {
      _view_projection_matrix = _projection_matrix * _view_matrix;

      _view_projection_matrix[3].x += _stabilization.x;
      _view_projection_matrix[3].y += _stabilization.y;

      _inv_view_projection_matrix = glm::inverse(double4x4{_view_projection_matrix});

      constexpr float4x4 bias_matrix{0.5f, 0.0f,  0.0f, 0.0f, //
                                     0.0f, -0.5f, 0.0f, 0.0f, //
                                     0.0f, 0.0f,  1.0f, 0.0f, //
                                     0.5f, 0.5f,  0.0f, 1.0f};

      _texture_matrix = bias_matrix * _view_projection_matrix;
   }

   float4x4 _world_matrix;
   float4x4 _view_matrix;
   float4x4 _projection_matrix;
   float4x4 _view_projection_matrix;
   float4x4 _inv_view_projection_matrix;

   float _near_clip = 1.0f;
   float _far_clip = 500.0f;

   float2 _stabilization = {0.0f, 0.0f};
   float4x4 _texture_matrix;
};

}