#include "world_edit.hpp"

#include <os/show_in_explorer.hpp>

#include <imgui.h>

namespace we {

void world_edit::ui_show_load_errors() noexcept
{
   ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x / 2.0f,
                            ImGui::GetIO().DisplaySize.y / 2.0f},
                           ImGuiCond_FirstUseEver, {0.5f, 0.5f});
   ImGui::SetNextWindowSize({ImGui::GetIO().DisplaySize.x / 2.0f,
                             ImGui::GetIO().DisplaySize.y / 2.0f},
                            ImGuiCond_FirstUseEver);

   if (ImGui::Begin("Load Errors", &_load_errors_open)) {
      ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);

      if (ImGui::TreeNode("Asset Load Errors", "Asset Load Errors %zu",
                          _world_asset_errors.size())) {
         if (ImGui::BeginTable("Messages", 3,
                               ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_Borders |
                                  ImGuiTableFlags_SizingStretchProp)) {

            ImGui::TableSetupColumn("Asset", ImGuiTableColumnFlags_None, 0.125f);
            ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_None, 0.375f);
            ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_None, 0.5f);
            ImGui::TableHeadersRow();

            int id = 0;

            for (const assets::error& error : _world_asset_errors) {
               ImGui::PushID(id++);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               ImGui::TextUnformatted(error.name.c_str(),
                                      error.name.c_str() + error.name.size());

               if (ImGui::BeginItemTooltip()) {
                  ImGui::TextUnformatted(error.name.c_str(),
                                         error.name.c_str() + error.name.size());

                  ImGui::EndTooltip();
               }

               ImGui::TableNextColumn();
               if (ImGui::TextLink(error.path.c_str())) {
                  os::try_show_in_explorer(error.path);
               }

               if (ImGui::IsItemHovered()) {
                  ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
               }

               if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                  ImGui::TextUnformatted(error.path.c_str(),
                                         error.path.c_str() +
                                            error.path.string_view().size());

                  ImGui::EndTooltip();
               }

               ImGui::TableNextColumn();
               ImGui::PushTextWrapPos();
               ImGui::TextUnformatted(error.message.c_str(),
                                      error.message.c_str() + error.message.size());
               ImGui::PopTextWrapPos();

               if (ImGui::BeginPopupContextItem("##message_context")) {
                  if (ImGui::MenuItem("Copy")) {
                     ImGui::SetClipboardText(error.message.c_str());
                  }

                  ImGui::EndPopup();
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::TreePop();
      }

      const float button_width =
         (ImGui::CalcItemWidth() - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

      if (ImGui::Button("Clear Asset Load Errors", {button_width, 0.0f})) {
         _world_asset_errors.clear();
      }

      ImGui::SetItemTooltip("Clear asset load error list.");
   }

   ImGui::End();
}

void world_edit::ui_show_new_load_error() noexcept
{
   ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x -
                               ImGui::GetStyle().WindowPadding.x,
                            ImGui::GetIO().DisplaySize.y -
                               ImGui::GetStyle().WindowPadding.y},
                           ImGuiCond_Always, {1.0f, 1.0f});

   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

   if (ImGui::Begin("##Load Errors", &_new_load_error_open,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_AlwaysAutoResize |
                       ImGuiWindowFlags_NoFocusOnAppearing)) {
      ImGui::Text("%zu errors occured while loading assets.\n\nView?",
                  _world_asset_errors.size());

      const float button_width =
         (ImGui::GetItemRectSize().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

      if (ImGui::Button("View", {button_width, 0.0f})) {
         _load_errors_open = true;
         _new_load_error_open = false;
      }

      ImGui::SameLine();

      if (ImGui::Button("No", {button_width, 0.0f})) {
         _new_load_error_open = false;
      }
   }

   ImGui::End();

   ImGui::PopStyleVar();
}

}