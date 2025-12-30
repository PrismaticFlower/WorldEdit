#include "world_edit.hpp"

#include "munge/message.hpp"
#include "munge/project.hpp"

#include "os/show_in_explorer.hpp"

#include "utility/file_pickers.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace std::literals;

namespace we {

namespace {

namespace custom {

struct command_set {
   const char* name = "";
   const char* description = "";
   std::vector<munge::project_custom_command> munge::project_custom_commands::* commands =
      nullptr;
};

struct command_group {
   const char* name = "";
   std::span<const command_set> sets;
};

constexpr char common_step_description[] =
   "Executed after munging files for common .lvls but before packing them.";
constexpr char common_pack_step_description[] =
   "Executed after packing common .lvls (except 'mission.lvl').";
constexpr char common_mission_child_pack_step_description[] =
   "Executed after packing 'mission.lvl' children but before packing "
   "'mission.lvl'.";
constexpr char common_mission_pack_step_description[] =
   "Executed after packing 'mission.lvl'.";
constexpr char common_fpm_pack_step_description[] =
   "Executed after packing common first person models.";

constexpr std::array<command_set, 5> common_command_sets = {{
   {"Common", common_step_description, &munge::project_custom_commands::common},
   {"Common Pack", common_pack_step_description, &munge::project_custom_commands::common_pack},
   {"Common Mission Child Pack", common_mission_child_pack_step_description,
    &munge::project_custom_commands::common_mission_child_pack},
   {"Common Mission Pack", common_mission_pack_step_description,
    &munge::project_custom_commands::common_mission_pack},
   {"Common FPM Pack", common_fpm_pack_step_description,
    &munge::project_custom_commands::common_fpm_pack},
}};

constexpr char load_step_description[] =
   "Executed after munging files for load .lvls but before packing them.";
constexpr char load_pack_step_description[] =
   "Executed after packing load .lvls.";

constexpr std::array<command_set, 2> load_command_sets = {{
   {"Load", load_step_description, &munge::project_custom_commands::load},
   {"Load Pack", load_pack_step_description, &munge::project_custom_commands::load_pack},
}};

constexpr char shell_step_description[] =
   "Executed after munging files for 'shell.lvl' but before packing them.";
constexpr char shell_pack_step_description[] =
   "Executed after packing 'shell.lvl'.";
constexpr char shell_ps2_pack_step_description[] =
   "Executed after packing 'shellps2.lvl'. This will only happen when munging "
   "for PS2.";

constexpr std::array<command_set, 3> shell_command_sets = {{
   {"Shell", shell_step_description, &munge::project_custom_commands::shell},
   {"Shell Pack", shell_pack_step_description, &munge::project_custom_commands::shell_pack},
   {"Shell PS2 Pack", shell_ps2_pack_step_description,
    &munge::project_custom_commands::shell_ps2_pack},
}};

constexpr char side_step_description[] =
   "Executed for each side after munging files for it but before packing them.";
constexpr char side_child_pack_step_description[] =
   "Executed for each side after packing child .lvls for it.";
constexpr char side_pack_step_description[] =
   "Executed for each side after packing the .lvls.";
constexpr char side_fpm_pack_step_description[] =
   "Executed for each side after packing it's First Person Model .lvls.";

constexpr std::array<command_set, 4> side_command_sets = {{
   {"Side", side_step_description, &munge::project_custom_commands::side},
   {"Side Child Pack", side_child_pack_step_description,
    &munge::project_custom_commands::side_child_pack},
   {"Side Pack", side_pack_step_description, &munge::project_custom_commands::side_pack},
   {"Side FPM Pack", side_fpm_pack_step_description,
    &munge::project_custom_commands::side_fpm_pack},
}};

constexpr char world_step_description[] =
   "Executed for each world after munging files for it but before packing "
   "them.";
constexpr char world_pack_step_description[] =
   "Executed for each 'world.req' after packing it's children and the main "
   "'world.lvl'.";

constexpr std::array<command_set, 2> world_command_sets = {{
   {"World", world_step_description, &munge::project_custom_commands::world},
   {"World Pack", world_pack_step_description, &munge::project_custom_commands::world_pack},
}};

constexpr std::array<command_group, 5> command_groups = {{
   {"Common", common_command_sets},
   {"Load", load_command_sets},
   {"Shell", shell_command_sets},
   {"Side", side_command_sets},
   {"World", world_command_sets},
}};

struct clean_set {
   const char* name = "";
   std::vector<std::string> munge::project_custom_clean_directories::* directories =
      nullptr;
};

constexpr std::array<clean_set, 5> clean_sets = {{
   {"Common", &munge::project_custom_clean_directories::common},
   {"Load", &munge::project_custom_clean_directories::load},
   {"Shell", &munge::project_custom_clean_directories::shell},
   {"Side", &munge::project_custom_clean_directories::side},
   {"World", &munge::project_custom_clean_directories::world},
}};

}

}

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

