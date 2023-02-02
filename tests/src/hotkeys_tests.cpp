#include "pch.h"

#include "hotkeys.hpp"

using namespace std::literals;

namespace we::tests {

TEST_CASE("hotkeys basic bind test", "[Hotkeys]")
{
   int called_count = 0;

   commands commands;
   null_output_stream output;

   commands.add("called", [&] { ++called_count; });

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; }, {{"called", {.key = key::a}}});

   hotkeys.notify_key_down(key::a);
   hotkeys.update(false, false);

   REQUIRE(called_count == 1);

   hotkeys.notify_key_up(key::a);
   hotkeys.update(false, false);

   REQUIRE(called_count == 1);
}

TEST_CASE("hotkeys toggle bind test", "[Hotkeys]")
{
   bool called = false;

   commands commands;
   null_output_stream output;

   commands.add("toggle_called", called);

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; },
                   {{"toggle_called", {.key = key::a}, {.toggle = true}}});

   hotkeys.notify_key_down(key::a);
   hotkeys.update(false, false);

   REQUIRE(called);

   hotkeys.notify_key_up(key::a);
   hotkeys.update(false, false);

   REQUIRE(not called);
}

TEST_CASE("hotkeys toggle bind release_toggles test", "[Hotkeys]")
{
   bool called = false;
   int called_count = 0;

   commands commands;
   null_output_stream output;

   commands.add("toggle_called", [&] {
      called = not called;
      called_count += 1;
   });

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; },
                   {{"toggle_called", {.key = key::a}, {.toggle = true}}});

   hotkeys.notify_key_down(key::a);
   hotkeys.update(false, false);
   hotkeys.release_toggles();

   REQUIRE(not called);
   REQUIRE(called_count == 2);

   hotkeys.notify_key_up(key::a);
   hotkeys.update(false, false);

   REQUIRE(not called);
   REQUIRE(called_count == 2);
}

TEST_CASE("hotkeys toggle bind mouse focus lost toggles release test",
          "[Hotkeys]")
{
   bool called = false;
   int called_count = 0;

   commands commands;
   null_output_stream output;

   commands.add("toggle_called", [&] {
      called = not called;
      called_count += 1;
   });

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; },
                   {{"toggle_called", {.key = key::mouse1}, {.toggle = true}}});

   hotkeys.notify_key_down(key::mouse1);
   hotkeys.update(false, false);

   REQUIRE(called);
   REQUIRE(called_count == 1);

   hotkeys.update(true, false);

   REQUIRE(not called);
   REQUIRE(called_count == 2);

   hotkeys.notify_key_up(key::mouse1);
   hotkeys.update(false, false);

   REQUIRE(not called);
   REQUIRE(called_count == 2);
}

TEST_CASE("hotkeys toggle bind keyboard focus lost toggles release test",
          "[Hotkeys]")
{
   bool called = false;
   int called_count = 0;

   commands commands;
   null_output_stream output;

   commands.add("toggle_called", [&] {
      called = not called;
      called_count += 1;
   });

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; },
                   {{"toggle_called", {.key = key::a}, {.toggle = true}}});

   hotkeys.notify_key_down(key::a);
   hotkeys.update(false, false);

   REQUIRE(called);
   REQUIRE(called_count == 1);

   hotkeys.update(false, true);

   REQUIRE(not called);
   REQUIRE(called_count == 2);

   hotkeys.notify_key_up(key::a);
   hotkeys.update(false, false);

   REQUIRE(not called);
   REQUIRE(called_count == 2);
}

TEST_CASE("hotkeys toggle bind mouse ignore ImGui focus test", "[Hotkeys]")
{
   bool called = false;

   commands commands;
   null_output_stream output;

   commands.add("toggle_called", called);

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; },
                   {{"toggle_called",
                     {.key = key::mouse1},
                     {.toggle = true, .ignore_imgui_focus = true}}});

   hotkeys.notify_key_down(key::mouse1);
   hotkeys.update(true, false);

   REQUIRE(called);

   hotkeys.notify_key_up(key::mouse1);
   hotkeys.update(true, false);

   REQUIRE(not called);
}

