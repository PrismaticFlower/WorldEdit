#include "pch.h"

#include "io/output_file.hpp"
#include "io/read_file.hpp"

using namespace std::literals;

namespace we::io::tests {

constexpr auto test_file_path = L"temp/output_file.txt"sv;

TEST_CASE("output file create mode", "[IO][OutputFile]")
{
   // write out file
   {
      output_file file{test_file_path, output_open_mode::create};

      const char array[] = "Raw Data";

      file.write_ln("Hello World!");
      file.write_ln("number: {}", 37);
      file.write("This is another line.\n");
      file.write("another number: {}\n", 64);
      file.write(std::as_bytes(std::span{array}));
   }

   const auto written_contents = io::read_file_to_string(test_file_path);
   const auto expected_contents = "Hello World!\n"
                                  "number: 37\n"
                                  "This is another line.\n"
                                  "another number: 64\n"
                                  "Raw Data\0"sv;

   REQUIRE(written_contents == expected_contents);
}

TEST_CASE("output file append mode", "[IO][OutputFile]")
{
   // write out file
   {
      output_file file{test_file_path, output_open_mode::create};

      file.write("Hello ");
   }

   // append to out file
   {
      output_file file{test_file_path, output_open_mode::append};

      file.write("World!");
   }

   const auto written_contents = io::read_file_to_string(test_file_path);
   const auto expected_contents = "Hello World!"sv;

   REQUIRE(written_contents == expected_contents);
}

TEST_CASE("output file trivially copyable object", "[IO][OutputFile]")
{
   struct test_struct {
      int number = 32;
      float number_as_a_float = 32.0f;
      unsigned long long huge_number = 0ull - 1ull; // compiler warnings, grrr

      bool operator==(const test_struct&) const noexcept = default;
   };

   // write out file
   {
      output_file file{test_file_path, output_open_mode::create};

      test_struct test_data;

      file.write_object(test_data);
   }

   const auto written_contents = io::read_file_to_bytes(test_file_path);

   test_struct read;
   std::memcpy(&read, written_contents.data(), written_contents.size());

   test_struct expected;

   REQUIRE(read == expected);
}

}
