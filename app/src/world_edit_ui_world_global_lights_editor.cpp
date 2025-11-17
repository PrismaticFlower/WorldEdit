#include "utility/string_icompare.hpp"
#include "world/utility/world_utilities.hpp"
#include "world_edit.hpp"

#include "edits/set_value.hpp"
#include "imgui_ext.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_world_global_lights_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   ImGui::Begin("Global Lights", &_world_global_lights_editor_open,
                ImGuiWindowFlags_AlwaysAutoResize);

   const auto global_light_editor = [&](const char* label, std::string* global_light,
                                        std::string_view ignored_light) {
      if (ImGui::BeginCombo(label, global_light->c_str())) {
         for (const auto& light : _world.lights) {
            if (light.light_type == world::light_type::directional) {
               ImGui::BeginDisabled(string::iequals(light.name, ignored_light));

               if (ImGui::Selectable(light.name.c_str(),
                                     string::iequals(*global_light, light.name))) {
                  _edit_stack_world.apply(edits::make_set_memory_value(global_light,
                                                                       light.name),
                                          _edit_context, {.closed = true});
               }

               ImGui::EndDisabled();
            }
         }

         if (ImGui::Selectable("<clear>")) {
            _edit_stack_world.apply(edits::make_set_memory_value(global_light,
                                                                 std::string{""}),
                                    _edit_context, {.closed = true});
         }

         ImGui::EndCombo();
      }

      if (ImGui::IsItemHovered() and
          ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_MouseLeft)) {
         const world::light* light = world::find_entity(_world.lights, *global_light);

         if (light) {
            _interaction_targets.selection.add(light->id);

            ImGui::SetWindowFocus("Selection");
         }
      }
   };

   global_light_editor("Global Light 1", &_world.global_lights.global_light_1,
                       _world.global_lights.global_light_2);
   global_light_editor("Global Light 2", &_world.global_lights.global_light_2,
                       _world.global_lights.global_light_1);

   const auto ambient_editor = [&](const char* label, float3* ambient_color) {
      if (float3 color = *ambient_color; ImGui::ColorEdit3(label, &color.x)) {
         _edit_stack_world.apply(edits::make_set_memory_value(ambient_color, color),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }
   };

   ambient_editor("Ambient Sky Color", &_world.global_lights.ambient_sky_color);
   ambient_editor("Ambient Ground Color", &_world.global_lights.ambient_ground_color);

   ui_texture_pick_widget("Global Environment Map",
                          &_world.global_lights.env_map_texture);

   ImGui::End();
}

}