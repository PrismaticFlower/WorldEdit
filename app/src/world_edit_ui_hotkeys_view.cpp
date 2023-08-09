#include "world_edit.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_hotkeys_view() noexcept
{
   ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x, 32.0f * _display_scale},
                           ImGuiCond_Always, {1.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({224.0f * _display_scale, -1.0f},
                                       {224.0f * _display_scale, -1.0f});

   ImGui::Begin("Hotkeys", nullptr,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs |
                   ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration);

   // No contents here, this window is filled in by the tools themselves.

   ImGui::End();
}

}