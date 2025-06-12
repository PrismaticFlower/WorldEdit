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
      SurfaceTextureMode(0, 0, 0, 0, 0, 0);
      SurfaceTextureRotation(2, 2, 2, 2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 256, 256, 0, 0);
      Layer(2);
   }
   Box()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2, 2, 2, 2, 2);
      SurfaceTextureMode(1, 1, 1, 1, 5, 1);
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
      SurfaceTextureMode(2, 2, 2, 2, 2, 2);
      SurfaceTextureRotation(0, 0, 0, 0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
}

Materials()
{
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
            .layer = {world::blocks_init, std::initializer_list<int8>{2, 0, 0}},
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
                              block_texture_mode::local_space_zy,
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

   (void)io::create_directory("temp/blocks");

   save_blocks("temp/blocks/test.blk", blocks);

   const auto written_blk = io::read_file_to_string("temp/blocks/test.blk");

   CHECK(written_blk == expected_blk);
}

TEST_CASE("world save blocks (ramps)", "[World][IO]")
{
   const std::string_view expected_blk = R"(Ramps(3)
{
   Ramp()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      SurfaceMaterials(0, 1, 2, 3, 4);
      SurfaceTextureMode(0, 0, 0, 0, 0);
      SurfaceTextureRotation(2, 2, 2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 256, 256);
      Layer(2);
   }
   Ramp()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2, 2, 2, 2);
      SurfaceTextureMode(1, 1, 1, 1, 5);
      SurfaceTextureRotation(1, 1, 1, 1, 1);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
   Ramp()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      SurfaceMaterials(0, 0, 0, 0, 0);
      SurfaceTextureMode(2, 2, 2, 2, 2);
      SurfaceTextureRotation(0, 0, 0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
}

Materials()
{
}

)";

   blocks blocks{
      .ramps =
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
            .layer = {world::blocks_init, std::initializer_list<int8>{2, 0, 0}},
            .description =
               {
                  world::blocks_init,
                  std::initializer_list{
                     world::block_description_ramp{
                        .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                        .position = {8.5f, 4.5f, 2.0f},
                        .size = {4.0f, 4.0f, 4.0f},
                        .surface_materials = {0, 1, 2, 3, 4},
                        .surface_texture_mode =
                           {
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
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-1, -2},
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
                           },
                     },
                     world::block_description_ramp{
                        .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                        .position = {10.0f, 16.0f, 12.0f},
                        .size = {8.0f, 4.0f, 8.0f},
                        .surface_materials = {2, 2, 2, 2, 2},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::local_space_zy,
                           },
                        .surface_texture_rotation =
                           {
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
                              std::array<int8, 2>{-2, -2},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{1024, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                     world::block_description_ramp{
                        .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                        .position = {6.0f, 6.0f, 6.0f},
                        .size = {5.0f, 5.0f, 5.0f},
                        .surface_materials = {0, 0, 0, 0, 0},
                        .surface_texture_mode =
                           {
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
                           },
                        .surface_texture_scale =
                           {
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
                           },
                     },
                  },
               },
            .ids = {world::blocks_init,
                    std::initializer_list{block_ramp_id{0}, block_ramp_id{1},
                                          block_ramp_id{2}}},

         },
   };

   (void)io::create_directory("temp/blocks");

   save_blocks("temp/blocks/test_ramps.blk", blocks);

   const auto written_blk =
      io::read_file_to_string("temp/blocks/test_ramps.blk");

   CHECK(written_blk == expected_blk);
}

