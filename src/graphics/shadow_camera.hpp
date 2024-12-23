#pragma once

#include "types.hpp"

namespace we::graphics {

class shadow_ortho_camera {
public:
   auto world_from_view() const noexcept -> const float4x4&;

   auto view_from_world() const noexcept -> const float4x4&;

   auto projection_from_view() const noexcept -> const float4x4&;

   auto projection_from_world() const noexcept -> const float4x4&;

   auto world_from_projection() const noexcept -> const float4x4&;

   auto near_clip() const noexcept;

   auto far_clip() const noexcept;

   shadow_ortho_camera() noexcept;

   void look_at(const float3 eye, const float3 at, const float3 up) noexcept;

   void set_projection(const float min_x, const float min_y, const float max_x,
                       const float max_y, const float min_z, const float max_z);

   void set_stabilization(const float2 stabilization);

   auto texture_from_world() const noexcept -> const float4x4&;

private:
   void update() noexcept;

   float4x4 _world_from_view;
   float4x4 _view_from_world;
   float4x4 _projection_from_view;
   float4x4 _projection_from_world;
   float4x4 _inv_view_projection_matrix;

   float _near_clip = 1.0f;
   float _far_clip = 500.0f;

   float2 _stabilization = {0.0f, 0.0f};
   float4x4 _texture_from_world;
};

}