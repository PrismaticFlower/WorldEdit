
#include "shadow_camera.hpp"
#include "math/matrix_funcs.hpp"

namespace we::graphics {

auto shadow_ortho_camera::world_matrix() const noexcept -> const float4x4&
{
   return _world_matrix;
}

auto shadow_ortho_camera::view_matrix() const noexcept -> const float4x4&
{
   return _view_matrix;
}

auto shadow_ortho_camera::projection_matrix() const noexcept -> const float4x4&
{
   return _projection_matrix;
}

auto shadow_ortho_camera::view_projection_matrix() const noexcept -> const float4x4&
{
   return _view_projection_matrix;
}

auto shadow_ortho_camera::inv_view_projection_matrix() const noexcept -> const float4x4&
{
   return _inv_view_projection_matrix;
}

auto shadow_ortho_camera::near_clip() const noexcept
{
   return _near_clip;
}

auto shadow_ortho_camera::far_clip() const noexcept
{
   return _far_clip;
}

shadow_ortho_camera::shadow_ortho_camera() noexcept
{
   update();
}

void shadow_ortho_camera::look_at(const float3 eye, const float3 at, const float3 up) noexcept
{
   _view_matrix = look_at_lh(eye, at, up);
   _world_matrix = inverse(_view_matrix);

   update();
}

void shadow_ortho_camera::set_projection(const float min_x, const float min_y,
                                         const float max_x, const float max_y,
                                         const float min_z, const float max_z)
{
   _projection_matrix = {{1.0f, 0.0f, 0.0f, 0.0f}, //
                         {0.0f, 1.0f, 0.0f, 0.0f}, //
                         {0.0f, 0.0f, 1.0f, 0.0f}, //
                         {0.0f, 0.0f, 0.0f, 1.0f}};

   _near_clip = min_z;
   _far_clip = max_z;

   const auto inv_x_range = 1.0f / (min_x - max_x);
   const auto inv_y_range = 1.0f / (max_y - min_y);
   const auto inv_z_range = 1.0f / (max_z - min_z);

   _projection_matrix[0].x = inv_x_range + inv_x_range;
   _projection_matrix[1].y = inv_y_range + inv_y_range;
   _projection_matrix[2].z = inv_z_range;

   _projection_matrix[3].x = -(max_x + min_x) * inv_x_range;
   _projection_matrix[3].y = -(max_y + min_y) * inv_y_range;
   _projection_matrix[3].z = -min_z * inv_z_range;

   update();
}

void shadow_ortho_camera::set_stabilization(const float2 stabilization)
{
   _stabilization = stabilization;

   update();
}

auto shadow_ortho_camera::texture_matrix() const noexcept -> const float4x4&
{
   return _texture_matrix;
}

void shadow_ortho_camera::update() noexcept
{
   _view_projection_matrix = _projection_matrix * _view_matrix;

   _view_projection_matrix[3].x += _stabilization.x;
   _view_projection_matrix[3].y += _stabilization.y;

   _inv_view_projection_matrix = inverse(_view_projection_matrix);

   constexpr float4x4 bias_matrix{{0.5f, 0.0f, 0.0f, 0.0f},  //
                                  {0.0f, -0.5f, 0.0f, 0.0f}, //
                                  {0.0f, 0.0f, 1.0f, 0.0f},  //
                                  {0.5f, 0.5f, 0.0f, 1.0f}};

   _texture_matrix = bias_matrix * _view_projection_matrix;
}

}