TEST_CASE("world save blocks (quads)", "[World][IO]")
{
   const std::string_view expected_blk = R"(Quads(3)
{
   Quad()
   {
      Vertex0(0, 0, 0);
      Vertex1(1, 0, 0);
      Vertex2(1, 0, 1);
      Vertex3(0, 0, 1);
      QuadSplit(0);
      SurfaceMaterials(0);
      SurfaceTextureMode(0);
      SurfaceTextureRotation(2);
      SurfaceTextureScale(-1, -2);
      SurfaceTextureOffset(256, 256);
      Layer(2);
   }
   Quad()
   {
      Vertex0(0, 0, 0);
      Vertex1(1.5, 0, 0);
      Vertex2(1, 0, 1);
      Vertex3(0, 0, 1.5);
      QuadSplit(1);
      SurfaceMaterials(2);
      SurfaceTextureMode(1);
      SurfaceTextureRotation(1);
      SurfaceTextureScale(0, 0);
      SurfaceTextureOffset(1024, 0);
   }
   Quad()
   {
      Vertex0(0, 0, 0);
      Vertex1(8, 0, 0);
      Vertex2(8, 0, 8);
      Vertex3(0, 0, 8);
      QuadSplit(0);
      SurfaceMaterials(0);
      SurfaceTextureMode(2);
      SurfaceTextureRotation(0);
      SurfaceTextureScale(0, 0);
      SurfaceTextureOffset(0, 0);
   }
}

Materials()
{
}

)";

   blocks blocks{
      .quads =
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
            .layer = {world::blocks_init, std::initializer_list<int8>{2, 0, 0}},
            .description =
               {
                  world::blocks_init,
                  std::initializer_list{
                     world::block_description_quad{
                        .vertices =
                           {
                              float3{0.0f, 0.0f, 0.0f},
                              float3{1.0f, 0.0f, 0.0f},
                              float3{1.0f, 0.0f, 1.0f},
                              float3{0.0f, 0.0f, 1.0f},
                           },
                        .quad_split = world::block_quad_split::regular,
                        .surface_materials = {0},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_auto,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d180,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{-1, -2},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{256, 256},
                           },
                     },
                     world::block_description_quad{
                        .vertices =
                           {
                              float3{0.0f, 0.0f, 0.0f},
                              float3{1.5f, 0.0f, 0.0f},
                              float3{1.0f, 0.0f, 1.0f},
                              float3{0.0f, 0.0f, 1.5f},
                           },
                        .quad_split = world::block_quad_split::alternate,
                        .surface_materials = {2},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_zy,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d90,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{1024, 0},
                           },
                     },
                     world::block_description_quad{
                        .vertices =
                           {
                              float3{0.0f, 0.0f, 0.0f},
                              float3{8.0f, 0.0f, 0.0f},
                              float3{8.0f, 0.0f, 8.0f},
                              float3{0.0f, 0.0f, 8.0f},
                           },
                        .quad_split = world::block_quad_split::regular,
                        .surface_materials = {0},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_xz,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d0,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                  },
               },
            .ids = {world::blocks_init,
                    std::initializer_list{block_quad_id{0}, block_quad_id{1},
                                          block_quad_id{2}}},

         },
   };

   (void)io::create_directory("temp/blocks");

   save_blocks("temp/blocks/test_quads.blk", blocks);

   const auto written_blk =
      io::read_file_to_string("temp/blocks/test_quads.blk");

   CHECK(written_blk == expected_blk);
}

TEST_CASE("world save blocks (cylinders)", "[World][IO]")
{
   const std::string_view expected_blk = R"(Cylinders(3)
{
   Cylinder()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      SurfaceMaterials(0, 1, 2);
      SurfaceTextureMode(0, 0, 0);
      SurfaceTextureRotation(2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 256, 256);
      Layer(2);
   }
   Cylinder()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2, 2);
      SurfaceTextureMode(1, 1, 5);
      SurfaceTextureRotation(1, 1, 1);
      SurfaceTextureScale(0, 0, 0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0, 0, 0);
   }
   Cylinder()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      SurfaceMaterials(0, 0, 0);
      SurfaceTextureMode(2, 2, 2);
      SurfaceTextureRotation(0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0);
   }
}