TEST_CASE("hotkeys toggle bind keyboard ignore ImGui focus test", "[Hotkeys]")
{
   bool called = false;

   commands commands;
   null_output_stream output;

   commands.add("toggle_called", called);

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; },
                   {{"toggle_called",
                     {.key = key::a},
                     {.toggle = true, .ignore_imgui_focus = true}}});

   hotkeys.notify_key_down(key::a);
   hotkeys.update(false, true);

   REQUIRE(called);

   hotkeys.notify_key_up(key::a);
   hotkeys.update(false, true);

   REQUIRE(not called);
}

TEST_CASE("hotkeys basic bind multi-down test", "[Hotkeys]")
{
   int called_count = 0;

   commands commands;
   null_output_stream output;

   commands.add("toggle_called", [&] { ++called_count; });

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; }, {{"toggle_called", {.key = key::a}}});

   hotkeys.notify_key_down(key::a);
   hotkeys.notify_key_down(key::a);
   hotkeys.notify_key_down(key::a);
   hotkeys.notify_key_down(key::a);
   hotkeys.notify_key_up(key::a);
   hotkeys.update(false, false);

   REQUIRE(called_count == 1);
}

TEST_CASE("hotkeys modified bind test", "[Hotkeys]")
{
   int called_count = 0;

   commands commands;
   null_output_stream output;

   commands.add("toggle_called", [&] { ++called_count; });

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; },
                   {{"toggle_called", {.key = key::a, .modifiers = {.shift = true}}}});

   hotkeys.notify_key_down(key::a);
   hotkeys.notify_key_down(key::shift);
   hotkeys.update(false, false);

   REQUIRE(called_count == 0);

   hotkeys.notify_key_up(key::a);
   hotkeys.notify_key_up(key::shift);

   hotkeys.notify_key_down(key::shift);
   hotkeys.notify_key_down(key::a);
   hotkeys.update(false, false);

   REQUIRE(called_count == 1);
}

TEST_CASE("hotkeys bind missing command test", "[Commands]")
{
   commands commands;
   null_output_stream output;

   hotkeys hotkeys{commands, output};

   REQUIRE_THROWS_AS(hotkeys.add_set("", [] { return true; },
                                     {{"test.command",
                                       {.key = key::a, .modifiers = {.shift = true}}}}),
                     unknown_command);
}

TEST_CASE("hotkeys bad command argument test", "[Commands]")
{
   commands commands;
   null_output_stream output;

   float value = 0.0f;

   commands.add("test.command"s, [&](const float v) { value = v; });

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [] { return true; }, {{"test.command one", {.key = key::a}}}); // executing this will cause a invalid_command_argument exception

   hotkeys.notify_key_down(key::a);
   hotkeys.update(false, false); // but hotkeys should catch it and print it out to error_output

   REQUIRE(value == 0.0f); // value is unchanged after the exception
}

TEST_CASE("hotkeys toggle bind deactivated release test", "[Hotkeys]")
{
   bool set_active = true;
   bool called = false;
   int called_count = 0;

   commands commands;
   null_output_stream output;

   commands.add("toggle_called", [&] {
      called = not called;
      called_count += 1;
   });

   hotkeys hotkeys{commands, output};

   hotkeys.add_set("", [&] { return set_active; },
                   {{"toggle_called", {.key = key::a}, {.toggle = true}}});

   hotkeys.notify_key_down(key::a);
   hotkeys.update(false, false);

   REQUIRE(called);
   REQUIRE(called_count == 1);

   set_active = false;

   hotkeys.update(false, false);

   REQUIRE(not called);
   REQUIRE(called_count == 2);

   hotkeys.notify_key_up(key::a);
   hotkeys.update(false, false);

   REQUIRE(not called);
   REQUIRE(called_count == 2);
}

}
