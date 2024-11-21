#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_value.hpp"
#include "imgui_ext.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/world_utilities.hpp"

namespace we {

void world_edit::ui_show_world_selection_move_path() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   bool open = true;

   if (ImGui::Begin("Move Path", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      float3 path_centre = {0.0f, 0.0f, 0.0f};

      if (const world::path* path =
             world::find_entity(_world.paths, _move_entire_path_id);
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

      const float3 start_path_centre = path_centre;

      const bool imgui_edited = ImGui::DragFloat3("Amount", &path_centre, 0.05f);
      const bool imgui_deactivated = ImGui::IsItemDeactivated();

      const bool gizmo_edited =
         _gizmos.gizmo_position({.name = "Move Path", .alignment = _editor_grid_size},
                                path_centre);
      const bool gizmo_close_edit = _gizmos.can_close_last_edit();

      if (imgui_edited or gizmo_edited) {
         const float3 move_delta = (path_centre - start_path_centre);

         world::path* path = world::find_entity(_world.paths, _move_entire_path_id);

         if (path) {
            edits::bundle_vector bundled_edits;

            bundled_edits.reserve(path->nodes.size());

            for (uint32 i = 0; i < path->nodes.size(); ++i) {
               const world::path::node& node = path->nodes[i];

               bundled_edits.push_back(
                  edits::make_set_vector_value(&path->nodes, i,
                                               &world::path::node::position,
                                               node.position + move_delta));
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
