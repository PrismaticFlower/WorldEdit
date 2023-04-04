#include "settings.hpp"

#include "imgui/imgui.h"

#include <cmath>

namespace we::settings {

void show_imgui_editor(settings& settings, bool& open, float display_scale) noexcept
{
   ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x / 2.0f,
                            ImGui::GetIO().DisplaySize.y / 2.0f},
                           ImGuiCond_Appearing, {0.5f, 0.5f});
   ImGui::SetNextWindowSizeConstraints({700.0f * display_scale, 500.0f * display_scale},
                                       {1200.0f * display_scale, 800.0f * display_scale});

   if (ImGui::Begin("Settings", &open)) {
      if (ImGui::BeginTabBar("Settings")) {
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

            ImGui::ColorEdit4("Barrier", &graphics.barrier_color.x);

            ImGui::ColorEdit4("AI Planning", &graphics.planning_color.x);

            ImGui::ColorEdit4("Sector", &graphics.sector_color.x);

            ImGui::ColorEdit4("Portal", &graphics.portal_color.x);

            ImGui::ColorEdit4("Hintnode", &graphics.hintnode_color.x);

            ImGui::ColorEdit4("Bundary", &graphics.boundary_color.x);

            ImGui::DragFloat("Light Volume Transparency",
                             &graphics.light_volume_alpha, 0.01f, 0.0f, 1.0f,
                             "%.3f", ImGuiSliderFlags_AlwaysClamp);

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

            ImGui::SeparatorText("Line Width");

            if (ImGui::SliderFloat("Line Width", &graphics.line_width, 0.5f,
                                   16.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp)) {
               graphics.line_width = std::round(graphics.line_width * 4.0f) / 4.0f;
            }

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
