#include "pch.h"

#include "munge/builtin/odf_munge.hpp"
#include "munge/feedback.hpp"

#include "io/read_file.hpp"

namespace we::munge::tests {

namespace {

bool golden_ref_test(const std::string_view file_name)
{
   const io::path input_path = R"(data\munge\odf)";
   const io::path output_path = io::compose_path(R"(temp\munge\odf)", file_name);

   const io::path output_file_path =
      io::compose_path(output_path, file_name, ".class");

   (void)io::create_directories(output_path);
   (void)io::remove(output_file_path);

   std::shared_ptr<async::thread_pool> thread_pool = async::thread_pool::make(
      async::thread_pool_init{.thread_count = 1, .low_priority_thread_count = 1});
   output output;
   munge_feedback feedback{output, output};
   tool_context context = {.output_path = output_path,
                           .feedback = feedback,
                           .thread_pool = *thread_pool};

   execute_odf_munge(io::compose_path(input_path, file_name, ".odf"), context);

   return io::read_file_to_bytes(output_file_path) ==
          io::read_file_to_bytes(io::compose_path(input_path, file_name, ".class"));
}

bool golden_ref_req_test(const std::string_view file_name)
{
   const io::path input_path = R"(data\munge\odf)";
   const io::path output_path = io::compose_path(R"(temp\munge\odf)", file_name);

   const io::path output_file_path =
      io::compose_path(output_path, file_name, ".class.req");

   (void)io::create_directories(output_path);
   (void)io::remove(output_file_path);

   std::shared_ptr<async::thread_pool> thread_pool = async::thread_pool::make(
      async::thread_pool_init{.thread_count = 1, .low_priority_thread_count = 1});
   output output;
   munge_feedback feedback{output, output};
   tool_context context = {.output_path = output_path,
                           .feedback = feedback,
                           .thread_pool = *thread_pool};

   execute_odf_munge(io::compose_path(input_path, file_name, ".odf"), context);

   return io::read_file_to_string(output_file_path) ==
          io::read_file_to_string(io::compose_path(input_path, file_name, ".class.req"));
}

}

TEST_CASE("odf_munge basic", "[Munge]")
{
   REQUIRE(golden_ref_test("basic"));
}

TEST_CASE("odf_munge explosion", "[Munge]")
{
   REQUIRE(golden_ref_test("explosion"));
}

TEST_CASE("odf_munge ordnance", "[Munge]")
{
   REQUIRE(golden_ref_test("ordnance"));
}

TEST_CASE("odf_munge weapon", "[Munge]")
{
   REQUIRE(golden_ref_test("weapon"));
}

TEST_CASE("odf_munge instance_properties", "[Munge]")
{
   REQUIRE(golden_ref_test("instance_properties"));
}

TEST_CASE("odf_munge parented", "[Munge]")
{
   REQUIRE(golden_ref_test("parented"));
}

TEST_CASE("odf_munge empty_property", "[Munge]")
{
   REQUIRE(golden_ref_test("empty_property"));
}

TEST_CASE("odf_munge req_test", "[Munge]")
{
   REQUIRE(golden_ref_req_test("req_fill"));
}

TEST_CASE("odf_munge parented_req", "[Munge]")
{
   REQUIRE(golden_ref_req_test("parented_req"));
}

TEST_CASE("odf_munge numeric_no_req", "[Munge]")
{
   REQUIRE(golden_ref_req_test("numeric_no_req"));
}

TEST_CASE("odf_munge underscore_req", "[Munge]")
{
   REQUIRE(golden_ref_req_test("underscore_req"));
}

TEST_CASE("odf_munge req_fill_trailing_space", "[Munge]")
{
   REQUIRE(golden_ref_req_test("req_fill_trailing_space"));
}

TEST_CASE("odf_munge req_fill_double_tokens", "[Munge]")
{
   REQUIRE(golden_ref_req_test("req_fill_double_tokens"));
}

}