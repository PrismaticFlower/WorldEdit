#pragma once

#include "math/bounding_box.hpp"
#include "types.hpp"

#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace we {

struct frustum;

namespace detail {
struct bvh_impl;
struct top_level_bvh_impl;
}

struct bvh_flags {
   bool backface_cull = true;
};

struct bvh_ray_flags {
   bool allow_backface_cull : 1 = true;
   bool accept_first_hit : 1 = false;
};

struct bvh {
   struct ray_hit {
      float distance;
      float3 normal;
   };

   bvh() noexcept;

   bvh(std::span<const std::array<uint16, 3>> indices,
       std::span<const float3> positions, bvh_flags flags) noexcept;

   bvh(bvh&&) noexcept;
   auto operator=(bvh&&) -> bvh&;

   ~bvh();

   bvh(const bvh&) = delete;
   auto operator=(const bvh&) -> bvh& = delete;

   [[nodiscard]] auto raycast(const float3& ray_origin,
                              const float3& ray_direction, const float max_distance,
                              const bvh_ray_flags flags = {}) const noexcept
      -> std::optional<ray_hit>;

   [[nodiscard]] bool intersects(const frustum& frustum) const noexcept;

   [[nodiscard]] auto get_debug_boxes() const noexcept
      -> std::vector<math::bounding_box>;

private:
   std::unique_ptr<detail::bvh_impl> _impl;

   friend struct detail::top_level_bvh_impl;
};

struct top_level_bvh {
   struct instance {
      quaternion inverse_rotation;
      float3 inverse_position;

      const bvh* bvh = nullptr;

      quaternion rotation;
      float3 position;
   };

   top_level_bvh() noexcept;

   explicit top_level_bvh(std::span<const instance> instances) noexcept;

   top_level_bvh(top_level_bvh&&) noexcept;
   auto operator=(top_level_bvh&&) -> top_level_bvh&;

   ~top_level_bvh();

   top_level_bvh(const top_level_bvh&) = delete;
   auto operator=(const top_level_bvh&) -> top_level_bvh& = delete;

   [[nodiscard]] auto raycast(const float3& ray_originWS,
                              const float3& ray_directionWS, const float max_distance,
                              const bvh_ray_flags flags = {}) const noexcept
      -> std::optional<float>;

   [[nodiscard]] auto get_debug_boxes() const noexcept
      -> std::vector<math::bounding_box>;

private:
   std::unique_ptr<detail::top_level_bvh_impl> _impl;
};

}
