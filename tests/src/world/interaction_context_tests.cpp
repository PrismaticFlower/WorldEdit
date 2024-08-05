#include "pch.h"

#include "world/interaction_context.hpp"

#include <algorithm>

namespace we::world::tests {

namespace {

const we::world::world test_world = {
   .name = "Test",

   .requirements = {{.file_type = "world", .entries = {"Test"}}},

   .layer_descriptions = {{.name = "[Base]"}},
   .game_modes = {{.name = "Common", .layers = {0}}},

   .terrain = {},
   .global_lights = {.env_map_texture = "sky"},

   .objects = {entities_init, std::initializer_list{object{.id = object_id{0}}}},

   .paths = {entities_init, std::initializer_list{path{
                               .name = "test_path",
                               .layer = 0,

                               .nodes = {{
                                            .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                                            .position = {0.0f, 0.0f, 0.0f},
                                         },
                                         {
                                            .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                                            .position = {1.0f, 1.0f, 1.0f},
                                         }},

                               .id = path_id{3},
                            }}},
};

auto make_test_node_mask(bool v0, bool v1, bool v2, bool v3) -> path_id_node_mask::node_mask
{
   path_id_node_mask::node_mask result;

   if (v0) result.set(0);
   if (v1) result.set(1);
   if (v2) result.set(2);
   if (v3) result.set(3);

   return result;
}

}

TEST_CASE("world interaction_target", "[World]")
{
   interaction_target target;

   CHECK(not target.holds_entity_id());

   auto check_type = [&]<typename T>(T value) {
      target = value;

      CHECK(target.holds_entity_id());
      CHECK(target.is<T>());
      CHECK(target.get<T>() == value);
   };

   check_type(object_id{1});
   check_type(light_id{2});
   check_type(path_id_node_mask{.id = path_id{3}});
   check_type(region_id{4});
   check_type(sector_id{5});
   check_type(portal_id{6});
   check_type(barrier_id{7});
   check_type(hintnode_id{8});
   check_type(planning_hub_id{9});
   check_type(planning_connection_id{10});
   check_type(boundary_id{11});
   check_type(measurement_id{12});

   interaction_target other_target;

   CHECK(not other_target.holds_entity_id());

   other_target = target;

   CHECK(target.holds_entity_id());

   CHECK(other_target.holds_entity_id());
   CHECK(other_target.is<measurement_id>());
   CHECK(other_target.get<measurement_id>() == measurement_id{12});
}

TEST_CASE("world creation_entity", "[World]")
{
   creation_entity entity{barrier{.name = "Barrier"}};

   CHECK(entity.holds_entity());
   CHECK(entity.is<barrier>());
   CHECK(entity.get<barrier>().name == "Barrier");

   auto check_type = [&]<typename T>(T value) {
      entity = value;

      CHECK(entity.holds_entity());
      CHECK(entity.is<T>());
      CHECK(entity.get<T>() == value);
   };

   check_type(object{.name = "Object", .position = {0.0f, 1.0f, 1.0f}});
   check_type(light{.name = "Light", .position = {0.0f, 8.0f, 1.0f}});
   check_type(path{.name = "Path"});
   check_type(region{.name = "Region", .size = {10.0f, 1.0f, 1.0f}});
   check_type(sector{.name = "Sector"});
   check_type(portal{.name = "Portal", .width = 10.0f, .height = 16.0f});
   check_type(barrier{.name = "Barrier", .size = {15.0f, 14.0f}});
   check_type(hintnode{.name = "Hintnode", .position = {2.0f, 2.0f, 2.0f}});
   check_type(planning_hub{.name = "Hub", .position = {0.0f, 8.0f, 0.0f}});
   check_type(planning_connection{.name = "Connection", .jet_jump = true});
   check_type(boundary{.name = "Boundary", .position = {0.5f, 0.0f, 0.5f}});
   check_type(entity_group{});
   check_type(measurement{.start = {0.0f, 0.5f, 0.0f}, .name = "Measurement"});

   creation_entity other_entity{creation_entity_none};

   CHECK(not other_entity.holds_entity());

   other_entity = std::move(entity);

   CHECK(not entity.holds_entity());

   CHECK(other_entity.holds_entity());
   CHECK(other_entity.is<measurement>());
   CHECK(other_entity.get<measurement>().name == "Measurement");
}

TEST_CASE("world interaction_context is_valid", "[World][ID]")
{
   CHECK(is_valid(object_id{0}, test_world));
   CHECK(not is_valid(object_id{1}, test_world));

   CHECK(is_valid(make_path_id_node_mask(path_id{3}, 0), test_world));
   CHECK(is_valid(make_path_id_node_mask(path_id{3}, 1), test_world));
   CHECK(not is_valid(make_path_id_node_mask(path_id{3}, 2), test_world));
   CHECK(not is_valid(make_path_id_node_mask(path_id{0}, 0), test_world));
}

TEST_CASE("world interaction_context is_selected", "[World][ID]")
{
   selection selection;

   selection.add(object_id{0});
   selection.add(region_id{0});

   CHECK(is_selected(object_id{0}, selection));
   CHECK(not is_selected(object_id{1}, selection));
}

TEST_CASE("world interaction_context is_selected path", "[World][ID]")
{
   selection selection;

   selection.add(make_path_id_node_mask(path_id{0}, 0));

   CHECK(is_selected(path_id{0}, selection));
   CHECK(not is_selected(path_id{1}, selection));
}

TEST_CASE("world interaction_context is_selected path mask", "[World][ID]")
{
   selection selection;

   selection.add(make_path_id_node_mask(path_id{0}, 1));
   selection.add(make_path_id_node_mask(path_id{0}, 2));

   path_id_node_mask id_node_mask{.id = path_id{0},
                                  .nodes = make_test_node_mask(false, true, true, false)};

   id_node_mask.nodes.set(1);
   id_node_mask.nodes.set(2);

   CHECK(is_selected(id_node_mask, selection));

   id_node_mask = {.id = path_id{0},
                   .nodes = make_test_node_mask(true, true, false, false)};

   CHECK(not is_selected(id_node_mask, selection));

   id_node_mask = {.id = path_id{2},
                   .nodes = make_test_node_mask(false, true, false, false)};

   CHECK(not is_selected(id_node_mask, selection));
}

TEST_CASE("world make_path_id_node_mask", "[World][ID]")
{
   path_id_node_mask id_node_mask{.id = path_id{2}};

   id_node_mask.nodes.set(2);

   CHECK(make_path_id_node_mask(path_id{2}, 2).id == id_node_mask.id);
   CHECK(make_path_id_node_mask(path_id{2}, 2).nodes == id_node_mask.nodes);
}

TEST_CASE("world selection", "[World][ID]")
{
   selection selection;

   const auto test_type = [&]<typename T>(const T) {
      const std::size_t start_size = selection.size();

      selection.add(T{0});
      selection.add(T{1});
      selection.add(T{1});

      REQUIRE(selection.size() == start_size + 2);

      CHECK(std::ranges::contains(selection.view(), selected_entity{T{0}}));
      CHECK(std::ranges::contains(selection.view(), selected_entity{T{1}}));
   };

   CHECK(selection.empty());
   CHECK(selection.size() == 0);

   test_type(object_id{});
   test_type(light_id{});
   test_type(region_id{});
   test_type(sector_id{});
   test_type(portal_id{});
   test_type(hintnode_id{});
   test_type(barrier_id{});
   test_type(planning_hub_id{});
   test_type(planning_connection_id{});
   test_type(boundary_id{});

   CHECK(selection[1] == selected_entity{object_id{1}});

   CHECK(not selection.empty());
   CHECK(selection.size() == 20);

   selection.add(make_path_id_node_mask(path_id{0}, 0));
   selection.add(make_path_id_node_mask(path_id{0}, 0));
   selection.add(make_path_id_node_mask(path_id{0}, 1));
   selection.add(make_path_id_node_mask(path_id{1}, 0));
   selection.add(make_path_id_node_mask(path_id{1}, 0));
   selection.add(make_path_id_node_mask(path_id{1}, 1));

   CHECK(selection.size() == 22);

   CHECK(std::ranges::contains(selection.view(),
                               selected_entity{
                                  path_id_node_mask{path_id{0},
                                                    make_test_node_mask(true, true, false,
                                                                        false)}}));
   CHECK(std::ranges::contains(selection.view(),
                               selected_entity{
                                  path_id_node_mask{path_id{1},
                                                    make_test_node_mask(true, true, false,
                                                                        false)}}));

   CHECK(not selection.empty());

   selection.clear();

   CHECK(selection.empty());
}

TEST_CASE("world selection remove path partial", "[World][ID]")
{
   selection selection;

   selection.add(path_id_node_mask{path_id{0},
                                   make_test_node_mask(true, true, true, true)});

   selection.remove(make_path_id_node_mask(path_id{0}, 1));

   REQUIRE(selection.size() == 1);
   CHECK(selection[0].get<path_id_node_mask>() ==
         path_id_node_mask{path_id{0}, make_test_node_mask(true, false, true, true)});

   selection.remove(make_path_id_node_mask(path_id{0}, 2));

   REQUIRE(selection.size() == 1);
   CHECK(selection[0].get<path_id_node_mask>() ==
         path_id_node_mask{path_id{0}, make_test_node_mask(true, false, false, true)});

   selection.remove(make_path_id_node_mask(path_id{0}, 0));

   REQUIRE(selection.size() == 1);
   CHECK(selection[0].get<path_id_node_mask>() ==
         path_id_node_mask{path_id{0}, make_test_node_mask(false, false, false, true)});

   selection.remove(make_path_id_node_mask(path_id{0}, 3));

   CHECK(selection.empty());
}

TEST_CASE("world path_id_node_mask node_mask core", "[World][ID]")
{
   path_id_node_mask::node_mask mask;

   mask.set(127);
   mask.set(0);

   CHECK(mask[0]);
   CHECK(mask[127]);

   for (uint32 i = 1; i < 127; ++i) CHECK(not mask[i]);
   for (uint32 i = 128; i < mask.size(); ++i) CHECK(not mask[i]);
}

TEST_CASE("world path_id_node_mask node_mask reset", "[World][ID]")
{
   path_id_node_mask::node_mask mask;

   mask.set(64);

   CHECK(mask[64]);

   mask.reset(64);

   CHECK(not mask[64]);
}

TEST_CASE("world path_id_node_mask node_mask out of bounds", "[World][ID]")
{
   path_id_node_mask::node_mask mask;

   mask.set(1000000);
   mask.reset(1000000);
   CHECK(not mask[1000000]);
}

TEST_CASE("world path_id_node_mask node_mask operator|", "[World][ID]")
{
   const auto mask = make_test_node_mask(true, false, false, false) |
                     make_test_node_mask(false, false, false, true);

   CHECK(mask == make_test_node_mask(true, false, false, true));
}

TEST_CASE("world path_id_node_mask node_mask operator&", "[World][ID]")
{
   const auto mask = make_test_node_mask(true, true, false, false) &
                     make_test_node_mask(false, true, false, true);

   CHECK(mask == make_test_node_mask(false, true, false, false));
}

}
