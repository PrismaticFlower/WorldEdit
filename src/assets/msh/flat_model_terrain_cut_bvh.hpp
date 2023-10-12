#pragma once

#include "flat_model_bvh.hpp"
#include "types.hpp"

#include <memory>
#include <optional>
#include <span>

namespace we::assets::msh {

namespace detail {
struct flat_model_terrain_cut_bvh_impl;
}

struct flat_model_terrain_cut;

struct flat_model_terrain_cut_bvh {
   flat_model_terrain_cut_bvh() noexcept;

   flat_model_terrain_cut_bvh(flat_model_terrain_cut_bvh&&) noexcept;

   ~flat_model_terrain_cut_bvh();

   flat_model_terrain_cut_bvh(const flat_model_terrain_cut_bvh&) = delete;
   auto operator=(const flat_model_terrain_cut_bvh&)
      -> flat_model_terrain_cut_bvh& = delete;
   auto operator=(flat_model_terrain_cut_bvh&&) -> flat_model_terrain_cut_bvh&;

   void build(std::span<flat_model_terrain_cut> cut) noexcept;

   [[nodiscard]] auto count_intersections(const float3 ray_origin,
                                          const float3 ray_direction) const noexcept
      -> uint32;

private:
   std::unique_ptr<detail::flat_model_terrain_cut_bvh_impl> _impl;
};

}
