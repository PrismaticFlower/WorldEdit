#include "animation.hpp"

#include "../animation.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include <numbers>

namespace we::world {

namespace {

auto linear_interpolate(const float3& a, const float a_time, const float3& b,
                        const float b_time, const float global_t) noexcept -> float3
{
   const float local_t = (global_t - a_time) / (b_time - a_time);

   return (1.0f - local_t) * a + local_t * b;
}

auto hermite_interpolate(const float3& p0, const float3& m0, float gloabl_t0,
                         const float3& p1, const float3& m1, float global_t1,
                         float global_t) noexcept -> float3
{
   const float t_delta = global_t1 - gloabl_t0;
   const float t = (global_t - gloabl_t0) / t_delta;
   const float t2 = t * t;

   const float h00 = (1.0f + 2.0f * t) * ((1.0f - t) * (1.0f - t));
   const float h10 = t * ((1.0f - t) * (1.0f - t));
   const float h01 = t2 * (3.0f - 2.0f * t);
   const float h11 = t2 * (t - 1.0f);

   return h00 * p0 + h10 * t_delta * m0 + h01 * p1 + h11 * t_delta * m1;
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

         switch (a.transition) {
         case world::animation_transition::pop: {
            euler_rotation = a.rotation;
         } break;
         case world::animation_transition::linear: {
            euler_rotation =
               linear_interpolate(a.rotation, a.time, b.rotation, b.time, t);
         } break;
         case world::animation_transition::spline: {
            euler_rotation =
               hermite_interpolate(a.rotation, a.tangent, a.time, b.rotation,
                                   a.tangent_next, b.time, t);
         } break;
         }
      }
      else if ((rotation_index + 1) == std::ssize(animation.rotation_keys) and
               animation.loop) {
         const rotation_key& a = animation.rotation_keys[rotation_index];
         rotation_key b = animation.rotation_keys[0];

         b.time = animation.runtime;

         switch (a.transition) {
         case world::animation_transition::pop: {
            euler_rotation = a.rotation;
         } break;
         case world::animation_transition::linear: {
            euler_rotation =
               linear_interpolate(a.rotation, a.time, b.rotation, b.time, t);
         } break;
         case world::animation_transition::spline: {
            euler_rotation =
               hermite_interpolate(a.rotation, a.tangent, a.time, b.rotation,
                                   a.tangent_next, b.time, t);
         } break;
         }
      }
      else {
         euler_rotation = animation.rotation_keys[rotation_index].rotation;
      }

      constexpr float degrees_to_radians = std::numbers::pi_v<float> / 180.0f;

      euler_rotation *= degrees_to_radians;

      rotation = base_rotation * make_quat_from_euler(euler_rotation);
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