      const float header_button_width =
         (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3.0f) / 4.0f;

      if (ImGui::Button("Munge", {header_button_width, 0.0f})) {
         _munge_manager.start_munge(
            not _settings.preferences.game_install_path.empty()
               ? io::compose_path(io::path{_settings.preferences.game_install_path}, "Addon")
               : "");
      }

      ImGui::SameLine();

      if (ImGui::Button("Clean", {header_button_width, 0.0f})) {
         _munge_manager.start_clean();
      }

      ImGui::SameLine();

      ImGui::Checkbox("Deploy", &_munge_manager.get_project().deploy);

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
         ImGui::BeginChild("##lines", {}, ImGuiChildFlags_Borders);

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
                               ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
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

               if (ImGui::BeginItemTooltip()) {
                  ImGui::TextUnformatted(warning.tool.c_str(),
                                         warning.tool.c_str() + warning.tool.size());

                  ImGui::EndTooltip();
               }

               ImGui::TableNextColumn();
               if (ImGui::TextLink(warning.file.c_str())) {
                  os::try_show_in_explorer(warning.file);
               }

               if (ImGui::IsItemHovered()) {
                  ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
               }

               if (ImGui::BeginItemTooltip()) {
                  ImGui::TextUnformatted(warning.file.c_str(),
                                         warning.file.c_str() +
                                            warning.file.string_view().size());

                  ImGui::EndTooltip();
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
                               ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
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

               if (ImGui::BeginItemTooltip()) {
                  ImGui::TextUnformatted(error.tool.c_str(),
                                         error.tool.c_str() + error.tool.size());

                  ImGui::EndTooltip();
               }

               ImGui::TableNextColumn();
               if (ImGui::TextLink(error.file.c_str())) {
                  os::try_show_in_explorer(error.file);
               }

               if (ImGui::IsItemHovered()) {
                  ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
               }

               if (ImGui::BeginItemTooltip()) {
                  ImGui::TextUnformatted(error.file.c_str(),
                                         error.file.c_str() +
                                            error.file.string_view().size());

                  ImGui::EndTooltip();
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

   if (not _munge_context.prompted_browse_modtools and
       _munge_manager.get_project().config.toolsfl_bin_path.empty()) {
      io::path& toolsfl_bin_path = _munge_manager.get_project().config.toolsfl_bin_path;

      ImGui::OpenPopup("Browse for Modtools");

      ImGui::SetNextWindowSizeConstraints({415.0f * _display_scale, -1.0f},
                                          {FLT_MAX, FLT_MAX});

      if (ImGui::BeginPopupModal("Browse for Modtools", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoSavedSettings)) {
         if (_munge_context.prompted_browse_modtools_failed) {
            ImGui::TextWrapped(
               "The directory you selected does not appear to be your modtools "
               "directory. Make sure you select the directory containing "
               "\"ToolsFL\".\n\nBrowse again?");
         }
         else {
            ImGui::TextWrapped(
               "In order to use munge features WorldEdit must know where "
               "your modtools are installed.\n\nBrowse for them?");
         }

         const float button_width =
            (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

         ImGui::SetItemDefaultFocus();

         if (ImGui::Button("Yes", {button_width, 0.0f})) {
            static const GUID browse_for_modtools_guid = {0xb144b02,
                                                          0x7537,
                                                          0x4c6d,
                                                          {0x9a, 0x6, 0x1f, 0x3b,
                                                           0x67, 0x64, 0xc8, 0x1c}};

            const std::optional<io::path> selected_path = utility::show_folder_picker({
               .title = L"Browse for Modtools",
               .ok_button_label = L"Select",
               .default_folder = _project_dir,
               .picker_guid = browse_for_modtools_guid,
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
                  _munge_context.prompted_browse_modtools_failed = true;
               }
            }
            else {
               _munge_context.prompted_browse_modtools_failed = true;
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
   else if (_munge_manager.get_project().deploy and
            _settings.preferences.game_install_path.empty()) {
      std::string& game_install_path = _settings.preferences.game_install_path;

      if (not _munge_context.prompted_browse_game_install) {
         ImGui::OpenPopup("Browse for Game Install");
      }

      ImGui::SetNextWindowSizeConstraints({415.0f * _display_scale, -1.0f},
                                          {FLT_MAX, FLT_MAX});

      if (ImGui::BeginPopupModal("Browse for Game Install", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoSavedSettings)) {
         if (_munge_context.prompted_browse_game_install_failed) {
            ImGui::Text(
               "The directory you selected does not appear to be your game "
               "install "
               "directory. Make sure you select the directory containing "
               "\"BattlefrontII.exe\". In almost all cases it will be the "
               "\"GameData\" directory.\n\nBrowse again?");
         }
         else {
            ImGui::Text("In order to automatically deploy your map for testing "
                        "WorldEdit must know where "
                        "your game is installed.\n\nBrowse for it?");
         }

         const float button_width =
            (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;

         ImGui::SetItemDefaultFocus();

         if (ImGui::Button("Yes", {button_width, 0.0f})) {
            static const GUID browse_for_game_install_guid = {0x7f3b89ad,
                                                              0xb6d,
                                                              0x47e8,
                                                              {0x9a, 0x2d, 0xcf,
                                                               0x5e, 0xc4, 0xb7,
                                                               0xd6, 0x3f}};

            const std::optional<io::path> selected_path = utility::show_folder_picker({
               .title = L"Browse for Game Install",
               .ok_button_label = L"Select",
               .default_folder = _project_dir,
               .picker_guid = browse_for_game_install_guid,
               .window = _window,
            });

            if (selected_path) {
               if (io::exists(io::compose_path(*selected_path,
                                               R"(..\BattlefrontII.exe)"))) {
                  game_install_path = selected_path->parent_path();

                  _munge_context.prompted_browse_game_install = true;
               }
               else if (io::exists(io::compose_path(*selected_path,
                                                    R"(GameData\BattlefrontII.exe)"))) {
                  game_install_path =
                     io::compose_path(*selected_path, R"(GameData)").string_view();

                  _munge_context.prompted_browse_game_install = true;
               }
               else if (io::exists(io::compose_path(*selected_path,
                                                    "BattlefrontII.exe"))) {
                  game_install_path = selected_path->string_view();

                  _munge_context.prompted_browse_game_install = true;
               }
               else {
                  _munge_context.prompted_browse_game_install_failed = true;
               }
            }
            else {
               _munge_context.prompted_browse_game_install_failed = true;
            }
         }

         ImGui::SameLine();

         if (ImGui::Button("No", {button_width, 0.0f})) {
            _munge_context.prompted_browse_game_install = true;

            ImGui::CloseCurrentPopup();
         }

         ImGui::SetItemTooltip("Deploy will not work.");

         ImGui::EndPopup();
      }
   }
}

void world_edit::ui_show_munge_config_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({960.0f * _display_scale, 698.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({640.0f * _display_scale, 0.0f},
                                       {FLT_MAX, FLT_MAX});
   if (ImGui::Begin("Munge Config", &_munge_config_editor_open,
                    ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
      munge::project_config& config = _munge_manager.get_project().config;

      if (ImGui::BeginTable("Common", 2,
                            ImGuiTableFlags_BordersInnerH |
                               ImGuiTableFlags_SizingStretchSame)) {
         ImGui::TableNextColumn();

         if (ImGui::BeginCombo("Tool Set", "SWBF2")) {

            ImGui::EndCombo();
         }

         ImGui::TableNextColumn();

         if (ImGui::BeginCombo("Platform", to_ui_string(config.platform))) {
            for (const munge::project_platform other :
                 {munge::project_platform::pc, munge::project_platform::ps2,
                  munge::project_platform::xbox}) {
               if (ImGui::Selectable(to_ui_string(other), config.platform == other)) {
                  config.platform = other;
               }
            }

            ImGui::EndCombo();
         }

         ImGui::TableNextRow();

         ImGui::TableNextColumn();

         ImGui::Checkbox("Use Builtin Munge Tools", &config.use_builtin_tools);

         ImGui::SetItemTooltip("Use builtin munge tools when possible. Does "
                               "not affect blocks munging.");

         ImGui::EndTable();
      }

      ImGui::Separator();

      ImGui::LabelText("ToolsFL\\bin Path", "%s", config.toolsfl_bin_path.c_str());

      ImGui::SetItemTooltip(
         "Path ToolsFL\\bin containing the modtools mungers. WorldEdit will "
         "normally find this on it's own and will prompt you if it can't.");

      if (ImGui::Button("Clear##ToolsFL\\bin Path", {ImGui::CalcItemWidth(), 0.0f})) {
         config.toolsfl_bin_path = {};
      }

      ImGui::SetItemTooltip("Clear ToolsFL\\bin Path. WorldEdit will prompt "
                            "for it again if needed.");

      ImGui::SeparatorText("Custom Munge Commands");

      for (const custom::command_group& group : custom::command_groups) {
         if (not ImGui::TreeNode(group.name)) continue;

         for (const custom::command_set& set : group.sets) {
            if (ImGui::TreeNode(&set, "%s Custom Commands", set.name)) {
               ImGui::PushTextWrapPos();
               ImGui::TextUnformatted(set.description);
               ImGui::PopTextWrapPos();

               ImGui::Separator();

               std::vector<munge::project_custom_command>& commands =
                  config.custom_commands.*set.commands;

               std::optional<std::size_t> swap_up_index;
               std::optional<std::size_t> swap_down_index;
               std::optional<std::size_t> delete_index;

               ImGui::Unindent();

               if (ImGui::BeginTable("Commands", 2, ImGuiTableFlags_SizingStretchProp)) {
                  for (std::size_t i = 0; i < commands.size(); ++i) {
                     ImGui::PushID(static_cast<int>(i));

                     ImGui::TableNextRow();
                     ImGui::TableNextColumn();

                     ImGui::BeginDisabled(i <= 0);

                     if (ImGui::ArrowButton("##up", ImGuiDir_Up) and i > 0) {
                        swap_up_index = i;
                     }

                     ImGui::EndDisabled();

                     ImGui::BeginDisabled(i + 1 >= commands.size());

                     if (ImGui::ArrowButton("##down", ImGuiDir_Down) and
                         i + 1 < commands.size()) {
                        swap_down_index = i;
                     }

                     ImGui::EndDisabled();

                     ImGui::TableNextColumn();

                     munge::project_custom_command& command = commands[i];

                     ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                     ImGui::InputText("##command", &command.command_line);

                     if (ImGui::BeginTable("Control", 3,
                                           ImGuiTableFlags_BordersInnerV |
                                              ImGuiTableFlags_SizingStretchSame)) {
                        ImGui::TableNextColumn();

                        if (ImGui::BeginCombo("Platform",
                                              to_ui_string(command.platform_filter))) {
                           for (const munge::project_platform_filter other :
                                {munge::project_platform_filter::all,
                                 munge::project_platform_filter::pc,
                                 munge::project_platform_filter::ps2,
                                 munge::project_platform_filter::xbox}) {
                              if (ImGui::Selectable(to_ui_string(other),
                                                    command.platform_filter == other)) {
                                 command.platform_filter = other;
                              }
                           }
                           ImGui::EndCombo();
                        }

                        ImGui::TableNextColumn();

                        ImGui::Checkbox("Detach", &command.detach);

                        ImGui::SetItemTooltip(
                           "Normally the munge process will wait for a process "
                           "launched from a command to exit before continuing. "
                           "Enabling Detach will cause it to continue "
                           "immediately and no output from the command will "
                           "show "
                           "up in WorldEdit.");

                        ImGui::TableNextColumn();

                        if (ImGui::Button("Remove",
                                          {ImGui::GetContentRegionAvail().x, 0.0f})) {
                           delete_index = i;
                        }

                        ImGui::EndTable();
                     }

                     ImGui::PopID();
                  }

                  ImGui::EndTable();
               }

               ImGui::Indent();

               if (swap_up_index and *swap_up_index > 0) {
                  std::swap(commands[*swap_up_index - 1], commands[*swap_up_index]);
               }

               if (swap_down_index and *swap_down_index + 1 < commands.size()) {
                  std::swap(commands[*swap_down_index + 1], commands[*swap_down_index]);
               }

               if (delete_index) {
                  commands.erase(commands.begin() + *delete_index);
               }

               ImGui::Separator();

               if (ImGui::Button("Add", {ImGui::GetContentRegionAvail().x, 0.0f})) {
                  commands.emplace_back();
               }

               ImGui::TreePop();
            }
         }

         ImGui::TreePop();
      }

      ImGui::SeparatorText("Custom Clean Directories");

      for (const custom::clean_set& set : custom::clean_sets) {
         if (not ImGui::TreeNode(&set, "%s Clean Directories", set.name)) {
            continue;
         }

         std::optional<std::size_t> delete_index;

         std::vector<std::string>& directories =
            config.custom_clean_directories.*set.directories;

         for (std::size_t i = 0; i < directories.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));

            ImGui::InputText("Directory", &directories[i]);

            if (ImGui::Button("Remove", {ImGui::CalcItemWidth(), 0.0f})) {
               delete_index = i;
            }

            ImGui::PopID();

            ImGui::Separator();
         }

         if (delete_index) {
            directories.erase(directories.begin() + *delete_index);
         }

         if (ImGui::Button("Add", {ImGui::CalcItemWidth(), 0.0f})) {
            directories.emplace_back();
         }

         ImGui::TreePop();
      }
   }

   ImGui::End();
}

}