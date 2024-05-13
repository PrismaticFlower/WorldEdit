#include "world_edit.hpp"

#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "math/quaternion_funcs.hpp"
#include "world/utility/world_utilities.hpp"

#include <numbers>

namespace we {

void world_edit::ui_show_world_selection_rotate_light_region() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   bool open = _selection_edit_tool == selection_edit_tool::rotate_light_region;

   if (ImGui::Begin("Rotate Light Region", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      const float3 last_rotation_amount = _rotate_selection_amount;

      float3 rotate_selection_amount_degrees =
         _rotate_selection_amount * 180.0f / std::numbers::pi_v<float>;

      const bool imgui_edited =
         ImGui::DragFloat3("Amount", &rotate_selection_amount_degrees, 1.0f);

      if (imgui_edited) {
         _rotate_selection_amount =
            rotate_selection_amount_degrees * std::numbers::pi_v<float> / 180.0f;
      }

      world::light* light = world::find_entity(_world.lights, _rotate_light_region_id);

      const bool gizmo_edited =
         _gizmo.show_rotate(light->position, _rotate_selection_amount);

      if (imgui_edited or gizmo_edited) {
         const float3 rotate_delta = (_rotate_selection_amount - last_rotation_amount);
         const quaternion rotation = make_quat_from_euler(rotate_delta);

         if (light) {
            _edit_stack_world.apply(edits::make_set_value(&light->region_rotation,
                                                          rotation * light->region_rotation),
                                    _edit_context);
         }
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
