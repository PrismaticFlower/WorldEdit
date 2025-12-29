#include "pch.h"

#include "assets/option_file.hpp"

#include "munge/builtin/texture_munge.hpp"
#include "munge/feedback.hpp"

#include "io/read_file.hpp"

namespace we::munge::tests {

namespace {

bool golden_ref_test(const std::string_view file_name)
{
   const io::path input_path = R"(data\munge\textures)";
   const io::path output_path = io::compose_path(R"(temp\munge\textures)", file_name);

   const io::path output_file_path =
      io::compose_path(output_path, file_name, ".texture");

   (void)io::create_directories(output_path);
   (void)io::remove(output_file_path);

   std::shared_ptr<async::thread_pool> thread_pool = async::thread_pool::make(
      async::thread_pool_init{.thread_count = 1, .low_priority_thread_count = 1});
   output output;
   munge_feedback feedback{output, output};
   tool_context context = {.output_path = output_path,
                           .feedback = feedback,
                           .thread_pool = *thread_pool};

   execute_texture_munge(io::compose_path(input_path, file_name, ".tga"), {}, context);

   return io::read_file_to_bytes(output_file_path) ==
          io::read_file_to_bytes(io::compose_path(input_path, file_name, ".texture"));
}

}

TEST_CASE("texture_munge 4x4_0080ff", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff"));
}

TEST_CASE("texture_munge 4x4_0080ff_maps1", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_maps1"));
}

TEST_CASE("texture_munge 4x4_0080ff_uborder", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_uborder"));
}

TEST_CASE("texture_munge 4x4_0080ff_vborder", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_vborder"));
}

TEST_CASE("texture_munge 4x4_0080ff_alphaborder", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_alphaborder"));
}

TEST_CASE("texture_munge 4x4_0080ff_saturation", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_saturation"));
}

TEST_CASE("texture_munge 4x4_80_bumpmap", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_80_bumpmap"));
}

TEST_CASE("texture_munge 4x4_80_hiqbumpmap", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_80_hiqbumpmap"));
}

TEST_CASE("texture_munge 4x4_ff0000_overridenzto1", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_ff0000_overridenzto1"));
}

TEST_CASE("texture_munge 4x4_0080ff_detailbias", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_detailbias"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_detail", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_detail"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_detail_from_alpha", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_detail_from_alpha"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_bump", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_bump"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_compressed", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_compressed"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_dxt1", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_dxt1"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_dxt3", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_dxt3"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_dxt5", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_dxt5"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_dxt1_alpha", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_dxt1_alpha"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_x8r8g8b8", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_x8r8g8b8"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_a4r4g4b4", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_a4r4g4b4"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_a1r5g5b5", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_a1r5g5b5"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_x1r5g5b5", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_x1r5g5b5"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_r5g6b5", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_r5g6b5"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_a8l8", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_a8l8"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_a8", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_a8"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_l8", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_l8"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_a4l4", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_a4l4"));
}

TEST_CASE("texture_munge 4x4_0080ff_format_v8u8", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_0080ff_format_v8u8"));
}

TEST_CASE("texture_munge 4x4_cube", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4_cube"));
}

TEST_CASE("texture_munge 4x4x4_volume", "[Munge]")
{
   REQUIRE(golden_ref_test("4x4x4_volume"));
}

TEST_CASE("texture_munge 8x8_0080ff_format_dxt1", "[Munge]")
{
   REQUIRE(golden_ref_test("8x8_0080ff_format_dxt1"));
}

}