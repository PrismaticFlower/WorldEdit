#include "settings.hpp"

#include <array>
#include <cmath>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we::settings {

void show_imgui_editor(settings& settings, bool& open, scale_factor display_scale) noexcept
{
   ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x / 2.0f,
                            ImGui::GetIO().DisplaySize.y / 2.0f},
                           ImGuiCond_Appearing, {0.5f, 0.5f});
   ImGui::SetNextWindowSizeConstraints({700.0f * display_scale, 500.0f * display_scale},
                                       {1200.0f * display_scale, 800.0f * display_scale});

   if (ImGui::Begin("Settings", &open)) {
      if (ImGui::BeginTabBar("Settings")) {
         if (ImGui::BeginTabItem("Preferences")) {
            ui& ui = settings.ui;
            preferences& preferences = settings.preferences;

            ImGui::SliderFloat("At Cursor Placement Re-Enable Distance",
                               &preferences.cursor_placement_reenable_distance,
                               1.0f, 64.0f, "%.0f");

            ImGui::SetItemTooltip(
               "If after Undoing or Redoing an edit an entity is being "
               "created with placement mode 'At Cursor' then the placement "
               "mode will temporarily be forced to be 'Manual'.\n\n 'At "
               "Cursor' placement will be reenabled will the mouse has moved "
               "this many pixels.\n\nAll of this is done to give you a chance "
               "to Redo changes that would otherwise by replaced by 'At "
               "Cursor' placement edits.");

            ImGui::SliderFloat("Terrain Editor Height Brush Stickiness",
                               &preferences.terrain_height_brush_stickiness,
                               0.0f, 64.0f, "%.0f");

            ImGui::SetItemTooltip(
               "When using a brush to edit the terrain's "
               "height the brush will be locked in "
               "place until the mouse is moved. Higher 'stickiness' values "
               "require more mouse movement to unlock and move the brush.");

            if (ImGui::InputText("Text Editor", &preferences.text_editor)) {
               std::erase(preferences.text_editor, '"');
            }

            ImGui::SetItemTooltip(
               "The path to the app to use when opening text "
               "files. The path can contain environment "
               "variables such as %%ProgramFiles%%. But may "
               "not contain quotes (\").");

            ImGui::DragFloat("Gizmo Scale", &ui.gizmo_scale, 0.125f, 0.0f,
                             10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

            constexpr static std::array extra_scaling_factors{0.5f, 0.75f,
                                                              1.0f, 1.25f,
                                                              1.5f, 1.75f,
                                                              2.0f};
            constexpr static std::array extra_scaling_factors_names{
               "0.5", "0.75", "1.0", "1.25", "1.5", "1.75", "2.0"};

            static_assert(extra_scaling_factors.size() ==
                          extra_scaling_factors_names.size());

            if (ImGui::BeginCombo("Extra UI Scaling", [](const float scaling) {
                   for (std::size_t i = 0; i < extra_scaling_factors.size(); ++i) {
                      if (scaling == extra_scaling_factors[i]) {
                         return extra_scaling_factors_names[i];
                      }
                   }

                   return "Custom";
                }(ui.extra_scaling))) {

               for (std::size_t i = 0; i < extra_scaling_factors.size(); ++i) {
                  if (ImGui::Selectable(extra_scaling_factors_names[i],
                                        ui.extra_scaling == extra_scaling_factors[i]))
                     ui.extra_scaling = extra_scaling_factors[i];
               }

               ImGui::EndCombo();
            }

            ImGui::SeparatorText("Reset");

            if (ImGui::Button("Reset to Defaults", {ImGui::CalcItemWidth(), 0.0f})) {
               ui = {};
               preferences = {};
            }

            ImGui::EndTabItem();
         }
         if (ImGui::BeginTabItem("Graphics")) {
            graphics& graphics = settings.graphics;

            ImGui::SeparatorText("Interaction Colours");

            ImGui::ColorEdit3("Hover", &graphics.hover_color.x);

            ImGui::ColorEdit3("Selected", &graphics.selected_color.x);

            ImGui::ColorEdit3("Creation", &graphics.creation_color.x);

            ImGui::SeparatorText("World Colours");

            ImGui::ColorEdit3("Path Node", &graphics.path_node_color.x);
            ImGui::ColorEdit3("Path Node Outline",
                              &graphics.path_node_outline_color.x);
            ImGui::ColorEdit3("Path Connection",
                              &graphics.path_node_connection_color.x);
            ImGui::ColorEdit3("Path Orientation",
                              &graphics.path_node_connection_color.x);

            ImGui::ColorEdit4("Region", &graphics.region_color.x);

            ImGui::ColorEdit3("Barrier Outline", &graphics.barrier_outline_color.x);

            ImGui::ColorEdit4("Barrier Overlay", &graphics.barrier_overlay_color.x);

            ImGui::ColorEdit3("AI Planning Hub Outline",
                              &graphics.planning_hub_outline_color.x);

            ImGui::ColorEdit4("AI Planning Hub Overlay",
                              &graphics.planning_hub_overlay_color.x);

            ImGui::ColorEdit3("AI Planning Connection Outline",
                              &graphics.planning_connection_outline_color.x);

            ImGui::ColorEdit4("AI Planning Connection Overlay",
                              &graphics.planning_connection_overlay_color.x);

            ImGui::ColorEdit4("Sector", &graphics.sector_color.x);

            ImGui::ColorEdit4("Portal", &graphics.portal_color.x);

            ImGui::ColorEdit4("Hintnode", &graphics.hintnode_color.x);

            ImGui::ColorEdit4("Boundary", &graphics.boundary_color.x);

            ImGui::ColorEdit3("Terrain Grid Color", &graphics.terrain_grid_color.x);

            ImGui::ColorEdit3("Overlay Grid Color", &graphics.overlay_grid_color.x);

            ImGui::SliderFloat("Light Volume Transparency", &graphics.light_volume_alpha,
                               0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::ColorEdit3("Terrain Cutter", &graphics.terrain_cutter_color.x);

            ImGui::ColorEdit3("Foliage Overlay Layer 1 Color",
                              &graphics.foliage_overlay_layer0_color.x);

            ImGui::ColorEdit3("Foliage Overlay Layer 2 Color",
                              &graphics.foliage_overlay_layer1_color.x);

            ImGui::ColorEdit3("Foliage Overlay Layer 3 Color",
                              &graphics.foliage_overlay_layer2_color.x);

            ImGui::ColorEdit3("Foliage Overlay Layer 4 Color",
                              &graphics.foliage_overlay_layer3_color.x);

            ImGui::SliderFloat("Foliage Overlay Transparency",
                               &graphics.foliage_overlay_transparency, 0.0f,
                               1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::SeparatorText("Visualizer Sizes");

            ImGui::DragFloat("Barrier Height", &graphics.barrier_height, 0.5f, 0.0f,
                             100000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::DragFloat("Boundary Height", &graphics.boundary_height, 0.5f,
                             0.0f, 100000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::DragFloat("AI Planning Hub Height",
                             &graphics.planning_hub_height, 0.5f, 0.0f,
                             100000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::DragFloat("AI Planning Connection Height",
                             &graphics.planning_connection_height, 0.5f, 0.0f,
                             100000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::SliderFloat("Path Node Size", &graphics.path_node_size, 0.5f,
                               32.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::DragFloat("Overlay Grid Major Spacing",
                             &graphics.overlay_grid_major_grid_spacing, 1.0f,
                             2.0f, 1e10f, "%.0f");

            ImGui::SetItemTooltip(
               "Number of grid squares between major grid squares.");

            ImGui::SeparatorText("Line Width");

            ImGui::DragFloat("Terrain Grid line Width",
                             &graphics.terrain_grid_line_width, 0.005f, 0.005f,
                             0.5f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::SetItemTooltip("Note this is relative to the terrain grid "
                                  "scale and is not in pixels!");

            ImGui::DragFloat("Overlay Grid line Width",
                             &graphics.overlay_grid_line_width, 0.005f, 0.005f,
                             0.5f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::SetItemTooltip("Note this is relative to the grid "
                                  "size and is not in pixels!");

            if (ImGui::SliderFloat("Line Width", &graphics.line_width, 0.5f,
                                   16.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp)) {
               graphics.line_width = std::round(graphics.line_width * 4.0f) / 4.0f;
            }

            ImGui::SeparatorText("Path Tessellation");

            ImGui::SliderFloat("Catmull-Rom Spline Target Tessellation",
                               &graphics.path_node_cr_spline_target_tessellation,
                               1.0f, 128.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::SetItemTooltip(
               "The tessellation when rendering paths with "
               "Catmull-Rom splines. Lower values produce more jagged "
               "visualizers while higher values produce more accurate "
               "visualizers but are slower to draw.");

            if (ImGui::SliderFloat("Catmull-Rom Spline Max Tessellation",
                                   &graphics.path_node_cr_spline_max_tessellation, 1.0f,
                                   512.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
               graphics.path_node_cr_spline_max_tessellation =
                  std::floor(graphics.path_node_cr_spline_max_tessellation);
            }

            ImGui::SetItemTooltip(
               "The max tessellation when rendering paths with "
               "Catmull-Rom splines. This clamps how many lines can be used to "
               "approximate the Catmull-Rom spline.\n\nLowering it further "
               "clamp "
               "the maximum cost of drawing the paths at the cost of a "
               "potentially more jagged visualizer.");

            ImGui::SeparatorText("Optional Visualizers");

            ImGui::Checkbox("Visualize Terrain Cutters",
                            &graphics.visualize_terrain_cutters);

            ImGui::SeparatorText("Reset");

            if (ImGui::Button("Reset to Defaults", {ImGui::CalcItemWidth(), 0.0f})) {
               graphics = {};
            }

            ImGui::EndTabItem();
         }
         if (ImGui::BeginTabItem("Camera")) {
            camera& camera = settings.camera;

            ImGui::SeparatorText("Movement");
            ImGui::DragFloat("Move Speed", &camera.move_speed, 0.05f, 1.0f, 1000.0f);
            ImGui::DragFloat("Step Size", &camera.step_size, 0.5f, 1.0f, 1000.0f);
            ImGui::DragFloat("Sprint Power", &camera.sprint_power, 0.005f, 1.0f, 1000.0f);

            ImGui::SeparatorText("Sensitivity");

            if (float scaled_sensitivity =
                   std::pow(camera.look_sensitivity, 1.0f / 6.0f) * 10.0f;
                ImGui::SliderFloat("Look Sensitivity", &scaled_sensitivity,
                                   1.0f, 10.0f, "%.1f")) {
               camera.look_sensitivity = std::pow(scaled_sensitivity / 10.0f, 6.0f);
            }

            if (float scaled_sensitivity =
                   std::pow(camera.pan_sensitivity, 1.0f / 4.0f) * 10.0f;
                ImGui::SliderFloat("Pan Sensitivity", &scaled_sensitivity, 1.0f,
                                   10.0f, "%.1f")) {
               camera.pan_sensitivity = std::pow(scaled_sensitivity / 10.0f, 4.0f);
            }

            ImGui::SeparatorText("View");

            ImGui::SliderAngle("Perspective Horizontal FOV", &camera.fov, 1.0f, 120.0f);

            ImGui::DragFloat("Orthographic View Width", &camera.view_width,
                             1.0f, 1.0f, 8192.0f);

            ImGui::SeparatorText("Reset");

            if (ImGui::Button("Reset to Defaults", {ImGui::CalcItemWidth(), 0.0f})) {
               camera = {};
            }

            ImGui::EndTabItem();
         }
         ImGui::EndTabBar();
      }
   }

   ImGui::End();
}

}
