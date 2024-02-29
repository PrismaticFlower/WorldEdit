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
      const float3 last_move_amount = _move_selection_amount;

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

      const bool imgui_edited =
         ImGui::DragFloat3("Amount", &_move_selection_amount, 0.05f);
      const bool gizmo_edited =
         _gizmo.show_translate(path_centre, quaternion{}, _move_selection_amount);

      if (imgui_edited or gizmo_edited) {
         const float3 move_delta = (_move_selection_amount - last_move_amount);

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
