#include "world_edit.hpp"

#include "edits/set_value.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

template<typename Value>
inline auto make_set_global_lights_value(Value world::global_lights::*value_member_ptr,
                                         Value new_value, Value original_value)
   -> std::unique_ptr<edits::set_global_value<world::global_lights, Value>>
{
   return std::make_unique<edits::set_global_value<world::global_lights, Value>>(
      &world::world::global_lights, value_member_ptr, std::move(new_value),
      std::move(original_value));
}

}

void world_edit::ui_show_world_global_lights_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   ImGui::Begin("Global Lights", &_world_global_lights_editor_open,
                ImGuiWindowFlags_AlwaysAutoResize);

   const auto global_light_editor =
      [&](const char* label, std::string world::global_lights::*global_light_ptr) {
         const std::string& global_light = _world.global_lights.*global_light_ptr;

         if (ImGui::BeginCombo(label, global_light.c_str())) {
            for (const auto& light : _world.lights) {
               if (light.light_type == world::light_type::directional) {
                  if (ImGui::Selectable(light.name.c_str())) {
                     _edit_stack_world.apply(make_set_global_lights_value(global_light_ptr,
                                                                          light.name,
                                                                          global_light),
                                             _edit_context, {.closed = true});
                  }
               }
            }
            ImGui::EndCombo();
         }
      };

   global_light_editor("Global Light 1", &world::global_lights::global_light_1);
   global_light_editor("Global Light 2", &world::global_lights::global_light_2);

   const auto ambient_editor = [&](const char* label,
                                   float3 world::global_lights::*ambient_ptr) {
      const float3 start_color = _world.global_lights.*ambient_ptr;
      float3 color = start_color;

      if (ImGui::ColorEdit3(label, &color.x)) {
         _edit_stack_world.apply(make_set_global_lights_value(ambient_ptr, color,
                                                              start_color),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }
   };

   ambient_editor("Ambient Sky Color", &world::global_lights::ambient_sky_color);
   ambient_editor("Ambient Ground Color", &world::global_lights::ambient_ground_color);

   if (std::string env_map_texture = _world.global_lights.env_map_texture;
       ImGui::InputText("Global Environment Map", &env_map_texture)) {
      _edit_stack_world.apply(make_set_global_lights_value(&world::global_lights::env_map_texture,
                                                           env_map_texture,
                                                           _world.global_lights.env_map_texture),
                              _edit_context);
   }

   if (ImGui::IsItemDeactivatedAfterEdit()) {
      _edit_stack_world.close_last();
   }

   ImGui::End();
}

}