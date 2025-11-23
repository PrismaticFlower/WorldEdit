
#include "pch.h"

#include "io/read_file.hpp"
#include "ucfb/reader.hpp"

using namespace std::literals;

namespace we::ucfb::tests {

TEST_CASE("ucfb reading", "[ucfb][reader]")
{
   const auto bytes = io::read_file_to_bytes("data/ucfb_read_test.lvl");

   ucfb::reader reader{bytes, ucfb::reader_options{.aligned_children = false}};

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

   SECTION("data read bytes tests")
   {
      auto shdr = reader.read_child_strict<"SHDR"_id>();

      auto rtyp = shdr.read_child_strict<"RTYP"_id>();
      REQUIRE(rtyp.read_string() == "zprepass"sv);

      shdr.read_child_strict<"NAME"_id>();

      auto info = shdr.read_child_strict<"INFO"_id>();

      const std::span<const std::byte> info_bytes = info.read_bytes(8);

      REQUIRE(info_bytes[0] == std::byte{0x01});
      REQUIRE(info_bytes[1] == std::byte{0x00});
      REQUIRE(info_bytes[2] == std::byte{0x00});
      REQUIRE(info_bytes[3] == std::byte{0x00});
      REQUIRE(info_bytes[4] == std::byte{0x1A});
      REQUIRE(info_bytes[5] == std::byte{0x00});
      REQUIRE(info_bytes[6] == std::byte{0x00});
      REQUIRE(info_bytes[7] == std::byte{0x00});

      info.reset_head();

      REQUIRE(info.read_multi<uint32, uint32>() == std::tuple<uint32, uint32>{1, 26});
   }

   SECTION("alignment tests")
   {
      auto shdr = reader.read_child_strict<"SHDR"_id>();
      shdr.read_child_strict<"RTYP"_id>();
      shdr.read_child_strict<"NAME"_id>();

      auto info = shdr.read_child_strict<"INFO"_id>();

      // unaligned reads (the default)
      REQUIRE(info.read<uint16>() == 1);
      REQUIRE(info.read<uint16>() == 0);
      REQUIRE(info.read<uint16>() == 26);
      REQUIRE(info.read<uint16>() == 0);

      info.reset_head();

      // the read head is aligned to four bytes after an aligned read
      REQUIRE(info.read_aligned<uint16>() == 1);
      REQUIRE(info.read_aligned<uint16>() == 26);
      REQUIRE_THROWS(info.read_aligned<uint32>() == 0);
   }

   SECTION("consume tests")
   {
      auto shdr = reader.read_child_strict<"SHDR"_id>();
      shdr.read_child_strict<"RTYP"_id>();
      shdr.read_child_strict<"NAME"_id>();

      auto info = shdr.read_child_strict<"INFO"_id>();

      info.consume(2);
      REQUIRE(info.read<uint16>() == 0);
      REQUIRE(info.read<uint32>() == 26);

      info.reset_head();

      // consumes can be aligned as well.
      info.consume_aligned(1);
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

TEST_CASE("ucfb aligned children", "[ucfb][reader]")
{
   const auto bytes =
      io::read_file_to_bytes("data/ucfb_child_alignment_test.lvl");

   ucfb::reader reader{bytes, ucfb::reader_options{.aligned_children = true}};

   REQUIRE(reader.id() == "ucfb"_id);
   REQUIRE(reader.size() == 44);

   ucfb::reader_strict<"modl"_id> modl = reader.read_child_strict<"modl"_id>();
   ucfb::reader_strict<"NAME"_id> name = modl.read_child_strict<"NAME"_id>();
   ucfb::reader_strict<"VRTX"_id> vrtx = modl.read_child_strict<"VRTX"_id>();

   CHECK(modl.size() == 36);
   CHECK(name.size() == 15);
   CHECK(vrtx.size() == 4);
}

TEST_CASE("ucfb unaligned children fail", "[ucfb][reader]")
{
   const auto bytes =
      io::read_file_to_bytes("data/ucfb_child_alignment_test.lvl");

   ucfb::reader reader{bytes, ucfb::reader_options{.aligned_children = false}};

   ucfb::reader_strict<"modl"_id> modl = reader.read_child_strict<"modl"_id>();
   ucfb::reader_strict<"NAME"_id> name = modl.read_child_strict<"NAME"_id>();
   REQUIRE_THROWS(modl.read_child_strict<"VRTX"_id>());
}

}
