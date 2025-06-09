#pragma once

#include "id.hpp"
#include "types.hpp"

#include "blocks/custom_mesh_library.hpp"
#include "blocks/dirty_range_tracker.hpp"
#include "blocks/mesh_vertex.hpp"

#include "container/pinned_vector.hpp"
#include "container/slim_bitset.hpp"

#include <array>
#include <string>

namespace we::world {

constexpr std::size_t max_blocks = 131'072;
constexpr std::size_t max_block_materials = 256;
constexpr std::size_t reserved_blocks = 2048;
constexpr pinned_vector_init blocks_init{.max_size = max_blocks,
                                         .initial_capacity = reserved_blocks};

constexpr int8 block_min_texture_scale = -7;
constexpr int8 block_max_texture_scale = 8;
constexpr uint16 block_max_texture_offset = 8191;

enum class block_texture_mode : uint8 {
   world_space_auto,

   world_space_zy,
   world_space_xz,
   world_space_xy,

   local_space_auto,

   local_space_zy,
   local_space_xz,
   local_space_xy,

   unwrapped,
};

enum class block_texture_rotation : uint8 { d0, d90, d180, d270 };

enum class block_quad_split : uint8 { regular, alternate };

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

struct block_description_ramp {
   quaternion rotation;
   float3 position;
   float3 size;
   std::array<uint8, 5> surface_materials = {};
   std::array<block_texture_mode, 5> surface_texture_mode = {};
   std::array<block_texture_rotation, 5> surface_texture_rotation = {};
   std::array<std::array<int8, 2>, 5> surface_texture_scale = {};
   std::array<std::array<uint16, 2>, 5> surface_texture_offset = {};

   bool operator==(const block_description_ramp&) const noexcept = default;
};

struct block_description_quad {
   std::array<float3, 4> vertices;
   block_quad_split quad_split = block_quad_split::regular;
   std::array<uint8, 1> surface_materials = {};
   std::array<block_texture_mode, 1> surface_texture_mode = {};
   std::array<block_texture_rotation, 1> surface_texture_rotation = {};
   std::array<std::array<int8, 2>, 1> surface_texture_scale = {};
   std::array<std::array<uint16, 2>, 1> surface_texture_offset = {};

   bool operator==(const block_description_quad&) const noexcept = default;
};

struct block_description_cylinder {
   quaternion rotation;
   float3 position;
   float3 size;
   std::array<uint8, 3> surface_materials = {};
   std::array<block_texture_mode, 3> surface_texture_mode = {};
   std::array<block_texture_rotation, 3> surface_texture_rotation = {};
   std::array<std::array<int8, 2>, 3> surface_texture_scale = {};
   std::array<std::array<uint16, 2>, 3> surface_texture_offset = {};

   bool operator==(const block_description_cylinder&) const noexcept = default;
};

struct block_description_stairway {
   quaternion rotation;
   float3 position;
   float3 size;
   float step_height = 0.125f;
   float first_step_offset = 0.0f;

   std::array<uint8, 6> surface_materials = {};
   std::array<block_texture_mode, 6> surface_texture_mode = {};
   std::array<block_texture_rotation, 6> surface_texture_rotation = {};
   std::array<std::array<int8, 2>, 6> surface_texture_scale = {};
   std::array<std::array<uint16, 2>, 6> surface_texture_offset = {};

   auto custom_mesh_desc() const noexcept -> block_custom_mesh_stairway_desc;

   bool operator==(const block_description_stairway&) const noexcept = default;
};

struct block_description_cone {
   quaternion rotation;
   float3 position;
   float3 size;
   std::array<uint8, 2> surface_materials = {};
   std::array<block_texture_mode, 2> surface_texture_mode = {};
   std::array<block_texture_rotation, 2> surface_texture_rotation = {};
   std::array<std::array<int8, 2>, 2> surface_texture_scale = {};
   std::array<std::array<uint16, 2>, 2> surface_texture_offset = {};

   bool operator==(const block_description_cone&) const noexcept = default;
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

struct blocks_ramps {
   blocks_bbox_soa bbox;

   pinned_vector<bool> hidden = blocks_init;

   pinned_vector<int8> layer = blocks_init;

   pinned_vector<block_description_ramp> description = blocks_init;

   pinned_vector<id<block_description_ramp>> ids = blocks_init;

   blocks_dirty_range_tracker dirty;

   void reserve(const std::size_t size) noexcept;

   auto size() const noexcept -> std::size_t;

   bool is_balanced() const noexcept;
};

struct blocks_quads {
   blocks_bbox_soa bbox;

