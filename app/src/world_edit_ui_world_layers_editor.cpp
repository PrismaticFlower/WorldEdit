#include "world_edit.hpp"

#include "edits/add_layer.hpp"
#include "edits/delete_layer.hpp"
#include "utility/string_icompare.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

void world_edit::ui_show_world_layers_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({400.0f * _display_scale, 256.0f * _display_scale},
                                       {400.0f * _display_scale, 768.0f * _display_scale});

   if (ImGui::Begin("Layers Editor", &_world_layers_editor_open)) {
      ImGui::SeparatorText("Create New Layer");

      ImGui::InputTextWithHint("##create", "New Layer Name", &_layer_editor_new_name,
                               ImGuiInputTextFlags_CharsNoBlank);
      ImGui::SameLine();

      const bool is_unique_layer_name = [&] {
         for (const auto& desc : _world.layer_descriptions) {
            if (string::iequals(desc.name, _layer_editor_new_name))
               return false;
         }

         return true;
      }();

      if (not is_unique_layer_name) ImGui::BeginDisabled();

      if (ImGui::Button("Create")) {
         _edit_stack_world.apply(edits::make_add_layer(std::move(_layer_editor_new_name),
                                                       _world),
                                 _edit_context);

         _layer_editor_new_name = "";
      }

      if (not is_unique_layer_name) ImGui::EndDisabled();

      if (not is_unique_layer_name and
          ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
         ImGui::SetTooltip("Layer name must be unique.");
      }

      ImGui::SeparatorText("Existing Layers");

      if (ImGui::BeginTable("Layers", 2,
                            ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerH |
                               ImGuiTableFlags_ScrollY)) {
         for (int i = 0; i < _world.layer_descriptions.size(); ++i) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::LabelText("##layer", _world.layer_descriptions[i].name.c_str());
            ImGui::TableNextColumn();

            ImGui::PushID(i);

            const bool base_layer = i == 0;

            if (base_layer) ImGui::BeginDisabled();

            if (ImGui::Button("Delete")) {
               _edit_stack_world.apply(edits::make_delete_layer(i, _world),
                                       _edit_context);
            }

            if (base_layer) ImGui::EndDisabled();

            if (base_layer and
                ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
               ImGui::SetTooltip("The base layer can not be deleted.");
            }

            ImGui::PopID();
         }

         ImGui::EndTable();
      }
   }

   ImGui::End();
}

}