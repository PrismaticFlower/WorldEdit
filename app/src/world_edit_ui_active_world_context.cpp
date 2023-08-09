#include "world_edit.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_world_active_context() noexcept
{
   ImGui::SetNextWindowPos({0.0f, 32.0f * _display_scale});
   ImGui::SetNextWindowSize({256.0f * _display_scale, 620.0f * _display_scale});

   ImGui::Begin("World Active Context", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove);

   ImGui::SeparatorText("Active Layers");
   ImGui::BeginChild("Active Layers", ImVec2{0.0f, 208.0f * _display_scale});

   if (ImGui::BeginTable("Active Layers", 3, ImGuiTableFlags_SizingStretchProp)) {
      for (int i = 0; i < _world.layer_descriptions.size(); ++i) {
         ImGui::PushID(i);

         ImGui::TableNextRow();

         ImGui::TableNextColumn();
         const std::string& name = _world.layer_descriptions[i].name;

         if (name.size() < 12) {
            ImGui::TextUnformatted(name.c_str(),
                                   name.c_str() + std::min(name.size(), 11ull));
         }
         else {
            std::array<char, 11> faded_name{};

            std::memcpy(faded_name.data(), name.data(), faded_name.size());

            faded_name[8] = faded_name[9] = faded_name[10] = '.';

            ImGui::TextUnformatted(faded_name.data(),
                                   faded_name.data() + faded_name.size());
         }

         if (ImGui::IsItemHovered()) ImGui::SetTooltip(name.c_str());

         ImGui::TableNextColumn();
         if (bool draw = _world_layers_draw_mask[i]; ImGui::Checkbox("Show", &draw)) {
            _world_layers_draw_mask[i] = draw;
            if (not draw) _world_layers_hit_mask[i] = false;
         }

         ImGui::TableNextColumn();
         if (bool hit = _world_layers_hit_mask[i]; ImGui::Checkbox("Select", &hit)) {
            _world_layers_hit_mask[i] = hit;
            if (hit) _world_layers_draw_mask[i] = true;
         }

         ImGui::PopID();
      }

      ImGui::EndTable();
   }

   ImGui::EndChild();

   ImGui::SeparatorText("Active Entities");
   ImGui::BeginChild("Active Entities", ImVec2{0.0f, 288.0f * _display_scale});

   if (ImGui::BeginTable("Active Entities", 3, ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Objects");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.objects;
          ImGui::Checkbox("Show##Objects", &draw)) {
         _world_draw_mask.objects = draw;
         if (not draw) _world_hit_mask.objects = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.objects;
          ImGui::Checkbox("Select##Objects", &hit)) {
         _world_hit_mask.objects = hit;
         if (hit) _world_draw_mask.objects = true;
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Lights");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.lights; ImGui::Checkbox("Show##Lights", &draw)) {
         _world_draw_mask.lights = draw;
         if (not draw) _world_hit_mask.lights = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.lights; ImGui::Checkbox("Select##Lights", &hit)) {
         _world_hit_mask.lights = hit;
         if (hit) _world_draw_mask.lights = true;
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Paths");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.paths; ImGui::Checkbox("Show##Paths", &draw)) {
         _world_draw_mask.paths = draw;
         if (not draw) _world_hit_mask.paths = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.paths; ImGui::Checkbox("Select##Paths", &hit)) {
         _world_hit_mask.paths = hit;
         if (hit) _world_draw_mask.paths = true;
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Regions");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.regions;
          ImGui::Checkbox("Show##Regions", &draw)) {
         _world_draw_mask.regions = draw;
         if (not draw) _world_hit_mask.regions = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.regions;
          ImGui::Checkbox("Select##Regions", &hit)) {
         _world_hit_mask.regions = hit;
         if (hit) _world_draw_mask.regions = true;
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Sectors");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.sectors;
          ImGui::Checkbox("Show##Sectors", &draw)) {
         _world_draw_mask.sectors = draw;
         if (not draw) _world_hit_mask.sectors = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.sectors;
          ImGui::Checkbox("Select##Sectors", &hit)) {
         _world_hit_mask.sectors = hit;
         if (hit) _world_draw_mask.sectors = true;
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Portals");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.portals;
          ImGui::Checkbox("Show##Portals", &draw)) {
         _world_draw_mask.portals = draw;
         if (not draw) _world_hit_mask.portals = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.portals;
          ImGui::Checkbox("Select##Portals", &hit)) {
         _world_hit_mask.portals = hit;
         if (hit) _world_draw_mask.portals = true;
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Hintnodes");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.hintnodes;
          ImGui::Checkbox("Show##Hintnodes", &draw)) {
         _world_draw_mask.hintnodes = draw;
         if (not draw) _world_hit_mask.hintnodes = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.hintnodes;
          ImGui::Checkbox("Select##Hintnodes", &hit)) {
         _world_hit_mask.hintnodes = hit;
         if (hit) _world_draw_mask.hintnodes = true;
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Barriers");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.barriers;
          ImGui::Checkbox("Show##Barriers", &draw)) {
         _world_draw_mask.barriers = draw;
         if (not draw) _world_hit_mask.barriers = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.barriers;
          ImGui::Checkbox("Select##Barriers", &hit)) {
         _world_hit_mask.barriers = hit;
         if (hit) _world_draw_mask.barriers = true;
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("AI Planning");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.planning_hubs or _world_draw_mask.planning_connections;
          ImGui::Checkbox("Show##AI Planning", &draw)) {
         _world_draw_mask.planning_hubs = draw;
         _world_draw_mask.planning_connections = draw;

         if (not draw) {
            _world_hit_mask.planning_hubs = false;
            _world_hit_mask.planning_connections = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.planning_hubs or _world_hit_mask.planning_connections;
          ImGui::Checkbox("Select##AI Planning", &hit)) {
         _world_hit_mask.planning_hubs = hit;
         _world_hit_mask.planning_connections = hit;

         if (hit) {
            _world_draw_mask.planning_hubs = true;
            _world_draw_mask.planning_connections = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Boundaries");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.boundaries;
          ImGui::Checkbox("Show##Boundaries", &draw)) {
         _world_draw_mask.boundaries = draw;
         if (not draw) _world_hit_mask.boundaries = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.boundaries;
          ImGui::Checkbox("Select##Boundaries", &hit)) {
         _world_hit_mask.boundaries = hit;
         if (hit) _world_draw_mask.boundaries = true;
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Terrain");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.terrain;
          ImGui::Checkbox("Show##Terrain", &draw)) {
         _world_draw_mask.terrain = draw;
         if (not draw) _world_hit_mask.terrain = false;
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.terrain;
          ImGui::Checkbox("Select##Terrain", &hit)) {
         _world_hit_mask.terrain = hit;
         if (hit) _world_draw_mask.terrain = true;
      }

      ImGui::EndTable();
   }

   ImGui::EndChild();

   ImGui::SeparatorText("Floor Collision");
   ImGui::DragFloat("Height", &_editor_floor_height, 1.0f,
                    _world.terrain.height_scale * -32768.0f, 1e10f);

   if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(
         "Height of the editor's 'floor' collision. Used when nothing else is "
         "hit. Useful for space maps and anything you can think of for it!");
   }

   ImGui::End();
}

}