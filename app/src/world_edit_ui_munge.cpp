#include "world_edit.hpp"

#include "munge/project.hpp"
#include "utility/file_pickers.hpp"
#include "utility/show_in_explorer.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace std::literals;

namespace we {

void world_edit::ui_show_munge_manager() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({960.0f * _display_scale, 698.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({640.0f * _display_scale, 0.0f},
                                       {FLT_MAX, FLT_MAX});

   if (ImGui::Begin("Munge Manager", &_munge_manager_open,
                    ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
      ImGui::BeginDisabled(_munge_manager.is_busy());

      if (ImGui::Button("Munge")) _munge_manager.start_munge();

      ImGui::SameLine();

      if (ImGui::Button("Clean")) _munge_manager.start_clean();

      ImGui::SeparatorText("Active");

      if (ImGui::BeginTable("Global", 4, ImGuiTableFlags_BordersInnerV)) {
         ImGui::TableNextColumn();
         ImGui::Checkbox("Addme", &_munge_manager.get_project().addme_active);

         ImGui::TableNextColumn();
         ImGui::Checkbox("Common", &_munge_manager.get_project().common_active);

         ImGui::TableNextColumn();
         ImGui::Checkbox("Load", &_munge_manager.get_project().load_active);

         ImGui::TableNextColumn();
         ImGui::Checkbox("Shell", &_munge_manager.get_project().shell_active);

         bool active_world = false;

         for (munge::project_child& world : _munge_manager.get_project().worlds) {
            active_world |= world.active;
         }

         ImGui::TableNextColumn();
         if (ImGui::Checkbox("<Worlds Active>", &active_world)) {
            for (munge::project_child& world : _munge_manager.get_project().worlds) {
               world.active = active_world;
            }
         }

         bool active_side = false;

         for (munge::project_child& side : _munge_manager.get_project().sides) {
            active_side |= side.active;
         }

         ImGui::TableNextColumn();
         if (ImGui::Checkbox("<Sides Active>", &active_side)) {
            for (munge::project_child& side : _munge_manager.get_project().sides) {
               side.active = active_side;
            }
         }

         ImGui::TableNextColumn();
         ImGui::Checkbox("Sound", &_munge_manager.get_project().sound_active);

         ImGui::EndTable();
      }

      if (ImGui::TreeNode("Worlds")) {
         if (ImGui::BeginTable("Worlds", 1,
                               ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterV)) {
            for (munge::project_child& world : _munge_manager.get_project().worlds) {
               ImGui::TableNextColumn();
               ImGui::Checkbox(world.name.c_str(), &world.active);
            }

            ImGui::EndTable();
         }

         ImGui::TreePop();
      }

      if (ImGui::TreeNode("Sides")) {
         if (ImGui::BeginTable("Sides", 1,
                               ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterV)) {
            for (munge::project_child& side : _munge_manager.get_project().sides) {
               ImGui::TableNextColumn();
               ImGui::Checkbox(side.name.c_str(), &side.active);
            }

            ImGui::EndTable();
         }

         ImGui::TreePop();
      }

      if (ImGui::TreeNode("Sound")) {
         ImGui::SeparatorText("Worlds");

         if (ImGui::BeginTable("Worlds", 3,
                               ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterV |
                                  ImGuiTableFlags_SizingStretchProp)) {
            for (munge::project_child_sound_world& sound :
                 _munge_manager.get_project().sound_worlds) {
               ImGui::PushID(sound.name.c_str());

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               ImGui::TextUnformatted(sound.name.c_str());

               ImGui::TableNextColumn();
               ImGui::Checkbox("Active", &sound.active);

               ImGui::TableNextColumn();
               ImGui::Checkbox("Localized", &sound.localized);

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         if (not _munge_manager.get_project().sound_shared.empty()) {
            ImGui::SeparatorText("Shared");

            ImGui::Checkbox("Enable Common Bank",
                            &_munge_manager.get_project().sound_common_bank);

            ImGui::SetItemTooltip(
               "When munging for PC produce common.bnk. This file will have "
               "all sound banks (.sfx/.asfx) packed into it instead of the "
               "separate .lvls.");

            if (ImGui::BeginTable("Shared", 2,
                                  ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterV |
                                     ImGuiTableFlags_SizingStretchProp)) {
               for (munge::project_child_sound_shared& sound :
                    _munge_manager.get_project().sound_shared) {
                  ImGui::PushID(sound.name.c_str());

                  ImGui::TableNextRow();

                  ImGui::TableNextColumn();
                  ImGui::TextUnformatted(sound.name.c_str());

                  ImGui::SetItemTooltip(
                     "Shared sound .lvls must all be munged when "
                     "any sound .lvl is munged.");

                  ImGui::TableNextColumn();
                  ImGui::Checkbox("Localized", &sound.localized);

                  ImGui::PopID();
               }

               ImGui::EndTable();
            }
         }

         ImGui::SeparatorText("Localizations");

         if (ImGui::BeginTable("Localizations", 3)) {
            std::optional<std::size_t> delete_localization;

            std::vector<munge::project_sound_localization>& sound_localizations =
               _munge_manager.get_project().sound_localizations;

            for (std::size_t i = 0; i < sound_localizations.size(); ++i) {
               munge::project_sound_localization& localization =
                  sound_localizations[i];

               ImGui::PushID(localization.language.c_str());

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               ImGui::InputText("Language", &localization.language);

               ImGui::TableNextColumn();
               ImGui::InputText("Output Directory", &localization.output_directory);

               ImGui::SetItemTooltip(
                  R"(Output directory for localized sound. This is relative to "_LVL_PC\sound".)");

               ImGui::TableNextColumn();
               if (ImGui::Button("X")) delete_localization = i;

               ImGui::PopID();
            }

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::InputTextWithHint("Language", "i.e. french",
                                     &_munge_context.new_sound_localization_language);

            ImGui::TableNextColumn();
            ImGui::InputTextWithHint("Output Directory", "i.e. fr",
                                     &_munge_context.new_sound_localization_directory);

            ImGui::SetItemTooltip(
               R"(Output directory for localized sound. This is relative to "_LVL_PC\sound".)");

            ImGui::TableNextColumn();

            ImGui::BeginDisabled(
               _munge_context.new_sound_localization_language.empty() or
               _munge_context.new_sound_localization_directory.empty());

            if (ImGui::Button("Add")) {
               sound_localizations.push_back({
                  .language = std::move(_munge_context.new_sound_localization_language),
                  .output_directory =
                     std::move(_munge_context.new_sound_localization_directory),
               });
            }

            ImGui::EndDisabled();

            if (delete_localization) {
               sound_localizations.erase(sound_localizations.begin() +
                                         *delete_localization);
            }

            ImGui::EndTable();
         }

         ImGui::TreePop();
      }

      ImGui::EndDisabled();

      ImGui::Separator();

      if (ImGui::TreeNodeEx("Output", ImGuiTreeNodeFlags_DefaultOpen)) {
         if (ImGui::BeginTable("Select Output", 2,
                               ImGuiTableFlags_SizingStretchSame |
                                  ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableNextColumn();
            if (ImGui::RadioButton("Display Standard Output",
                                   _munge_context.show_stdout)) {
               if (not _munge_context.show_stdout) {
                  _munge_context.selected_output_lines.clear();
               }

               _munge_context.show_stdout = true;
            }

            ImGui::TableNextColumn();
            if (ImGui::RadioButton("Display Standard Error Output",
                                   not _munge_context.show_stdout)) {
               if (_munge_context.show_stdout) {
                  _munge_context.selected_output_lines.clear();
               }

               _munge_context.show_stdout = false;
            }

            ImGui::EndTable();
         }

         ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});
         ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                             {ImGui::GetStyle().WindowPadding.x, 0.0f});
         ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});

         ImGui::SetNextWindowBgAlpha(1.0f);
         ImGui::BeginChild("##lines", {}, ImGuiChildFlags_Border);

         const std::span<const std::string_view> lines =
            _munge_context.show_stdout
               ? _munge_manager.view_standard_output_lines()
               : _munge_manager.view_standard_error_lines();

         std::vector<bool>& selected_lines = _munge_context.selected_output_lines;

         if (selected_lines.size() != lines.size()) {
            selected_lines.reserve(std::bit_ceil(lines.size()));
            selected_lines.resize(lines.size());
         }

         ImGuiMultiSelectIO* ms_io =
            ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_BoxSelect1d, -1,
                                    static_cast<int>(lines.size()));
         ImGuiListClipper clipper;

         clipper.Begin(static_cast<int>(lines.size()));

         if (ms_io->RangeSrcItem != -1) {
            clipper.IncludeItemByIndex(static_cast<int>(ms_io->RangeSrcItem));
         }

         const uint32 selected_color =
            ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg));

