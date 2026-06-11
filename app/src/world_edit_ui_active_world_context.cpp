#include "world_edit.hpp"

#include <imgui.h>

namespace we {

namespace {

auto operator~(const world::active_entity_types l) noexcept -> world::active_entity_types
{
   return {
      .objects = not l.objects,
      .lights = not l.lights,
      .paths = not l.paths,
      .regions = not l.regions,
      .sectors = not l.sectors,
      .portals = not l.portals,
      .hintnodes = not l.hintnodes,
      .barriers = not l.barriers,
      .planning_hubs = not l.planning_hubs,
      .planning_connections = not l.planning_connections,
      .boundaries = not l.boundaries,
      .measurements = not l.measurements,
      .terrain = not l.terrain,
      .blocks = not l.blocks,
   };
}

auto operator&(const world::active_entity_types l,
               const world::active_entity_types r) noexcept -> world::active_entity_types
{
   return {
      .objects = (l.objects & r.objects) != 0,
      .lights = (l.lights & r.lights) != 0,
      .paths = (l.paths & r.paths) != 0,
      .regions = (l.regions & r.regions) != 0,
      .sectors = (l.sectors & r.sectors) != 0,
      .portals = (l.portals & r.portals) != 0,
      .hintnodes = (l.hintnodes & r.hintnodes) != 0,
      .barriers = (l.barriers & r.barriers) != 0,
      .planning_hubs = (l.planning_hubs & r.planning_hubs) != 0,
      .planning_connections = (l.planning_connections & r.planning_connections) != 0,
      .boundaries = (l.boundaries & r.boundaries) != 0,
      .measurements = (l.measurements & r.measurements) != 0,
      .terrain = (l.terrain & r.terrain) != 0,
      .blocks = (l.blocks & r.blocks) != 0,
   };
}

auto operator|(const world::active_entity_types l,
               const world::active_entity_types r) noexcept -> world::active_entity_types
{
   return {
      .objects = (l.objects | r.objects) != 0,
      .lights = (l.lights | r.lights) != 0,
      .paths = (l.paths | r.paths) != 0,
      .regions = (l.regions | r.regions) != 0,
      .sectors = (l.sectors | r.sectors) != 0,
      .portals = (l.portals | r.portals) != 0,
      .hintnodes = (l.hintnodes | r.hintnodes) != 0,
      .barriers = (l.barriers | r.barriers) != 0,
      .planning_hubs = (l.planning_hubs | r.planning_hubs) != 0,
      .planning_connections = (l.planning_connections | r.planning_connections) != 0,
      .boundaries = (l.boundaries | r.boundaries) != 0,
      .measurements = (l.measurements | r.measurements) != 0,
      .terrain = (l.terrain | r.terrain) != 0,
      .blocks = (l.blocks | r.blocks) != 0,
   };
}

}

void world_edit::ui_show_world_active_context() noexcept
{
   ImGui::SetNextWindowPos({0.0f, 32.0f * _display_scale});
   ImGui::SetNextWindowSize({256.0f * _display_scale, 698.0f * _display_scale});

   ImGui::Begin("World Active Context", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove);

   ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                       {ImGui::GetStyle().FramePadding.x, 2.0f * _display_scale});

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
            if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
               _world_layers_draw_mask = {false};
               _world_layers_hit_mask = {false};
            }
            else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
               _world_layers_draw_mask = {true};
            }
            else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
               for (int other_layer = 0;
                    other_layer < _world.layer_descriptions.size(); ++other_layer) {
                  if (other_layer != i) {
                     _world_layers_draw_mask[other_layer] = not draw;
                     if (draw) _world_layers_hit_mask[other_layer] = false;
                  }
               }

               _world_layers_draw_mask[i] = draw;
               if (not draw) _world_layers_hit_mask[i] = false;
            }
            else {
               _world_layers_draw_mask[i] = draw;
               if (not draw) _world_layers_hit_mask[i] = false;
            }
         }

         ImGui::TableNextColumn();
         if (bool hit = _world_layers_hit_mask[i]; ImGui::Checkbox("Select", &hit)) {
            if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
               _world_layers_hit_mask = {false};
            }
            else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
               _world_layers_draw_mask = {true};
               _world_layers_hit_mask = {true};
            }
            else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
               for (int other_layer = 0;
                    other_layer < _world.layer_descriptions.size(); ++other_layer) {
                  if (other_layer != i) {
                     _world_layers_hit_mask[other_layer] = not hit;
                     if (not hit) _world_layers_draw_mask[other_layer] = true;
                  }
               }

               _world_layers_hit_mask[i] = hit;
               if (hit) _world_layers_draw_mask[i] = true;
            }
            else {
               _world_layers_hit_mask[i] = hit;
               if (hit) _world_layers_draw_mask[i] = true;
            }
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
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .objects = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.objects = draw;
            if (not draw) _world_hit_mask.objects = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.objects;
          ImGui::Checkbox("Select##Objects", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .objects = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.objects = hit;
            if (hit) _world_draw_mask.objects = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Lights");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.lights; ImGui::Checkbox("Show##Lights", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .lights = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.lights = draw;
            if (not draw) _world_hit_mask.lights = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.lights; ImGui::Checkbox("Select##Lights", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .lights = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.lights = hit;
            if (hit) _world_draw_mask.lights = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Paths");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.paths; ImGui::Checkbox("Show##Paths", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .paths = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.paths = draw;
            if (not draw) _world_hit_mask.paths = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.paths; ImGui::Checkbox("Select##Paths", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .paths = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.paths = hit;
            if (hit) _world_draw_mask.paths = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Regions");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.regions;
          ImGui::Checkbox("Show##Regions", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .regions = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.regions = draw;
            if (not draw) _world_hit_mask.regions = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.regions;
          ImGui::Checkbox("Select##Regions", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .regions = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.regions = hit;
            if (hit) _world_draw_mask.regions = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Sectors");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.sectors;
          ImGui::Checkbox("Show##Sectors", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .sectors = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.sectors = draw;
            if (not draw) _world_hit_mask.sectors = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.sectors;
          ImGui::Checkbox("Select##Sectors", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .sectors = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.sectors = hit;
            if (hit) _world_draw_mask.sectors = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Portals");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.portals;
          ImGui::Checkbox("Show##Portals", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .portals = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.portals = draw;
            if (not draw) _world_hit_mask.portals = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.portals;
          ImGui::Checkbox("Select##Portals", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .portals = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.portals = hit;
            if (hit) _world_draw_mask.portals = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Hintnodes");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.hintnodes;
          ImGui::Checkbox("Show##Hintnodes", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .hintnodes = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.hintnodes = draw;
            if (not draw) _world_hit_mask.hintnodes = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.hintnodes;
          ImGui::Checkbox("Select##Hintnodes", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .hintnodes = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.hintnodes = hit;
            if (hit) _world_draw_mask.hintnodes = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Barriers");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.barriers;
          ImGui::Checkbox("Show##Barriers", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .barriers = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.barriers = draw;
            if (not draw) _world_hit_mask.barriers = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.barriers;
          ImGui::Checkbox("Select##Barriers", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .barriers = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.barriers = hit;
            if (hit) _world_draw_mask.barriers = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("AI Planning");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.planning_hubs or _world_draw_mask.planning_connections;
          ImGui::Checkbox("Show##AI Planning", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .planning_hubs = true,
               .planning_connections = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.planning_hubs = draw;
            _world_draw_mask.planning_connections = draw;

            if (not draw) {
               _world_hit_mask.planning_hubs = false;
               _world_hit_mask.planning_connections = false;
            }
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.planning_hubs or _world_hit_mask.planning_connections;
          ImGui::Checkbox("Select##AI Planning", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .planning_hubs = true,
               .planning_connections = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.planning_hubs = hit;
            _world_hit_mask.planning_connections = hit;

            if (hit) {
               _world_draw_mask.planning_hubs = true;
               _world_draw_mask.planning_connections = true;
            }
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Boundaries");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.boundaries;
          ImGui::Checkbox("Show##Boundaries", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .boundaries = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.boundaries = draw;
            if (not draw) _world_hit_mask.boundaries = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.boundaries;
          ImGui::Checkbox("Select##Boundaries", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .boundaries = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.boundaries = hit;
            if (hit) _world_draw_mask.boundaries = true;
         }
      }

      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Terrain");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.terrain;
          ImGui::Checkbox("Show##Terrain", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .terrain = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.terrain = draw;
            if (not draw) _world_hit_mask.terrain = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.terrain;
          ImGui::Checkbox("Select##Terrain", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .terrain = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.terrain = hit;
            if (hit) _world_draw_mask.terrain = true;
         }
      }

      ImGui::TableNextColumn();
      ImGui::TextUnformatted("Blocks");

      ImGui::TableNextColumn();
      if (bool draw = _world_draw_mask.blocks; ImGui::Checkbox("Show##Blocks", &draw)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_draw_mask = world::active_entity_types{};
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types draw_mask = {
               .blocks = true,
            };

            if (not draw) draw_mask = ~draw_mask;

            _world_draw_mask = draw_mask;
            _world_hit_mask = draw_mask & _world_hit_mask;
         }
         else {
            _world_draw_mask.blocks = draw;
            if (not draw) _world_hit_mask.blocks = false;
         }
      }

      ImGui::TableNextColumn();
      if (bool hit = _world_hit_mask.blocks; ImGui::Checkbox("Select##Blocks", &hit)) {
         if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            _world_hit_mask = world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            _world_draw_mask = ~world::active_entity_types{};
            _world_hit_mask = ~world::active_entity_types{};
         }
         else if (ImGui::IsKeyDown(ImGuiMod_Alt)) {
            world::active_entity_types hit_mask = {
               .blocks = true,
            };

            if (not hit) hit_mask = ~hit_mask;

            _world_draw_mask = hit_mask | _world_draw_mask;
            _world_hit_mask = hit_mask;
         }
         else {
            _world_hit_mask.blocks = hit;
            if (hit) _world_draw_mask.blocks = true;
         }
      }

      ImGui::EndTable();
   }

   ImGui::EndChild();

   ImGui::PopStyleVar();

   ImGui::SeparatorText("Floor");
   ImGui::DragFloat("Height", &_editor_floor_height, 1.0f,
                    _world.terrain.height_scale * -32768.0f, 1e10f);

   ImGui::SetItemTooltip(
      "Height of the editor's 'floor' collision. Used for the grid and as "
      "collision when nothing else is hit.");

   ImGui::DragFloat("Grid Size", &_editor_grid_size, 1.0f, 1.0f, 1e10f, "%.3f");

   ImGui::Checkbox("Show Floor Grid", &_draw_overlay_grid);

   ImGui::Checkbox("Show Terrain Grid", &_draw_terrain_grid);

   ImGui::End();
}

}