Materials()
{
}

)";

   blocks blocks{
      .cylinders =
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
            .layer = {world::blocks_init, std::initializer_list<int8>{2, 0, 0}},
            .description =
               {
                  world::blocks_init,
                  std::initializer_list{
                     world::block_description_cylinder{
                        .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                        .position = {8.5f, 4.5f, 2.0f},
                        .size = {4.0f, 4.0f, 4.0f},
                        .surface_materials = {0, 1, 2},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_auto,
                              block_texture_mode::world_space_auto,
                              block_texture_mode::world_space_auto,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d180,
                              block_texture_rotation::d180,
                              block_texture_rotation::d180,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-1, -2},
                              std::array<int8, 2>{0, 0},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{256, 256},
                           },
                     },
                     world::block_description_cylinder{
                        .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                        .position = {10.0f, 16.0f, 12.0f},
                        .size = {8.0f, 4.0f, 8.0f},
                        .surface_materials = {2, 2, 2},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::local_space_zy,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d90,
                              block_texture_rotation::d90,
                              block_texture_rotation::d90,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-2, -2},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{1024, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                     world::block_description_cylinder{
                        .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                        .position = {6.0f, 6.0f, 6.0f},
                        .size = {5.0f, 5.0f, 5.0f},
                        .surface_materials = {0, 0, 0},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_xz,
                              block_texture_mode::world_space_xz,
                              block_texture_mode::world_space_xz,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d0,
                              block_texture_rotation::d0,
                              block_texture_rotation::d0,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                  },
               },
            .ids = {world::blocks_init,
                    std::initializer_list{block_cylinder_id{0}, block_cylinder_id{1}, block_cylinder_id{2}}},

         },
   };

   (void)io::create_directory("temp/blocks");

   save_blocks("temp/blocks/test_cylinders.blk", blocks);

   const auto written_blk =
      io::read_file_to_string("temp/blocks/test_cylinders.blk");

   CHECK(written_blk == expected_blk);
}

TEST_CASE("world save blocks (stairways)", "[World][IO]")
{
   const std::string_view expected_blk = R"(Stairways(3)
{
   Stairway()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      StepHeight(0.1);
      FirstStepOffset(0);
      SurfaceMaterials(0, 1, 2, 3, 4, 5);
      SurfaceTextureMode(0, 0, 0, 0, 0, 0);
      SurfaceTextureRotation(2, 2, 2, 2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 256, 256, 0, 0);
      Layer(2);
   }
   Stairway()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      StepHeight(0.25);
      FirstStepOffset(0.125);
      SurfaceMaterials(2, 2, 2, 2, 2, 2);
      SurfaceTextureMode(1, 1, 1, 1, 5, 1);
      SurfaceTextureRotation(1, 1, 1, 1, 1, 1);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
   Stairway()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      StepHeight(1);
      FirstStepOffset(0);
      SurfaceMaterials(0, 0, 0, 0, 0, 0);
      SurfaceTextureMode(2, 2, 2, 2, 2, 2);
      SurfaceTextureRotation(0, 0, 0, 0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
}

Materials()
{
}

)";

   blocks blocks{
      .stairways =
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
            .layer = {world::blocks_init, std::initializer_list<int8>{2, 0, 0}},
            .description =
               {
                  world::blocks_init,
                  std::initializer_list{
                     world::block_description_stairway{
                        .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                        .position = {8.5f, 4.5f, 2.0f},

                        .mesh_description =
                           world::block_custom_mesh_description_stairway{
                              .size = {4.0f, 4.0f, 4.0f},
                              .step_height = 0.1f,
                              .first_step_offset = 0.0f,
                           },

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
                     world::block_description_stairway{
                        .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                        .position = {10.0f, 16.0f, 12.0f},
                        .mesh_description =
                           world::block_custom_mesh_description_stairway{
                              .size = {8.0f, 4.0f, 8.0f},
                              .step_height = 0.25f,
                              .first_step_offset = 0.125f,
                           },
                        .surface_materials = {2, 2, 2, 2, 2, 2},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::local_space_zy,
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
                     world::block_description_stairway{
                        .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                        .position = {6.0f, 6.0f, 6.0f},
                        .mesh_description =
                           world::block_custom_mesh_description_stairway{
                              .size = {5.0f, 5.0f, 5.0f},
                              .step_height = 1.0f,
                              .first_step_offset = 0.0f,
                           },
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
            .mesh = {world::blocks_init,
                     std::initializer_list{
                        blocks_custom_mesh_library::null_handle(),
                        blocks_custom_mesh_library::null_handle(),
                        blocks_custom_mesh_library::null_handle(),
                     }},
            .ids = {world::blocks_init,
                    std::initializer_list{block_stairway_id{0}, block_stairway_id{1}, block_stairway_id{2}}},

         },
   };

   (void)io::create_directory("temp/blocks");

   save_blocks("temp/blocks/test_stairways.blk", blocks);

   const auto written_blk =
      io::read_file_to_string("temp/blocks/test_stairways.blk");

   CHECK(written_blk == expected_blk);
}

TEST_CASE("world save blocks (cones)", "[World][IO]")
{
   const std::string_view expected_blk = R"(Cones(3)
{
   Cone()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      SurfaceMaterials(0, 1);
      SurfaceTextureMode(0, 0);
      SurfaceTextureRotation(2, 2);
      SurfaceTextureScale(0, 0, -1, -2);
      SurfaceTextureOffset(0, 0, 256, 256);
      Layer(2);
   }
   Cone()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2);
      SurfaceTextureMode(1, 5);
      SurfaceTextureRotation(1, 1);
      SurfaceTextureScale(0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0);
   }
   Cone()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      SurfaceMaterials(0, 0);
      SurfaceTextureMode(2, 2);
      SurfaceTextureRotation(0, 0);
      SurfaceTextureScale(0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0);
   }
}

