#pragma once

#include "types.hpp"

#include <memory>
#include <optional>
#include <span>

namespace we::assets::msh {

namespace detail {
class flat_model_bvh_impl;
}

struct mesh;

struct ray_hit {
   float distance;
   float3 normal;
};

class flat_model_bvh {
public:
   flat_model_bvh() noexcept;

   flat_model_bvh(flat_model_bvh&&) noexcept;

   ~flat_model_bvh();

   flat_model_bvh(const flat_model_bvh&) = delete;
   auto operator=(const flat_model_bvh&) -> flat_model_bvh& = delete;
   auto operator=(flat_model_bvh&&) -> flat_model_bvh&;

   void build(std::span<mesh> meshes) noexcept;

   [[nodiscard]] auto query(const float3 ray_origin, const float3 ray_direction) const noexcept
      -> std::optional<ray_hit>;

private:
   std::unique_ptr<detail::flat_model_bvh_impl> _impl;
};

}
