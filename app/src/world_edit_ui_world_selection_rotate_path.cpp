#include "world_edit.hpp"

#include "imgui_ext.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"

#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "world/utility/world_utilities.hpp"

#include <numbers>

namespace we {

void world_edit::ui_show_world_selection_rotate_path() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   bool open = true;

   if (ImGui::Begin("Rotate Path", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      float3 path_centre = {0.0f, 0.0f, 0.0f};

      if (const world::path* path =
             world::find_entity(_world.paths, _selection_rotate_path_context.id);
          path) {
         for (std::size_t i = 0; i < path->nodes.size(); ++i) {
            const world::path::node& node = path->nodes[i];

            path_centre += node.position;
         }

         path_centre /= static_cast<float>(path->nodes.size());
      }
      else {
         open = false;
      }

      const float3 last_rotation_amount = _selection_rotate_path_context.rotation;

      float3 rotate_selection_amount_degrees =
         _selection_rotate_path_context.rotation * 180.0f / std::numbers::pi_v<float>;

      const bool imgui_edited =
         ImGui::DragFloat3("Amount", &rotate_selection_amount_degrees, 1.0f);
      const bool imgui_deactivated = ImGui::IsItemDeactivated();

      if (imgui_edited) {
         _selection_rotate_path_context.rotation =
            rotate_selection_amount_degrees * std::numbers::pi_v<float> / 180.0f;
      }

      const bool gizmo_edited =
         _gizmos.gizmo_rotation({.name = "Rotate Selection Around Centre",
                                 .gizmo_positionWS = path_centre},
                                _selection_rotate_path_context.rotation);
      const bool gizmo_close_edit = _gizmos.can_close_last_edit();

      if (imgui_edited or gizmo_edited) {
         const float3 rotate_delta =
            (_selection_rotate_path_context.rotation - last_rotation_amount);
         const quaternion rotation = make_quat_from_euler(rotate_delta);

         world::path* path =
            world::find_entity(_world.paths, _selection_rotate_path_context.id);

         if (path) {
            edits::bundle_vector bundled_edits;

            bundled_edits.reserve(path->nodes.size());

            for (uint32 node_index = 0; node_index < path->nodes.size(); ++node_index) {
               const world::path::node& node = path->nodes[node_index];

               bundled_edits.push_back(
                  edits::make_set_vector_value(&path->nodes, node_index,
                                               &world::path::node::rotation,
                                               rotation * node.rotation));
               bundled_edits.push_back(edits::make_set_vector_value(
                  &path->nodes, node_index, &world::path::node::position,
                  (rotation * (node.position - path_centre)) + path_centre));
            }

            _edit_stack_world.apply(edits::make_bundle(std::move(bundled_edits)),
                                    _edit_context);
         }
         else {
            open = false;
         }
      }

      if (imgui_deactivated or gizmo_close_edit) {
         _edit_stack_world.close_last();
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
