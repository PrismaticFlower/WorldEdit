#include "pch.h"

#include "key_input_manager.hpp"

using namespace std::literals;

namespace we::tests {

TEST_CASE("key_input_manager basic bind test", "[KeyInputManager]")
{
   key_input_manager manager;

   int called_count = 0;

   manager.bind({.key = key::a}, {}, [&] { ++called_count; });

   manager.notify_key_down(key::a);
   manager.update(false);

   REQUIRE(called_count == 1);

   manager.notify_key_up(key::a);
   manager.update(false);

   REQUIRE(called_count == 1);
}

TEST_CASE("key_input_manager toggle bind test", "[KeyInputManager]")
{
   key_input_manager manager;

   bool called = false;

   manager.bind({.key = key::a}, {.toggle = true}, [&] { called = not called; });

   manager.notify_key_down(key::a);
   manager.update(false);

   REQUIRE(called);

   manager.notify_key_up(key::a);
   manager.update(false);

   REQUIRE(not called);
}

TEST_CASE("key_input_manager unbind test", "[KeyInputManager]")
{
   key_input_manager manager;

   int called_count = 0;

   manager.bind({.key = key::a}, {}, [&] { ++called_count; });

   manager.notify_key_down(key::a);
   manager.notify_key_up(key::a);
   manager.update(false);

   REQUIRE(called_count == 1);

   manager.unbind({.key = key::a});
   manager.notify_key_down(key::a);
   manager.notify_key_up(key::a);
   manager.update(false);

   REQUIRE(called_count == 1);
}

TEST_CASE("key_input_manager toggle bind release_toggles test",
          "[KeyInputManager]")
{
   key_input_manager manager;

   bool a_called = false;
   int a_called_count = 0;
   bool b_called = false;
   int b_called_count = 0;

   manager.bind({.key = key::a}, {.toggle = true}, [&] {
      a_called = not a_called;
      ++a_called_count;
   });
   manager.bind({.key = key::b, .modifiers = {.shift = true}}, {.toggle = true}, [&] {
      b_called = not b_called;
      ++b_called_count;
   });

   manager.notify_key_down(key::a);
   manager.notify_key_down(key::shift);
   manager.notify_key_down(key::b);
   manager.update(false);

   REQUIRE(a_called);
   REQUIRE(a_called_count == 1);
   REQUIRE(b_called);
   REQUIRE(b_called_count == 1);

   manager.release_toggles();

   REQUIRE(not a_called);
   REQUIRE(a_called_count == 2);
   REQUIRE(not b_called);
   REQUIRE(b_called_count == 2);

   manager.notify_key_up(key::a);
   manager.notify_key_up(key::shift);
   manager.notify_key_up(key::b);
   manager.update(false);

   REQUIRE(not a_called);
   REQUIRE(a_called_count == 2);
   REQUIRE(not b_called);
   REQUIRE(b_called_count == 2);
}

TEST_CASE("key_input_manager toggle bind release_unmodified_toggles test",
          "[KeyInputManager]")
{
   key_input_manager manager;

   bool called = false;
   int called_count = 0;

   manager.bind({.key = key::a}, {.toggle = true}, [&] {
      called = not called;
      ++called_count;
   });

   manager.notify_key_down(key::a);
   manager.update(false);

   REQUIRE(called);
   REQUIRE(called_count == 1);

   manager.release_unmodified_toggles();

   REQUIRE(not called);
   REQUIRE(called_count == 2);

   manager.notify_key_up(key::a);
   manager.update(false);

   REQUIRE(not called);
   REQUIRE(called_count == 2);
}

TEST_CASE("key_input_manager basic bind multi-down test", "[KeyInputManager]")
{
   key_input_manager manager;

   int called_count = 0;

   manager.bind({.key = key::a}, {}, [&] { ++called_count; });

   manager.notify_key_down(key::a);
   manager.notify_key_down(key::a);
   manager.notify_key_down(key::a);
   manager.notify_key_down(key::a);
   manager.notify_key_up(key::a);
   manager.update(false);

   REQUIRE(called_count == 1);
}

TEST_CASE("key_input_manager modified bind test", "[KeyInputManager]")
{
   key_input_manager manager;

   int called_count = 0;

   manager.bind({.key = key::a, .modifiers = {.shift = true}}, {},
                [&] { ++called_count; });

   manager.notify_key_down(key::a);
   manager.notify_key_down(key::shift);
   manager.update(false);

   REQUIRE(called_count == 0);

   manager.notify_key_up(key::a);
   manager.notify_key_up(key::shift);

   manager.notify_key_down(key::shift);
   manager.notify_key_down(key::a);
   manager.update(false);

   REQUIRE(called_count == 1);
}

TEST_CASE("key_input_manager ignore_unmodified_bindings test",
          "[KeyInputManager]")
{
   key_input_manager manager;

   int a_called_count = 0;
   int ctrl_a_called_count = 0;

   manager.bind({.key = key::a}, {}, [&] { ++a_called_count; });
   manager.bind({.key = key::a, .modifiers{.ctrl = true}}, {},
                [&] { ++ctrl_a_called_count; });

   manager.notify_key_down(key::a);
   manager.update(true);
   manager.notify_key_up(key::a);

   manager.notify_key_down(key::control);
   manager.notify_key_down(key::a);
   manager.update(true);
   manager.notify_key_up(key::a);
   manager.notify_key_up(key::control);

   REQUIRE(a_called_count == 0);
   REQUIRE(ctrl_a_called_count == 1);
}

}
