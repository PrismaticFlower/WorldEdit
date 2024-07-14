#pragma once

#include "math/bounding_box.hpp"
#include "types.hpp"

#include <optional>
#include <span>
#include <vector>

namespace we {

struct bvh;
struct frustum;

}

namespace we::assets::msh {

struct mesh;

struct ray_hit {
   float distance;
   float3 normal;
};

class flat_model_bvh {
public:
   flat_model_bvh() noexcept;

   explicit flat_model_bvh(std::span<mesh> meshes) noexcept;

   flat_model_bvh(flat_model_bvh&&) noexcept;
   auto operator=(flat_model_bvh&&) noexcept -> flat_model_bvh&;

   ~flat_model_bvh();

   flat_model_bvh(const flat_model_bvh&) = delete;
   auto operator=(const flat_model_bvh&) -> flat_model_bvh& = delete;

   [[nodiscard]] auto query(const float3 ray_origin, const float3 ray_direction) const noexcept
      -> std::optional<ray_hit>;

   [[nodiscard]] bool intersects(const frustum& frustum) const noexcept;

   [[nodiscard]] auto get_child_bvhs() const noexcept -> std::span<const bvh>;

   [[nodiscard]] auto get_debug_boxes() const noexcept
      -> std::vector<std::vector<math::bounding_box>>;

private:
   std::vector<bvh> _bvhs;
};
}
