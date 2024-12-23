#include "math/quaternion_funcs.hpp"
#include "world_edit.hpp"

#include <imgui.h>

#include <fmt/core.h>

namespace we {

void world_edit::ui_show_camera_controls() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   if (ImGui::Begin("Camera##Controls", &_camera_controls_open,
                    ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SeparatorText("Movement");
      ImGui::DragFloat("Move Speed", &_settings.camera.move_speed, 0.05f, 1.0f, 1000.0f);
      ImGui::DragFloat("Step Size", &_settings.camera.step_size, 0.5f, 1.0f, 1000.0f);
      ImGui::DragFloat("Sprint Power", &_settings.camera.sprint_power, 0.005f,
                       1.0f, 1000.0f);

      ImGui::SeparatorText("Sensitivity");

      if (float scaled_sensitivity =
             std::pow(_settings.camera.look_sensitivity, 1.0f / 6.0f) * 10.0f;
          ImGui::SliderFloat("Look Sensitivity", &scaled_sensitivity, 1.0f,
                             10.0f, "%.1f")) {
         _settings.camera.look_sensitivity =
            std::pow(scaled_sensitivity / 10.0f, 6.0f);
      }

      if (float scaled_sensitivity =
             std::pow(_settings.camera.pan_sensitivity, 1.0f / 4.0f) * 10.0f;
          ImGui::SliderFloat("Pan Sensitivity", &scaled_sensitivity, 1.0f,
                             10.0f, "%.1f")) {
         _settings.camera.pan_sensitivity = std::pow(scaled_sensitivity / 10.0f, 4.0f);
      }

      ImGui::SeparatorText("View");

      if (_camera.projection() == graphics::camera_projection::perspective) {
         ImGui::SliderAngle("Horizontal FOV", &_settings.camera.fov, 1.0f, 120.0f);
      }
      else {
         ImGui::DragFloat("View Width", &_settings.camera.view_width, 1.0f,
                          1.0f, 8192.0f);
      }

      if (float far_clip = _camera.far_clip();
          ImGui::DragFloat("View Distance", &far_clip, 1.0f, _camera.near_clip(), 1e10f)) {
         _camera.far_clip(far_clip);
      }

      if (ImGui::BeginCombo("Projection", _camera.projection() ==
                                                graphics::camera_projection::perspective
                                             ? "Perspective"
                                             : "Orthographic")) {
         if (ImGui::Selectable("Perspective")) {
            _camera.projection(graphics::camera_projection::perspective);
         }
         if (ImGui::Selectable("Orthographic")) {
            _camera.projection(graphics::camera_projection::orthographic);
         }

         ImGui::EndCombo();
      }

      if (ImGui::Checkbox("Orbit Mode", &_orbit_camera_active)) {
         if (_orbit_camera_active) setup_orbit_camera();
      }

      if (float zoom = _camera.zoom();
          ImGui::DragFloat("Zoom", &zoom, 0.025f, 1.0f, 10.0f)) {
         _camera.zoom(zoom);
      }

      ImGui::SeparatorText("Position");

      if (float3 position = _camera.position();
          ImGui::DragFloat3("Position", &position.x)) {
         _camera.position(position);
      }

      ImGui::Separator();

      if (ImGui::Button("Copy Camera Shot", {ImGui::CalcItemWidth(), 0.0f})) {
         const quaternion rotation =
            make_quat_from_matrix(_camera.world_from_view());
         const float3 positionWS = _camera.position();

         ImGui::SetClipboardText(
            fmt::format("AddCameraShot({}, {}, {}, {}, {}, {}, {});",
                        rotation.w, rotation.x, rotation.y, rotation.z,
                        positionWS.x, positionWS.y, positionWS.z)
               .c_str());
      }
   }
   ImGui::End();
}

}