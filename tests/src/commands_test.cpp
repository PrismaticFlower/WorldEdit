#include "pch.h"

#include "commands.hpp"
#include "types.hpp"

using namespace std::literals;

namespace we::tests {

TEST_CASE("commands floating point test", "[Commands]")
{
   commands commands;

   float value_float = 0.0f;
   double value_double = 0.0;

   commands.add("value.float"s, value_float);
   commands.add("value.double"s, value_double);

   commands.execute("value.float 1.0");
   commands.execute("value.double 1.0");

   REQUIRE(value_float == 1.0f);
   REQUIRE(value_double == 1.0);
}

TEST_CASE("commands integral test", "[Commands]")
{
   commands commands;

   int8 i8 = 0;
   int16 i16 = 0;
   int32 i32 = 0;
   int64 i64 = 0;

   uint8 u8 = 0;
   uint16 u16 = 0;
   uint32 u32 = 0;
   uint64 u64 = 0;

   commands.add("value.i8"s, i8);
   commands.add("value.i16"s, i16);
   commands.add("value.i32"s, i32);
   commands.add("value.i64"s, i64);

   commands.add("value.u8"s, u8);
   commands.add("value.u16"s, u16);
   commands.add("value.u32"s, u32);
   commands.add("value.u64"s, u64);

   commands.execute("value.i8 1");
   commands.execute("value.i16 1");
   commands.execute("value.i32 1");
   commands.execute("value.i64 1");

   commands.execute("value.u8 1");
   commands.execute("value.u16 1");
   commands.execute("value.u32 1");
   commands.execute("value.u64 1");

   REQUIRE(i8 == 1);
   REQUIRE(i16 == 1);
   REQUIRE(i32 == 1);
   REQUIRE(i64 == 1);

   REQUIRE(u8 == 1);
   REQUIRE(u16 == 1);
   REQUIRE(u32 == 1);
   REQUIRE(u64 == 1);
}

TEST_CASE("commands bool test", "[Commands]")
{
   commands commands;

   bool value = false;

   commands.add("value.bool"s, value);

   commands.execute("value.bool");

   REQUIRE(value);

   commands.execute("value.bool");

   REQUIRE(not value);

   commands.execute("value.bool 0");

   REQUIRE(not value);

   commands.execute("value.bool 1");

   REQUIRE(value);
}

TEST_CASE("commands string test", "[Commands]")
{
   commands commands;

   std::string value = "A Value"s;

   commands.add("value.string"s, value);

   commands.execute("value.string Such value, much wow!");

   REQUIRE(value == "Such value, much wow!"sv);
}

TEST_CASE("commands callback floating point test", "[Commands]")
{
   commands commands;

   float value_float = 0.0f;
   double value_double = 0.0;

   commands.add("use.float"s, [&](const float value) { value_float = value; });
   commands.add("use.double"s, [&](const double value) { value_double = value; });

   commands.execute("use.float 1.0");
   commands.execute("use.double 1.0");

   REQUIRE(value_float == 1.0f);
   REQUIRE(value_double == 1.0);
}

TEST_CASE("commands callback integral test", "[Commands]")
{
   commands commands;
   int8 i8 = 0;
   int16 i16 = 0;
   int32 i32 = 0;
   int64 i64 = 0;

   uint8 u8 = 0;
   uint16 u16 = 0;
   uint32 u32 = 0;
   uint64 u64 = 0;

   commands.add("use.i8"s, [&](const int8 i) { i8 = i; });
   commands.add("use.i16"s, [&](const int16 i) { i16 = i; });
   commands.add("use.i32"s, [&](const int32 i) { i32 = i; });
   commands.add("use.i64"s, [&](const int64 i) { i64 = i; });

   commands.add("use.u8"s, [&](const uint8 i) { u8 = i; });
   commands.add("use.u16"s, [&](const uint16 i) { u16 = i; });
   commands.add("use.u32"s, [&](const uint32 i) { u32 = i; });
   commands.add("use.u64"s, [&](const uint64 i) { u64 = i; });

   commands.execute("use.i8 1");
   commands.execute("use.i16 1");
   commands.execute("use.i32 1");
   commands.execute("use.i64 1");

   commands.execute("use.u8 1");
   commands.execute("use.u16 1");
   commands.execute("use.u32 1");
   commands.execute("use.u64 1");

   REQUIRE(i8 == 1);
   REQUIRE(i16 == 1);
   REQUIRE(i32 == 1);
   REQUIRE(i64 == 1);

   REQUIRE(u8 == 1);
   REQUIRE(u16 == 1);
   REQUIRE(u32 == 1);
   REQUIRE(u64 == 1);
}

TEST_CASE("commands callback bool test", "[Commands]")
{
   commands commands;

   bool value = false;

   commands.add("use.bool"s, [&](const bool v) { value = v; });

   commands.execute("use.bool 0");

   REQUIRE(not value);

   commands.execute("use.bool 1");

   REQUIRE(value);
}

TEST_CASE("commands callback string_view test", "[Commands]")
{
   commands commands;

   std::string value = "A Value"s;

   commands.add("use.string"s,
                [&](const std::string_view str) { value = std::string{str}; });

   commands.execute("use.string Such value, much wow!");

   REQUIRE(value == "Such value, much wow!"sv);
}

TEST_CASE("commands callback test", "[Commands]")
{
   commands commands;

   bool called = false;

   commands.add("use.noargs"s, [&]() { called = true; });

   commands.execute("use.noargs");

   REQUIRE(called);
}

TEST_CASE("commands floating point bad argument test", "[Commands]")
{
   commands commands;

   float value_float = 0.0f;
   double value_double = 0.0;

   commands.add("value.float"s, value_float);
   commands.add("value.double"s, value_double);

   REQUIRE_THROWS_AS(commands.execute("value.float ONE"), invalid_command_argument);
   REQUIRE_THROWS_AS(commands.execute("value.double ONE"), invalid_command_argument);
}

TEST_CASE("commands integral bad argument test", "[Commands]")
{
   commands commands;

   int8 i8 = 0;
   int16 i16 = 0;
   int32 i32 = 0;
   int64 i64 = 0;

   uint8 u8 = 0;
   uint16 u16 = 0;
   uint32 u32 = 0;
   uint64 u64 = 0;

   commands.add("value.i8"s, i8);
   commands.add("value.i16"s, i16);
   commands.add("value.i32"s, i32);
   commands.add("value.i64"s, i64);

   commands.add("value.u8"s, u8);
   commands.add("value.u16"s, u16);
   commands.add("value.u32"s, u32);
   commands.add("value.u64"s, u64);

   REQUIRE_THROWS_AS(commands.execute("value.i8 ONE"), invalid_command_argument);
   REQUIRE_THROWS_AS(commands.execute("value.i16 ONE"), invalid_command_argument);
   REQUIRE_THROWS_AS(commands.execute("value.i32 ONE"), invalid_command_argument);
   REQUIRE_THROWS_AS(commands.execute("value.i64 ONE"), invalid_command_argument);

   REQUIRE_THROWS_AS(commands.execute("value.u8 ONE"), invalid_command_argument);
   REQUIRE_THROWS_AS(commands.execute("value.u16 ONE"), invalid_command_argument);
   REQUIRE_THROWS_AS(commands.execute("value.u32 ONE"), invalid_command_argument);
   REQUIRE_THROWS_AS(commands.execute("value.u64 ONE"), invalid_command_argument);
}

TEST_CASE("commands unknown command test", "[Commands]")
{
   commands commands;

   REQUIRE_THROWS_AS(commands.execute("i.dont.exist"), unknown_command);
}

TEST_CASE("commands has command test", "[Commands]")
{
   commands commands;

   commands.add("basic.command"s, []() {});

   REQUIRE(commands.has_command("basic.command"sv));
   REQUIRE(not commands.has_command("complex.command"sv));
}

}
