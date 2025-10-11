#include "world_edit.hpp"

#include "edits/creation_entity_set.hpp"
#include "world/io/load_entity_group.hpp"
#include "world/utility/entity_group_utilities.hpp"
#include "world/utility/world_utilities.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace std::literals;

namespace we {

void world_edit::ui_show_main_menu_bar() noexcept
{
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

   bool open_clear_edit_stack_confirm = false;

   if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
         if (ImGui::MenuItem("Open Data Folder")) open_project_with_picker();

         ImGui::Separator();

         const bool loaded_project = not _project_dir.empty();

         if (ImGui::BeginMenu("Load World", loaded_project)) {
            auto worlds_path = io::compose_path(_project_dir, "Worlds"sv);

            for (auto& known_world : _project_world_paths) {
               const std::string_view relative_world_path =
                  known_world.string_view().substr(worlds_path.string_view().size() + 1);

               if (ImGui::MenuItem(relative_world_path.data())) { // string_views from paths are null terminated
                  load_world(known_world);
               }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Browse...")) load_world_with_picker();

            ImGui::EndMenu();
         }

         if (not loaded_project and
             ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("You must open a data folder to load a world.");
         }

         const bool loaded_world = not _world_path.empty();

         if (ImGui::MenuItem("Save World",
                             get_display_string(
                                _hotkeys.query_binding("Global", "Save")))) {
            save_world(_world_path);
         }

         if (ImGui::MenuItem("Save World As...", nullptr, nullptr, loaded_world)) {
            save_world_with_picker();
         }

         ImGui::Separator();

         if (ImGui::MenuItem("Close World", nullptr, nullptr, loaded_world)) {
            close_world();
         }

         ImGui::Separator();

         if (ImGui::MenuItem("Save Selection as Entity Group")) {
            world::entity_group group =
               world::make_entity_group_from_selection(_world,
                                                       _interaction_targets.selection);

            world::centre_entity_group(group);

            save_entity_group_with_picker(group);
         }

         if (ImGui::MenuItem("Save World as Entity Group")) {
            save_entity_group_with_picker(world::make_entity_group_from_world(_world));
         }

         if (ImGui::BeginMenu("Save Layer as Entity Group")) {
            for (int32 i = 0; i < std::ssize(_world.layer_descriptions); ++i) {
               if (ImGui::MenuItem(_world.layer_descriptions[i].name.c_str())) {
                  save_entity_group_with_picker(
                     world::make_entity_group_from_layer(_world, i));
               }
            }

            ImGui::EndMenu();
         }

         ImGui::Separator();

         if (ImGui::MenuItem("Export Selection")) _export_selection_open = true;

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
         if (ImGui::MenuItem("Undo",
                             get_display_string(
                                _hotkeys.query_binding("Global", "Undo")))) {
            undo();
         }
         if (ImGui::MenuItem("Redo",
                             get_display_string(
                                _hotkeys.query_binding("Global", "Redo")))) {
            redo();
         }

         ImGui::Separator();

         if (ImGui::MenuItem("Delete",
                             get_display_string(
                                _hotkeys.query_binding("Global", "Delete")))) {
            delete_selected();
         }

         if (ImGui::MenuItem("Cut",
                             get_display_string(_hotkeys.query_binding("Global", "Cut")))) {
            cut_selected();
         }

         if (ImGui::MenuItem("Copy",
                             get_display_string(
                                _hotkeys.query_binding("Global", "Copy")))) {
            copy_selected();
         }

         if (ImGui::MenuItem("Paste",
                             get_display_string(
                                _hotkeys.query_binding("Global", "Paste")))) {
            paste();
         }

         ImGui::Separator();

         if (ImGui::MenuItem("Clear Undo/Redo Stacks")) {
            open_clear_edit_stack_confirm = true;
         }

         if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
               "Been editing for a while and large memory usage getting you "
               "down? Hit this to clear your undo stack and release the "
               "memory.");
         }

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("View")) {
         ImGui::MenuItem("Hotkeys",
                         get_display_string(
                            _hotkeys.query_binding("Global", "Show Hotkeys")),
                         &_hotkeys_view_show);

         if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Display a window showing context dependent "
                              "hotkeys during editing.");
         }

         ImGui::MenuItem("Camera Controls",
                         get_display_string(
                            _hotkeys.query_binding("Global",
                                                   "Show Camera Controls")),
                         &_camera_controls_open);

         ImGui::Separator();

         ImGui::MenuItem("World Global Lights Editor",
                         get_display_string(_hotkeys.query_binding(
                            "Global", "Show World Global Lights Editor")),
                         &_world_global_lights_editor_open);

         ImGui::MenuItem("World Layers Editor",
                         get_display_string(
                            _hotkeys.query_binding("Global",
                                                   "Show World Layers Editor")),
                         &_world_layers_editor_open);

         ImGui::MenuItem("World Game Mode Editor",
                         get_display_string(_hotkeys.query_binding(
                            "Global", "Show World Game Mode Editor")),
                         &_world_game_mode_editor_open);

         ImGui::MenuItem("World Requirements (.req) Editor",
                         get_display_string(_hotkeys.query_binding(
                            "Global", "Show World Requirements Editor")),
                         &_world_requirements_editor_open);

         ImGui::MenuItem("World Explorer",
                         get_display_string(
                            _hotkeys.query_binding("Global",
                                                   "Show World Explorer")),
                         &_world_explorer_open);

         ImGui::MenuItem("World Stats",
                         get_display_string(
                            _hotkeys.query_binding("Global",
                                                   "Show World Stats")),
                         &_world_stats_open);

         if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Display a window showing basic world stats such "
                              "as object count, undo stack size, etc");
         }

         ImGui::MenuItem("Object Class Browser",
                         get_display_string(_hotkeys.query_binding(
                            "Global", "Show Object Class Browser")),
                         &_object_class_browser_open);

         ImGui::Separator();

         ImGui::MenuItem("Measurement Tool",
                         get_display_string(
                            _hotkeys.query_binding("Global",
                                                   "Show Measurement Tool")),
                         &_measurement_tool_open);

         ImGui::MenuItem("Render Environment Map",
                         get_display_string(
                            _hotkeys.query_binding("Global",
                                                   "Render Environment Map")),
                         &_render_env_map_open);

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Terrain")) {
         if (ImGui::MenuItem("Terrain Height Editor",
                             get_display_string(_hotkeys.query_binding(
                                "Global", "Show Terrain Height Editor")))) {
            _terrain_edit_tool = terrain_edit_tool::editor;
            _terrain_editor_config.edit_target = terrain_edit_target::height;
         }

         if (ImGui::MenuItem("Terrain Texture Editor",
                             get_display_string(_hotkeys.query_binding(
                                "Global", "Show Terrain Texture Editor")))) {
            _terrain_edit_tool = terrain_edit_tool::editor;
            _terrain_editor_config.edit_target = terrain_edit_target::texture;
         }

         if (ImGui::MenuItem("Terrain Colour Editor",
                             get_display_string(_hotkeys.query_binding(
                                "Global", "Show Terrain Colour Editor")))) {
            _terrain_edit_tool = terrain_edit_tool::editor;
            _terrain_editor_config.edit_target = terrain_edit_target::color;
         }

         if (ImGui::MenuItem("Import Terrain Heightmap")) {
            _terrain_edit_tool = terrain_edit_tool::import_heightmap;
            _terrain_import_heightmap_context = {};
            _edit_stack_world.close_last();
         }

         if (ImGui::MenuItem("Import Terrain Texture Weight Map")) {
            _terrain_edit_tool = terrain_edit_tool::import_texture_weight_map;
            _terrain_import_texture_weight_map_context = {};
            _edit_stack_world.close_last();
         }

         if (ImGui::MenuItem("Import Terrain Color Map")) {
            _terrain_edit_tool = terrain_edit_tool::import_color_map;
            _terrain_import_color_map_context = {};
            _edit_stack_world.close_last();
         }

         if (ImGui::MenuItem("Resize Terrain")) {
            _terrain_edit_tool = terrain_edit_tool::resize;
            _terrain_resize_context = {};
         }

         ImGui::SetItemTooltip("Resize the terrain, scaling it like an image.");

         if (ImGui::MenuItem("Crop Terrain")) {
            _terrain_edit_tool = terrain_edit_tool::crop;
            _terrain_crop_context = {};
         }

         ImGui::SetItemTooltip(
            "Crop the terrain, reducing it's size but perfectly maintaining "
            "the uncropped terrain area.");

         if (ImGui::MenuItem("Extend Terrain")) {
            _terrain_edit_tool = terrain_edit_tool::extend;
            _terrain_extend_context = {};
         }

         ImGui::SetItemTooltip("Extend the terrain, increasing it's size while "
                               "keeping the current terrain area the same.");

         if (ImGui::MenuItem("Floor Terrain")) floor_terrain();

         ImGui::SetItemTooltip(
            "Floor the terrain, lowering it as much as possible while "
            "keeping the distance between it's highest and lowest points the "
            "same..");

         if (ImGui::MenuItem("Bake Terrain Lighting",
                             get_display_string(_hotkeys.query_binding(
                                "Global", "Show Bake Terrain Lighting")))) {
            _terrain_edit_tool = terrain_edit_tool::light_baker;
         }

         ImGui::SetItemTooltip("Also known as Burn Terrain in Zero Editor.");

         ImGui::Separator();

         if (ImGui::MenuItem("Water Editor",
                             get_display_string(
                                _hotkeys.query_binding("Global",
                                                       "Show Water Editor")))) {
            _terrain_edit_tool = terrain_edit_tool::water_editor;
            _water_editor_context = {};
         }

         if (ImGui::MenuItem("Foliage Editor",
                             get_display_string(_hotkeys.query_binding("Global", "Show Foliage Editor")))) {
            _terrain_edit_tool = terrain_edit_tool::foliage_editor;
            _foliage_editor_context = {};
         }

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Create")) {
         ui_show_create_menu_items();

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Animation")) {
         if (ImGui::MenuItem("Animation Editor",
                             get_display_string(_hotkeys.query_binding("Global", "Show Animation Editor")),
                             _animation_editor_open)) {
            _animation_editor_open = not _animation_editor_open;
            _animation_editor_context = {};
         }

         if (ImGui::MenuItem("Animation Group Editor",
                             get_display_string(_hotkeys.query_binding("Global", "Show Animation Group Editor")),
                             _animation_group_editor_open)) {
            _animation_group_editor_open = not _animation_group_editor_open;
            _animation_group_editor_context = {};
         }

         if (ImGui::MenuItem("Animation Hierarchy Editor",
                             get_display_string(_hotkeys.query_binding("Global", "Show Animation Hierarchy Editor")),
                             _animation_hierarchy_editor_open)) {
            _animation_hierarchy_editor_open = not _animation_hierarchy_editor_open;
            _animation_hierarchy_editor_context = {};
         }

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Blocks")) {
         if (ImGui::MenuItem("Blocks Editor",
                             get_display_string(_hotkeys.query_binding("Global", "Show Blocks Editor")),
                             _block_editor_open)) {
            _block_editor_open = not _block_editor_open;
            _block_editor_context = {};
         }

         if (ImGui::MenuItem("Material Editor",
                             get_display_string(_hotkeys.query_binding("Global", "Show Blocks Material Editor")),
                             _block_material_editor_open)) {
            _block_material_editor_open = not _block_material_editor_open;
            _block_material_editor_context = {};
         }

         ImGui::Separator();

         if (ImGui::MenuItem("Draw Box")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::box;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Ramp")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::ramp;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Quadrilateral")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::quad;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Cylinder")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::cylinder;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Stairs")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::stairway;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Stairs (Floating)")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::stairway_floating;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Cone")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::cone;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Hemisphere")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::hemisphere;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Pyramid")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::pyramid;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Ring")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::ring;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Beveled Box")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::beveled_box;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Curve")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::curve;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Arch")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::arch;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         if (ImGui::MenuItem("Draw Terrain Cutter (Box)")) {
            _block_editor_open = true;
            _block_editor_config.draw_type = draw_block_type::terrain_cut_box;
            _block_editor_context = {.activate_tool = block_edit_tool::draw};
         }

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Munge")) {
         if (ImGui::MenuItem("Munge",
                             get_display_string(_hotkeys.query_binding("Global", "Run Munge")),
                             nullptr, not _munge_manager.is_busy())) {
            _munge_manager_open = true;

            if (not _munge_manager.is_busy()) {
               _munge_manager.start_munge(
                  io::path{_settings.preferences.game_install_path});
            }
         }

         if (ImGui::MenuItem("Clean", nullptr, nullptr, not _munge_manager.is_busy())) {
            _munge_manager_open = true;

            if (not _munge_manager.is_busy()) _munge_manager.start_clean();
         }

         ImGui::Separator();

         ImGui::MenuItem("Show Munge Manager",
                         get_display_string(
                            _hotkeys.query_binding("Global",
                                                   "Show Munge Manager")),
                         &_munge_manager_open);

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Settings")) {
         ImGui::MenuItem("Settings Editor", nullptr, &_settings_editor_open);
         ImGui::MenuItem("Hotkeys Editor", nullptr, &_hotkeys_editor_open);
         ImGui::MenuItem("About", nullptr, &_about_window_open);

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Developer")) {
         if (ImGui::MenuItem("Reload Shaders")) {
            try {
               _renderer->reload_shaders();
            }
            catch (graphics::gpu::exception& e) {
               handle_gpu_error(e);
            }
         }

         if (ImGui::MenuItem("Reset Thumbnails")) {
            _renderer->reset_thumbnails();
         }

         ImGui::MenuItem("Show GPU Profiler", nullptr, &_settings.graphics.show_profiler);

         ImGui::SetItemTooltip("Some GPU work is not measured.");

         ImGui::MenuItem("Show ImGui Demo", nullptr, &_imgui_demo_open);

         ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
   }

   ImGui::PopStyleVar();

   // Clear Edit Stack Confirmation Window
   if (open_clear_edit_stack_confirm) {
      ImGui::OpenPopup("Clear Undo/Redo Stacks?");
   }

   const ImVec2 imgui_center = ImGui::GetMainViewport()->GetCenter();
   ImGui::SetNextWindowPos(imgui_center, ImGuiCond_Appearing, {0.5f, 0.5f});

   if (ImGui::BeginPopupModal("Clear Undo/Redo Stacks?", nullptr,
                              ImGuiWindowFlags_AlwaysAutoResize)) {

      ImGui::Text("Clear the Undo/Redo stacks to free memory? This "
                  "(unsurprisingly) cannot be undone!");
      ImGui::Separator();

      if (ImGui::Button("OK", {120.0f, 0.0f})) {
         _edit_stack_world = {};
         ImGui::CloseCurrentPopup();
      }
      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();
      if (ImGui::Button("Cancel", {120.0f, 0.0f})) {
         ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
   }
}

