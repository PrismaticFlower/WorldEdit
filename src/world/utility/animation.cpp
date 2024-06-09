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

auto evaluate_animation(const animation& animation, const quaternion& base_rotation,
                        const float3& base_position, float t) noexcept -> float4x4
{
   if (animation.loop and t > animation.runtime) {
      t = std::fmod(t, animation.runtime);
   }

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
      float3 local_position;

      if ((position_index + 1) < std::ssize(animation.position_keys)) {
         const position_key& a = animation.position_keys[position_index];
         const position_key& b = animation.position_keys[position_index + 1];

         const float local_t = (t - a.time) / (b.time - a.time);

         switch (a.transition) {
         case world::animation_transition::pop: {
            local_position = a.position;
         } break;
         case world::animation_transition::linear: {
            local_position = (1.0f - local_t) * a.position + local_t * b.position;
         } break;
         case world::animation_transition::spline: {
            local_position = hermite_interpolate(a.position, a.tangent, b.position,
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
            local_position = a.position;
         } break;
         case world::animation_transition::linear: {
            local_position = (1.0f - local_t) * a.position + local_t * b.position;
         } break;
         case world::animation_transition::spline: {
            local_position = hermite_interpolate(a.position, a.tangent, b.position,
                                                 a.tangent_next, local_t);
         } break;
         }
      }
      else {
         local_position = animation.position_keys[position_index].position;
      }

      position += base_rotation * local_position;
   }

   float4x4 transform = to_matrix(rotation);

   transform[3] = float4{position, 1.0f};

   return transform;
}

auto evaluate_animation_local_translation(
   const animation& animation, const std::vector<float>& timepoints,
   const std::vector<animation_solver::subkey>& subkeys, float t) noexcept -> float4x4
{
   if (animation.loop and t > animation.runtime) {
      t = std::fmod(t, animation.runtime);
   }

   std::ptrdiff_t subkey_index = std::ssize(timepoints) - 1;

   if (auto it = std::lower_bound(timepoints.begin(), timepoints.end(), t);
       it != timepoints.end()) {
      subkey_index = std::distance(timepoints.begin(), it);
   }

   if (subkey_index < 0) return subkeys.back().transform;

   const animation_solver::subkey& subkey = subkeys[subkey_index];

   float4x4 transform = subkey.transform;
   const float3 last_rotation = subkey.rotation;
   const float3 last_position = subkey.position;

   float3 rotation = last_rotation;

   std::ptrdiff_t rotation_index = -1;

   for (std::ptrdiff_t i = 0; i < std::ssize(animation.rotation_keys); ++i) {
      if (t >= animation.rotation_keys[i].time) rotation_index = i;
   }

   if (rotation_index >= 0) {
      if ((rotation_index + 1) < std::ssize(animation.rotation_keys)) {
         const rotation_key& a = animation.rotation_keys[rotation_index];
         const rotation_key& b = animation.rotation_keys[rotation_index + 1];

         const float local_t = (t - a.time) / (b.time - a.time);

         switch (a.transition) {
         case world::animation_transition::pop: {
            rotation = a.rotation;
         } break;
         case world::animation_transition::linear: {
            rotation = (1.0f - local_t) * a.rotation + local_t * b.rotation;
         } break;
         case world::animation_transition::spline: {
            rotation = hermite_interpolate(a.rotation, a.tangent, b.rotation,
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
            rotation = a.rotation;
         } break;
         case world::animation_transition::linear: {
            rotation = (1.0f - local_t) * a.rotation + local_t * b.rotation;
         } break;
         case world::animation_transition::spline: {
            rotation = hermite_interpolate(a.rotation, a.tangent, b.rotation,
                                           a.tangent_next, local_t);
         } break;
         }
      }
      else {
         rotation = animation.rotation_keys[rotation_index].rotation;
      }
   }

   float3 position = last_position;

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
            position = a.position;
         } break;
         case world::animation_transition::linear: {
            position = (1.0f - local_t) * a.position + local_t * b.position;
         } break;
         case world::animation_transition::spline: {
            position = hermite_interpolate(a.position, a.tangent, b.position,
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
            position = a.position;
         } break;
         case world::animation_transition::linear: {
            position = (1.0f - local_t) * a.position + local_t * b.position;
         } break;
         case world::animation_transition::spline: {
            position = hermite_interpolate(a.position, a.tangent, b.position,
                                           a.tangent_next, local_t);
         } break;
         }
      }
      else {
         position = animation.position_keys[position_index].position;
      }
   }

   float3 delta_rotation = rotation - last_rotation;
   float3 delta_position = position - last_position;

   constexpr float degrees_to_radians = std::numbers::pi_v<float> / 180.0f;

   float4x4 delta_transform =
      make_rotation_matrix_from_euler(delta_rotation * degrees_to_radians);

   delta_transform[3] = float4{delta_position, 1.0f};

   transform = transform * delta_transform;

   return transform;
}