         ImGui::PushStyleColor(ImGuiCol_Header, 0);
         ImGui::PushStyleColor(ImGuiCol_HeaderHovered, 0);
         ImGui::PushStyleColor(ImGuiCol_HeaderActive, 0);

         while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
               ImGui::PushID(i);

               const ImVec2 cursor = ImGui::GetCursorPos();
               const ImVec2 screen_cursor = ImGui::GetCursorScreenPos();

               ImGui::SetNextItemSelectionUserData(i);
               ImGui::Selectable("##selectable", selected_lines[i]);

               ImGui::SetCursorPos(cursor);

               if (selected_lines[i]) {
                  const ImVec2 text_size =
                     ImGui::CalcTextSize(lines[i].data(),
                                         lines[i].data() + lines[i].size());

                  ImGui::GetWindowDrawList()
                     ->AddRectFilled({screen_cursor.x, screen_cursor.y},
                                     {screen_cursor.x + text_size.x,
                                      screen_cursor.y + text_size.y},
                                     selected_color);
               }

               ImGui::SetCursorPos(cursor);

               ImGui::TextUnformatted(lines[i].data(),
                                      lines[i].data() + lines[i].size());

               ImGui::PopID();
            }
         }

         ImGui::PopStyleColor();
         ImGui::PopStyleColor();
         ImGui::PopStyleColor();

         if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY();
         }

         ms_io = ImGui::EndMultiSelect();

         ImGuiSelectionExternalStorage selection_edit;

         selection_edit.UserData = &selected_lines;
         selection_edit.AdapterSetItemSelected =
            [](ImGuiSelectionExternalStorage* self, int idx, bool selected) {
               (*static_cast<std::vector<bool>*>(self->UserData))[idx] = selected;
            };
         selection_edit.ApplyRequests(ms_io);

         if (ImGui::Shortcut(ImGuiKey_C | ImGuiMod_Ctrl)) {
            std::size_t copy_size = 0;

            for (std::size_t i = 0; i < lines.size(); ++i) {
               if (selected_lines[i]) copy_size += lines[i].size() + 1;
            }

            std::string copied_string;
            copied_string.reserve(copy_size);

            for (std::size_t i = 0; i < lines.size(); ++i) {
               if (not selected_lines[i]) continue;

               copied_string += lines[i];
               copied_string += '\n';
            }

            ImGui::SetClipboardText(copied_string.c_str());
         }

         ImGui::EndChild();

         ImGui::PopStyleVar();
         ImGui::PopStyleVar();
         ImGui::PopStyleVar();

         ImGui::TreePop();
      }

      const munge::report& report = _munge_manager.get_munge_report();

      if (ImGui::TreeNode("Warnings", "Warnings %zu", report.warnings.size())) {
         if (ImGui::BeginTable("Messages", 3,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_SizingStretchProp)) {

            ImGui::TableSetupColumn("Tool");
            ImGui::TableSetupColumn("File");
            ImGui::TableSetupColumn("Message");
            ImGui::TableHeadersRow();

            int id = 0;

            for (const munge::message& warning : report.warnings) {
               ImGui::PushID(id++);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               ImGui::TextUnformatted(warning.tool.c_str(),
                                      warning.tool.c_str() + warning.tool.size());

               ImGui::TableNextColumn();
               if (ImGui::TextLink(warning.file.c_str())) {
                  utility::try_show_in_explorer(warning.file);
               }

               if (ImGui::IsItemHovered()) {
                  ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
               }

               ImGui::TableNextColumn();
               ImGui::TextUnformatted(warning.message.c_str(),
                                      warning.message.c_str() + warning.message.size());

               if (ImGui::BeginPopupContextItem("##message_context")) {
                  if (ImGui::MenuItem("Copy")) {
                     ImGui::SetClipboardText(warning.message.c_str());
                  }

                  ImGui::EndPopup();
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::TreePop();
      }

      if (ImGui::TreeNode("Errors", "Errors %zu", report.errors.size())) {
         if (ImGui::BeginTable("Messages", 3,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_SizingStretchProp)) {

            ImGui::TableSetupColumn("Tool");
            ImGui::TableSetupColumn("File");
            ImGui::TableSetupColumn("Message");
            ImGui::TableHeadersRow();

            int id = 0;

            for (const munge::message& error : report.errors) {
               ImGui::PushID(id++);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               ImGui::TextUnformatted(error.tool.c_str(),
                                      error.tool.c_str() + error.tool.size());

               ImGui::TableNextColumn();
               if (ImGui::TextLink(error.file.c_str())) {
                  utility::try_show_in_explorer(error.file);
               }

               if (ImGui::IsItemHovered()) {
                  ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
               }

               ImGui::TableNextColumn();
               ImGui::TextUnformatted(error.message.c_str(),
                                      error.message.c_str() + error.message.size());

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
   }

   ImGui::End();

   if (io::path& toolsfl_bin_path = _munge_manager.get_project().config.toolsfl_bin_path;
       toolsfl_bin_path.empty()) {
      if (not _munge_context.tried_auto_find_toolsfl) {
         const io::path candidate_toolsfl_bin_path =
            io::compose_path(_project_dir.parent_path(), R"(ToolsFL\bin)");

         if (io::exists(io::compose_path(candidate_toolsfl_bin_path,
                                         "LevelPack.exe"))) {
            toolsfl_bin_path = candidate_toolsfl_bin_path;
         }

         _munge_context.tried_auto_find_toolsfl = true;
      }
      else if (not _munge_context.prompted_browse_modtools) {
         ImGui::OpenPopup("Browse For Modtools");
      }

      if (ImGui::BeginPopupModal("Browse For Modtools")) {
         if (_munge_context.prompted_browse_failed) {
            ImGui::TextWrapped(
               "The directory you selected does not appear to be your modtools "
               "directory. Make sure you select the directory containing "
               "\"ToolsFL\".\n\nBrowse again?");
         }
         else {
            ImGui::TextWrapped(
               "In order to use munge features WorldEdit must know where "
               "your modtools are installed.\n\n\nBrowse for them?");
         }

         const float button_width =
            (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

         if (ImGui::Button("Yes", {button_width, 0.0f})) {
            const std::optional<io::path> selected_path = utility::show_folder_picker({
               .title = L"Browse for Modtools",
               .ok_button_label = L"Select",
               .default_folder = _project_dir,
               .window = _window,
            });

            if (selected_path) {
               if (io::exists(io::compose_path(*selected_path,
                                               R"(ToolsFL\bin\LevelPack.exe)"))) {
                  toolsfl_bin_path =
                     io::compose_path(*selected_path, R"(ToolsFL\bin)");

                  _munge_context.prompted_browse_modtools = true;
               }
               else if (io::exists(io::compose_path(*selected_path,
                                                    R"(bin\LevelPack.exe)"))) {
                  toolsfl_bin_path = io::compose_path(*selected_path, R"(bin)");

                  _munge_context.prompted_browse_modtools = true;
               }
               else if (io::exists(
                           io::compose_path(*selected_path, "LevelPack.exe"))) {
                  toolsfl_bin_path = *selected_path;

                  _munge_context.prompted_browse_modtools = true;
               }
               else {
                  _munge_context.prompted_browse_failed = true;
               }
            }
            else {
               _munge_context.prompted_browse_failed = true;
            }
         }

         ImGui::SameLine();

         if (ImGui::Button("No", {button_width, 0.0f})) {
            _munge_context.prompted_browse_modtools = true;

            ImGui::CloseCurrentPopup();
         }

         ImGui::SetItemTooltip("You will be unabled to use munge features.");

         ImGui::EndPopup();
      }
   }
}

}