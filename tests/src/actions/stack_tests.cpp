#include "pch.h"

#include "actions/stack.hpp"

using namespace std::literals;

namespace we::actions::tests {

namespace {

struct dummy_edit_state {
   int apply_call_count = 0;
   int revert_call_count = 0;
};

struct dummy_edit : edit<dummy_edit_state> {
   void apply(dummy_edit_state& target) const noexcept override
   {
      ++target.apply_call_count;
   }

   void revert(dummy_edit_state& target) const noexcept override
   {
      ++target.revert_call_count;
   }
};

struct dummy_edit_bools_state {
   bool toggles[3] = {false, false, false};
};

struct dummy_ordering_edit : edit<dummy_edit_bools_state> {
   dummy_ordering_edit(int index) : index{index} {}

   void apply(dummy_edit_bools_state& target) const noexcept override
   {
      target.toggles[index] = not target.toggles[index];
   }

   void revert(dummy_edit_bools_state& target) const noexcept override
   {
      target.toggles[index] = not target.toggles[index];
   }

   int index = 0;
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

TEST_CASE("edits stack applied_top", "[Edits]")
{
   stack<dummy_edit_state> stack;
   dummy_edit_state state;

   REQUIRE(stack.applied_top() == nullptr);

   auto unique_edit = std::make_unique<dummy_edit>();
   auto* edit = unique_edit.get();

   stack.apply(std::move(unique_edit), state);

   REQUIRE(stack.applied_top() == edit);
}

}