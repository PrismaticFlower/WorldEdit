#include "world_edit.hpp"

#include "edits/creation_entity_set.hpp"
#include "math/vector_funcs.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_measurement_tool() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 310.0f * _display_scale},
                                       {std::numeric_limits<float>::max(),
                                        310.0f * _display_scale});

   if (ImGui::Begin("Measure", &_measurement_tool_open,
                    ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SeparatorText("Active");

      if (bool draw = _world_draw_mask.measurements; ImGui::Checkbox("Show", &draw)) {
         _world_draw_mask.measurements = draw;
         if (not draw) _world_hit_mask.measurements = false;
      }

      ImGui::SameLine();

      if (bool hit = _world_hit_mask.measurements; ImGui::Checkbox("Select", &hit)) {
         _world_hit_mask.measurements = hit;
         if (hit) _world_draw_mask.measurements = true;
      }

      ImGui::SeparatorText("Measurements");

      if (ImGui::Button("New Measurement", {ImGui::CalcItemWidth(), 0.0f})) {
         _edit_stack_world.apply(edits::make_creation_entity_set(world::measurement{}),
                                 _edit_context);
         _entity_creation_context = {};
      }

      if (ImGui::BeginTable("Measurements", 2,
                            ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY |
                               ImGuiTableFlags_SizingStretchProp)) {
         ImGui::TableSetupColumn("Name");
         ImGui::TableSetupColumn("Length");
         ImGui::TableHeadersRow();

         for (const world::measurement& measurement : _world.measurements) {
            ImGui::PushID(std::to_underlying(measurement.id));

            const bool is_selected =
               world::is_selected(measurement.id, _interaction_targets.selection);

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            const bool select =
               ImGui::Selectable(measurement.name.c_str(), is_selected,
                                 ImGuiSelectableFlags_SpanAllColumns);
            const bool hover = ImGui::IsItemHovered();
            ImGui::TableNextColumn();
            ImGui::Text("%.2fm", distance(measurement.start, measurement.end));

            if (select) {
               if (ImGui::GetIO().KeyCtrl) {
                  _interaction_targets.selection.remove(measurement.id);
               }
               else {
                  if (not ImGui::GetIO().KeyShift) {
                     _interaction_targets.selection.clear();
                  }

                  _interaction_targets.selection.add(measurement.id);
               }
            }

            if (hover) {
               _interaction_targets.hovered_entity = measurement.id;
            }

            ImGui::PopID();
         }

         ImGui::EndTable();
      }
   }

   ImGui::End();
}

}