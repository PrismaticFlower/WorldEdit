#include "pch.h"

#include "io/read_file.hpp"

#include "math/vector_funcs.hpp"

#include "world/io/save_blocks.hpp"

namespace we::world::tests {

TEST_CASE("world save blocks (boxes)", "[World][IO]")
{
   const std::string_view expected_blk = R"(Boxes(3)
{
   Box()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      SurfaceMaterials(0, 1, 2, 3, 4, 5);
      SurfaceTextureMode(1, 1, 1, 1, 1, 1);
      SurfaceTextureRotation(2, 2, 2, 2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 256, 256, 0, 0);
   }
   Box()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2, 2, 2, 2, 2);
      SurfaceTextureMode(2, 2, 2, 2, 5, 2);
      SurfaceTextureRotation(1, 1, 1, 1, 1, 1);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
   Box()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      SurfaceMaterials(0, 0, 0, 0, 0, 0);
      SurfaceTextureMode(3, 3, 3, 3, 3, 3);
      SurfaceTextureRotation(0, 0, 0, 0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
}

)";

   blocks blocks{
      .boxes =
         {
            .bbox =
               {
                  .min_x = {world::blocks_init, std::initializer_list{-1.0f, -1.0f, -1.0f}},
                  .min_y = {world::blocks_init, std::initializer_list{-1.0f, -1.0f, -1.0f}},
                  .min_z = {world::blocks_init, std::initializer_list{-1.0f, -1.0f, -1.0f}},
                  .max_x = {world::blocks_init, std::initializer_list{1.0f, 1.0f, 1.0f}},
                  .max_y = {world::blocks_init, std::initializer_list{1.0f, 1.0f, 1.0f}},
                  .max_z = {world::blocks_init, std::initializer_list{1.0f, 1.0f, 1.0f}},
               },
            .hidden = {world::blocks_init, std::initializer_list{true, true, true}},
            .description =
               {
                  world::blocks_init,
                  std::initializer_list{
                     world::block_description_box{
                        .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                        .position = {8.5f, 4.5f, 2.0f},
                        .size = {4.0f, 4.0f, 4.0f},
                        .surface_materials = {0, 1, 2, 3, 4, 5},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_auto,
                              block_texture_mode::world_space_auto,
                              block_texture_mode::world_space_auto,
                              block_texture_mode::world_space_auto,
                              block_texture_mode::world_space_auto,
                              block_texture_mode::world_space_auto,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d180,
                              block_texture_rotation::d180,
                              block_texture_rotation::d180,
                              block_texture_rotation::d180,
                              block_texture_rotation::d180,
                              block_texture_rotation::d180,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-1, -2},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{256, 256},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                     world::block_description_box{
                        .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                        .position = {10.0f, 16.0f, 12.0f},
                        .size = {8.0f, 4.0f, 8.0f},
                        .surface_materials = {2, 2, 2, 2, 2, 2},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::unwrapped,
                              block_texture_mode::world_space_zy,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d90,
                              block_texture_rotation::d90,
                              block_texture_rotation::d90,
                              block_texture_rotation::d90,
                              block_texture_rotation::d90,
                              block_texture_rotation::d90,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-2, -2},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{1024, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                     world::block_description_box{
                        .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                        .position = {6.0f, 6.0f, 6.0f},
                        .size = {5.0f, 5.0f, 5.0f},
                        .surface_materials = {0, 0, 0, 0, 0, 0},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_xz,
                              block_texture_mode::world_space_xz,
                              block_texture_mode::world_space_xz,
                              block_texture_mode::world_space_xz,
                              block_texture_mode::world_space_xz,
                              block_texture_mode::world_space_xz,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d0,
                              block_texture_rotation::d0,
                              block_texture_rotation::d0,
                              block_texture_rotation::d0,
                              block_texture_rotation::d0,
                              block_texture_rotation::d0,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                  },
               },
            .ids = {world::blocks_init,
                    std::initializer_list{block_box_id{0}, block_box_id{1},
                                          block_box_id{2}}},

         },
   };

   (void)io::create_directory("temp/world");

   save_blocks("temp/world/test.blk", blocks);

   const auto written_blk = io::read_file_to_string("temp/world/test.blk");

   CHECK(written_blk == expected_blk);
}

}