#pragma once

#include "math/bounding_box.hpp"
#include "types.hpp"

#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace we {

namespace detail {
struct bvh_impl;
}

struct bvh_flags {
   bool backface_cull = true;
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

   [[nodiscard]] auto raycast(const float3& ray_origin, const float3& ray_direction,
                              const float max_distance) const noexcept
      -> std::optional<ray_hit>;

   [[nodiscard]] auto get_debug_boxes() const noexcept
      -> std::vector<math::bounding_box>;

private:
   std::unique_ptr<detail::bvh_impl> _impl;
};

}
