#pragma once

#include "types.hpp"

namespace we::graphics {

struct camera_ray {
   float3 origin;
   float3 direction;
};

enum class camera_projection { perspective, orthographic };

class camera {
public:
   auto right() const noexcept -> float3;

   auto left() const noexcept -> float3;

   auto up() const noexcept -> float3;

   auto down() const noexcept -> float3;

   auto forward() const noexcept -> float3;

   auto back() const noexcept -> float3;

   auto world_matrix() const noexcept -> const float4x4&;

   auto view_matrix() const noexcept -> const float4x4&;

   auto projection_matrix() const noexcept -> const float4x4&;

   auto view_projection_matrix() const noexcept -> const float4x4&;

   auto inv_view_projection_matrix() const noexcept -> const float4x4&;

   auto position() const noexcept -> float3;

   void position(const float3 new_position) noexcept;

   auto aspect_ratio() const noexcept -> float;

   void aspect_ratio(const float new_ratio) noexcept;

   auto fov() const noexcept -> float;

   void fov(const float new_fov) noexcept;

   void near_clip(const float new_near_clip) noexcept;

   auto near_clip() const noexcept -> float;

   void far_clip(const float new_far_clip) noexcept;

   auto far_clip() const noexcept -> float;

   auto pitch() -> float;

   void pitch(const float new_pitch);

   auto yaw() -> float;

   void yaw(const float new_yaw);

   auto view_width() const noexcept -> float;

   void view_width(const float new_view_width) noexcept;

   auto zoom() const noexcept -> float;

   void zoom(const float new_zoom) noexcept;

   auto projection() const noexcept -> camera_projection;

   void projection(const camera_projection new_projection) noexcept;

private:
   void update() noexcept;

   float4x4 _world_matrix;
   float4x4 _view_matrix;
   float4x4 _projection_matrix;
   float4x4 _view_projection_matrix;
   float4x4 _inv_view_projection_matrix;

   float _near_clip = 1.0f;
   float _far_clip = 20000.0f;

   float3 _position{0.0f, 0.0f, 0.0f};

   float _aspect_ratio = 1.0f;
   float _fov = 1.2217305f;
   float _zoom = 1.0f;

   float _pitch = 0.0f;
   float _yaw = 0.0f;

   float _view_width = 256.0f;

   camera_projection _projection = camera_projection::perspective;
};

auto unproject_depth_value(const camera& camera, const float depth) noexcept -> float;

auto make_camera_ray(const camera& camera, const float2 cursor_position,
                     const float2 window_size) noexcept -> camera_ray;

}
