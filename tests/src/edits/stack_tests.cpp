#include "pch.h"

#include "edits/stack.hpp"

using namespace std::literals;

namespace we::edits::tests {

namespace {

struct dummy_edit_state {
   int apply_call_count = 0;
   int revert_call_count = 0;
};

struct dummy_edit : edit<dummy_edit_state> {
   void apply(dummy_edit_state& target) noexcept override
   {
      ++target.apply_call_count;
   }

   void revert(dummy_edit_state& target) noexcept override
   {
      ++target.revert_call_count;
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}
};

struct dummy_edit_bools_state {
   bool toggles[3] = {false, false, false};
};

struct dummy_ordering_edit : edit<dummy_edit_bools_state> {
   dummy_ordering_edit(int index) : index{index} {}

   void apply(dummy_edit_bools_state& target) noexcept override
   {
      target.toggles[index] = not target.toggles[index];
   }

   void revert(dummy_edit_bools_state& target) noexcept override
   {
      target.toggles[index] = not target.toggles[index];
   }

   bool is_coalescable([[maybe_unused]] const edit& other) const noexcept override
   {
      return false;
   }

   void coalesce([[maybe_unused]] edit& other) noexcept override {}

   int index = 0;
};

struct dummy_edit_coalesce : edit<int> {
   void apply(int& target) noexcept override
   {
      target = new_value;
   }

   void revert(int& target) noexcept override
   {
      target = old_value;
   }

   bool is_coalescable(const edit& other) const noexcept override
   {
      return static_cast<const dummy_edit_coalesce*>(&other) != nullptr;
   }

   void coalesce(edit& other) noexcept override
   {
      this->new_value = static_cast<dummy_edit_coalesce&>(other).new_value;
   }

   dummy_edit_coalesce(int new_value, int old_value)
      : new_value{new_value}, old_value{old_value}
   {
   }

