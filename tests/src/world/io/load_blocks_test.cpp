#include "pch.h"

#include "world/io/load_blocks.hpp"

#include <span>

namespace we::world::tests {

namespace {

/// @brief Checks if the block at index has a unique ID across all other blocks.
/// @param index The index of the blocks to check.
/// @param ids The block IDs.
/// @return If the block ID is unique or not.
bool is_unique_id(std::size_t index, const auto& ids)
{
   if (index >= ids.size()) throw std::out_of_range{"out of range"};

   const auto entity_id = ids[index];

   for (std::size_t i = 0; i < ids.size(); ++i) {
      if (i == index) continue;

      if (entity_id == ids[i]) return false;
   }

   return true;
}

}

TEST_CASE("world load blocks (boxes)", "[World][IO]")
{
   null_output_stream output;
   layer_remap layer_remap;

   layer_remap.set(2, 2);

   const blocks blocks = load_blocks("data/blocks/boxes.blk", layer_remap, output);

   REQUIRE(blocks.boxes.size() == 3);

   const blocks_boxes& boxes = blocks.boxes;

   CHECK(boxes.bbox.min_x[0] == 4.5f);
   CHECK(boxes.bbox.min_y[0] == 0.5f);
   CHECK(boxes.bbox.min_z[0] == -2.0f);
   CHECK(boxes.bbox.max_x[0] == 12.5f);
   CHECK(boxes.bbox.max_y[0] == 8.5f);
   CHECK(boxes.bbox.max_z[0] == 6.0f);
   CHECK(boxes.hidden[0] == false);
   CHECK(boxes.layer[0] == 2);
   CHECK(boxes.description[0].rotation == quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   CHECK(boxes.description[0].position == float3{8.5f, 4.5f, 2.0f});
   CHECK(boxes.description[0].size == float3{4.0f, 4.0f, 4.0f});
   CHECK(boxes.description[0].surface_materials ==
         std::array<uint8, 6>{0, 1, 2, 3, 4, 5});
   CHECK(boxes.description[0].surface_texture_mode ==
         std::array<block_texture_mode, 6>{
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
         });
   CHECK(boxes.description[0].surface_texture_rotation ==
         std::array<block_texture_rotation, 6>{
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
         });
   CHECK(boxes.description[0].surface_texture_scale ==
         std::array<std::array<int8, 2>, 6>{
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{-1, -2},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
         });
   CHECK(boxes.description[0].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 6>{
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{256, 256},
            std::array<uint16, 2>{0, 0},
         });
   CHECK(is_unique_id(0, boxes.ids));

   CHECK(boxes.bbox.min_x[1] == 2.00001764f);
   CHECK(boxes.bbox.min_y[1] == 12.0000086f);
   CHECK(boxes.bbox.min_z[1] == 4.00001764f);
   CHECK(boxes.bbox.max_x[1] == 17.9999828f);
   CHECK(boxes.bbox.max_y[1] == 19.9999905f);
   CHECK(boxes.bbox.max_z[1] == 19.9999828f);
   CHECK(boxes.hidden[1] == false);
   CHECK(boxes.description[1].rotation == quaternion{0.707106f, 0.0f, 0.707106f, 0.0f});
   CHECK(boxes.description[1].position == float3{10.0f, 16.0f, 12.0f});
   CHECK(boxes.description[1].size == float3{8.0f, 4.0f, 8.0f});
   CHECK(boxes.description[1].surface_materials ==
         std::array<uint8, 6>{2, 2, 2, 2, 2, 2});
   CHECK(boxes.description[1].surface_texture_mode ==
         std::array<block_texture_mode, 6>{
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::local_space_zy,
            block_texture_mode::world_space_xz,
         });
   CHECK(boxes.description[1].surface_texture_rotation ==
         std::array<block_texture_rotation, 6>{
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
         });
   CHECK(boxes.description[1].surface_texture_scale ==
         std::array<std::array<int8, 2>, 6>{
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{-2, -2},
         });
   CHECK(boxes.description[1].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 6>{
            std::array<uint16, 2>{1024, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
         });
   CHECK(is_unique_id(1, boxes.ids));

   CHECK(boxes.bbox.min_x[2] == 1.0f);
   CHECK(boxes.bbox.min_y[2] == 1.0f);
   CHECK(boxes.bbox.min_z[2] == 1.0f);
   CHECK(boxes.bbox.max_x[2] == 11.0f);
   CHECK(boxes.bbox.max_y[2] == 11.0f);
   CHECK(boxes.bbox.max_z[2] == 11.0f);
   CHECK(boxes.hidden[2] == false);
   CHECK(boxes.description[2].rotation == quaternion{0.0f, 0.0f, 0.0f, 1.0f});
   CHECK(boxes.description[2].position == float3{6.0f, 6.0f, 6.0f});
   CHECK(boxes.description[2].size == float3{5.0f, 5.0f, 5.0f});
   CHECK(boxes.description[2].surface_materials ==
         std::array<uint8, 6>{0, 0, 0, 0, 0, 0});
   CHECK(boxes.description[2].surface_texture_mode ==
         std::array<block_texture_mode, 6>{
            block_texture_mode::world_space_auto,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xy,
            block_texture_mode::local_space_auto,
            block_texture_mode::local_space_zy,
         });
   CHECK(boxes.description[2].surface_texture_rotation ==
         std::array<block_texture_rotation, 6>{
            block_texture_rotation::d0,
            block_texture_rotation::d90,
            block_texture_rotation::d180,
            block_texture_rotation::d270,
            block_texture_rotation::d0,
            block_texture_rotation::d0,
         });
   CHECK(boxes.description[2].surface_texture_scale ==
         std::array<std::array<int8, 2>, 6>{
            std::array<int8, 2>{-7, -6},
            std::array<int8, 2>{-5, -4},
            std::array<int8, 2>{-3, -2},
            std::array<int8, 2>{-1, 0},
            std::array<int8, 2>{1, 2},
            std::array<int8, 2>{3, 4},
         });
   CHECK(boxes.description[2].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 6>{
            std::array<uint16, 2>{0, 1},
            std::array<uint16, 2>{2, 3},
            std::array<uint16, 2>{4, 5},
            std::array<uint16, 2>{6, 7},
            std::array<uint16, 2>{8, 9},
            std::array<uint16, 2>{10, 11},
         });
   CHECK(is_unique_id(2, boxes.ids));

   REQUIRE(boxes.dirty.size() == 1);
   CHECK(boxes.dirty[0] == blocks_dirty_range{0, 3});
}

TEST_CASE("world load blocks (ramps)", "[World][IO]")
{
   null_output_stream output;
   layer_remap layer_remap;

   layer_remap.set(2, 2);

   const blocks blocks = load_blocks("data/blocks/ramps.blk", layer_remap, output);

   REQUIRE(blocks.ramps.size() == 3);

   const blocks_ramps& ramps = blocks.ramps;

   CHECK(ramps.bbox.min_x[0] == 4.5f);
   CHECK(ramps.bbox.min_y[0] == 0.5f);
   CHECK(ramps.bbox.min_z[0] == -2.0f);
   CHECK(ramps.bbox.max_x[0] == 12.5f);
   CHECK(ramps.bbox.max_y[0] == 8.5f);
   CHECK(ramps.bbox.max_z[0] == 6.0f);
   CHECK(ramps.hidden[0] == false);
   CHECK(ramps.layer[0] == 2);
   CHECK(ramps.description[0].rotation == quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   CHECK(ramps.description[0].position == float3{8.5f, 4.5f, 2.0f});
   CHECK(ramps.description[0].size == float3{4.0f, 4.0f, 4.0f});
   CHECK(ramps.description[0].surface_materials == std::array<uint8, 5>{0, 1, 2, 3, 4});
   CHECK(ramps.description[0].surface_texture_mode ==
         std::array<block_texture_mode, 5>{
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
         });
   CHECK(ramps.description[0].surface_texture_rotation ==
         std::array<block_texture_rotation, 5>{
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
         });
   CHECK(ramps.description[0].surface_texture_scale ==
         std::array<std::array<int8, 2>, 5>{
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{-1, -2},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
         });
   CHECK(ramps.description[0].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 5>{
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{256, 256},
         });
   CHECK(is_unique_id(0, ramps.ids));

   CHECK(ramps.bbox.min_x[1] == 2.00001764f);
   CHECK(ramps.bbox.min_y[1] == 12.0000086f);
   CHECK(ramps.bbox.min_z[1] == 4.00001764f);
   CHECK(ramps.bbox.max_x[1] == 17.9999828f);
   CHECK(ramps.bbox.max_y[1] == 19.9999905f);
   CHECK(ramps.bbox.max_z[1] == 19.9999828f);
   CHECK(ramps.hidden[1] == false);
   CHECK(ramps.description[1].rotation == quaternion{0.707106f, 0.0f, 0.707106f, 0.0f});
   CHECK(ramps.description[1].position == float3{10.0f, 16.0f, 12.0f});
   CHECK(ramps.description[1].size == float3{8.0f, 4.0f, 8.0f});
   CHECK(ramps.description[1].surface_materials == std::array<uint8, 5>{2, 2, 2, 2, 2});
   CHECK(ramps.description[1].surface_texture_mode ==
         std::array<block_texture_mode, 5>{
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::local_space_zy,
         });
   CHECK(ramps.description[1].surface_texture_rotation ==
         std::array<block_texture_rotation, 5>{
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
         });
   CHECK(ramps.description[1].surface_texture_scale ==
         std::array<std::array<int8, 2>, 5>{
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{-2, -2},
         });
   CHECK(ramps.description[1].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 5>{
            std::array<uint16, 2>{1024, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
         });
   CHECK(is_unique_id(1, ramps.ids));

   CHECK(ramps.bbox.min_x[2] == 1.0f);
   CHECK(ramps.bbox.min_y[2] == 1.0f);
   CHECK(ramps.bbox.min_z[2] == 1.0f);
   CHECK(ramps.bbox.max_x[2] == 11.0f);
   CHECK(ramps.bbox.max_y[2] == 11.0f);
   CHECK(ramps.bbox.max_z[2] == 11.0f);
   CHECK(ramps.hidden[2] == false);
   CHECK(ramps.description[2].rotation == quaternion{0.0f, 0.0f, 0.0f, 1.0f});
   CHECK(ramps.description[2].position == float3{6.0f, 6.0f, 6.0f});
   CHECK(ramps.description[2].size == float3{5.0f, 5.0f, 5.0f});
   CHECK(ramps.description[2].surface_materials == std::array<uint8, 5>{0, 0, 0, 0, 0});
   CHECK(ramps.description[2].surface_texture_mode ==
         std::array<block_texture_mode, 5>{
            block_texture_mode::world_space_auto,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xy,
            block_texture_mode::local_space_auto,
         });
   CHECK(ramps.description[2].surface_texture_rotation ==
         std::array<block_texture_rotation, 5>{
            block_texture_rotation::d0,
            block_texture_rotation::d90,
            block_texture_rotation::d180,
            block_texture_rotation::d270,
            block_texture_rotation::d0,
         });
   CHECK(ramps.description[2].surface_texture_scale ==
         std::array<std::array<int8, 2>, 5>{
            std::array<int8, 2>{-7, -6},
            std::array<int8, 2>{-5, -4},
            std::array<int8, 2>{-3, -2},
            std::array<int8, 2>{-1, 0},
            std::array<int8, 2>{1, 2},
         });
   CHECK(ramps.description[2].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 5>{
            std::array<uint16, 2>{0, 1},
            std::array<uint16, 2>{2, 3},
            std::array<uint16, 2>{4, 5},
            std::array<uint16, 2>{6, 7},
            std::array<uint16, 2>{8, 9},
         });
   CHECK(is_unique_id(2, ramps.ids));

   REQUIRE(ramps.dirty.size() == 1);
   CHECK(ramps.dirty[0] == blocks_dirty_range{0, 3});
}

TEST_CASE("world load blocks (quads)", "[World][IO]")
{
   null_output_stream output;
   layer_remap layer_remap;

   layer_remap.set(2, 2);

   const blocks blocks = load_blocks("data/blocks/quads.blk", layer_remap, output);

   REQUIRE(blocks.quads.size() == 3);

   const blocks_quads& quads = blocks.quads;

   CHECK(quads.bbox.min_x[0] == 0.0f);
   CHECK(quads.bbox.min_y[0] == 0.0f);
   CHECK(quads.bbox.min_z[0] == 0.0f);
   CHECK(quads.bbox.max_x[0] == 1.0f);
   CHECK(quads.bbox.max_y[0] == 0.0f);
   CHECK(quads.bbox.max_z[0] == 1.0f);
   CHECK(quads.hidden[0] == false);
   CHECK(quads.layer[0] == 2);
   CHECK(quads.description[0].vertices[0] == float3{0.0f, 0.0f, 0.0f});
   CHECK(quads.description[0].vertices[1] == float3{1.0f, 0.0f, 0.0f});
   CHECK(quads.description[0].vertices[2] == float3{1.0f, 0.0f, 1.0f});
   CHECK(quads.description[0].vertices[3] == float3{0.0f, 0.0f, 1.0f});
   CHECK(quads.description[0].quad_split == block_quad_split::regular);
   CHECK(quads.description[0].surface_materials == std::array<uint8, 1>{0});
   CHECK(quads.description[0].surface_texture_mode ==
         std::array<block_texture_mode, 1>{
            block_texture_mode::world_space_auto,
         });
   CHECK(quads.description[0].surface_texture_rotation ==
         std::array<block_texture_rotation, 1>{
            block_texture_rotation::d180,
         });
   CHECK(quads.description[0].surface_texture_scale ==
         std::array<std::array<int8, 2>, 1>{
            std::array<int8, 2>{-1, -2},
         });
   CHECK(quads.description[0].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 1>{
            std::array<uint16, 2>{256, 256},
         });
   CHECK(is_unique_id(0, quads.ids));

   CHECK(quads.bbox.min_x[1] == 0.0f);
   CHECK(quads.bbox.min_y[1] == 0.0f);
   CHECK(quads.bbox.min_z[1] == 0.0f);
   CHECK(quads.bbox.max_x[1] == 1.5f);
   CHECK(quads.bbox.max_y[1] == 0.0f);
   CHECK(quads.bbox.max_z[1] == 1.5f);
   CHECK(quads.hidden[1] == false);
   CHECK(quads.description[1].vertices[0] == float3{0.0f, 0.0f, 0.0f});
   CHECK(quads.description[1].vertices[1] == float3{1.5f, 0.0f, 0.0f});
   CHECK(quads.description[1].vertices[2] == float3{1.0f, 0.0f, 1.0f});
   CHECK(quads.description[1].vertices[3] == float3{0.0f, 0.0f, 1.5f});
   CHECK(quads.description[1].quad_split == block_quad_split::alternate);
   CHECK(quads.description[1].surface_materials == std::array<uint8, 1>{2});
   CHECK(quads.description[1].surface_texture_mode ==
         std::array<block_texture_mode, 1>{
            block_texture_mode::world_space_zy,
         });
   CHECK(quads.description[1].surface_texture_rotation ==
         std::array<block_texture_rotation, 1>{
            block_texture_rotation::d90,
         });
   CHECK(quads.description[1].surface_texture_scale ==
         std::array<std::array<int8, 2>, 1>{
            std::array<int8, 2>{0, 0},
         });
   CHECK(quads.description[1].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 1>{
            std::array<uint16, 2>{1024, 0},
         });
   CHECK(is_unique_id(1, quads.ids));

   CHECK(quads.bbox.min_x[2] == 0.0f);
   CHECK(quads.bbox.min_y[2] == 0.0f);
   CHECK(quads.bbox.min_z[2] == 0.0f);
   CHECK(quads.bbox.max_x[2] == 8.0f);
   CHECK(quads.bbox.max_y[2] == 0.0f);
   CHECK(quads.bbox.max_z[2] == 8.0f);
   CHECK(quads.hidden[2] == false);
   CHECK(quads.description[2].vertices[0] == float3{0.0f, 0.0f, 0.0f});
   CHECK(quads.description[2].vertices[1] == float3{8.0f, 0.0f, 0.0f});
   CHECK(quads.description[2].vertices[2] == float3{8.0f, 0.0f, 8.0f});
   CHECK(quads.description[2].vertices[3] == float3{0.0f, 0.0f, 8.0f});
   CHECK(quads.description[2].quad_split == block_quad_split::regular);
   CHECK(quads.description[2].surface_materials == std::array<uint8, 1>{0});
   CHECK(quads.description[2].surface_texture_mode ==
         std::array<block_texture_mode, 1>{
            block_texture_mode::world_space_xz,
         });
   CHECK(quads.description[2].surface_texture_rotation ==
         std::array<block_texture_rotation, 1>{
            block_texture_rotation::d0,
         });
   CHECK(quads.description[2].surface_texture_scale ==
         std::array<std::array<int8, 2>, 1>{
            std::array<int8, 2>{0, 0},
         });
   CHECK(quads.description[2].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 1>{
            std::array<uint16, 2>{0, 0},
         });
   CHECK(is_unique_id(2, quads.ids));

   REQUIRE(quads.dirty.size() == 1);
   CHECK(quads.dirty[0] == blocks_dirty_range{0, 3});
}

TEST_CASE("world load blocks (cylinders)", "[World][IO]")
{
   null_output_stream output;
   layer_remap layer_remap;

   layer_remap.set(2, 2);

   const blocks blocks = load_blocks("data/blocks/cylinders.blk", layer_remap, output);

   REQUIRE(blocks.cylinders.size() == 3);

   const blocks_cylinders& cylinders = blocks.cylinders;

   CHECK(cylinders.bbox.min_x[0] == 4.5f);
   CHECK(cylinders.bbox.min_y[0] == 0.5f);
   CHECK(cylinders.bbox.min_z[0] == -2.0f);
   CHECK(cylinders.bbox.max_x[0] == 12.5f);
   CHECK(cylinders.bbox.max_y[0] == 8.5f);
   CHECK(cylinders.bbox.max_z[0] == 6.0f);
   CHECK(cylinders.hidden[0] == false);
   CHECK(cylinders.layer[0] == 2);
   CHECK(cylinders.description[0].rotation == quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   CHECK(cylinders.description[0].position == float3{8.5f, 4.5f, 2.0f});
   CHECK(cylinders.description[0].size == float3{4.0f, 4.0f, 4.0f});
   CHECK(cylinders.description[0].surface_materials == std::array<uint8, 3>{0, 1, 2});
   CHECK(cylinders.description[0].surface_texture_mode ==
         std::array<block_texture_mode, 3>{
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
         });
   CHECK(cylinders.description[0].surface_texture_rotation ==
         std::array<block_texture_rotation, 3>{
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
         });
   CHECK(cylinders.description[0].surface_texture_scale ==
         std::array<std::array<int8, 2>, 3>{
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{-1, -2},
            std::array<int8, 2>{0, 0},
         });
   CHECK(cylinders.description[0].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 3>{
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{256, 256},
         });
   CHECK(is_unique_id(0, cylinders.ids));

   CHECK(cylinders.bbox.min_x[1] == 2.00001764f);
   CHECK(cylinders.bbox.min_y[1] == 12.0000086f);
   CHECK(cylinders.bbox.min_z[1] == 4.00001764f);
   CHECK(cylinders.bbox.max_x[1] == 17.9999828f);
   CHECK(cylinders.bbox.max_y[1] == 19.9999905f);
   CHECK(cylinders.bbox.max_z[1] == 19.9999828f);
   CHECK(cylinders.hidden[1] == false);
   CHECK(cylinders.description[1].rotation ==
         quaternion{0.707106f, 0.0f, 0.707106f, 0.0f});
   CHECK(cylinders.description[1].position == float3{10.0f, 16.0f, 12.0f});
   CHECK(cylinders.description[1].size == float3{8.0f, 4.0f, 8.0f});
   CHECK(cylinders.description[1].surface_materials == std::array<uint8, 3>{2, 2, 2});
   CHECK(cylinders.description[1].surface_texture_mode ==
         std::array<block_texture_mode, 3>{
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::local_space_zy,
         });
   CHECK(cylinders.description[1].surface_texture_rotation ==
         std::array<block_texture_rotation, 3>{
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
         });
   CHECK(cylinders.description[1].surface_texture_scale ==
         std::array<std::array<int8, 2>, 3>{
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{-2, -2},
         });
   CHECK(cylinders.description[1].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 3>{
            std::array<uint16, 2>{1024, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
         });
   CHECK(is_unique_id(1, cylinders.ids));

   CHECK(cylinders.bbox.min_x[2] == 1.0f);
   CHECK(cylinders.bbox.min_y[2] == 1.0f);
   CHECK(cylinders.bbox.min_z[2] == 1.0f);
   CHECK(cylinders.bbox.max_x[2] == 11.0f);
   CHECK(cylinders.bbox.max_y[2] == 11.0f);
   CHECK(cylinders.bbox.max_z[2] == 11.0f);
   CHECK(cylinders.hidden[2] == false);
   CHECK(cylinders.description[2].rotation == quaternion{0.0f, 0.0f, 0.0f, 1.0f});
   CHECK(cylinders.description[2].position == float3{6.0f, 6.0f, 6.0f});
   CHECK(cylinders.description[2].size == float3{5.0f, 5.0f, 5.0f});
   CHECK(cylinders.description[2].surface_materials == std::array<uint8, 3>{0, 0, 0});
   CHECK(cylinders.description[2].surface_texture_mode ==
         std::array<block_texture_mode, 3>{
            block_texture_mode::world_space_auto,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_xz,
         });
   CHECK(cylinders.description[2].surface_texture_rotation ==
         std::array<block_texture_rotation, 3>{
            block_texture_rotation::d0,
            block_texture_rotation::d90,
            block_texture_rotation::d180,
         });
   CHECK(cylinders.description[2].surface_texture_scale ==
         std::array<std::array<int8, 2>, 3>{
            std::array<int8, 2>{-7, -6},
            std::array<int8, 2>{-5, -4},
            std::array<int8, 2>{-3, -2},
         });
   CHECK(cylinders.description[2].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 3>{
            std::array<uint16, 2>{0, 1},
            std::array<uint16, 2>{2, 3},
            std::array<uint16, 2>{4, 5},
         });
   CHECK(is_unique_id(2, cylinders.ids));

   REQUIRE(cylinders.dirty.size() == 1);
   CHECK(cylinders.dirty[0] == blocks_dirty_range{0, 3});
}

TEST_CASE("world load blocks (stairways)", "[World][IO]")
{
   null_output_stream output;
   layer_remap layer_remap;

   layer_remap.set(2, 2);

   const blocks blocks = load_blocks("data/blocks/stairways.blk", layer_remap, output);

   REQUIRE(blocks.stairways.size() == 3);

   const blocks_stairways& stairways = blocks.stairways;

   CHECK(stairways.bbox.min_x[0] == 6.5f);
   CHECK(stairways.bbox.min_y[0] == 0.5f);
   CHECK(stairways.bbox.min_z[0] == 0.0f);
   CHECK(stairways.bbox.max_x[0] == 10.5f);
   CHECK(stairways.bbox.max_y[0] == 4.5f);
   CHECK(stairways.bbox.max_z[0] == 4.0f);
   CHECK(stairways.hidden[0] == false);
   CHECK(stairways.layer[0] == 2);
   CHECK(stairways.description[0].rotation == quaternion{0.0f, 1.0f, 0.0f, 0.0f});
   CHECK(stairways.description[0].position == float3{8.5f, 4.5f, 2.0f});
   CHECK(stairways.description[0].size == float3{4.0f, 4.0f, 4.0f});
   CHECK(stairways.description[0].step_height == 0.1f);
   CHECK(stairways.description[0].first_step_offset == 0.0f);
   CHECK(stairways.description[0].surface_materials ==
         std::array<uint8, 6>{0, 1, 2, 3, 4, 5});
   CHECK(stairways.description[0].surface_texture_mode ==
         std::array<block_texture_mode, 6>{
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_zy,
         });
   CHECK(stairways.description[0].surface_texture_rotation ==
         std::array<block_texture_rotation, 6>{
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
            block_texture_rotation::d180,
         });
   CHECK(stairways.description[0].surface_texture_scale ==
         std::array<std::array<int8, 2>, 6>{
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{-1, -2},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
         });
   CHECK(stairways.description[0].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 6>{
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{256, 256},
            std::array<uint16, 2>{0, 0},
         });
   CHECK(stairways.mesh[0] == blocks.custom_meshes.debug_query_handle(
                                 stairways.description[0].custom_mesh_desc()));
   CHECK(is_unique_id(0, stairways.ids));

   CHECK(stairways.bbox.min_x[1] == 6.00000858f);
   CHECK(stairways.bbox.min_y[1] == 16.0000000f);
   CHECK(stairways.bbox.min_z[1] == 8.00000858f);
   CHECK(stairways.bbox.max_x[1] == 13.9999914f);
   CHECK(stairways.bbox.max_y[1] == 20.1249905f);
   CHECK(stairways.bbox.max_z[1] == 15.9999914f);
   CHECK(stairways.hidden[1] == false);
   CHECK(stairways.description[1].rotation ==
         quaternion{0.707106f, 0.0f, 0.707106f, 0.0f});
   CHECK(stairways.description[1].position == float3{10.0f, 16.0f, 12.0f});
   CHECK(stairways.description[1].size == float3{8.0f, 4.0f, 8.0f});
   CHECK(stairways.description[1].step_height == 0.25f);
   CHECK(stairways.description[1].first_step_offset == 0.125f);
   CHECK(stairways.description[1].surface_materials ==
         std::array<uint8, 6>{2, 2, 2, 2, 2, 2});
   CHECK(stairways.description[1].surface_texture_mode ==
         std::array<block_texture_mode, 6>{
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xz,
            block_texture_mode::local_space_zy,
            block_texture_mode::world_space_xz,
         });
   CHECK(stairways.description[1].surface_texture_rotation ==
         std::array<block_texture_rotation, 6>{
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
            block_texture_rotation::d90,
         });
   CHECK(stairways.description[1].surface_texture_scale ==
         std::array<std::array<int8, 2>, 6>{
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{0, 0},
            std::array<int8, 2>{-2, -2},
         });
   CHECK(stairways.description[1].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 6>{
            std::array<uint16, 2>{1024, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
            std::array<uint16, 2>{0, 0},
         });
   CHECK(blocks.custom_meshes.debug_ref_count(
            stairways.description[1].custom_mesh_desc()) == 1);
   CHECK(stairways.mesh[1] == blocks.custom_meshes.debug_query_handle(
                                 stairways.description[1].custom_mesh_desc()));
   CHECK(is_unique_id(1, stairways.ids));

   CHECK(stairways.bbox.min_x[2] == 3.5f);
   CHECK(stairways.bbox.min_y[2] == 1.0f);
   CHECK(stairways.bbox.min_z[2] == 3.5f);
   CHECK(stairways.bbox.max_x[2] == 8.5f);
   CHECK(stairways.bbox.max_y[2] == 6.0f);
   CHECK(stairways.bbox.max_z[2] == 8.5f);
   CHECK(stairways.hidden[2] == false);
   CHECK(stairways.description[2].rotation == quaternion{0.0f, 0.0f, 0.0f, 1.0f});
   CHECK(stairways.description[2].position == float3{6.0f, 6.0f, 6.0f});
   CHECK(stairways.description[2].size == float3{5.0f, 5.0f, 5.0f});
   CHECK(stairways.description[2].step_height == 1.0f);
   CHECK(stairways.description[2].first_step_offset == 0.0f);
   CHECK(stairways.description[2].surface_materials ==
         std::array<uint8, 6>{0, 0, 0, 0, 0, 0});
   CHECK(stairways.description[2].surface_texture_mode ==
         std::array<block_texture_mode, 6>{
            block_texture_mode::world_space_auto,
            block_texture_mode::world_space_zy,
            block_texture_mode::world_space_xz,
            block_texture_mode::world_space_xy,
            block_texture_mode::local_space_auto,
            block_texture_mode::local_space_zy,
         });
   CHECK(stairways.description[2].surface_texture_rotation ==
         std::array<block_texture_rotation, 6>{
            block_texture_rotation::d0,
            block_texture_rotation::d90,
            block_texture_rotation::d180,
            block_texture_rotation::d270,
            block_texture_rotation::d0,
            block_texture_rotation::d0,
         });
   CHECK(stairways.description[2].surface_texture_scale ==
         std::array<std::array<int8, 2>, 6>{
            std::array<int8, 2>{-7, -6},
            std::array<int8, 2>{-5, -4},
            std::array<int8, 2>{-3, -2},
            std::array<int8, 2>{-1, 0},
            std::array<int8, 2>{1, 2},
            std::array<int8, 2>{3, 4},
         });
   CHECK(stairways.description[2].surface_texture_offset ==
         std::array<std::array<uint16, 2>, 6>{
            std::array<uint16, 2>{0, 1},
            std::array<uint16, 2>{2, 3},
            std::array<uint16, 2>{4, 5},
            std::array<uint16, 2>{6, 7},
            std::array<uint16, 2>{8, 9},
            std::array<uint16, 2>{10, 11},
         });
   CHECK(blocks.custom_meshes.debug_ref_count(
            stairways.description[2].custom_mesh_desc()) == 1);
   CHECK(stairways.mesh[2] == blocks.custom_meshes.debug_query_handle(
                                 stairways.description[2].custom_mesh_desc()));
   CHECK(is_unique_id(2, stairways.ids));

   REQUIRE(stairways.dirty.size() == 1);
   CHECK(stairways.dirty[0] == blocks_dirty_range{0, 3});
}

TEST_CASE("world load blocks (materials)", "[World][IO]")
{
   null_output_stream output;
   layer_remap layer_remap;

   const blocks blocks = load_blocks("data/blocks/materials.blk", layer_remap, output);

   REQUIRE(blocks.materials.size() == max_block_materials);

   const std::span<const block_material> materials = blocks.materials;

   CHECK(materials[0].name == "rocks");
   CHECK(materials[0].diffuse_map == "rocks_diffuse");
   CHECK(materials[0].normal_map == "rocks_normal");
   CHECK(materials[0].detail_map == "rocks_detail");
   CHECK(materials[0].env_map == "skycube");
   CHECK(materials[0].detail_tiling == std::array<uint8, 2>{4, 4});
   CHECK(not materials[0].tile_normal_map);
   CHECK(materials[0].specular_lighting);
   CHECK(materials[0].specular_color == float3{0.5f, 0.5f, 0.5f});
   CHECK(materials[0].foley_group == block_foley_group::stone);

   const block_material empty_material;

   CHECK(materials[1] == empty_material);

   CHECK(materials[2].name == "snow");
   CHECK(materials[2].diffuse_map == "snow_diffuse");
   CHECK(materials[2].normal_map == "snow_detail_normal");
   CHECK(materials[2].detail_map == "snow_detail");
   CHECK(materials[2].env_map == "skycube");
   CHECK(materials[2].detail_tiling == std::array<uint8, 2>{3, 3});
   CHECK(materials[2].tile_normal_map);
   CHECK(materials[2].specular_lighting);
   CHECK(materials[2].specular_color == float3{0.75f, 0.75f, 0.75f});
   CHECK(materials[2].foley_group == block_foley_group::snow);

   for (uint32 i = 3; i < max_block_materials; ++i) {
      CHECK(materials[i] == empty_material);
   }

   REQUIRE(blocks.materials_dirty.size() == 1);
   CHECK(blocks.materials_dirty[0] == blocks_dirty_range{0, max_block_materials});
}

}