Materials()
{
}

)";

   blocks blocks{
      .cones =
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
            .layer = {world::blocks_init, std::initializer_list<int8>{2, 0, 0}},
            .description =
               {
                  world::blocks_init,
                  std::initializer_list{
                     world::block_description_cone{
                        .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                        .position = {8.5f, 4.5f, 2.0f},
                        .size = {4.0f, 4.0f, 4.0f},
                        .surface_materials = {0, 1},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_auto,
                              block_texture_mode::world_space_auto,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d180,
                              block_texture_rotation::d180,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-1, -2},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{256, 256},
                           },
                     },
                     world::block_description_cone{
                        .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                        .position = {10.0f, 16.0f, 12.0f},
                        .size = {8.0f, 4.0f, 8.0f},
                        .surface_materials = {2, 2},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_zy,
                              block_texture_mode::local_space_zy,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d90,
                              block_texture_rotation::d90,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-2, -2},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{1024, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                     world::block_description_cone{
                        .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                        .position = {6.0f, 6.0f, 6.0f},
                        .size = {5.0f, 5.0f, 5.0f},
                        .surface_materials = {0, 0},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_xz,
                              block_texture_mode::world_space_xz,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d0,
                              block_texture_rotation::d0,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                  },
               },
            .ids = {world::blocks_init,
                    std::initializer_list{block_cone_id{0}, block_cone_id{1},
                                          block_cone_id{2}}},

         },
   };

   (void)io::create_directory("temp/blocks");

   save_blocks("temp/blocks/test_cones.blk", blocks);

   const auto written_blk =
      io::read_file_to_string("temp/blocks/test_cones.blk");

   CHECK(written_blk == expected_blk);
}

TEST_CASE("world save blocks (hemispheres)", "[World][IO]")
{
   const std::string_view expected_blk = R"(Hemispheres(3)
{
   Hemisphere()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      SurfaceMaterials(0, 1);
      SurfaceTextureMode(0, 0);
      SurfaceTextureRotation(2, 2);
      SurfaceTextureScale(0, 0, -1, -2);
      SurfaceTextureOffset(0, 0, 256, 256);
      Layer(2);
   }
   Hemisphere()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2);
      SurfaceTextureMode(1, 5);
      SurfaceTextureRotation(1, 1);
      SurfaceTextureScale(0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0);
   }
   Hemisphere()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      SurfaceMaterials(0, 0);
      SurfaceTextureMode(2, 2);
      SurfaceTextureRotation(0, 0);
      SurfaceTextureScale(0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0);
   }
}

