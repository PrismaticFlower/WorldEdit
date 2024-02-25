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

   const auto global_light_editor = [&](const char* label, std::string* global_light) {
      if (ImGui::BeginCombo(label, global_light->c_str())) {
         for (const auto& light : _world.lights) {
            if (light.light_type == world::light_type::directional) {
               if (ImGui::Selectable(light.name.c_str())) {
                  _edit_stack_world.apply(edits::make_set_memory_value(global_light,
                                                                       light.name),
                                          _edit_context, {.closed = true});
               }
            }
         }
         ImGui::EndCombo();
      }
   };

   global_light_editor("Global Light 1", &_world.global_lights.global_light_1);
   global_light_editor("Global Light 2", &_world.global_lights.global_light_2);

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

   if (absl::InlinedVector<char, 256>
          env_map_texture{_world.global_lights.env_map_texture.begin(),
                          _world.global_lights.env_map_texture.end()};
       ImGui::InputText("Global Environment Map", &env_map_texture)) {
      _edit_stack_world
         .apply(edits::make_set_memory_value(&_world.global_lights.env_map_texture,
                                             std::string{env_map_texture.data(),
                                                         env_map_texture.size()}),
                _edit_context);
   }

   if (ImGui::IsItemDeactivatedAfterEdit()) {
      _edit_stack_world.close_last();
   }

   ImGui::End();
}

}