   pinned_vector<bool> hidden = blocks_init;

   pinned_vector<int8> layer = blocks_init;

   pinned_vector<block_description_quad> description = blocks_init;

   pinned_vector<id<block_description_quad>> ids = blocks_init;

   blocks_dirty_range_tracker dirty;

   void reserve(const std::size_t size) noexcept;

   auto size() const noexcept -> std::size_t;

   bool is_balanced() const noexcept;
};

struct blocks_cylinders {
   blocks_bbox_soa bbox;

   pinned_vector<bool> hidden = blocks_init;

   pinned_vector<int8> layer = blocks_init;

   pinned_vector<block_description_cylinder> description = blocks_init;

   pinned_vector<id<block_description_cylinder>> ids = blocks_init;

   blocks_dirty_range_tracker dirty;

   void reserve(const std::size_t size) noexcept;

   auto size() const noexcept -> std::size_t;

   bool is_balanced() const noexcept;
};

struct blocks_stairways {
   blocks_bbox_soa bbox;

   pinned_vector<bool> hidden = blocks_init;

   pinned_vector<int8> layer = blocks_init;

   pinned_vector<block_description_stairway> description = blocks_init;

   pinned_vector<block_custom_mesh_handle> mesh = blocks_init;

   pinned_vector<id<block_description_stairway>> ids = blocks_init;

   blocks_dirty_range_tracker dirty;

   void reserve(const std::size_t size) noexcept;

   auto size() const noexcept -> std::size_t;

   bool is_balanced() const noexcept;
};

struct blocks_cones {
   blocks_bbox_soa bbox;

   pinned_vector<bool> hidden = blocks_init;

   pinned_vector<int8> layer = blocks_init;

   pinned_vector<block_description_cone> description = blocks_init;

   pinned_vector<id<block_description_cone>> ids = blocks_init;

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
   blocks_ramps ramps;
   blocks_quads quads;
   blocks_cylinders cylinders;
   blocks_stairways stairways;
   blocks_cones cones;

   pinned_vector<block_material> materials = get_blank_materials();
   blocks_dirty_range_tracker materials_dirty;

   blocks_custom_mesh_library custom_meshes;

   struct next_ids {
      id_generator<block_description_box> boxes;
      id_generator<block_description_ramp> ramps;
      id_generator<block_description_quad> quads;
      id_generator<block_description_cylinder> cylinders;
      id_generator<block_description_stairway> stairways;
      id_generator<block_description_cone> cones;
   } next_id;

   bool empty() const noexcept;

   void untracked_fill_dirty_ranges() noexcept;

   void untracked_clear_dirty_ranges() noexcept;

   static auto get_blank_materials() noexcept -> pinned_vector<block_material>;
};

using block_box_id = id<block_description_box>;
using block_ramp_id = id<block_description_ramp>;
using block_quad_id = id<block_description_quad>;
using block_cylinder_id = id<block_description_cylinder>;
using block_stairway_id = id<block_description_stairway>;
using block_cone_id = id<block_description_cone>;

enum class block_type {
   box,
   ramp,
   quad,
   cylinder,
   stairway,
   cone,
};

struct block_id {
   block_id() = default;

   block_id(block_box_id id) noexcept;

   block_id(block_ramp_id id) noexcept;

   block_id(block_quad_id id) noexcept;

   block_id(block_cylinder_id id) noexcept;

   block_id(block_stairway_id id) noexcept;

   block_id(block_cone_id id) noexcept;

   bool is_box() const noexcept;

   auto get_box() const noexcept -> block_box_id;

   bool is_ramp() const noexcept;

   auto get_ramp() const noexcept -> block_ramp_id;

   bool is_quad() const noexcept;

   auto get_quad() const noexcept -> block_quad_id;

   bool is_cylinder() const noexcept;

   auto get_cylinder() const noexcept -> block_cylinder_id;

   bool is_stairway() const noexcept;

   auto get_stairway() const noexcept -> block_stairway_id;

   bool is_cone() const noexcept;

   auto get_cone() const noexcept -> block_cone_id;

   auto type() const noexcept -> block_type;

   bool operator==(const block_id& other) const noexcept;

   /// @brief Special sentinal value with type box and an id of UINT32_MAX.
   static block_id none;

private:
   block_type id_type = block_type::box;

   union {
      block_box_id box = block_box_id{0xffffffffu};
      block_ramp_id ramp;
      block_quad_id quad;
      block_cylinder_id cylinder;
      block_stairway_id stairway;
      block_cone_id cone;
   } id;
};

}
