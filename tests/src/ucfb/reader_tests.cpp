
#include "pch.h"

#include "ucfb/reader.hpp"
#include "utility/read_file.hpp"

using namespace std::literals;

namespace sk::ucfb::tests {

TEST_CASE("ucfb reading", "[ucfb][reader]")
{
   const auto bytes = utility::read_file_to_bytes("data/ucfb_read_test.lvl");

   ucfb::reader reader{bytes};

   SECTION("reader id and size matches expected")
   {
      REQUIRE(reader.id() == "ucfb"_id);
      REQUIRE(reader.size() == 1204);
   }

   SECTION("children tests")
   {
      // reading a child chunk is fine
      REQUIRE_NOTHROW(reader.read_child());

      // reading past the end raises an exception
      REQUIRE_THROWS(reader.read_child());

      // but resetting the head let's a us read the child again
      reader.reset_head();

      // child reads can also be strict
      REQUIRE_NOTHROW(reader.read_child_strict<"SHDR"_id>());

      reader.reset_head();

      // failed strict reads throw
      REQUIRE_THROWS(reader.read_child_strict<"BAD_"_id>());
   }

   SECTION("data read tests")
   {
      auto shdr = reader.read_child_strict<"SHDR"_id>();

      auto rtyp = shdr.read_child_strict<"RTYP"_id>();
      REQUIRE(rtyp.read_string() == "zprepass"sv);

      shdr.read_child_strict<"NAME"_id>();

      auto info = shdr.read_child_strict<"INFO"_id>();

      REQUIRE(info.read<uint32>() == 1);
      REQUIRE(info.read<uint32>() == 26);
      REQUIRE_THROWS(info.read<uint32>() == 0);

      info.reset_head();

      REQUIRE(info.read_multi<uint32, uint32>() == std::tuple<uint32, uint32>{1, 26});
   }

   SECTION("alignment tests")
   {
      auto shdr = reader.read_child_strict<"SHDR"_id>();
      shdr.read_child_strict<"RTYP"_id>();
      shdr.read_child_strict<"NAME"_id>();

      auto info = shdr.read_child_strict<"INFO"_id>();

      // the read head is aligned to four bytes after a read
      REQUIRE(info.read<uint16>() == 1);
      REQUIRE(info.read<uint16>() == 26);
      REQUIRE_THROWS(info.read<uint32>() == 0);

      info.reset_head();

      // unless we use unaligned reads
      REQUIRE(info.read_unaligned<uint16>() == 1);
      REQUIRE(info.read_unaligned<uint16>() == 0);
      REQUIRE(info.read_unaligned<uint16>() == 26);
      REQUIRE(info.read_unaligned<uint16>() == 0);
   }

   SECTION("consume tests")
   {
      auto shdr = reader.read_child_strict<"SHDR"_id>();
      shdr.read_child_strict<"RTYP"_id>();
      shdr.read_child_strict<"NAME"_id>();

      auto info = shdr.read_child_strict<"INFO"_id>();

      info.consume(4);
      REQUIRE(info.read<uint32>() == 26);

      info.reset_head();

      // consumes can be unaligned as well.
      info.consume_unaligned(2);
      REQUIRE(info.read<uint16>() == 0);
      REQUIRE(info.read<uint32>() == 26);
   }

   SECTION("trace tests")
   {
      auto shdr = reader.read_child_strict<"SHDR"_id>();
      shdr.read_child_strict<"RTYP"_id>();
      shdr.read_child_strict<"NAME"_id>();
      shdr.read_child_strict<"INFO"_id>();

      auto pipe = shdr.read_child_strict<"PIPE"_id>();
      auto pipe_info = pipe.read_child_strict<"INFO"_id>();

      const auto expected_trace =
         R"(ucfb at offset 0
SHDR at offset 8
PIPE at offset 76
INFO at offset 84
   Read head of INFO is 0
   Bytes at read head: 01 00 00 00 02 00 00 00)"sv;

      REQUIRE(pipe_info.trace() == expected_trace);
   }
}

}