#include "version.h"
#include "world_edit.hpp"

#include <imgui.h>

using namespace std::literals;

namespace we {

void world_edit::ui_show_about_window() noexcept
{
   ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x / 2.0f,
                            ImGui::GetIO().DisplaySize.y / 2.0f},
                           ImGuiCond_Appearing, {0.5f, 0.5f});

   if (ImGui::Begin("About", &_about_window_open, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("WorldEdit - v%i.%i", WE_MAJOR_VERSION, WE_MINOR_VERSION);
   }

   ImGui::End();
}

}