Materials()
{
}

)";

   blocks blocks{
      .hemispheres =
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
            .layer = {world::blocks_init, std::initializer_list<int8>{2, 0, 0}},
            .description =
               {
                  world::blocks_init,
                  std::initializer_list{
                     world::block_description_hemisphere{
                        .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                        .position = {8.5f, 4.5f, 2.0f},
                        .size = {4.0f, 4.0f, 4.0f},
                        .surface_materials = {0, 1},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_auto,
                              block_texture_mode::world_space_auto,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d180,
                              block_texture_rotation::d180,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-1, -2},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{256, 256},
                           },
                     },
                     world::block_description_hemisphere{
                        .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                        .position = {10.0f, 16.0f, 12.0f},
                        .size = {8.0f, 4.0f, 8.0f},
                        .surface_materials = {2, 2},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_zy,
                              block_texture_mode::local_space_zy,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d90,
                              block_texture_rotation::d90,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-2, -2},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{1024, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                     world::block_description_hemisphere{
                        .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                        .position = {6.0f, 6.0f, 6.0f},
                        .size = {5.0f, 5.0f, 5.0f},
                        .surface_materials = {0, 0},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_xz,
                              block_texture_mode::world_space_xz,
                           },
                        .surface_texture_rotation =
                           {
                              block_texture_rotation::d0,
                              block_texture_rotation::d0,
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{0, 0},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                  },
               },
            .ids = {world::blocks_init,
                    std::initializer_list{block_hemisphere_id{0}, block_hemisphere_id{1},
                                          block_hemisphere_id{2}}},

         },
   };

   (void)io::create_directory("temp/blocks");

   save_blocks("temp/blocks/test_hemispheres.blk", blocks);

   const auto written_blk =
      io::read_file_to_string("temp/blocks/test_hemispheres.blk");

   CHECK(written_blk == expected_blk);
}

