#include "pch.h"

#include "munge/builtin/model_munge.hpp"
#include "munge/feedback.hpp"

#include "assets/option_file.hpp"

#include "io/read_file.hpp"

namespace we::munge::tests {

namespace {

bool golden_ref_test(const std::string_view file_name, const uint64 offset)
{
   const io::path input_path = R"(data\munge\model)";
   const io::path output_path = io::compose_path(R"(temp\munge\model)", file_name);

   const io::path output_file_path =
      io::compose_path(output_path, file_name, ".model");

   (void)io::create_directories(output_path);
   (void)io::remove(output_file_path);

   std::shared_ptr<async::thread_pool> thread_pool = async::thread_pool::make(
      async::thread_pool_init{.thread_count = 1, .low_priority_thread_count = 1});
   output output;
   munge_feedback feedback{output, output};
   tool_context context = {.output_path = output_path,
                           .feedback = feedback,
                           .thread_pool = *thread_pool};

   execute_model_munge(io::compose_path(input_path, file_name, ".msh"), {}, context);

   const std::vector<std::byte> expected_bytes =
      io::read_file_to_bytes(io::compose_path(input_path, file_name, ".part"));
   const std::vector<std::byte> written_bytes =
      io::read_file_to_bytes(output_file_path);

   if (offset > written_bytes.size()) return false;
   if (offset + expected_bytes.size() > written_bytes.size()) return false;

   return std::memcmp(written_bytes.data() + offset, expected_bytes.data(),
                      expected_bytes.size()) == 0;
}

}

TEST_CASE("model_munge skeleton_basic", "[Munge]")
{
   REQUIRE(golden_ref_test("skeleton_basic", 8));
}

TEST_CASE("model_munge skeleton_bln2", "[Munge]")
{
   REQUIRE(golden_ref_test("skeleton_bln2", 8));
}

TEST_CASE("model_munge skeleton_cloth_collision", "[Munge]")
{
   REQUIRE(golden_ref_test("skeleton_cloth_collision", 8));
}

TEST_CASE("model_munge skeleton_keep", "[Munge]")
{
   REQUIRE(golden_ref_test("skeleton_keep", 8));
}

TEST_CASE("model_munge skeleton_keepall", "[Munge]")
{
   REQUIRE(golden_ref_test("skeleton_keepall", 8));
}

TEST_CASE("model_munge skeleton_hp", "[Munge]")
{
   REQUIRE(golden_ref_test("skeleton_hp", 8));
}

TEST_CASE("model_munge skeleton_hp_unkept_parent", "[Munge]")
{
   REQUIRE(golden_ref_test("skeleton_hp_unkept_parent", 8));
}

TEST_CASE("model_munge skeleton_unkept_bone", "[Munge]")
{
   REQUIRE(golden_ref_test("skeleton_unkept_bone", 8));
}

TEST_CASE("model_munge skeleton_unkept_bone2", "[Munge]")
{
   REQUIRE(golden_ref_test("skeleton_unkept_bone2", 8));
}

TEST_CASE("model_munge model_basic", "[Munge]")
{
   REQUIRE(golden_ref_test("model_basic", 488));
}

TEST_CASE("model_munge model_bone_geom", "[Munge]")
{
   REQUIRE(golden_ref_test("model_bone_geom", 372));
}

TEST_CASE("model_munge model_bone_geom_wght", "[Munge]")
{
   REQUIRE(golden_ref_test("model_bone_geom_wght", 500));
}

TEST_CASE("model_munge model_boundingboxoffset", "[Munge]")
{
   REQUIRE(golden_ref_test("model_boundingboxoffset", 1048));
}

TEST_CASE("model_munge model_boundingboxoffsetnz", "[Munge]")
{
   REQUIRE(golden_ref_test("model_boundingboxoffsetnz", 1056));
}

TEST_CASE("model_munge model_boundingboxscale", "[Munge]")
{
   REQUIRE(golden_ref_test("model_boundingboxscale", 1048));
}

TEST_CASE("model_munge model_wght", "[Munge]")
{
   REQUIRE(golden_ref_test("model_wght", 488));
}

TEST_CASE("model_munge model_wght_softskin", "[Munge]")
{
   REQUIRE(golden_ref_test("model_wght_softskin", 496));
}

TEST_CASE("model_munge model_clrl", "[Munge]")
{
   REQUIRE(golden_ref_test("model_clrl", 488));
}

TEST_CASE("model_munge model_clrl_ambientlighting", "[Munge]")
{
   REQUIRE(golden_ref_test("model_clrl_ambientlighting", 856));
}

TEST_CASE("model_munge model_clrl_nomerge", "[Munge]")
{
   REQUIRE(golden_ref_test("model_clrl_nomerge", 496));
}

TEST_CASE("model_munge model_clrl_vertexlighting", "[Munge]")
{
   REQUIRE(golden_ref_test("model_clrl_vertexlighting", 504));
}

TEST_CASE("model_munge model_lods", "[Munge]")
{
   REQUIRE(golden_ref_test("model_lods", 124));
}

TEST_CASE("model_munge model_lowres_only", "[Munge]")
{
   REQUIRE(golden_ref_test("model_lowres_only", 700));
}

TEST_CASE("model_munge model_mtrl_00_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_00", 304));
}

