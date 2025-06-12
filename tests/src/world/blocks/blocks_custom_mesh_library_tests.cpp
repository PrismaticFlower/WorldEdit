#include "pch.h"

#include "world/blocks/custom_mesh.hpp"
#include "world/blocks/custom_mesh_library.hpp"

using namespace std::literals;

namespace we::world::tests {

TEST_CASE("world blocks_custom_mesh_library acquire-free", "[World]")
{
   blocks_custom_mesh_library library;
   const block_custom_mesh_description stairway0 =
      block_custom_mesh_description_stairway{.size = {2.0f, 2.0f, 2.0f},
                                             .step_height = 0.25f};
   const block_custom_mesh_description stairway1 =
      block_custom_mesh_description_stairway{.size = {2.0f, 2.0f, 2.0f},
                                             .step_height = 0.25f,
                                             .first_step_offset = 0.1f};

   block_custom_mesh_handle handle0 = library.add(stairway0);

   CHECK(library.debug_ref_count(stairway0) == 1);

   block_custom_mesh_handle handle1 = library.add(stairway1);

   CHECK(library.debug_ref_count(stairway1) == 1);

   block_custom_mesh_handle handle2 = library.add(stairway0);

   CHECK(library.debug_ref_count(stairway0) == 2);

   library.remove(handle0);

   CHECK(library.debug_ref_count(stairway0) == 1);

   library.remove(handle1);

   CHECK(library.debug_ref_count(stairway1) == 0);

   library.remove(handle2);

   CHECK(library.debug_ref_count(stairway0) == 0);
}

TEST_CASE("world blocks_custom_mesh_library acquire-free events", "[World]")
{
   blocks_custom_mesh_library library;

   const block_custom_mesh_description stairway0 =
      block_custom_mesh_description_stairway{.size = {2.0f, 2.0f, 2.0f},
                                             .step_height = 0.25f};
   const block_custom_mesh_description stairway1 =
      block_custom_mesh_description_stairway{.size = {2.0f, 2.0f, 2.0f},
                                             .step_height = 0.25f,
                                             .first_step_offset = 0.1f};

   block_custom_mesh_handle handle0 = library.add(stairway0);
   block_custom_mesh_handle handle1 = library.add(stairway1);
   block_custom_mesh_handle handle2 = library.add(stairway0);

   library.remove(handle0);
   library.remove(handle1);
   library.remove(handle2);

   REQUIRE(library.events().size() == 4);

   using event_type = blocks_custom_mesh_library::event_type;
   using event = blocks_custom_mesh_library::event;

   CHECK(library.events()[0] == event{event_type::mesh_added, handle0});
   CHECK(library.events()[1] == event{event_type::mesh_added, handle1});
   CHECK(library.events()[2] == event{event_type::mesh_removed, handle1});
   CHECK(library.events()[3] == event{event_type::mesh_removed, handle0});

   CHECK(library.debug_ref_count(stairway0) == 0);
   CHECK(library.debug_ref_count(stairway1) == 0);

   library.clear_events();

   CHECK(library.events().empty());
}

TEST_CASE("world blocks_custom_mesh_library events ref", "[World]")
{
   blocks_custom_mesh_library library;

   const block_custom_mesh_description stairway0 =
      block_custom_mesh_description_stairway{.size = {2.0f, 2.0f, 2.0f},
                                             .step_height = 0.25f};
   const block_custom_mesh_description stairway1 =
      block_custom_mesh_description_stairway{.size = {2.0f, 2.0f, 2.0f},
                                             .step_height = 0.25f,
                                             .first_step_offset = 0.1f};

   block_custom_mesh_handle handle0 = library.add(stairway0);
   block_custom_mesh_handle handle1 = library.add(stairway1);
   block_custom_mesh_handle handle2 = library.add(stairway0);

   library.remove(handle0);
   library.remove(handle1);
   library.remove(handle2);

   CHECK(library.debug_ref_count(stairway0, false) == 1);
   CHECK(library.debug_ref_count(stairway1, false) == 1);

   library.clear_events();

   CHECK(library.debug_ref_count(stairway0, false) == 0);
   CHECK(library.debug_ref_count(stairway1, false) == 0);
}

TEST_CASE("world blocks_custom_mesh_library restore events", "[World]")
{
   blocks_custom_mesh_library library;

   const block_custom_mesh_description stairway0 =
      block_custom_mesh_description_stairway{.size = {2.0f, 2.0f, 2.0f},
                                             .step_height = 0.25f};
   const block_custom_mesh_description stairway1 =
      block_custom_mesh_description_stairway{.size = {2.0f, 2.0f, 2.0f},
                                             .step_height = 0.25f,
                                             .first_step_offset = 0.1f};

   block_custom_mesh_handle handle0 = library.add(stairway0);
   block_custom_mesh_handle handle1 = library.add(stairway1);
   [[maybe_unused]] block_custom_mesh_handle handle2 = library.add(stairway0);

   library.issue_restore_events();

   using event_type = blocks_custom_mesh_library::event_type;
   using event = blocks_custom_mesh_library::event;

   REQUIRE(library.events().size() == 3);

   CHECK(library.events()[0] == event{event_type::cleared});
   CHECK(library.events()[1] == event{event_type::mesh_added, handle0});
   CHECK(library.events()[2] == event{event_type::mesh_added, handle1});

   CHECK(library.debug_ref_count(stairway0, false) == 3);
   CHECK(library.debug_ref_count(stairway1, false) == 2);
}

TEST_CASE("world blocks_custom_mesh_library null handle", "[World]")
{
   blocks_custom_mesh_library library;

   const block_custom_mesh& mesh =
      library[blocks_custom_mesh_library::null_handle()];

   CHECK(mesh.vertices.size() == 0);
   CHECK(mesh.triangles.size() == 0);
   CHECK(mesh.occluders.size() == 0);
   CHECK(mesh.collision_vertices.size() == 0);
   CHECK(mesh.collision_triangles.size() == 0);
   CHECK(mesh.collision_occluders.size() == 0);
   CHECK(mesh.snap_points.size() == 0);
   CHECK(mesh.snap_edges.size() == 0);

   // Removing the null_handle is a no-op.
   library.remove(blocks_custom_mesh_library::null_handle());
}

}