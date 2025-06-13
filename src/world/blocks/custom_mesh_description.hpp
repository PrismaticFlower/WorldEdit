#pragma once

#include "types.hpp"

namespace we::world {

struct block_custom_mesh_description_stairway {
   float3 size = {1.0f, 1.0f, 1.0f};
   float step_height = 0.125f;
   float first_step_offset = 0.0f;

   bool operator==(const block_custom_mesh_description_stairway&) const noexcept = default;
};

struct block_custom_mesh_description_ring {
   float inner_radius = 16.0f;
   float outer_radius = 4.0f;
   float height = 4.0f;
   uint16 segments = 16;
   bool flat_shading = false;
   float texture_loops = 1.0f;

   bool operator==(const block_custom_mesh_description_ring&) const noexcept = default;
};

enum class block_custom_mesh_type {
   stairway,
   ring,
};

struct block_custom_mesh_description {
   block_custom_mesh_description() = default;

   block_custom_mesh_description(const block_custom_mesh_description& other) noexcept;

   block_custom_mesh_description(const block_custom_mesh_description_stairway& stairway) noexcept;

   block_custom_mesh_description(const block_custom_mesh_description_ring& ring) noexcept;

   auto operator=(const block_custom_mesh_description& other) noexcept
      -> block_custom_mesh_description&;

   block_custom_mesh_type type = block_custom_mesh_type::stairway;

   union {
      block_custom_mesh_description_stairway stairway = {};
      block_custom_mesh_description_ring ring;
   };
};

bool operator==(const block_custom_mesh_description& left,
                const block_custom_mesh_description& right) noexcept;

}