void build_local_translation_transforms(const animation& animation,
                                        const quaternion& base_rotation,
                                        const float3& base_position,
                                        std::vector<float>& timepoints,
                                        std::vector<animation_solver::subkey>& subkeys)
{
   timepoints.clear();
   subkeys.clear();

   float4x4 transform = to_matrix(base_rotation);
   transform[3] = {base_position, 1.0f};

   float3 last_rotation;
   float3 last_position;

   for (uint32 end_index = 1; end_index < animation.rotation_keys.size(); ++end_index) {
      const rotation_key& start = animation.rotation_keys[end_index - 1];
      const rotation_key& end = animation.rotation_keys[end_index];

      const float3 start_end_delta = abs(start.rotation - end.rotation);
      const float delta_max =
         std::max(std::max(start_end_delta.x, start_end_delta.y),
                  start_end_delta.z);

      const uint32 steps = static_cast<uint32>(delta_max * 4.0f + 0.5f);
      const float inv_steps = 1.0f / static_cast<float>(steps);

      for (uint32 step_index = 0; step_index < steps; ++step_index) {
         const float step_norm = step_index * inv_steps;
         const float t = (1.0f - step_norm) * start.time + step_norm * end.time;

         float3 rotation = last_rotation;

         switch (start.transition) {
         case world::animation_transition::pop: {
            rotation = start.rotation;
         } break;
         case world::animation_transition::linear: {
            rotation = (1.0f - step_norm) * start.rotation + step_norm * end.rotation;
         } break;
         case world::animation_transition::spline: {
            rotation = hermite_interpolate(start.rotation, start.tangent, end.rotation,
                                           start.tangent_next, step_norm);
         } break;
         }

         float3 position = last_position;

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
                  position = a.position;
               } break;
               case world::animation_transition::linear: {
                  position = (1.0f - local_t) * a.position + local_t * b.position;
               } break;
               case world::animation_transition::spline: {
                  position = hermite_interpolate(a.position, a.tangent, b.position,
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
                  position = a.position;
               } break;
               case world::animation_transition::linear: {
                  position = (1.0f - local_t) * a.position + local_t * b.position;
               } break;
               case world::animation_transition::spline: {
                  position = hermite_interpolate(a.position, a.tangent, b.position,
                                                 a.tangent_next, local_t);
               } break;
               }
            }
            else {
               position = animation.position_keys[position_index].position;
            }
         }

         float3 delta_rotation = rotation - last_rotation;
         float3 delta_position = position - last_position;

         constexpr float degrees_to_radians = std::numbers::pi_v<float> / 180.0f;

         float4x4 delta_transform =
            make_rotation_matrix_from_euler(delta_rotation * degrees_to_radians);

         delta_transform[3] = float4{delta_position, 1.0f};

         transform = transform * delta_transform;

         last_rotation = rotation;
         last_position = position;

         timepoints.emplace_back(t);
         subkeys.emplace_back(last_rotation, last_position, transform);
      }
   }

   if (animation.loop and not animation.rotation_keys.empty()) {
      const rotation_key& start = animation.rotation_keys.back();
      rotation_key end = animation.rotation_keys.front();

      end.time = animation.runtime;

      const float3 start_end_delta = abs(start.rotation - end.rotation);
      const float delta_max =
         std::max(std::max(start_end_delta.x, start_end_delta.y),
                  start_end_delta.z);

      const uint32 steps = static_cast<uint32>(delta_max * 4.0f + 0.5f);
      const float inv_steps = 1.0f / static_cast<float>(steps);

      for (uint32 step_index = 0; step_index < steps; ++step_index) {
         const float step_norm = step_index * inv_steps;
         const float t = (1.0f - step_norm) * start.time + step_norm * end.time;

         float3 rotation = last_rotation;

         switch (start.transition) {
         case world::animation_transition::pop: {
            rotation = start.rotation;
         } break;
         case world::animation_transition::linear: {
            rotation = (1.0f - step_norm) * start.rotation + step_norm * end.rotation;
         } break;
         case world::animation_transition::spline: {
            rotation = hermite_interpolate(start.rotation, start.tangent, end.rotation,
                                           start.tangent_next, step_norm);
         } break;
         }

         float3 position = last_position;

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
                  position = a.position;
               } break;
               case world::animation_transition::linear: {
                  position = (1.0f - local_t) * a.position + local_t * b.position;
               } break;
               case world::animation_transition::spline: {
                  position = hermite_interpolate(a.position, a.tangent, b.position,
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
                  position = a.position;
               } break;
               case world::animation_transition::linear: {
                  position = (1.0f - local_t) * a.position + local_t * b.position;
               } break;
               case world::animation_transition::spline: {
                  position = hermite_interpolate(a.position, a.tangent, b.position,
                                                 a.tangent_next, local_t);
               } break;
               }
            }
            else {
               position = animation.position_keys[position_index].position;
            }
         }

         float3 delta_rotation = rotation - last_rotation;
         float3 delta_position = position - last_position;

         constexpr float degrees_to_radians = std::numbers::pi_v<float> / 180.0f;

         float4x4 delta_transform =
            make_rotation_matrix_from_euler(delta_rotation * degrees_to_radians);

         delta_transform[3] = float4{delta_position, 1.0f};

         transform = transform * delta_transform;

         last_rotation = rotation;
         last_position = position;

         timepoints.emplace_back(t);
         subkeys.emplace_back(last_rotation, last_position, transform);
      }
   }

   if (timepoints.empty()) {
      timepoints.emplace_back(0.0f);
      subkeys.emplace_back(float3{}, float3{}, transform);
   }
}
}

