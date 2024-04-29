#include "animation.hpp"

#include "../animation.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <numbers>

namespace we::world {

namespace {

auto hermite_interpolate(const float3& p0, const float3& m0, const float3& p1,
                         const float3& m1, float t) noexcept -> float3
{
   const float t2 = t * t;
   const float t3 = t * t * t;

   const float3 h0 = (2.0f * t3 - 3.0f * t2 + 1.0f) * p0;
   const float3 h1 = (t3 - 2.0f * t2 + t) * m0;
   const float3 h2 = (-2.0f * t3 + 3.0f * t2) * p1;
   const float3 h3 = (t3 - t2) * m1;

   return h0 + h1 + h2 + h3;
}

}

auto evaluate_animation(const animation& animation, const quaternion& base_rotation,
                        const float3& base_position, float t) noexcept -> float4x4
{
   quaternion rotation = base_rotation;

   std::ptrdiff_t rotation_index = -1;

   for (std::ptrdiff_t i = 0; i < std::ssize(animation.rotation_keys); ++i) {
      if (t >= animation.rotation_keys[i].time) rotation_index = i;
   }

   if (rotation_index >= 0) {
      float3 euler_rotation = {0.0f, 0.0f, 0.0f};

      if ((rotation_index + 1) < std::ssize(animation.rotation_keys)) {
         const rotation_key& a = animation.rotation_keys[rotation_index];
         const rotation_key& b = animation.rotation_keys[rotation_index + 1];

         const float local_t = (t - a.time) / (b.time - a.time);

         switch (a.transition) {
         case world::animation_transition::pop: {
            euler_rotation = a.rotation;
         } break;
         case world::animation_transition::linear: {
            euler_rotation = (1.0f - local_t) * a.rotation + local_t * b.rotation;
         } break;
         case world::animation_transition::spline: {
            euler_rotation = hermite_interpolate(a.rotation, a.tangent, b.rotation,
                                                 a.tangent_next, local_t);
         } break;
         }
      }
      else if ((rotation_index + 1) == std::ssize(animation.rotation_keys) and
               animation.loop) {
         const rotation_key& a = animation.rotation_keys[rotation_index];
         rotation_key b = animation.rotation_keys[0];

         b.time = animation.runtime;

         const float local_t = (t - a.time) / (b.time - a.time);

         switch (a.transition) {
         case world::animation_transition::pop: {
            euler_rotation = a.rotation;
         } break;
         case world::animation_transition::linear: {
            euler_rotation = (1.0f - local_t) * a.rotation + local_t * b.rotation;
         } break;
         case world::animation_transition::spline: {
            euler_rotation = hermite_interpolate(a.rotation, a.tangent, b.rotation,
                                                 a.tangent_next, local_t);
         } break;
         }
      }
      else {
         euler_rotation = animation.rotation_keys[rotation_index].rotation;
      }

      constexpr float degrees_to_radians = std::numbers::pi_v<float> / 180.0f;

      euler_rotation *= degrees_to_radians;

      rotation = rotation * make_quat_from_euler({euler_rotation.x, 0.0f, 0.0f});
      rotation = rotation * make_quat_from_euler({0.0f, euler_rotation.y, 0.0f});
      rotation = rotation * make_quat_from_euler({0.0f, 0.0f, euler_rotation.z});
   }

   float3 position = base_position;

   std::ptrdiff_t position_index = -1;

   for (std::ptrdiff_t i = 0; i < std::ssize(animation.position_keys); ++i) {
      if (t >= animation.position_keys[i].time) position_index = i;
   }

   if (position_index >= 0) {
      if ((position_index + 1) < std::ssize(animation.position_keys)) {
         const position_key& a = animation.position_keys[position_index];
         const position_key& b = animation.position_keys[position_index + 1];

         const float local_t = (t - a.time) / (b.time - a.time);

         switch (a.transition) {
         case world::animation_transition::pop: {
            position += a.position;
         } break;
         case world::animation_transition::linear: {
            position += (1.0f - local_t) * a.position + local_t * b.position;
         } break;
         case world::animation_transition::spline: {
            position += hermite_interpolate(a.position, a.tangent, b.position,
                                            a.tangent_next, local_t);
         } break;
         }
      }
      else if ((position_index + 1) == std::ssize(animation.position_keys) and
               animation.loop) {
         const position_key& a = animation.position_keys[position_index];
         position_key b = animation.position_keys[0];

         b.time = animation.runtime;

         const float local_t = (t - a.time) / (b.time - a.time);

         switch (a.transition) {
         case world::animation_transition::pop: {
            position += a.position;
         } break;
         case world::animation_transition::linear: {
            position += (1.0f - local_t) * a.position + local_t * b.position;
         } break;
         case world::animation_transition::spline: {
            position += hermite_interpolate(a.position, a.tangent, b.position,
                                            a.tangent_next, local_t);
         } break;
         }
      }
      else {
         position += animation.position_keys[position_index].position;
      }
   }

   float4x4 transform = to_matrix(rotation);

   transform[3] = float4{position, 1.0f};

   return transform;
}

}