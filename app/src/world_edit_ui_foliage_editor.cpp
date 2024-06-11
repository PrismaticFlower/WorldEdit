#include "world_edit.hpp"

#include "edits/imgui_ext.hpp"
#include "edits/set_terrain_area.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/raycast_terrain.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_foliage_editor() noexcept
{
   const int32 foliage_map_length = _world.terrain.length / 2;

   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({520.0f * _display_scale, 620.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   bool open = _terrain_edit_tool == terrain_edit_tool::foliage_editor;

   if (ImGui::Begin("Foliage Editor", &open)) {
      if (ImGui::CollapsingHeader("Edit", ImGuiTreeNodeFlags_DefaultOpen)) {
         ImGui::SeparatorText("Layer");

         if (ImGui::BeginTable("Layers", 2, ImGuiTableFlags_NoSavedSettings)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Layer 1", _foliage_editor_config.layer == 0)) {
               _foliage_editor_config.layer = 0;
            }
            ImGui::TableNextColumn();
            ImGui::ColorEdit3("Layer 1 Color",
                              &_settings.graphics.foliage_overlay_layer0_color.x,
                              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Layer 2", _foliage_editor_config.layer == 1)) {
               _foliage_editor_config.layer = 1;
            }
            ImGui::TableNextColumn();
            ImGui::ColorEdit3("Layer 2 Color",
                              &_settings.graphics.foliage_overlay_layer1_color.x,
                              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Layer 3", _foliage_editor_config.layer == 2)) {
               _foliage_editor_config.layer = 2;
            }
            ImGui::TableNextColumn();
            ImGui::ColorEdit3("Layer 3 Color",
                              &_settings.graphics.foliage_overlay_layer2_color.x,
                              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Layer 4", _foliage_editor_config.layer == 3)) {
               _foliage_editor_config.layer = 3;
            }
            ImGui::TableNextColumn();
            ImGui::ColorEdit3("Layer 4 Color",
                              &_settings.graphics.foliage_overlay_layer3_color.x,
                              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            ImGui::PopStyleVar();
            ImGui::EndTable();
         }

         ImGui::SeparatorText("Brush Mode");

         if (ImGui::Selectable("Paint", _foliage_editor_config.brush_mode ==
                                           foliage_brush_mode::paint)) {
            _foliage_editor_config.brush_mode = foliage_brush_mode::paint;
         }

         if (ImGui::Selectable("Erase", _foliage_editor_config.brush_mode ==
                                           foliage_brush_mode::erase)) {
            _foliage_editor_config.brush_mode = foliage_brush_mode::erase;
         }

         ImGui::SeparatorText("Brush Settings");

         if (int32 size = std::max(_foliage_editor_config.brush_size_x,
                                   _foliage_editor_config.brush_size_y);
             ImGui::SliderInt("Size", &size, 0, foliage_map_length / 2, "%d",
                              ImGuiSliderFlags_AlwaysClamp)) {
            _foliage_editor_config.brush_size_x = size;
            _foliage_editor_config.brush_size_y = size;
         }

         ImGui::SliderInt("Size (X)", &_foliage_editor_config.brush_size_x, 0,
                          foliage_map_length / 2, "%d", ImGuiSliderFlags_AlwaysClamp);

         ImGui::SliderInt("Size (Y)", &_foliage_editor_config.brush_size_y, 0,
                          foliage_map_length / 2, "%d", ImGuiSliderFlags_AlwaysClamp);

         ImGui::SeparatorText("Tools");

         if (ImGui::Selectable("Clear World")) {
            container::dynamic_array_2d<world::foliage_patch> new_foliage_map =
               _world.terrain.foliage_map;

            switch (_foliage_editor_config.layer) {
            case 0:
               for (world::foliage_patch& v : new_foliage_map) v.layer0 = false;
               break;
            case 1:
               for (world::foliage_patch& v : new_foliage_map) v.layer1 = false;
               break;
            case 2:
               for (world::foliage_patch& v : new_foliage_map) v.layer2 = false;
               break;
            case 3:
               for (world::foliage_patch& v : new_foliage_map) v.layer3 = false;
               break;
            }

            _edit_stack_world.apply(edits::make_set_terrain_area_foliage_map(
                                       0, 0, std::move(new_foliage_map)),
                                    _edit_context, {.closed = true});
         }
      }

      if (ImGui::CollapsingHeader("Settings")) {
         ImGui::BeginDisabled(_world.terrain.version != world::version::swbf2);

         ImGui::Checkbox("Enabled", &_world.terrain.active_flags.foliage,
                         _edit_stack_world, _edit_context);

         ImGui::EndDisabled();

         if (_world.terrain.version != world::version::swbf2) {
            ImGui::SetItemTooltip(
               "Change the terrain version to SWBF2 (v22) to edit this.");
         }
      }
   }

   ImGui::End();

   if (not open) {
      _terrain_edit_tool = terrain_edit_tool::none;

      return;
   }

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Foliage Editing");

      ImGui::Text("Cycle Brush Mode");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Foliage Editing", "Cycle Brush Mode")));

      ImGui::Text("Set Layer 1");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Foliage Editing", "Set Layer 1")));

      ImGui::Text("Set Layer 2");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Foliage Editing", "Set Layer 2")));

      ImGui::Text("Set Layer 3");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Foliage Editing", "Set Layer 3")));

      ImGui::Text("Set Layer 4");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Foliage Editing", "Set Layer 4")));

      ImGui::Text("Increase Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Foliage Editing", "Increase Brush Size")));

      ImGui::Text("Decrease Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Foliage Editing", "Decrease Brush Size")));

      ImGui::End();
   }

   graphics::camera_ray ray =
      make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                      {ImGui::GetMainViewport()->Size.x,
                       ImGui::GetMainViewport()->Size.y});

   float hit_distance = -1.0f;

   if (auto hit = world::raycast(ray.origin, ray.direction, _world.terrain); hit) {
      hit_distance = *hit;
   }

   if (hit_distance < 0.0f) return;

   const float3 cursor_positionWS = ray.origin + ray.direction * hit_distance;

   const float foliage_grid_scale = _world.terrain.grid_scale * 2.0f;
   const float half_grid_scale = _world.terrain.grid_scale;

   const float2 foliage_point =
      floor(float2{cursor_positionWS.x, cursor_positionWS.z - half_grid_scale} /
            foliage_grid_scale) +
      (foliage_map_length / 2.0f);

   const int32 foliage_x = static_cast<int32>(foliage_point.x);
   const int32 foliage_y = static_cast<int32>(foliage_point.y);

   if (_foliage_editor_context.brush_held != _foliage_editor_context.brush_active) {
      _foliage_editor_context.brush_active = _foliage_editor_context.brush_held;
      _edit_stack_world.close_last();
   }

   const int32 brush_size_x = _foliage_editor_config.brush_size_x;
   const int32 brush_size_y = _foliage_editor_config.brush_size_y;

   if (_foliage_editor_context.brush_active) {
      int32 left = std::clamp(foliage_x - brush_size_x, 0, foliage_map_length - 1);
      int32 top = std::clamp(foliage_y - brush_size_y, 0, foliage_map_length - 1);
      int32 right = std::clamp(foliage_x + brush_size_x + 1, 0, foliage_map_length);
      int32 bottom = std::clamp(foliage_y + brush_size_y + 1, 0, foliage_map_length);

      if (left >= right or top >= bottom) return;

      container::dynamic_array_2d<world::foliage_patch> area{right - left, bottom - top};

      for (int32 y = top; y < bottom; ++y) {
         for (int32 x = left; x < right; ++x) {
            area[{x - left, y - top}] = _world.terrain.foliage_map[{x, y}];
         }
      }

      if (_foliage_editor_config.brush_mode == foliage_brush_mode::paint) {
         switch (_foliage_editor_config.layer) {
         case 0:
            for (world::foliage_patch& v : area) v.layer0 = true;
            break;
         case 1:
            for (world::foliage_patch& v : area) v.layer1 = true;
            break;
         case 2:
            for (world::foliage_patch& v : area) v.layer2 = true;
            break;
         case 3:
            for (world::foliage_patch& v : area) v.layer3 = true;
            break;
         }
      }
      else if (_foliage_editor_config.brush_mode == foliage_brush_mode::erase) {
         switch (_foliage_editor_config.layer) {
         case 0:
            for (world::foliage_patch& v : area) v.layer0 = false;
            break;
         case 1:
            for (world::foliage_patch& v : area) v.layer1 = false;
            break;
         case 2:
            for (world::foliage_patch& v : area) v.layer2 = false;
            break;
         case 3:
            for (world::foliage_patch& v : area) v.layer3 = false;
            break;
         }
      }

      _edit_stack_world.apply(edits::make_set_terrain_area_foliage_map(left, top,
                                                                       std::move(area)),
                              _edit_context);
   }

   const auto get_position = [&](int32 x, int32 y) {
      return float3{
         (x - (foliage_map_length / 2.0f)) * foliage_grid_scale,
         _world.terrain.height_map[{std::clamp(x * 2, 0, _world.terrain.length - 1),
                                    std::clamp(y * 2, 0, _world.terrain.length - 1)}] *
            _world.terrain.height_scale,
         (y + 0.5f - (foliage_map_length / 2.0f)) * foliage_grid_scale};
   };

   if ((brush_size_x * 2) * (brush_size_y * 2) <= 1024) {
      for (int32 y = -brush_size_y; y <= brush_size_y + 1; ++y) {
         for (int32 x = -brush_size_x; x <= brush_size_x; ++x) {
            _tool_visualizers.add_line_overlay(get_position(foliage_x - x, foliage_y + y),
                                               get_position(foliage_x - x + 1,
                                                            foliage_y + y),
                                               0xff'ff'ff'ffu);
         }
      }

      for (int32 x = -brush_size_x; x <= brush_size_x + 1; ++x) {
         for (int32 y = -brush_size_y; y <= brush_size_y; ++y) {
            _tool_visualizers.add_line_overlay(get_position(foliage_x + x, foliage_y - y),
                                               get_position(foliage_x + x,
                                                            foliage_y - y + 1),
                                               0xff'ff'ff'ffu);
         }
      }
   }
   else {
      for (int32 x = -brush_size_x; x <= brush_size_x; ++x) {
         _tool_visualizers.add_line_overlay(get_position(foliage_x - x,
                                                         foliage_y - brush_size_y),
                                            get_position(foliage_x - x + 1,
                                                         foliage_y - brush_size_y),
                                            0xff'ff'ff'ffu);
      }

      for (int32 x = -brush_size_x; x <= brush_size_x; ++x) {
         _tool_visualizers.add_line_overlay(get_position(foliage_x - x,
                                                         foliage_y + brush_size_y + 1),
                                            get_position(foliage_x - x + 1,
                                                         foliage_y + brush_size_y + 1),
                                            0xff'ff'ff'ffu);
      }

      for (int32 y = -brush_size_y; y <= brush_size_y; ++y) {
         _tool_visualizers.add_line_overlay(get_position(foliage_x - brush_size_x,
                                                         foliage_y - y),
                                            get_position(foliage_x - brush_size_x,
                                                         foliage_y - y + 1),
                                            0xff'ff'ff'ffu);
      }

      for (int32 y = -brush_size_y; y <= brush_size_y; ++y) {
         _tool_visualizers.add_line_overlay(get_position(foliage_x + brush_size_x + 1,
                                                         foliage_y - y),
                                            get_position(foliage_x + brush_size_x + 1,
                                                         foliage_y - y + 1),
                                            0xff'ff'ff'ffu);
      }
   }
}
}
