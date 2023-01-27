#pragma once

#include "types.hpp"

#include <memory>
#include <optional>
#include <span>

namespace we::assets::terrain {

namespace detail {
class terrain_collision_impl;
}

struct terrain;

struct ray_hit {
   float distance;
};

class terrain_collision {
public:
   terrain_collision();

   terrain_collision(const terrain& terrain) noexcept;

   terrain_collision(terrain_collision&&) noexcept;
   auto operator=(terrain_collision&&) noexcept -> terrain_collision&;

   ~terrain_collision();

   terrain_collision(const terrain_collision&) = delete;
   auto operator=(const terrain_collision&) -> terrain_collision& = delete;

   [[nodiscard]] auto raycast(const float3 ray_origin, const float3 ray_direction) const noexcept
      -> std::optional<ray_hit>;

private:
   std::unique_ptr<detail::terrain_collision_impl> _impl;
};

}
