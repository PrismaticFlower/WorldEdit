#include "world_edit.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_export_selection() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({384.0f * _display_scale, 112.0f * _display_scale});

   if (ImGui::Begin("Export Selection", &_export_selection_open,
                    ImGuiWindowFlags_NoResize)) {
      ImGui::Checkbox("Copy Textures", &_export_selection_config.copy_textures);

      ImGui::SetItemTooltip(
         "Copy referenced textures to the output directory.");

      ImGui::Checkbox("Include Terrain", &_export_selection_config.include_terrain);

      ImGui::SetItemTooltip("Include the terrain in the exported mesh in "
                            "addition to selected objects and blocks.");

      if (ImGui::Button("Export", {ImGui::GetContentRegionAvail().x, 0.0f})) {
         export_selection_with_picker();

         _export_selection_open = false;
      }
   }

   ImGui::End();
}

}
