#pragma once

#include "types.hpp"

namespace we::world {

struct block_custom_mesh_description_stairway {
   float3 size = {1.0f, 1.0f, 1.0f};
   float step_height = 0.125f;
   float first_step_offset = 0.0f;

   bool operator==(const block_custom_mesh_description_stairway&) const noexcept = default;
};

enum class block_custom_mesh_type {
   stairway,
};

struct block_custom_mesh_description {
   block_custom_mesh_description() = default;

   block_custom_mesh_description(const block_custom_mesh_description& other) noexcept;

   block_custom_mesh_description(const block_custom_mesh_description_stairway& stairway) noexcept;

   auto operator=(const block_custom_mesh_description& other) noexcept
      -> block_custom_mesh_description&;

   block_custom_mesh_type type = block_custom_mesh_type::stairway;

   union {
      block_custom_mesh_description_stairway stairway = {};
   };
};

bool operator==(const block_custom_mesh_description& left,
                const block_custom_mesh_description& right) noexcept;

}