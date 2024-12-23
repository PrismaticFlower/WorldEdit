
#include "shadow_camera.hpp"
#include "math/matrix_funcs.hpp"

namespace we::graphics {

auto shadow_ortho_camera::world_from_view() const noexcept -> const float4x4&
{
   return _world_from_view;
}

auto shadow_ortho_camera::view_from_world() const noexcept -> const float4x4&
{
   return _view_from_world;
}

auto shadow_ortho_camera::projection_from_view() const noexcept -> const float4x4&
{
   return _projection_from_view;
}

auto shadow_ortho_camera::projection_from_world() const noexcept -> const float4x4&
{
   return _projection_from_world;
}

auto shadow_ortho_camera::world_from_projection() const noexcept -> const float4x4&
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
   _view_from_world = look_at_lh(eye, at, up);
   _world_from_view = inverse(_view_from_world);

   update();
}

void shadow_ortho_camera::set_projection(const float min_x, const float min_y,
                                         const float max_x, const float max_y,
                                         const float min_z, const float max_z)
{
   _projection_from_view = {{1.0f, 0.0f, 0.0f, 0.0f}, //
                            {0.0f, 1.0f, 0.0f, 0.0f}, //
                            {0.0f, 0.0f, 1.0f, 0.0f}, //
                            {0.0f, 0.0f, 0.0f, 1.0f}};

   _near_clip = min_z;
   _far_clip = max_z;

   const auto inv_x_range = 1.0f / (min_x - max_x);
   const auto inv_y_range = 1.0f / (max_y - min_y);
   const auto inv_z_range = 1.0f / (max_z - min_z);

   _projection_from_view[0].x = inv_x_range + inv_x_range;
   _projection_from_view[1].y = inv_y_range + inv_y_range;
   _projection_from_view[2].z = inv_z_range;

   _projection_from_view[3].x = -(max_x + min_x) * inv_x_range;
   _projection_from_view[3].y = -(max_y + min_y) * inv_y_range;
   _projection_from_view[3].z = -min_z * inv_z_range;

   update();
}

void shadow_ortho_camera::set_stabilization(const float2 stabilization)
{
   _stabilization = stabilization;

   update();
}

auto shadow_ortho_camera::texture_from_world() const noexcept -> const float4x4&
{
   return _texture_from_world;
}

void shadow_ortho_camera::update() noexcept
{
   _projection_from_world = _projection_from_view * _view_from_world;

   _projection_from_world[3].x += _stabilization.x;
   _projection_from_world[3].y += _stabilization.y;

   _inv_view_projection_matrix = inverse(_projection_from_world);

   constexpr float4x4 bias_matrix{{0.5f, 0.0f, 0.0f, 0.0f},  //
                                  {0.0f, -0.5f, 0.0f, 0.0f}, //
                                  {0.0f, 0.0f, 1.0f, 0.0f},  //
                                  {0.5f, 0.5f, 0.0f, 1.0f}};

   _texture_from_world = bias_matrix * _projection_from_world;
}

}