   int new_value = 0;
   int old_value = 0;
};

}

TEST_CASE("edits stack core tests", "[Edits]")
{
   stack<dummy_edit_state> stack;
   dummy_edit_state state;

   stack.apply(std::make_unique<dummy_edit>(), state);

   REQUIRE(state.apply_call_count == 1);
   REQUIRE(state.revert_call_count == 0);
   REQUIRE(stack.applied_size() == 1);
   REQUIRE(stack.reverted_size() == 0);

   stack.revert(state);

   REQUIRE(state.apply_call_count == 1);
   REQUIRE(state.revert_call_count == 1);
   REQUIRE(stack.applied_size() == 0);
   REQUIRE(stack.reverted_size() == 1);

   stack.reapply(state);

   REQUIRE(state.apply_call_count == 2);
   REQUIRE(state.revert_call_count == 1);
   REQUIRE(stack.applied_size() == 1);
   REQUIRE(stack.reverted_size() == 0);
}

TEST_CASE("edits stack count function tests", "[Edits]")
{
   stack<dummy_edit_state> stack;
   dummy_edit_state state;

   stack.apply(std::make_unique<dummy_edit>(), state);
   stack.apply(std::make_unique<dummy_edit>(), state);
   stack.apply(std::make_unique<dummy_edit>(), state);

   REQUIRE(state.apply_call_count == 3);
   REQUIRE(state.revert_call_count == 0);
   REQUIRE(stack.applied_size() == 3);
   REQUIRE(stack.reverted_size() == 0);

   stack.revert(2, state);

   REQUIRE(state.apply_call_count == 3);
   REQUIRE(state.revert_call_count == 2);
   REQUIRE(stack.applied_size() == 1);
   REQUIRE(stack.reverted_size() == 2);

   stack.revert(2, state);

   REQUIRE(state.apply_call_count == 3);
   REQUIRE(state.revert_call_count == 3);
   REQUIRE(stack.applied_size() == 0);
   REQUIRE(stack.reverted_size() == 3);

   stack.reapply(2, state);

   REQUIRE(state.apply_call_count == 5);
   REQUIRE(state.revert_call_count == 3);
   REQUIRE(stack.applied_size() == 2);
   REQUIRE(stack.reverted_size() == 1);

   stack.reapply(2, state);

   REQUIRE(state.apply_call_count == 6);
   REQUIRE(state.revert_call_count == 3);
   REQUIRE(stack.applied_size() == 3);
   REQUIRE(stack.reverted_size() == 0);
}

TEST_CASE("edits stack _all function tests", "[Edits]")
{
   stack<dummy_edit_state> stack;
   dummy_edit_state state;

   stack.apply(std::make_unique<dummy_edit>(), state);
   stack.apply(std::make_unique<dummy_edit>(), state);
   stack.apply(std::make_unique<dummy_edit>(), state);

   REQUIRE(state.apply_call_count == 3);
   REQUIRE(state.revert_call_count == 0);
   REQUIRE(stack.applied_size() == 3);
   REQUIRE(stack.reverted_size() == 0);

   stack.revert_all(state);

   REQUIRE(state.apply_call_count == 3);
   REQUIRE(state.revert_call_count == 3);
   REQUIRE(stack.applied_size() == 0);
   REQUIRE(stack.reverted_size() == 3);

   stack.reapply_all(state);

   REQUIRE(state.apply_call_count == 6);
   REQUIRE(state.revert_call_count == 3);
   REQUIRE(stack.applied_size() == 3);
   REQUIRE(stack.reverted_size() == 0);
}

TEST_CASE("edits stack ordering tests", "[Edits]")
{
   stack<dummy_edit_bools_state> stack;
   dummy_edit_bools_state state;

   stack.apply(std::make_unique<dummy_ordering_edit>(0), state);
   stack.apply(std::make_unique<dummy_ordering_edit>(1), state);
   stack.apply(std::make_unique<dummy_ordering_edit>(2), state);

   REQUIRE(state.toggles[0]);
   REQUIRE(state.toggles[1]);
   REQUIRE(state.toggles[2]);

   stack.revert(2, state);

   REQUIRE(state.toggles[0]);
   REQUIRE(not state.toggles[1]);
   REQUIRE(not state.toggles[2]);

   stack.reapply(1, state);

   REQUIRE(state.toggles[0]);
   REQUIRE(state.toggles[1]);
   REQUIRE(not state.toggles[2]);
}

TEST_CASE("edits stack empty function tests", "[Edits]")
{
   stack<dummy_edit_state> stack;
   dummy_edit_state state;

   REQUIRE(stack.applied_empty());
   REQUIRE(stack.reverted_empty());

   stack.apply(std::make_unique<dummy_edit>(), state);
   stack.apply(std::make_unique<dummy_edit>(), state);
   stack.apply(std::make_unique<dummy_edit>(), state);

   REQUIRE(not stack.applied_empty());
   REQUIRE(stack.reverted_empty());

   stack.revert(2, state);

   REQUIRE(not stack.applied_empty());
   REQUIRE(not stack.reverted_empty());

   stack.revert(1, state);

   REQUIRE(stack.applied_empty());
   REQUIRE(not stack.reverted_empty());
}

TEST_CASE("edits stack revert closes", "[Edits]")
{
   stack<dummy_edit_state> stack;
   dummy_edit_state state;

   auto unique_first_edit = std::make_unique<dummy_edit>();

   const dummy_edit* first_edit =
      unique_first_edit.get(); // Never, ever, ever, hold onto edit pointers outside test code.

   stack.apply(std::move(unique_first_edit), state);
   stack.apply(std::make_unique<dummy_edit>(), state);

   stack.revert(state);

   REQUIRE(first_edit->is_closed());
}

TEST_CASE("edits stack apply coalesce", "[Edits]")
{
   stack<int> stack;
   int state = 0;

   stack.apply(std::make_unique<dummy_edit_coalesce>(1, 0), state);
   stack.apply(std::make_unique<dummy_edit_coalesce>(2, 1), state);

   REQUIRE(state == 2);
   REQUIRE(stack.applied_size() == 1);

   stack.revert(state);

   REQUIRE(state == 0);
}

TEST_CASE("edits stack apply closed first no coalesce", "[Edits]")
{
   stack<int> stack;
   int state = 0;

   auto edit = std::make_unique<dummy_edit_coalesce>(1, 0);

   edit->close();

   stack.apply(std::move(edit), state);
   stack.apply(std::make_unique<dummy_edit_coalesce>(2, 1), state);

   REQUIRE(state == 2);
   REQUIRE(stack.applied_size() == 2);

   stack.revert(state);

   REQUIRE(state == 1);
}

TEST_CASE("edits stack apply closed second no coalesce", "[Edits]")
{
   stack<int> stack;
   int state = 0;

   stack.apply(std::make_unique<dummy_edit_coalesce>(1, 0), state);

   auto next_edit = std::make_unique<dummy_edit_coalesce>(2, 1);

   next_edit->close();

   stack.apply(std::move(next_edit), state);

   REQUIRE(state == 2);
   REQUIRE(stack.applied_size() == 2);

   stack.revert(state);

   REQUIRE(state == 1);
}

TEST_CASE("edits stack close_last", "[Edits]")
{
   stack<int> stack;
   int state = 0;

   stack.close_last(); // calling on an empty stack should be safe
   stack.apply(std::make_unique<dummy_edit_coalesce>(1, 0), state);
   stack.close_last();
   stack.apply(std::make_unique<dummy_edit_coalesce>(2, 1), state);

   REQUIRE(state == 2);
   REQUIRE(stack.applied_size() == 2);

   stack.revert(state);

   REQUIRE(state == 1);
}

TEST_CASE("edits stack transparent tests", "[Edits]")
{
   stack<dummy_edit_state> stack;
   dummy_edit_state state;

   stack.apply(std::make_unique<dummy_edit>(), state);
   stack.apply(std::make_unique<dummy_edit>(), state, {.transparent = true});

   REQUIRE(state.apply_call_count == 2);
   REQUIRE(state.revert_call_count == 0);
   REQUIRE(stack.applied_size() == 2);
   REQUIRE(stack.reverted_size() == 0);

   stack.revert(state);

   REQUIRE(state.apply_call_count == 2);
   REQUIRE(state.revert_call_count == 2);
   REQUIRE(stack.applied_size() == 0);
   REQUIRE(stack.reverted_size() == 2);

   stack.reapply(state);

   REQUIRE(state.apply_call_count == 4);
   REQUIRE(state.revert_call_count == 2);
   REQUIRE(stack.applied_size() == 2);
   REQUIRE(stack.reverted_size() == 0);
}

TEST_CASE("edits stack mixed-transparent no coalesce", "[Edits]")
{
   stack<int> stack;
   int state = 0;

   stack.apply(std::make_unique<dummy_edit_coalesce>(1, 0), state,
               {.transparent = true});
   stack.apply(std::make_unique<dummy_edit_coalesce>(2, 1), state);

   REQUIRE(state == 2);
   REQUIRE(stack.applied_size() == 2);

   stack.revert(state);

   REQUIRE(state == 1);

   stack.revert(state);

   REQUIRE(state == 0);
}

TEST_CASE("edits stack mixed-transparent after no coalesce", "[Edits]")
{
   stack<int> stack;
   int state = 0;

   stack.apply(std::make_unique<dummy_edit_coalesce>(1, 0), state);
   stack.apply(std::make_unique<dummy_edit_coalesce>(2, 1), state,
               {.transparent = true});

   REQUIRE(state == 2);
   REQUIRE(stack.applied_size() == 2);

   stack.revert(state);

   REQUIRE(state == 0);
}

TEST_CASE("edits stack transparent coalesce", "[Edits]")
{
   stack<int> stack;
   int state = 0;

   stack.apply(std::make_unique<dummy_edit_coalesce>(1, 0), state,
               {.transparent = true});
   stack.apply(std::make_unique<dummy_edit_coalesce>(2, 1), state,
               {.transparent = true});

   REQUIRE(state == 2);
   REQUIRE(stack.applied_size() == 1);

   stack.revert(state);

   REQUIRE(state == 0);
}

TEST_CASE("edits stack modified flag tests", "[Edits]")
{
   stack<dummy_edit_state> stack;
   dummy_edit_state state;

   CHECK(not stack.modified_flag());

   stack.apply(std::make_unique<dummy_edit>(), state);

   CHECK(stack.modified_flag());

   stack.clear_modified_flag();

   CHECK(not stack.modified_flag());

   stack.revert(state);

   CHECK(stack.modified_flag());

   stack.clear_modified_flag();

   CHECK(not stack.modified_flag());

   stack.reapply(state);

   CHECK(stack.modified_flag());

   stack.clear_modified_flag();

   CHECK(not stack.modified_flag());
}

}