void animation_solver::init(const animation& animation, const quaternion& base_rotation,
                            const float3& base_position) noexcept
{
   if (animation.local_translation) {
      build_local_translation_transforms(animation, base_rotation,
                                         base_position, _timepoints, _subkeys);
   }
   else {
      _base_rotation = base_rotation;
      _base_position = base_position;
   }
}

auto animation_solver::evaluate(const animation& animation, float t) const noexcept
   -> float4x4
{
   if (animation.local_translation) {
      return evaluate_animation_local_translation(animation, _timepoints, _subkeys, t);
   }
   else {
      return evaluate_animation(animation, _base_rotation, _base_position, t);
   }
}

auto make_position_key_for_time(const animation& animation, float t) noexcept -> position_key
{
   position_key new_key = {.time = t};

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
         case world::animation_transition::pop:
         case world::animation_transition::linear: {
            new_key.position = (1.0f - local_t) * a.position + local_t * b.position;
         } break;
         case world::animation_transition::spline: {
            new_key.position = hermite_interpolate(a.position, a.tangent, b.position,
                                                   a.tangent_next, local_t);
         } break;
         }

         new_key.transition = a.transition;
         new_key.tangent = (1.0f - local_t) * a.tangent + local_t * b.tangent;
         new_key.tangent_next = b.tangent;
      }
      else if ((position_index + 1) == std::ssize(animation.position_keys) and
               animation.loop) {
         const position_key& a = animation.position_keys[position_index];
         position_key b = animation.position_keys[0];

         b.time = animation.runtime;

         const float local_t = (t - a.time) / (b.time - a.time);

         switch (a.transition) {
         case world::animation_transition::pop:
         case world::animation_transition::linear: {
            new_key.position = (1.0f - local_t) * a.position + local_t * b.position;
         } break;
         case world::animation_transition::spline: {
            new_key.position = hermite_interpolate(a.position, a.tangent, b.position,
                                                   a.tangent_next, local_t);
         } break;
         }

         new_key.transition = a.transition;
         new_key.tangent = (1.0f - local_t) * a.tangent + local_t * b.tangent;
         new_key.tangent_next = b.tangent;
      }
      else if (position_index - 1 >= 0) {
         const position_key& a = animation.position_keys[position_index - 1];
         const position_key& b = animation.position_keys[position_index];

         const float local_t = (t - a.time) / (b.time - a.time);

         new_key.position = (1.0f - local_t) * a.position + local_t * b.position;
         new_key.transition = a.transition;
         new_key.tangent = (1.0f - local_t) * a.tangent + local_t * b.tangent;
         new_key.tangent_next = b.tangent;
      }
      else {
         new_key = animation.position_keys[position_index];
         new_key.time = t;
      }
   }

   return new_key;
}