TEST_CASE("model_munge model_mtrl_00_00_attachlight", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_00_attachlight", 328));
}

TEST_CASE("model_munge model_mtrl_00_00_bump", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_00_bump", 312));
}

TEST_CASE("model_munge model_mtrl_00_00_keepmaterial", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_00_keepmaterial", 328));
}

TEST_CASE("model_munge model_mtrl_00_00_reflected", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_00_reflected", 344));
}

TEST_CASE("model_munge model_mtrl_00_00_reflective", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_00_reflective", 344));
}

TEST_CASE("model_munge model_mtrl_00_01", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_01", 304));
}

TEST_CASE("model_munge model_mtrl_00_02", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_02", 304));
}

TEST_CASE("model_munge model_mtrl_00_04", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_04", 304));
}

TEST_CASE("model_munge model_mtrl_00_05", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_05", 304));
}

TEST_CASE("model_munge model_mtrl_00_06", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_06", 304));
}

TEST_CASE("model_munge model_mtrl_00_07", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_07", 304));
}

TEST_CASE("model_munge model_mtrl_00_08", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_08", 304));
}

TEST_CASE("model_munge model_mtrl_00_09", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_09", 304));
}

TEST_CASE("model_munge model_mtrl_00_0A", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_0A", 304));
}

TEST_CASE("model_munge model_mtrl_00_0B", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_0B", 304));
}

TEST_CASE("model_munge model_mtrl_00_0C", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_0C", 304));
}

TEST_CASE("model_munge model_mtrl_00_0D", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_0D", 304));
}

TEST_CASE("model_munge model_mtrl_00_0E", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_0E", 304));
}

TEST_CASE("model_munge model_mtrl_00_0F", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_0F", 304));
}

TEST_CASE("model_munge model_mtrl_00_10", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_10", 304));
}

TEST_CASE("model_munge model_mtrl_00_11", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_11", 304));
}

TEST_CASE("model_munge model_mtrl_00_12", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_12", 304));
}

TEST_CASE("model_munge model_mtrl_00_13", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_13", 304));
}

TEST_CASE("model_munge model_mtrl_00_14", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_14", 304));
}

TEST_CASE("model_munge model_mtrl_00_15", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_15", 304));
}

TEST_CASE("model_munge model_mtrl_00_16", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_16", 304));
}

TEST_CASE("model_munge model_mtrl_00_17", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_17", 304));
}

TEST_CASE("model_munge model_mtrl_00_18", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_18", 304));
}

TEST_CASE("model_munge model_mtrl_00_19", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_19", 304));
}

TEST_CASE("model_munge model_mtrl_00_1A", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_1A", 304));
}

TEST_CASE("model_munge model_mtrl_00_1B", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_1B", 304));
}

TEST_CASE("model_munge model_mtrl_00_1C", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_1C", 304));
}

TEST_CASE("model_munge model_mtrl_00_1D", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_00_1D", 304));
}

TEST_CASE("model_munge model_mtrl_01_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_01_00", 304));
}

TEST_CASE("model_munge model_mtrl_01_00_additiveemissive", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_01_00_additiveemissive", 336));
}

TEST_CASE("model_munge model_mtrl_02_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_02_00", 304));
}

TEST_CASE("model_munge model_mtrl_04_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_04_00", 304));
}

TEST_CASE("model_munge model_mtrl_04_04", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_04_04", 304));
}

TEST_CASE("model_munge model_mtrl_04_1C", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_04_1C", 304));
}

TEST_CASE("model_munge model_mtrl_08_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_08_00", 304));
}

TEST_CASE("model_munge model_mtrl_10_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_10_00", 304));
}

TEST_CASE("model_munge model_mtrl_20_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_20_00", 304));
}

TEST_CASE("model_munge model_mtrl_40_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_40_00", 304));
}

TEST_CASE("model_munge model_mtrl_80_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_80_00", 304));
}

