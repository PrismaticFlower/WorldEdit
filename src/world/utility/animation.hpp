#pragma once

#include "../animation.hpp"

namespace we::world {

/// @brief Allows evaluating an animation at specific times.
struct animation_solver {
   /// @brief Initialize the solver, potentially reusing the memory allocated by a previous initialization of this solver.
   /// @param animation The animation to initialize the solver to.
   /// @param base_rotation The starting rotation of the animation. This is normally the object's rotation.
   /// @param base_position The starting position of the animation. This is normally the object's position.
   void init(const animation& animation, const quaternion& base_rotation,
             const float3& base_position) noexcept;

   /// @brief Evaluate an animation at a time.
   /// @param animation The animation to evaluate. This should match the animation passed to animation_solver::init.
   /// @param t The time to evaluate the animation at.
   /// @return The transform of the animation at t
   auto evaluate(const animation& animation, float t) const noexcept -> float4x4;

   struct subkey {
      float3 rotation;
      float3 position;
      float4x4 transform;
   };

private:
   quaternion _base_rotation;
   float3 _base_position;
   std::vector<float> _timepoints;
   std::vector<subkey> _subkeys;
};

auto make_position_key_for_time(const animation& animation, float t) noexcept
   -> position_key;

auto make_rotation_key_for_time(const animation& animation, float t) noexcept
   -> rotation_key;

}
