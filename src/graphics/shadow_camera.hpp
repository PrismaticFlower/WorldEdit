#pragma once

#include "types.hpp"

namespace we::graphics {

class shadow_ortho_camera {
public:
   auto world_matrix() const noexcept -> const float4x4&;

   auto view_matrix() const noexcept -> const float4x4&;

   auto projection_matrix() const noexcept -> const float4x4&;

   auto view_projection_matrix() const noexcept -> const float4x4&;

   auto inv_view_projection_matrix() const noexcept -> const float4x4&;

   auto near_clip() const noexcept;

   auto far_clip() const noexcept;

   shadow_ortho_camera() noexcept;

   void look_at(const float3 eye, const float3 at, const float3 up) noexcept;

   void set_projection(const float min_x, const float min_y, const float max_x,
                       const float max_y, const float min_z, const float max_z);

   void set_stabilization(const float2 stabilization);

   auto texture_matrix() const noexcept -> const float4x4&;

private:
   void update() noexcept;

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