#include "world_edit.hpp"

#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/world_utilities.hpp"

namespace we {

void world_edit::ui_show_world_selection_move_sector_point() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   bool open = true;

   if (ImGui::Begin("Move Sector Point", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      const float3 last_move_amount = _move_selection_amount;

      if (world::sector* sector =
             world::find_entity(_world.sectors, _move_sector_point_id);
          sector and _move_sector_point_index < sector->points.size()) {
         float2 point = sector->points[_move_sector_point_index];
         float3 sector_centre = {0.0f, 0.0f, 0.0f};

         const bool imgui_edited =
            ImGui::DragFloat3("Amount", &_move_selection_amount, 0.05f);
         const bool imgui_deactivated = ImGui::IsItemDeactivated();

         const bool gizmo_edited =
            _gizmo.show_translate(float3{point.x, sector->base + (sector->height / 2.0f),
                                         point.y},
                                  quaternion{}, _move_selection_amount);
         const bool gizmo_close_edit = _gizmo.can_close_last_edit();

         if (imgui_edited or gizmo_edited) {
            const float3 move_delta = (_move_selection_amount - last_move_amount);

            _edit_stack_world.apply(edits::make_set_vector_value(
                                       &sector->points, _move_sector_point_index,
                                       point + float2{move_delta.x, move_delta.z}),
                                    _edit_context);
         }

         if (imgui_deactivated or gizmo_close_edit) {
            _edit_stack_world.close_last();
         }
      }
      else {
         open = false;
      }

      if (ImGui::Button("Done", {ImGui::CalcItemWidth(), 0.0f})) {
         open = false;
      }
   }

   if (not open) {
      _edit_stack_world.close_last();
      _selection_edit_tool = selection_edit_tool::none;
   }

   ImGui::End();
}

}
