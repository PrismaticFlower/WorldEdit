#include "world_edit.hpp"

#include <imgui.h>

using namespace std::literals;

namespace we {

void world_edit::update_ui() noexcept
{
   if (_imgui_demo_open) ImGui::ShowDemoWindow(&_imgui_demo_open);
   if (_hotkeys_editor_open) {
      _hotkeys.show_imgui(_hotkeys_editor_open, _display_scale);
   }

   ui_show_main_menu_bar();
   ui_show_world_active_context();

   if (_hotkeys_view_show) ui_show_hotkeys_view();

   if (not _interaction_targets.selection.empty()) {
      ui_show_world_selection_editor();
   }

   if (_interaction_targets.creation_entity.holds_entity()) {
      ui_show_world_creation_editor();
   }

   if (_world_global_lights_editor_open) {
      ui_show_world_global_lights_editor();
   }

   if (_world_layers_editor_open) {
      ui_show_world_layers_editor();
   }

   if (_world_game_mode_editor_open) {
      ui_show_world_game_mode_editor();
   }

   if (_world_requirements_editor_open) {
      ui_show_world_requirements_editor();
   }

   if (_world_explorer_open) {
      ui_show_world_explorer();
   }

   if (_world_stats_open) {
      ui_show_world_stats();
   }

   if (_object_class_browser_open) {
      ui_show_object_class_browser();
   }

   if (_camera_controls_open) {
      ui_show_camera_controls();
   }

   if (_settings_editor_open) {
      settings::show_imgui_editor(_settings, _settings_editor_open, _display_scale);

      if (not _settings_editor_open) {
         settings::save(".settings", _settings);
      }
   }

   if (_about_window_open) {
      ui_show_about_window();
   }

   if (_interaction_targets.selection.empty()) {
      _selection_edit_tool = selection_edit_tool::none;
   }

   if (_selection_edit_tool == selection_edit_tool::move) {
      ui_show_world_selection_move();
   }

   if (_selection_edit_tool == selection_edit_tool::move_with_cursor) {
      ui_show_world_selection_move_with_cursor();
   }

   if (_selection_edit_tool == selection_edit_tool::move_path) {
      ui_show_world_selection_move_path();
   }

   if (_selection_edit_tool == selection_edit_tool::move_sector_point) {
      ui_show_world_selection_move_sector_point();
   }

   if (_selection_edit_tool == selection_edit_tool::rotate) {
      ui_show_world_selection_rotate();
   }

   if (_selection_edit_tool == selection_edit_tool::rotate_around_centre) {
      ui_show_world_selection_rotate_around_centre();
   }

   if (_selection_edit_tool == selection_edit_tool::set_layer) {
      ui_show_world_selection_set_layer();
   }

   if (_terrain_edit_tool == terrain_edit_tool::editor) {
      ui_show_terrain_editor();
   }
   else if (_terrain_edit_tool == terrain_edit_tool::import_heightmap) {
      ui_show_terrain_import_height_map();
   }
   else if (_terrain_edit_tool == terrain_edit_tool::import_texture_weight_map) {
      ui_show_terrain_import_texture_weight_map();
   }
   else if (_terrain_edit_tool == terrain_edit_tool::import_color_map) {
      ui_show_terrain_import_color_map();
   }
   else if (_terrain_edit_tool == terrain_edit_tool::resize) {
      ui_show_terrain_resize();
   }
   else if (_terrain_edit_tool == terrain_edit_tool::crop) {
      ui_show_terrain_crop();
   }
   else if (_terrain_edit_tool == terrain_edit_tool::extend) {
      ui_show_terrain_extend();
   }
   else if (_terrain_edit_tool == terrain_edit_tool::water_editor) {
      ui_show_water_editor();
   }
   else if (_terrain_edit_tool == terrain_edit_tool::foliage_editor) {
      ui_show_foliage_editor();
   }

   if (_render_env_map_open) {
      ui_show_render_env_map();
   }

   if (_measurement_tool_open) {
      ui_show_measurement_tool();
   }

   if (ImGui::BeginPopup("Create Entity", ImGuiWindowFlags_AlwaysAutoResize |
                                             ImGuiWindowFlags_NoDecoration)) {
      ui_show_create_menu_items();

      ImGui::EndPopup();
   }
}
}