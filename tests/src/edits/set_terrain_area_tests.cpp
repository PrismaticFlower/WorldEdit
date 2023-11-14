#include "pch.h"

#include "edits/set_terrain_area.hpp"
#include "world/world.hpp"

using namespace std::literals;
using we::world::dirty_rect;

namespace we::edits::tests {

namespace {

constexpr int32 terrain_length = 32;

auto make_2d_array(uint32 width, uint32 height, int16 value)
   -> container::dynamic_array_2d<int16>
{
   container::dynamic_array_2d<int16> array{width, height};

   for (int16& v : array) v = value;

   return array;
}

bool check_area(const world::dirty_rect& rect, const int16 expected,
                const world::terrain& terrain) noexcept
{
   for (uint32 y = rect.top; y < rect.bottom; ++y) {
      for (uint32 x = rect.left; x < rect.right; ++x) {
         if (terrain.height_map[{x, y}] != expected) return false;
      }
   }

   return true;
}

bool is_zeroed(const container::dynamic_array_2d<int16>& array) noexcept
{
   for (int16 v : array)
      if (v) return false;

   return true;
}

}

TEST_CASE("edits set_terrain_area simple", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 1));

   edit->apply(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 1, world.terrain));
   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));
   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce simple", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 2, world.terrain));
   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));
   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce contained by old", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(4, 4, make_2d_array(4, 4, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 0, 8, 4}, 1, world.terrain));
   CHECK(check_area({0, 4, 4, 8}, 1, world.terrain));
   CHECK(check_area({4, 4, 8, 8}, 2, world.terrain));

   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));
   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce contained by new", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_terrain_area(4, 4, make_2d_array(4, 4, 1));
   auto other_edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 2, world.terrain));

   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));
   REQUIRE(world.terrain.height_map_dirty.size() == 1);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce bottom right", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(4, 4, make_2d_array(8, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 0, 8, 4}, 1, world.terrain));
   CHECK(check_area({0, 0, 4, 8}, 1, world.terrain));
   CHECK(check_area({4, 4, 12, 12}, 2, world.terrain));

   REQUIRE(tracker.size() == 3);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 4, .top = 8, .right = 12, .bottom = 12});
   CHECK(tracker[2] == dirty_rect{.left = 8, .top = 4, .right = 12, .bottom = 8});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));
   CHECK(check_area({4, 4, 12, 12}, 0, world.terrain));

   REQUIRE(tracker.size() == 3);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 4, .top = 8, .right = 12, .bottom = 12});
   CHECK(tracker[2] == dirty_rect{.left = 8, .top = 4, .right = 12, .bottom = 8});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce right", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(4, 0, make_2d_array(8, 7, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 0, 4, 8}, 1, world.terrain));
   CHECK(check_area({4, 7, 8, 8}, 1, world.terrain));
   CHECK(check_area({4, 0, 12, 7}, 2, world.terrain));

   REQUIRE(tracker.size() == 2);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 8, .top = 0, .right = 12, .bottom = 7});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));
   CHECK(check_area({4, 0, 12, 7}, 0, world.terrain));

   REQUIRE(tracker.size() == 2);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 8, .top = 0, .right = 12, .bottom = 7});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce bottom", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(0, 4, make_2d_array(7, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 0, 8, 4}, 1, world.terrain));
   CHECK(check_area({7, 4, 8, 8}, 1, world.terrain));
   CHECK(check_area({0, 4, 7, 12}, 2, world.terrain));

   REQUIRE(tracker.size() == 2);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 8, .right = 7, .bottom = 12});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));
   CHECK(check_area({0, 4, 7, 12}, 0, world.terrain));

   REQUIRE(tracker.size() == 2);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 8, .right = 7, .bottom = 12});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce top left", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(4, 4, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({8, 4, 12, 12}, 1, world.terrain));
   CHECK(check_area({4, 8, 8, 12}, 1, world.terrain));
   CHECK(check_area({0, 0, 8, 8}, 2, world.terrain));

   REQUIRE(tracker.size() == 3);
   CHECK(tracker[0] == dirty_rect{.left = 4, .top = 4, .right = 12, .bottom = 12});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 4});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 4, .right = 4, .bottom = 8});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({4, 4, 12, 12}, 0, world.terrain));
   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));

   REQUIRE(tracker.size() == 3);
   CHECK(tracker[0] == dirty_rect{.left = 4, .top = 4, .right = 12, .bottom = 12});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 4});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 4, .right = 4, .bottom = 8});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce left", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(4, 0, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(0, 0, make_2d_array(5, 4, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({4, 4, 5, 8}, 1, world.terrain));
   CHECK(check_area({5, 0, 12, 8}, 1, world.terrain));
   CHECK(check_area({0, 0, 5, 4}, 2, world.terrain));

   REQUIRE(tracker.size() == 2);
   CHECK(tracker[0] == dirty_rect{.left = 4, .top = 0, .right = 12, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 4, .bottom = 4});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({4, 0, 12, 8}, 0, world.terrain));
   CHECK(check_area({0, 0, 5, 4}, 0, world.terrain));

   REQUIRE(tracker.size() == 2);
   CHECK(tracker[0] == dirty_rect{.left = 4, .top = 0, .right = 12, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 4, .bottom = 4});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce top", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(0, 4, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(0, 0, make_2d_array(4, 5, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 5, 4, 12}, 1, world.terrain));
   CHECK(check_area({4, 4, 8, 12}, 1, world.terrain));
   CHECK(check_area({0, 0, 4, 5}, 2, world.terrain));

   REQUIRE(tracker.size() == 2);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 4, .right = 8, .bottom = 12});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 4, .bottom = 4});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 4, 8, 12}, 0, world.terrain));
   CHECK(check_area({0, 0, 4, 5}, 0, world.terrain));

   REQUIRE(tracker.size() == 2);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 4, .right = 8, .bottom = 12});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 4, .bottom = 4});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce small left", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(4, 2, make_2d_array(6, 4, 1));
   auto other_edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({8, 2, 10, 6}, 1, world.terrain));
   CHECK(check_area({0, 0, 8, 8}, 2, world.terrain));

   REQUIRE(tracker.size() == 3);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 2});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 6, .right = 8, .bottom = 8});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 10, .bottom = 6});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({4, 4, 10, 6}, 0, world.terrain));
   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));

   REQUIRE(tracker.size() == 3);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 2});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 6, .right = 8, .bottom = 8});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 10, .bottom = 6});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce small right", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(0, 2, make_2d_array(4, 4, 1));
   auto other_edit = make_set_terrain_area(2, 0, make_2d_array(8, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 2, 2, 6}, 1, world.terrain));
   CHECK(check_area({2, 0, 10, 8}, 2, world.terrain));

   REQUIRE(tracker.size() == 3);
   CHECK(tracker[0] == dirty_rect{.left = 2, .top = 0, .right = 10, .bottom = 2});
   CHECK(tracker[1] == dirty_rect{.left = 2, .top = 6, .right = 10, .bottom = 8});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 10, .bottom = 6});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 2, 4, 6}, 0, world.terrain));
   CHECK(check_area({2, 0, 10, 8}, 0, world.terrain));

   REQUIRE(tracker.size() == 3);
   CHECK(tracker[0] == dirty_rect{.left = 2, .top = 0, .right = 10, .bottom = 2});
   CHECK(tracker[1] == dirty_rect{.left = 2, .top = 6, .right = 10, .bottom = 8});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 10, .bottom = 6});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce small top", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(2, 2, make_2d_array(4, 8, 1));
   auto other_edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({2, 8, 6, 10}, 1, world.terrain));
   CHECK(check_area({0, 0, 8, 8}, 2, world.terrain));

   REQUIRE(tracker.size() == 4);
   CHECK(tracker[0] == dirty_rect{.left = 2, .top = 2, .right = 6, .bottom = 10});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 2});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 2, .bottom = 8});
   CHECK(tracker[3] == dirty_rect{.left = 6, .top = 2, .right = 8, .bottom = 8});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({2, 2, 6, 10}, 0, world.terrain));
   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));

   REQUIRE(tracker.size() == 4);
   CHECK(tracker[0] == dirty_rect{.left = 2, .top = 2, .right = 6, .bottom = 10});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 0, .right = 8, .bottom = 2});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 2, .bottom = 8});
   CHECK(tracker[3] == dirty_rect{.left = 6, .top = 2, .right = 8, .bottom = 8});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce small bottom", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(2, 0, make_2d_array(4, 8, 1));
   auto other_edit = make_set_terrain_area(0, 2, make_2d_array(8, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({2, 0, 6, 2}, 1, world.terrain));
   CHECK(check_area({0, 2, 8, 10}, 2, world.terrain));

   REQUIRE(tracker.size() == 4);
   CHECK(tracker[0] == dirty_rect{.left = 2, .top = 0, .right = 6, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 8, .right = 8, .bottom = 10});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 2, .bottom = 8});
   CHECK(tracker[3] == dirty_rect{.left = 6, .top = 2, .right = 8, .bottom = 8});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({2, 0, 6, 8}, 0, world.terrain));
   CHECK(check_area({0, 2, 8, 10}, 0, world.terrain));

   REQUIRE(tracker.size() == 4);
   CHECK(tracker[0] == dirty_rect{.left = 2, .top = 0, .right = 6, .bottom = 8});
   CHECK(tracker[1] == dirty_rect{.left = 0, .top = 8, .right = 8, .bottom = 10});
   CHECK(tracker[2] == dirty_rect{.left = 0, .top = 2, .right = 2, .bottom = 8});
   CHECK(tracker[3] == dirty_rect{.left = 6, .top = 2, .right = 8, .bottom = 8});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce edge join", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(8, 0, make_2d_array(8, 8, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 1, world.terrain));
   CHECK(check_area({8, 0, 16, 8}, 2, world.terrain));

   REQUIRE(tracker.size() == 1);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 16, .bottom = 8});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));
   CHECK(check_area({8, 0, 16, 8}, 0, world.terrain));

   REQUIRE(tracker.size() == 1);
   CHECK(tracker[0] == dirty_rect{.left = 0, .top = 0, .right = 16, .bottom = 8});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce edge touching", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(8, 0, make_2d_array(8, 7, 2));

   REQUIRE(edit->is_coalescable(*other_edit));

   edit->coalesce(*other_edit);
   edit->apply(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 1, world.terrain));
   CHECK(check_area({8, 0, 16, 7}, 2, world.terrain));

   REQUIRE(world.terrain.height_map_dirty.size() == 2);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});
   CHECK(world.terrain.height_map_dirty[1] == world::dirty_rect{8, 0, 16, 7});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({0, 0, 8, 8}, 0, world.terrain));
   CHECK(check_area({8, 0, 16, 7}, 0, world.terrain));

   REQUIRE(world.terrain.height_map_dirty.size() == 2);
   CHECK(world.terrain.height_map_dirty[0] == world::dirty_rect{0, 0, 8, 8});
   CHECK(world.terrain.height_map_dirty[1] == world::dirty_rect{8, 0, 16, 7});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce complex", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(8, 8, make_2d_array(8, 8, 1));

   std::array subsequent_edits{make_set_terrain_area(10, 10, make_2d_array(4, 4, 2)),
                               make_set_terrain_area(14, 14, make_2d_array(8, 8, 3)),
                               make_set_terrain_area(14, 12, make_2d_array(2, 4, 4)),
                               make_set_terrain_area(6, 10, make_2d_array(4, 3, 5)),
                               make_set_terrain_area(17, 20, make_2d_array(2, 4, 6)),
                               make_set_terrain_area(22, 16, make_2d_array(2, 6, 7)),
                               make_set_terrain_area(14, 23, make_2d_array(8, 4, 8)),
                               make_set_terrain_area(1, 8, make_2d_array(6, 8, 9))};

   for (auto& other_edit : subsequent_edits) {
      REQUIRE(edit->is_coalescable(*other_edit));

      edit->coalesce(*other_edit);
   }

   edit->apply(edit_context);

   CHECK(check_area({8, 8, 16, 10}, 1, world.terrain));
   CHECK(check_area({14, 10, 16, 12}, 1, world.terrain));
   CHECK(check_area({8, 13, 10, 14}, 1, world.terrain));
   CHECK(check_area({8, 14, 14, 16}, 1, world.terrain));

   CHECK(check_area({10, 10, 14, 14}, 2, world.terrain));

   CHECK(check_area({16, 14, 18, 16}, 3, world.terrain));
   CHECK(check_area({14, 16, 22, 20}, 3, world.terrain));
   CHECK(check_area({14, 20, 17, 22}, 3, world.terrain));
   CHECK(check_area({19, 20, 22, 22}, 3, world.terrain));

   CHECK(check_area({14, 12, 16, 16}, 4, world.terrain));

   CHECK(check_area({7, 10, 10, 13}, 5, world.terrain));

   CHECK(check_area({17, 20, 19, 23}, 6, world.terrain));

   CHECK(check_area({22, 16, 24, 22}, 7, world.terrain));

   CHECK(check_area({14, 24, 22, 27}, 8, world.terrain));

   CHECK(check_area({1, 8, 7, 16}, 9, world.terrain));

   REQUIRE(tracker.size() == 10);
   CHECK(tracker[0] == world::dirty_rect{8, 8, 16, 16});
   CHECK(tracker[1] == world::dirty_rect{16, 14, 22, 16});
   CHECK(tracker[2] == world::dirty_rect{17, 22, 19, 24});
   CHECK(tracker[3] == world::dirty_rect{14, 16, 24, 22});
   CHECK(tracker[4] == world::dirty_rect{14, 24, 22, 27});
   CHECK(tracker[5] == world::dirty_rect{14, 23, 17, 24});
   CHECK(tracker[6] == world::dirty_rect{19, 23, 22, 24});
   CHECK(tracker[7] == world::dirty_rect{1, 8, 7, 10});
   CHECK(tracker[8] == world::dirty_rect{1, 13, 7, 16});
   CHECK(tracker[9] == world::dirty_rect{1, 10, 8, 13});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({8, 8, 16, 16}, 0, world.terrain));
   CHECK(check_area({10, 10, 14, 14}, 0, world.terrain));
   CHECK(check_area({14, 14, 22, 22}, 0, world.terrain));
   CHECK(check_area({14, 12, 16, 16}, 0, world.terrain));
   CHECK(check_area({6, 10, 10, 13}, 0, world.terrain));
   CHECK(check_area({17, 20, 19, 24}, 0, world.terrain));
   CHECK(check_area({22, 16, 24, 22}, 0, world.terrain));
   CHECK(check_area({14, 23, 22, 27}, 0, world.terrain));
   CHECK(check_area({1, 8, 7, 16}, 0, world.terrain));

   REQUIRE(tracker.size() == 10);
   CHECK(tracker[0] == world::dirty_rect{8, 8, 16, 16});
   CHECK(tracker[1] == world::dirty_rect{16, 14, 22, 16});
   CHECK(tracker[2] == world::dirty_rect{17, 22, 19, 24});
   CHECK(tracker[3] == world::dirty_rect{14, 16, 24, 22});
   CHECK(tracker[4] == world::dirty_rect{14, 24, 22, 27});
   CHECK(tracker[5] == world::dirty_rect{14, 23, 17, 24});
   CHECK(tracker[6] == world::dirty_rect{19, 23, 22, 24});
   CHECK(tracker[7] == world::dirty_rect{1, 8, 7, 10});
   CHECK(tracker[8] == world::dirty_rect{1, 13, 7, 16});
   CHECK(tracker[9] == world::dirty_rect{1, 10, 8, 13});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce corner bottom left", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(8, 8, make_2d_array(16, 8, 1));

   std::array subsequent_edits{make_set_terrain_area(16, 8, make_2d_array(8, 16, 2)),
                               make_set_terrain_area(12, 12, make_2d_array(8, 8, 3))};

   for (auto& other_edit : subsequent_edits) {
      REQUIRE(edit->is_coalescable(*other_edit));

      edit->coalesce(*other_edit);
   }

   edit->apply(edit_context);

   CHECK(check_area({8, 8, 16, 12}, 1, world.terrain));
   CHECK(check_area({8, 12, 12, 16}, 1, world.terrain));

   CHECK(check_area({16, 8, 24, 12}, 2, world.terrain));
   CHECK(check_area({20, 12, 24, 20}, 2, world.terrain));
   CHECK(check_area({16, 20, 24, 24}, 2, world.terrain));

   CHECK(check_area({12, 12, 20, 20}, 3, world.terrain));

   REQUIRE(tracker.size() == 3);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 16});
   CHECK(tracker[1] == world::dirty_rect{16, 16, 24, 24});
   CHECK(tracker[2] == world::dirty_rect{12, 16, 16, 20});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({8, 8, 24, 16}, 0, world.terrain));
   CHECK(check_area({16, 8, 24, 24}, 0, world.terrain));
   CHECK(check_area({12, 12, 20, 20}, 0, world.terrain));

   REQUIRE(tracker.size() == 3);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 16});
   CHECK(tracker[1] == world::dirty_rect{16, 16, 24, 24});
   CHECK(tracker[2] == world::dirty_rect{12, 16, 16, 20});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce top 3 sides", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(8, 16, make_2d_array(16, 8, 1));

   std::array subsequent_edits{make_set_terrain_area(8, 8, make_2d_array(4, 16, 2)),
                               make_set_terrain_area(20, 8, make_2d_array(4, 16, 3)),
                               make_set_terrain_area(11, 8, make_2d_array(10, 10, 4))};

   for (auto& other_edit : subsequent_edits) {
      REQUIRE(edit->is_coalescable(*other_edit));

      edit->coalesce(*other_edit);
   }

   edit->apply(edit_context);

   CHECK(check_area({12, 18, 20, 24}, 1, world.terrain));

   CHECK(check_area({8, 8, 11, 24}, 2, world.terrain));
   CHECK(check_area({11, 18, 12, 24}, 2, world.terrain));

   CHECK(check_area({21, 8, 24, 24}, 3, world.terrain));
   CHECK(check_area({20, 18, 21, 24}, 3, world.terrain));

   CHECK(check_area({11, 8, 21, 18}, 4, world.terrain));

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 24});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({8, 16, 24, 24}, 0, world.terrain));
   CHECK(check_area({8, 8, 12, 24}, 0, world.terrain));
   CHECK(check_area({20, 8, 24, 24}, 0, world.terrain));
   CHECK(check_area({11, 8, 21, 18}, 0, world.terrain));

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 24});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce bottom 3 sides", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(8, 8, make_2d_array(16, 8, 1));

   std::array subsequent_edits{make_set_terrain_area(8, 8, make_2d_array(4, 16, 2)),
                               make_set_terrain_area(20, 8, make_2d_array(4, 16, 3)),
                               make_set_terrain_area(11, 14, make_2d_array(10, 10, 4))};

   for (auto& other_edit : subsequent_edits) {
      REQUIRE(edit->is_coalescable(*other_edit));

      edit->coalesce(*other_edit);
   }

   edit->apply(edit_context);

   CHECK(check_area({12, 8, 20, 14}, 1, world.terrain));

   CHECK(check_area({8, 8, 12, 14}, 2, world.terrain));
   CHECK(check_area({8, 14, 11, 24}, 2, world.terrain));

   CHECK(check_area({20, 8, 14, 14}, 3, world.terrain));
   CHECK(check_area({21, 14, 24, 24}, 3, world.terrain));

   CHECK(check_area({11, 14, 21, 24}, 4, world.terrain));

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 24});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({8, 8, 24, 16}, 0, world.terrain));
   CHECK(check_area({8, 8, 12, 24}, 0, world.terrain));
   CHECK(check_area({20, 8, 24, 24}, 0, world.terrain));
   CHECK(check_area({11, 14, 21, 24}, 0, world.terrain));

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 24});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce left 3 sides", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(16, 8, make_2d_array(8, 16, 1));

   std::array subsequent_edits{make_set_terrain_area(8, 8, make_2d_array(16, 4, 2)),
                               make_set_terrain_area(8, 20, make_2d_array(16, 4, 3)),
                               make_set_terrain_area(8, 11, make_2d_array(10, 10, 4))};

   for (auto& other_edit : subsequent_edits) {
      REQUIRE(edit->is_coalescable(*other_edit));

      edit->coalesce(*other_edit);
   }

   edit->apply(edit_context);

   CHECK(check_area({18, 12, 24, 20}, 1, world.terrain));

   CHECK(check_area({8, 8, 24, 11}, 2, world.terrain));
   CHECK(check_area({18, 11, 24, 12}, 2, world.terrain));

   CHECK(check_area({8, 21, 24, 24}, 3, world.terrain));
   CHECK(check_area({18, 20, 24, 21}, 3, world.terrain));

   CHECK(check_area({8, 11, 18, 21}, 4, world.terrain));

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 24});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({16, 8, 24, 24}, 0, world.terrain));
   CHECK(check_area({8, 8, 24, 12}, 0, world.terrain));
   CHECK(check_area({8, 20, 24, 24}, 0, world.terrain));
   CHECK(check_area({8, 11, 18, 21}, 0, world.terrain));

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 24});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area coalesce right 3 sides", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};
   world::dirty_rect_tracker& tracker = world.terrain.height_map_dirty;

   auto edit = make_set_terrain_area(8, 8, make_2d_array(8, 16, 1));

   std::array subsequent_edits{make_set_terrain_area(8, 8, make_2d_array(16, 4, 2)),
                               make_set_terrain_area(8, 20, make_2d_array(16, 4, 3)),
                               make_set_terrain_area(14, 11, make_2d_array(10, 10, 4))};

   for (auto& other_edit : subsequent_edits) {
      REQUIRE(edit->is_coalescable(*other_edit));

      edit->coalesce(*other_edit);
   }

   edit->apply(edit_context);

   CHECK(check_area({8, 12, 14, 20}, 1, world.terrain));

   CHECK(check_area({8, 8, 24, 11}, 2, world.terrain));
   CHECK(check_area({8, 11, 14, 12}, 2, world.terrain));

   CHECK(check_area({8, 21, 24, 24}, 3, world.terrain));
   CHECK(check_area({8, 20, 14, 21}, 3, world.terrain));

   CHECK(check_area({14, 11, 24, 21}, 4, world.terrain));

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 24});

   world.terrain.untracked_clear_dirty_rects();

   edit->revert(edit_context);

   CHECK(check_area({8, 8, 16, 24}, 0, world.terrain));
   CHECK(check_area({8, 8, 24, 12}, 0, world.terrain));
   CHECK(check_area({8, 20, 24, 24}, 0, world.terrain));
   CHECK(check_area({14, 11, 24, 21}, 0, world.terrain));

   REQUIRE(tracker.size() == 1);

   CHECK(tracker[0] == world::dirty_rect{8, 8, 24, 24});

   CHECK(is_zeroed(world.terrain.height_map));
}

TEST_CASE("edits set_terrain_area not coalescable simple", "[Edits]")
{
   world::world world{.terrain = {.length = terrain_length}};
   world::interaction_targets interaction_targets;
   world::edit_context edit_context{world, interaction_targets.creation_entity};

   auto edit = make_set_terrain_area(0, 0, make_2d_array(8, 8, 1));
   auto other_edit = make_set_terrain_area(9, 9, make_2d_array(8, 8, 2));

   REQUIRE(not edit->is_coalescable(*other_edit));
}

}