         switch (a.transition) {
         case world::animation_transition::pop: {
            local_position = a.position;
         } break;
         case world::animation_transition::linear: {
            local_position =
               linear_interpolate(a.position, a.time, b.position, b.time, t);
         } break;
         case world::animation_transition::spline: {
            local_position =
               hermite_interpolate(a.position, a.tangent, a.time, b.position,
                                   a.tangent_next, b.time, t);
         } break;
         }
      }
      else if ((position_index + 1) == std::ssize(animation.position_keys) and
               animation.loop) {
         const position_key& a = animation.position_keys[position_index];
         position_key b = animation.position_keys[0];

         b.time = animation.runtime;

         switch (a.transition) {
         case world::animation_transition::pop: {
            local_position = a.position;
         } break;
         case world::animation_transition::linear: {
            local_position =
               linear_interpolate(a.position, a.time, b.position, b.time, t);
         } break;
         case world::animation_transition::spline: {
            local_position =
               hermite_interpolate(a.position, a.tangent, a.time, b.position,
                                   a.tangent_next, b.time, t);
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

         switch (a.transition) {
         case world::animation_transition::pop: {
            rotation = a.rotation;
         } break;
         case world::animation_transition::linear: {
            rotation = linear_interpolate(a.rotation, a.time, b.rotation, b.time, t);
         } break;
         case world::animation_transition::spline: {
            rotation = hermite_interpolate(a.rotation, a.tangent, a.time,
                                           b.rotation, a.tangent_next, b.time, t);
         } break;
         }
      }
      else if ((rotation_index + 1) == std::ssize(animation.rotation_keys) and
               animation.loop) {
         const rotation_key& a = animation.rotation_keys[rotation_index];
         rotation_key b = animation.rotation_keys[0];

         b.time = animation.runtime;

         switch (a.transition) {
         case world::animation_transition::pop: {
            rotation = a.rotation;
         } break;
         case world::animation_transition::linear: {
            rotation = linear_interpolate(a.rotation, a.time, b.rotation, b.time, t);
         } break;
         case world::animation_transition::spline: {
            rotation = hermite_interpolate(a.rotation, a.tangent, a.time,
                                           b.rotation, a.tangent_next, b.time, t);
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

         switch (a.transition) {
         case world::animation_transition::pop: {
            position = a.position;
         } break;
         case world::animation_transition::linear: {
            position = linear_interpolate(a.position, a.time, b.position, b.time, t);
         } break;
         case world::animation_transition::spline: {
            position = hermite_interpolate(a.position, a.tangent, a.time,
                                           b.position, a.tangent_next, b.time, t);
         } break;
         }
      }
      else if ((position_index + 1) == std::ssize(animation.position_keys) and
               animation.loop) {
         const position_key& a = animation.position_keys[position_index];
         position_key b = animation.position_keys[0];

         b.time = animation.runtime;

         switch (a.transition) {
         case world::animation_transition::pop: {
            position = a.position;
         } break;
         case world::animation_transition::linear: {
            position = linear_interpolate(a.position, a.time, b.position, b.time, t);
         } break;
         case world::animation_transition::spline: {
            position = hermite_interpolate(a.position, a.tangent, a.time,
                                           b.position, a.tangent_next, b.time, t);
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
      float delta_max = std::max(std::max(start_end_delta.x, start_end_delta.y),
                                 start_end_delta.z);

      if (start.transition == animation_transition::spline) {
         const float3 tangent_delta = abs(start.tangent - start.tangent_next);
         const float tangent_delta_max =
            std::max(std::max(tangent_delta.x, tangent_delta.y), tangent_delta.z);

         delta_max = std::max(delta_max, tangent_delta_max);
      }

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
            rotation = hermite_interpolate(start.rotation, start.tangent,
                                           start.time, end.rotation,
                                           start.tangent_next, end.time, t);
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

               switch (a.transition) {
               case world::animation_transition::pop: {
                  position = a.position;
               } break;
               case world::animation_transition::linear: {
                  position =
                     linear_interpolate(a.position, a.time, b.position, b.time, t);
               } break;
               case world::animation_transition::spline: {
                  position =
                     hermite_interpolate(a.position, a.tangent, a.time,
                                         b.position, a.tangent_next, b.time, t);
               } break;
               }
            }
            else if ((position_index + 1) == std::ssize(animation.position_keys) and
                     animation.loop) {
               const position_key& a = animation.position_keys[position_index];
               position_key b = animation.position_keys[0];

               b.time = animation.runtime;

               switch (a.transition) {
               case world::animation_transition::pop: {
                  position = a.position;
               } break;
               case world::animation_transition::linear: {
                  position =
                     linear_interpolate(a.position, a.time, b.position, b.time, t);
               } break;
               case world::animation_transition::spline: {
                  position =
                     hermite_interpolate(a.position, a.tangent, a.time,
                                         b.position, a.tangent_next, b.time, t);
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
            rotation = hermite_interpolate(start.rotation, start.tangent,
                                           start.time, end.rotation,
                                           start.tangent_next, end.time, t);
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

               switch (a.transition) {
               case world::animation_transition::pop: {
                  position = a.position;
               } break;
               case world::animation_transition::linear: {
                  position =
                     linear_interpolate(a.position, a.time, b.position, b.time, t);
               } break;
               case world::animation_transition::spline: {
                  position =
                     hermite_interpolate(a.position, a.tangent, a.time,
                                         b.position, a.tangent_next, b.time, t);
               } break;
               }
            }
            else if ((position_index + 1) == std::ssize(animation.position_keys) and
                     animation.loop) {
               const position_key& a = animation.position_keys[position_index];
               position_key b = animation.position_keys[0];

               b.time = animation.runtime;

               switch (a.transition) {
               case world::animation_transition::pop: {
                  position = a.position;
               } break;
               case world::animation_transition::linear: {
                  position =
                     linear_interpolate(a.position, a.time, b.position, b.time, t);
               } break;
               case world::animation_transition::spline: {
                  position =
                     hermite_interpolate(a.position, a.tangent, a.time,
                                         b.position, a.tangent_next, b.time, t);
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
            new_key.position =
               hermite_interpolate(a.position, a.tangent, a.time, b.position,
                                   a.tangent_next, b.time, t);
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
            new_key.position =
               hermite_interpolate(a.position, a.tangent, a.time, b.position,
                                   a.tangent_next, b.time, t);
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
            new_key.rotation =
               hermite_interpolate(a.rotation, a.tangent, a.time, b.rotation,
                                   a.tangent_next, b.time, t);
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
            new_key.rotation =
               hermite_interpolate(a.rotation, a.tangent, a.time, b.rotation,
                                   a.tangent_next, b.time, t);
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

auto make_rotation_tangent(const animation& animation, int32 index, float smoothness,
                           const key_override override) noexcept -> float3
{
   const std::ptrdiff_t key_count = std::ssize(animation.rotation_keys);

   if (not animation.loop) {
      if (index - 1 < 0) return {};
      if (index + 1 >= key_count) return {};
   }

   std::ptrdiff_t back_index = index - 1;

   if (back_index < 0 or back_index >= key_count) {
      back_index = (back_index + key_count) % key_count;
   }

   std::ptrdiff_t forward_index = index + 1;

   if (forward_index < 0 or forward_index >= key_count) {
      forward_index = (forward_index + key_count) % key_count;
   }

   const world::rotation_key& key_back = animation.rotation_keys[back_index];
   const world::rotation_key& key_forward = animation.rotation_keys[forward_index];

   const float3 rotation_back =
      back_index == override.index ? override.rotation : key_back.rotation;
   const float3 rotation_forward =
      forward_index == override.index ? override.rotation : key_forward.rotation;

   const float time_back =
      index - 1 < 0 ? key_back.time - animation.runtime : key_back.time;
   const float time_forward = index + 1 >= key_count
                                 ? key_forward.time + animation.runtime
                                 : key_forward.time;

   return smoothness *
          ((rotation_forward - rotation_back) / (time_forward - time_back));
}

auto make_position_tangent(const animation& animation, int32 index, float smoothness,
                           const key_override override) noexcept -> float3
{
   const std::ptrdiff_t key_count = std::ssize(animation.position_keys);

   if (not animation.loop) {
      if (index - 1 < 0) return {};
      if (index + 1 >= key_count) return {};
   }

   std::ptrdiff_t back_index = index - 1;

   if (back_index < 0 or back_index >= key_count) {
      back_index = (back_index + key_count) % key_count;
   }

   std::ptrdiff_t forward_index = index + 1;

   if (forward_index < 0 or forward_index >= key_count) {
      forward_index = (forward_index + key_count) % key_count;
   }

   const world::position_key& key_back = animation.position_keys[back_index];
   const world::position_key& key_forward = animation.position_keys[forward_index];

   const float3 position_back =
      back_index == override.index ? override.position : key_back.position;
   const float3 position_forward =
      forward_index == override.index ? override.position : key_forward.position;

   const float time_back =
      index - 1 < 0 ? key_back.time - animation.runtime : key_back.time;
   const float time_forward = index + 1 >= key_count
                                 ? key_forward.time + animation.runtime
                                 : key_forward.time;

   return smoothness *
          ((position_forward - position_back) / (time_forward - time_back));
}

}