TEST_CASE("model_munge model_mtrl_84_00", "[Munge]")
{
   REQUIRE(golden_ref_test("model_mtrl_84_00", 304));
}

TEST_CASE("model_munge model_noprojectionlights", "[Munge]")
{
   REQUIRE(golden_ref_test("model_noprojectionlights", 504));
}

TEST_CASE("model_munge model_req", "[Munge]")
{
   const std::string_view file_name = "model_req";
   const io::path input_path = R"(data\munge\model)";
   const io::path output_path = io::compose_path(R"(temp\munge\model)", file_name);

   const io::path output_file_path =
      io::compose_path(output_path, file_name, ".model.req");

   (void)io::create_directories(output_path);
   (void)io::remove(output_file_path);

   std::shared_ptr<async::thread_pool> thread_pool = async::thread_pool::make(
      async::thread_pool_init{.thread_count = 1, .low_priority_thread_count = 1});
   output output;
   munge_feedback feedback{output, output};
   tool_context context = {.output_path = output_path,
                           .feedback = feedback,
                           .thread_pool = *thread_pool};

   execute_model_munge(io::compose_path(input_path, file_name, ".msh"), {}, context);

   const std::string expected_req =
      io::read_file_to_string(io::compose_path(input_path, file_name, ".model.req"));
   const std::string written_req = io::read_file_to_string(output_file_path);

   REQUIRE(expected_req == written_req);
}

TEST_CASE("model_munge model_scale", "[Munge]")
{
   REQUIRE(golden_ref_test("model_scale", 0));
}

TEST_CASE("model_munge model_segments_merge", "[Munge]")
{
   REQUIRE(golden_ref_test("model_segments_merge", 136));
}

TEST_CASE("model_munge model_shadow_cube", "[Munge]")
{
   REQUIRE(golden_ref_test("model_shadow_cube", 136));
}

TEST_CASE("model_munge model_shadow_hiresshadow", "[Munge]")
{
   REQUIRE(golden_ref_test("model_shadow_hiresshadow", 140));
}

TEST_CASE("model_munge model_shadow_open", "[Munge]")
{
   REQUIRE(golden_ref_test("model_shadow_open", 136));
}

TEST_CASE("model_munge model_shadow_skinned", "[Munge]")
{
   REQUIRE(golden_ref_test("model_shadow_skinned", 1748));
}

TEST_CASE("model_munge model_shadow_skinned_softskin", "[Munge]")
{
   REQUIRE(golden_ref_test("model_shadow_skinned_softskin", 1756));
}

TEST_CASE("model_munge model_shadow_tri", "[Munge]")
{
   REQUIRE(golden_ref_test("model_shadow_tri", 136));
}

TEST_CASE("model_munge model_shadow_unhidden", "[Munge]")
{
   REQUIRE(golden_ref_test("model_shadow_unhidden", 140));
}

TEST_CASE("model_munge model_shadowvolume", "[Munge]")
{
   REQUIRE(golden_ref_test("model_shadowvolume", 136));
}

TEST_CASE("model_munge model_skin_bone_parent", "[Munge]")
{
   // This case results in a slightly smaller bounding sphere than what modelmunge produces. It however is
   // still oversized and I find it hard to believe it'll cause any issues. But if there end up being issues with
   // the bounding sphere here (and model_munge model_skin_transform) is a good place to revisit.

   REQUIRE(golden_ref_test("model_skin_bone_parent", 380));
}

TEST_CASE("model_munge model_skin_split_softskin", "[Munge]")
{
   REQUIRE(golden_ref_test("model_skin_split_softskin", 1752));
}

TEST_CASE("model_munge model_skin_split", "[Munge]")
{
   REQUIRE(golden_ref_test("model_skin_split", 1688));
}

TEST_CASE("model_munge model_skin_split_generate_tangents", "[Munge]")
{
   REQUIRE(golden_ref_test("model_skin_split_generate_tangents", 1704));
}

TEST_CASE("model_munge model_skin_split2", "[Munge]")
{
   REQUIRE(golden_ref_test("model_skin_split2", 1744));
}

TEST_CASE("model_munge model_skin_transform", "[Munge]")
{
   REQUIRE(golden_ref_test("model_skin_transform", 500));
}

TEST_CASE("model_munge model_skinned_segments_merge", "[Munge]")
{
   REQUIRE(golden_ref_test("model_skinned_segments_merge", 508));
}

TEST_CASE("model_munge model_softskin_split", "[Munge]")
{
   REQUIRE(golden_ref_test("model_softskin_split", 1692));
}

TEST_CASE("model_munge model_tangents", "[Munge]")
{
   REQUIRE(golden_ref_test("model_tangents", 128));
}