auto make_rotation_key_for_time(const animation& animation, float t) noexcept -> rotation_key
{
   rotation_key new_key = {.time = t};

   std::ptrdiff_t rotation_index = -1;

   for (std::ptrdiff_t i = 0; i < std::ssize(animation.rotation_keys); ++i) {
      if (t >= animation.rotation_keys[i].time) rotation_index = i;
   }

   if (rotation_index >= 0) {
      if ((rotation_index + 1) < std::ssize(animation.rotation_keys)) {
         const rotation_key& a = animation.rotation_keys[rotation_index];
         const rotation_key& b = animation.rotation_keys[rotation_index + 1];

         const float local_t = (t - a.time) / (b.time - a.time);

         switch (a.transition) {
         case world::animation_transition::pop:
         case world::animation_transition::linear: {
            new_key.rotation = (1.0f - local_t) * a.rotation + local_t * b.rotation;
         } break;
         case world::animation_transition::spline: {
            new_key.rotation = hermite_interpolate(a.rotation, a.tangent, b.rotation,
                                                   a.tangent_next, local_t);
         } break;
         }

         new_key.transition = a.transition;
         new_key.tangent = (1.0f - local_t) * a.tangent + local_t * b.tangent;
         new_key.tangent_next = b.tangent;
      }
      else if ((rotation_index + 1) == std::ssize(animation.rotation_keys) and
               animation.loop) {
         const rotation_key& a = animation.rotation_keys[rotation_index];
         rotation_key b = animation.rotation_keys[0];

         b.time = animation.runtime;

         const float local_t = (t - a.time) / (b.time - a.time);

         switch (a.transition) {
         case world::animation_transition::pop:
         case world::animation_transition::linear: {
            new_key.rotation = (1.0f - local_t) * a.rotation + local_t * b.rotation;
         } break;
         case world::animation_transition::spline: {
            new_key.rotation = hermite_interpolate(a.rotation, a.tangent, b.rotation,
                                                   a.tangent_next, local_t);
         } break;
         }

         new_key.transition = a.transition;
         new_key.tangent = (1.0f - local_t) * a.tangent + local_t * b.tangent;
         new_key.tangent_next = b.tangent;
      }
      else if (rotation_index - 1 >= 0) {
         const rotation_key& a = animation.rotation_keys[rotation_index - 1];
         const rotation_key& b = animation.rotation_keys[rotation_index];

         const float local_t = (t - a.time) / (b.time - a.time);

         new_key.rotation = (1.0f - local_t) * a.rotation + local_t * b.rotation;
         new_key.transition = a.transition;
         new_key.tangent = (1.0f - local_t) * a.tangent + local_t * b.tangent;
         new_key.tangent_next = b.tangent;
      }
      else {
         new_key = animation.rotation_keys[rotation_index];
         new_key.time = t;
      }
   }

   return new_key;
}

}