void world_edit::ui_show_create_menu_items() noexcept
{
   const int8 creation_layer =
      _last_created_entities.last_layer < std::ssize(_world.layer_descriptions)
         ? _last_created_entities.last_layer
         : 0;

   if (ImGui::MenuItem("Object")) {
      const world::object* base_object =
         world::find_entity(_world.objects, _last_created_entities.last_object);

      world::object new_object;

      if (base_object) {
         new_object = *base_object;

         new_object.name =
            world::create_unique_name(_world.objects, base_object->name);
         new_object.layer = creation_layer;
         new_object.id = world::max_id;
      }
      else {
         new_object = world::object{.name = "",
                                    .layer = creation_layer,
                                    .class_name = lowercase_string{""sv},
                                    .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_object),
                                                              _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.objects = true;
   }

   if (ImGui::MenuItem("Light")) {
      const world::light* base_light =
         world::find_entity(_world.lights, _last_created_entities.last_light);

      world::light new_light;

      if (base_light) {
         new_light = *base_light;

         new_light.name = world::create_unique_name(_world.lights, base_light->name);
         new_light.layer = creation_layer;
         new_light.id = world::max_id;
      }
      else {
         new_light =
            world::light{.name = "Light", .layer = creation_layer, .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_light),
                                                              _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.lights = true;
   }

   if (ImGui::MenuItem("Path")) {
      const world::path* base_path =
         world::find_entity(_world.paths, _last_created_entities.last_path);

      _edit_stack_world.apply(
         edits::make_creation_entity_set(
            world::path{.name = world::create_unique_name(_world.paths,
                                                          base_path ? base_path->name : "Path 0"),
                        .layer = base_path ? base_path->layer : creation_layer,
                        .nodes = {world::path::node{}},
                        .id = world::max_id},
            _object_classes),
         _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.paths = true;
   }

   if (ImGui::MenuItem("Region")) {
      const world::region* base_region =
         world::find_entity(_world.regions, _last_created_entities.last_region);

      world::region new_region;

      if (base_region) {
         new_region = *base_region;

         new_region.name = world::create_unique_name(_world.regions, _world.lights,
                                                     base_region->name);
         new_region.layer = creation_layer;
         new_region.id = world::max_id;
      }
      else {
         new_region = world::region{.name = world::create_unique_name(_world.lights, "Region0"),
                                    .layer = creation_layer,
                                    .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_region),
                                                              _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.regions = true;
   }

   if (ImGui::MenuItem("Sector")) {
      const world::sector* base_sector =
         world::find_entity(_world.sectors, _last_created_entities.last_sector);

      _edit_stack_world.apply(edits::make_creation_entity_set(
                                 world::sector{.name = world::create_unique_name(
                                                  _world.sectors,
                                                  base_sector ? base_sector->name : "Sector0"),
                                               .base = 0.0f,
                                               .height = 10.0f,
                                               .points = {{0.0f, 0.0f}},
                                               .id = world::max_id},
                                 _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.sectors = true;
   }

   if (ImGui::MenuItem("Portal")) {
      const world::portal* base_portal =
         world::find_entity(_world.portals, _last_created_entities.last_portal);

      world::portal new_portal;

      if (base_portal) {
         new_portal = *base_portal;

         new_portal.name =
            world::create_unique_name(_world.portals, base_portal->name);
         new_portal.id = world::max_id;
      }
      else {
         new_portal = world::portal{.name = "Portal0", .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_portal),
                                                              _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.portals = true;
   }

   if (ImGui::MenuItem("Hintnode")) {
      const world::hintnode* base_hintnode =
         world::find_entity(_world.hintnodes, _last_created_entities.last_hintnode);

      world::hintnode new_hintnode;

      if (base_hintnode) {
         new_hintnode = *base_hintnode;

         new_hintnode.name =
            world::create_unique_name(_world.hintnodes, base_hintnode->name);
         new_hintnode.layer = creation_layer;
         new_hintnode.id = world::max_id;
      }
      else {
         new_hintnode = world::hintnode{.name = "Hint0",
                                        .layer = creation_layer,
                                        .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_hintnode),
                                                              _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.hintnodes = true;
   }

   if (ImGui::MenuItem("Barrier")) {
      const world::barrier* base_barrier =
         world::find_entity(_world.barriers, _last_created_entities.last_barrier);

      world::barrier new_barrier;

      if (base_barrier) {
         new_barrier = *base_barrier;

         new_barrier.name =
            world::create_unique_name(_world.barriers, base_barrier->name);
         new_barrier.id = world::max_id;
      }
      else {
         new_barrier = world::barrier{.name = "Barrier0", .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_barrier),
                                                              _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.barriers = true;
   }

   if (ImGui::MenuItem("AI Planning Hub")) {
      const world::planning_hub* base_hub =
         world::find_entity(_world.planning_hubs,
                            _last_created_entities.last_planning_hub);

      world::planning_hub new_hub;

      if (base_hub) {
         new_hub = *base_hub;

         new_hub.name =
            world::create_unique_name(_world.planning_hubs, base_hub->name);
         new_hub.id = world::max_id;
      }
      else {
         new_hub = world::planning_hub{.name = "Hub0", .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_hub),
                                                              _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.planning_hubs = true;
      _world_draw_mask.planning_connections = true;
   }

   if (ImGui::MenuItem("AI Planning Connection") and not _world.planning_hubs.empty()) {
      const world::planning_connection* base_connection =
         world::find_entity(_world.planning_connections,
                            _last_created_entities.last_planning_connection);

      world::planning_connection new_connection;

      if (base_connection) {
         new_connection = *base_connection;

         new_connection.name = world::create_unique_name(_world.planning_connections,
                                                         base_connection->name);
         new_connection.id = world::max_id;
      }
      else {
         new_connection = world::planning_connection{.name = "Connection0",
                                                     .start_hub_index = 0,
                                                     .end_hub_index = 0,
                                                     .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_connection),
                                                              _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.planning_hubs = true;
      _world_draw_mask.planning_connections = true;
   }

   if (ImGui::MenuItem("Boundary")) {
      const world::boundary* base_boundary =
         world::find_entity(_world.boundaries, _last_created_entities.last_boundary);

      world::boundary new_boundary;

      if (base_boundary) {
         new_boundary = *base_boundary;

         new_boundary.name =
            world::create_unique_name(_world.boundaries, base_boundary->name);
         new_boundary.id = world::max_id;
      }
      else {
         new_boundary = world::boundary{.name = "Boundary", .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_boundary),
                                                              _object_classes),
                              _edit_context);
      _entity_creation_context = {};
      _world_draw_mask.boundaries = true;
   }

   if (ImGui::BeginMenu("From Group")) {
      ImGui::InputText("Filter", &_entity_group_filter);

      if (_recent_entity_groups.size() > 0) {
         ImGui::SeparatorText("Recent");
      }

      std::string_view group_to_create;

      for (uint32 i = 0; i < _recent_entity_groups.size(); ++i) {
         if (ImGui::Selectable(_recent_entity_groups[i].c_str())) {
            group_to_create = _recent_entity_groups[i];
         }
      }

      ImGui::SeparatorText("Groups");

      ImGui::BeginChild("Groups",
                        {0.0f, ImGui::GetTextLineHeightWithSpacing() * 16.0f});

      _asset_libraries.entity_groups.view_existing(
         [&](const std::span<const assets::stable_string> assets) noexcept {
            for (std::string_view group : assets) {
               if (not _entity_group_filter.empty() and
                   not string::icontains(group, _entity_group_filter)) {
                  continue;
               }

               ImGui::PushID(group.data(), group.data() + group.size());

               const ImVec2 cursor = ImGui::GetCursorPos();

               if (ImGui::Selectable("##group")) {
                  group_to_create = group;

                  ImGui::CloseCurrentPopup();
               }

               ImGui::SetCursorPos(cursor);

               ImGui::TextUnformatted(group.data(), group.data() + group.size());

               ImGui::PopID();
            }
         });

      ImGui::EndChild();

      ImGui::EndMenu();

      if (not group_to_create.empty()) {
         try {
            _edit_stack_world.apply(
               edits::make_creation_entity_set(
                  world::load_entity_group(_asset_libraries.entity_groups.query_path(
                                              lowercase_string{group_to_create}),
                                           *_stream),
                  _object_classes),
               _edit_context);

            _recent_entity_groups.insert(lowercase_string{group_to_create});
         }
         catch (std::exception& e) {
            MessageBoxA(_window, e.what(), "Failed to load entity group!", MB_OK);
         }
      }
   }
}

}