TEST_CASE("world save blocks (pyramids)", "[World][IO]")
{
   const std::string_view expected_blk = R"(Pyramids(3)
{
   Pyramid()
   {
      Rotation(0, 1, 0, 0);
      Position(8.5, 4.5, 2);
      Size(4, 4, 4);
      SurfaceMaterials(0, 1, 2, 3, 4);
      SurfaceTextureMode(0, 0, 0, 0, 0);
      SurfaceTextureRotation(2, 2, 2, 2, 2);
      SurfaceTextureScale(0, 0, -1, -2, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 256, 256);
      Layer(2);
   }
   Pyramid()
   {
      Rotation(0.707106, 0, 0.707106, 0);
      Position(10, 16, 12);
      Size(8, 4, 8);
      SurfaceMaterials(2, 2, 2, 2, 2);
      SurfaceTextureMode(1, 1, 1, 1, 5);
      SurfaceTextureRotation(1, 1, 1, 1, 1);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, -2, -2);
      SurfaceTextureOffset(1024, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
   Pyramid()
   {
      Rotation(0, 0, 0, 1);
      Position(6, 6, 6);
      Size(5, 5, 5);
      SurfaceMaterials(0, 0, 0, 0, 0);
      SurfaceTextureMode(2, 2, 2, 2, 2);
      SurfaceTextureRotation(0, 0, 0, 0, 0);
      SurfaceTextureScale(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      SurfaceTextureOffset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   }
}

Materials()
{
}

)";

   blocks blocks{
      .pyramids =
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
            .layer = {world::blocks_init, std::initializer_list<int8>{2, 0, 0}},
            .description =
               {
                  world::blocks_init,
                  std::initializer_list{
                     world::block_description_pyramid{
                        .rotation = {0.0f, 1.0f, 0.0f, 0.0f},
                        .position = {8.5f, 4.5f, 2.0f},
                        .size = {4.0f, 4.0f, 4.0f},
                        .surface_materials = {0, 1, 2, 3, 4},
                        .surface_texture_mode =
                           {
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
                           },
                        .surface_texture_scale =
                           {
                              std::array<int8, 2>{0, 0},
                              std::array<int8, 2>{-1, -2},
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
                           },
                     },
                     world::block_description_pyramid{
                        .rotation = {0.707106f, 0.0f, 0.707106f, 0.0f},
                        .position = {10.0f, 16.0f, 12.0f},
                        .size = {8.0f, 4.0f, 8.0f},
                        .surface_materials = {2, 2, 2, 2, 2},
                        .surface_texture_mode =
                           {
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::world_space_zy,
                              block_texture_mode::local_space_zy,
                           },
                        .surface_texture_rotation =
                           {
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
                              std::array<int8, 2>{-2, -2},
                           },
                        .surface_texture_offset =
                           {
                              std::array<uint16, 2>{1024, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                              std::array<uint16, 2>{0, 0},
                           },
                     },
                     world::block_description_pyramid{
                        .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                        .position = {6.0f, 6.0f, 6.0f},
                        .size = {5.0f, 5.0f, 5.0f},
                        .surface_materials = {0, 0, 0, 0, 0},
                        .surface_texture_mode =
                           {
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
                           },
                        .surface_texture_scale =
                           {
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
                           },
                     },
                  },
               },
            .ids = {world::blocks_init,
                    std::initializer_list{block_pyramid_id{0}, block_pyramid_id{1},
                                          block_pyramid_id{2}}},

         },
   };

   (void)io::create_directory("temp/blocks");

   save_blocks("temp/blocks/test_pyramids.blk", blocks);

   const auto written_blk =
      io::read_file_to_string("temp/blocks/test_pyramids.blk");

   CHECK(written_blk == expected_blk);
}

TEST_CASE("world save blocks (materials)", "[World][IO]")
{
   const std::string_view expected_blk = R"(Materials()
{
   Material(0)
   {
      Name("rocks");
      DiffuseMap("rocks_diffuse");
      NormalMap("rocks_normal");
      DetailMap("rocks_detail");
      EnvMap("skycube");
      DetailTiling(4, 4);
      TileNormalMap(0);
      SpecularLighting(1);
      SpecularColor(0.5, 0.5, 0.5);
      FoleyFXGroup(0);
   }
   Material(2)
   {
      Name("snow");
      DiffuseMap("snow_diffuse");
      NormalMap("snow_detail_normal");
      DetailMap("snow_detail");
      EnvMap("skycube");
      DetailTiling(3, 3);
      TileNormalMap(1);
      SpecularLighting(1);
      SpecularColor(0.75, 0.75, 0.75);
      FoleyFXGroup(4);
   }
}

)";

   blocks blocks;

   blocks.materials[0] = {
      .name = "rocks",

      .diffuse_map = "rocks_diffuse",
      .normal_map = "rocks_normal",
      .detail_map = "rocks_detail",
      .env_map = "skycube",

      .detail_tiling = {4, 4},
      .tile_normal_map = false,
      .specular_lighting = true,

      .specular_color = {0.5f, 0.5f, 0.5f},
   };

   blocks.materials[2] = {
      .name = "snow",

      .diffuse_map = "snow_diffuse",
      .normal_map = "snow_detail_normal",
      .detail_map = "snow_detail",
      .env_map = "skycube",

      .detail_tiling = {3, 3},
      .tile_normal_map = true,
      .specular_lighting = true,

      .specular_color = {0.75f, 0.75f, 0.75f},

      .foley_group = block_foley_group::snow,
   };

   (void)io::create_directory("temp/blocks");

   save_blocks("temp/blocks/test_materials.blk", blocks);

   const auto written_blk =
      io::read_file_to_string("temp/blocks/test_materials.blk");

   CHECK(written_blk == expected_blk);
}
}