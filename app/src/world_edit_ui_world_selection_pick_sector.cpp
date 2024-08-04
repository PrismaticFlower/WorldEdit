#include "world_edit.hpp"

#include "edits/set_value.hpp"
#include "world/utility/world_utilities.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_world_selection_pick_sector() noexcept
{
   world::portal* portal =
      world::find_entity(_world.portals, _selection_pick_sector_context.id);

   if (not portal) {
      _selection_edit_tool = selection_edit_tool::pick_sector;
      return;
   }

   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   bool open = _selection_edit_tool == selection_edit_tool::pick_sector;

   if (ImGui::Begin("Pick Sector", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::TextWrapped("Pick a sector with the cursor to link to %s.",
                         portal->name.c_str());

      if (std::exchange(_selection_pick_sector_context.clicked, false)) {
         const world::sector* sector =
            _interaction_targets.hovered_entity->is<world::sector_id>()
               ? world::find_entity(_world.sectors,
                                    _interaction_targets.hovered_entity->get<world::sector_id>())
               : nullptr;

         _edit_stack_world
            .apply(edits::make_set_value(_selection_pick_sector_context.index ==
                                               selection_pick_sector_index::_1
                                            ? &portal->sector1
                                            : &portal->sector2,
                                         sector ? sector->name : ""),
                   _edit_context, {.closed = true});

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