TEST_CASE("model_munge model_skip", "[Munge]")
{
   REQUIRE(golden_ref_test("model_skip", 544));
}

TEST_CASE("model_munge game_model_basic", "[Munge]")
{
   REQUIRE(golden_ref_test("game_model_basic", 2416));
}

TEST_CASE("model_munge game_model_lodbias", "[Munge]")
{
   REQUIRE(golden_ref_test("game_model_lodbias", 2416));
}

TEST_CASE("model_munge game_model_lodgroup_bigmodel", "[Munge]")
{
   REQUIRE(golden_ref_test("game_model_lodgroup_bigmodel", 2476));
}

TEST_CASE("model_munge game_model_lodgroup_hugemodel", "[Munge]")
{
   REQUIRE(golden_ref_test("game_model_lodgroup_hugemodel", 2476));
}

TEST_CASE("model_munge game_model_lodgroup_soldier", "[Munge]")
{
   REQUIRE(golden_ref_test("game_model_lodgroup_soldier", 2456));
}

TEST_CASE("model_munge collision_basic", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_basic", 308));
}

TEST_CASE("model_munge collision_bbox", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_bbox", 196));
}

TEST_CASE("model_munge collision_flags_b", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_flags_b", 888));
}

TEST_CASE("model_munge collision_flags_f", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_flags_f", 888));
}

TEST_CASE("model_munge collision_flags_o", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_flags_o", 888));
}

TEST_CASE("model_munge collision_flags_s", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_flags_s", 888));
}

TEST_CASE("model_munge collision_flags_svbtof", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_flags_svbtof", 908));
}

TEST_CASE("model_munge collision_flags_svbtof2", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_flags_svbtof2", 908));
}

TEST_CASE("model_munge collision_flags_t", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_flags_t", 888));
}

TEST_CASE("model_munge collision_flags_v", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_flags_v", 888));
}

TEST_CASE("model_munge collision_decagon", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_decagon", 1752));
}

TEST_CASE("model_munge collision_grid", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_grid", 2012));
}

TEST_CASE("model_munge collision_huge_circle", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_huge_circle", 18028));
}

TEST_CASE("model_munge collision_quad", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_quad", 836));
}

TEST_CASE("model_munge collision_quad_donotmergecollision", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_quad_donotmergecollision", 916));
}

TEST_CASE("model_munge collision_tri", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_tri", 784));
}

TEST_CASE("model_munge collision_tri_pair", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_tri_pair", 948));
}

TEST_CASE("model_munge collision_vrtx", "[Munge]")
{
   REQUIRE(golden_ref_test("collision_vrtx", 160));
}

TEST_CASE("model_munge cprimitive_basic", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_basic", 312));
}

TEST_CASE("model_munge cprimitive_bbox_box", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_bbox_box", 204));
}

TEST_CASE("model_munge cprimitive_bbox_cylinder", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_bbox_cylinder", 220));
}

TEST_CASE("model_munge cprimitive_bbox_sphere", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_bbox_sphere", 212));
}

TEST_CASE("model_munge cprimitive_flags_b", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_flags_b", 192));
}

TEST_CASE("model_munge cprimitive_flags_f", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_flags_f", 192));
}

TEST_CASE("model_munge cprimitive_flags_o", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_flags_o", 192));
}

TEST_CASE("model_munge cprimitive_flags_s", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_flags_s", 192));
}

TEST_CASE("model_munge cprimitive_flags_svbtof", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_flags_svbtof", 204));
}

TEST_CASE("model_munge cprimitive_flags_svbtof2", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_flags_svbtof2", 212));
}

TEST_CASE("model_munge cprimitive_flags_t", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_flags_t", 192));
}

TEST_CASE("model_munge cprimitive_flags_v", "[Munge]")
{
   REQUIRE(golden_ref_test("cprimitive_flags_v", 192));
}

TEST_CASE("model_munge cloth_basic", "[Munge]")
{
   REQUIRE(golden_ref_test("cloth_basic", 992));
}

TEST_CASE("model_munge cloth_no_model", "[Munge]")
{
   REQUIRE(golden_ref_test("cloth_no_model", 436));
}

TEST_CASE("model_munge cloth_nofixedweights", "[Munge]")
{
   REQUIRE(golden_ref_test("cloth_nofixedweights", 1128));
}

TEST_CASE("model_munge cloth_reparent", "[Munge]")
{
   REQUIRE(golden_ref_test("cloth_reparent", 900));
}

TEST_CASE("model_munge cloth_scale", "[Munge]")
{
   REQUIRE(golden_ref_test("cloth_scale", 1080));
}

}