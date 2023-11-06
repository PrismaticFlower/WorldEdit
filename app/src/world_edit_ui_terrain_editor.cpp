#include "edits/set_value.hpp"
#include "world_edit.hpp"

#include "imgui.h"

namespace we {

namespace {

template<typename Value>
inline auto make_set_terrain_value(Value world::terrain::*value_member_ptr,
                                   Value new_value, Value original_value)
   -> std::unique_ptr<edits::set_global_value<world::terrain, Value>>
{
   return std::make_unique<edits::set_global_value<world::terrain, Value>>(
      &world::world::terrain, value_member_ptr, std::move(new_value),
      std::move(original_value));
}

}

void world_edit::ui_show_terrain_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 620.0f * _display_scale},
                                       {std::numeric_limits<float>::max(),
                                        620.0f * _display_scale});

   if (ImGui::Begin("Terrain Editor")) {
      if (bool active = _world.terrain.active_flags.terrain;
          ImGui::Checkbox("Terrain Enabled", &active)) {
         world::active_flags flags = _world.terrain.active_flags;

         flags.terrain = active;

         _edit_stack_world.apply(make_set_terrain_value(&world::terrain::active_flags,
                                                        flags, _world.terrain.active_flags),
                                 _edit_context, {.closed = true});
      }

      ImGui::SeparatorText("Advanced");

      ImGui::Text("Edit these if you're the adventurous sort or just know what "
                  "you're doing.");

      if (float height_scale = _world.terrain.height_scale;
          ImGui::DragFloat("Height Scale", &height_scale, 0.0025f)) {
         _edit_stack_world.apply(make_set_terrain_value(&world::terrain::height_scale,
                                                        std::max(static_cast<float>(height_scale),
                                                                 0.0f),
                                                        _world.terrain.height_scale),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }

      if (float grid_scale = _world.terrain.grid_scale;
          ImGui::DragFloat("Grid Scale", &grid_scale, 1.0f, 1.0f, 16777216.0f)) {
         _edit_stack_world.apply(make_set_terrain_value(&world::terrain::grid_scale,
                                                        std::max(static_cast<float>(grid_scale),
                                                                 1.0f),
                                                        _world.terrain.grid_scale),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }
   }

   ImGui::End();
}

}