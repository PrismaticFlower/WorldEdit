#pragma once

#include "id.hpp"
#include "types.hpp"

#include "blocks/dirty_range_tracker.hpp"

#include "container/pinned_vector.hpp"
#include "container/slim_bitset.hpp"

#include <array>
#include <string>

namespace we::world {

constexpr std::size_t max_blocks = 1'048'576;
constexpr std::size_t max_block_materials = 256;
constexpr std::size_t reserved_blocks = 2048;
constexpr pinned_vector_init blocks_init{.max_size = max_blocks,
                                         .initial_capacity = reserved_blocks};

constexpr int8 block_min_texture_scale = -7;
constexpr int8 block_max_texture_scale = 8;
constexpr uint16 block_max_texture_offset = 8191;

enum class block_texture_mode : uint8 {
   tangent_space_xyz,

   world_space_auto,

   world_space_zy,
   world_space_xz,
   world_space_xy,

   unwrapped,
};

enum class block_texture_rotation : uint8 { d0, d90, d180, d270 };

enum class block_foley_group : uint8 {
   stone,
   dirt,
   grass,
   metal,
   snow,
   terrain,
   water,
   wood,
};

struct block_description_box {
   quaternion rotation;
   float3 position;
   float3 size;
   std::array<uint8, 6> surface_materials = {};
   std::array<block_texture_mode, 6> surface_texture_mode = {};
   std::array<block_texture_rotation, 6> surface_texture_rotation = {};
   std::array<std::array<int8, 2>, 6> surface_texture_scale = {};
   std::array<std::array<uint16, 2>, 6> surface_texture_offset = {};

   bool operator==(const block_description_box&) const noexcept = default;
};

struct blocks_bbox_soa {
   pinned_vector<float> min_x = blocks_init;
   pinned_vector<float> min_y = blocks_init;
   pinned_vector<float> min_z = blocks_init;
   pinned_vector<float> max_x = blocks_init;
   pinned_vector<float> max_y = blocks_init;
   pinned_vector<float> max_z = blocks_init;
};

struct blocks_boxes {
   blocks_bbox_soa bbox;

   pinned_vector<bool> hidden = blocks_init;

   pinned_vector<int8> layer = blocks_init;

   pinned_vector<block_description_box> description = blocks_init;

   pinned_vector<id<block_description_box>> ids = blocks_init;

   blocks_dirty_range_tracker dirty;

   void reserve(const std::size_t size) noexcept;

   auto size() const noexcept -> std::size_t;

   bool is_balanced() const noexcept;
};

struct block_material {
   std::string name;

   std::string diffuse_map;
   std::string normal_map;
   std::string detail_map;
   std::string env_map;

   std::array<uint8, 2> detail_tiling = {0, 0};
   bool tile_normal_map = false;
   bool specular_lighting = false;

   float3 specular_color = {1.0f, 1.0f, 1.0f};

   block_foley_group foley_group = block_foley_group::stone;

   bool operator==(const block_material&) const noexcept = default;
};

struct blocks {
   blocks_boxes boxes;

   pinned_vector<block_material> materials = get_blank_materials();
   blocks_dirty_range_tracker materials_dirty;

   struct next_ids {
      id_generator<block_description_box> boxes;
   } next_id;

   void untracked_fill_dirty_ranges() noexcept;

   void untracked_clear_dirty_ranges() noexcept;

   static auto get_blank_materials() noexcept -> pinned_vector<block_material>;
};

using block_box_id = id<block_description_box>;

enum class block_type {
   box,
};

struct block_id {
   block_id() = default;

   block_id(block_box_id id) noexcept;

   bool is_box() const noexcept;

   auto get_box() const noexcept -> block_box_id;

   auto type() const noexcept -> block_type;

   bool operator==(const block_id& other) const noexcept;

   bool operator==(const block_box_id box_id) const noexcept;

   /// @brief Special sentinal value with type box and an id of UINT32_MAX.
   static block_id none;

private:
   block_type id_type = block_type::box;

   union {
      block_box_id box = block_box_id{0xffffffffu};
   } id;
};

}
