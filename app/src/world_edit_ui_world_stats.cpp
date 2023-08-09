#include "world_edit.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_world_stats() noexcept
{
   ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize, ImGuiCond_Appearing,
                           {1.0f, 1.0f});
   ImGui::SetNextWindowSizeConstraints({224.0f * _display_scale, -1.0f},
                                       {224.0f * _display_scale, -1.0f});

   if (ImGui::Begin("World Stats", &_world_stats_open,
                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                       ImGuiWindowFlags_NoFocusOnAppearing)) {
      ImGui::Text("Object Count: %i", static_cast<int>(_world.objects.size()));
      ImGui::Text("Light Count: %i", static_cast<int>(_world.lights.size()));
      ImGui::Text("Path Count: %i", static_cast<int>(_world.paths.size()));
      ImGui::Text("Region Count: %i", static_cast<int>(_world.regions.size()));
      ImGui::Text("Sector Count: %i", static_cast<int>(_world.sectors.size()));
      ImGui::Text("Portal Count: %i", static_cast<int>(_world.portals.size()));
      ImGui::Text("Hintnode Count: %i", static_cast<int>(_world.hintnodes.size()));
      ImGui::Text("Barrier Count: %i", static_cast<int>(_world.barriers.size()));
      ImGui::Text("AI Planning Hub Count: %i",
                  static_cast<int>(_world.planning_hubs.size()));
      ImGui::Text("AI Planning Connection Count: %i",
                  static_cast<int>(_world.planning_connections.size()));
      ImGui::Text("Boundaries: %i", static_cast<int>(_world.boundaries.size()));
      ImGui::Text("Undo Stack Size: %i",
                  static_cast<int>(_edit_stack_world.applied_size()));
      ImGui::Text("Redo Stack Size: %i",
                  static_cast<int>(_edit_stack_world.reverted_size()));
   }
   ImGui::End();
}

}