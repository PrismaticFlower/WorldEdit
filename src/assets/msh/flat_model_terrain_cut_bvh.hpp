#pragma once

#include "flat_model_bvh.hpp"
#include "types.hpp"

#include <memory>
#include <optional>
#include <span>

namespace we {

struct bvh;

}

namespace we::assets::msh {

struct flat_model_terrain_cut;

struct flat_model_terrain_cut_bvh {
   flat_model_terrain_cut_bvh() noexcept;

   explicit flat_model_terrain_cut_bvh(std::span<flat_model_terrain_cut> cut) noexcept;

   flat_model_terrain_cut_bvh(flat_model_terrain_cut_bvh&&) noexcept;
   auto operator=(flat_model_terrain_cut_bvh&&) noexcept
      -> flat_model_terrain_cut_bvh&;

   ~flat_model_terrain_cut_bvh();

   flat_model_terrain_cut_bvh(const flat_model_terrain_cut_bvh&) = delete;
   auto operator=(const flat_model_terrain_cut_bvh&)
      -> flat_model_terrain_cut_bvh& = delete;

   [[nodiscard]] auto count_intersections(const float3 ray_origin,
                                          const float3 ray_direction) const noexcept
      -> uint32;

private:
   std::vector<bvh> _bvhs;
};

}
