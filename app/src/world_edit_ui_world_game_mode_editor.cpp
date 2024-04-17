#include "world_edit.hpp"

#include "edits/add_game_mode.hpp"
#include "edits/delete_game_mode.hpp"
#include "edits/game_mode_link_layer.hpp"
#include "edits/game_mode_unlink_layer.hpp"
#include "edits/rename_game_mode.hpp"
#include "imgui_ext.hpp"
#include "utility/string_icompare.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

bool is_unique_game_mode_name(std::string_view name, const world::world& world) noexcept
{
   for (const auto& desc : world.game_modes) {
      if (string::iequals(name, desc.name)) return false;
   }

   return true;
};

int imgui_game_mode_letter_filter(ImGuiInputTextCallbackData* data) noexcept
{
   const wchar_t c = data->EventChar;

   // Reject non-printable chars.
   if (c < L' ') return 1;

   // Reject common filesystem reserved chars.
   if (c == L'<') return 1;
   if (c == L'>') return 1;
   if (c == L':') return 1;
   if (c == L'"') return 1;
   if (c == L'/') return 1;
   if (c == L'\\') return 1;
   if (c == L'|') return 1;
   if (c == L'?') return 1;
   if (c == L'*') return 1;

   return 0;
}

auto make_game_mode_imgui_id(const uint32 index) noexcept -> const void*
{
   return reinterpret_cast<const void*>(static_cast<std::intptr_t>(index));
}

}

void world_edit::ui_show_world_game_mode_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({400.0f * _display_scale, 256.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({400.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("Game Mode Editor", &_world_game_mode_editor_open)) {
      ImGui::SeparatorText("Create New Game Mode");

      ImGui::InputTextWithHint("##create", "New Game Mode Name",
                               &_game_mode_editor_new_name,
                               ImGuiInputTextFlags_CallbackCharFilter,
                               imgui_game_mode_letter_filter);
      ImGui::SameLine();

      const bool new_game_mode_name_is_unique =
         is_unique_game_mode_name(_game_mode_editor_new_name, _world);
      const bool has_game_mode_name_name = not _game_mode_editor_new_name.empty();
      const bool can_create_game_mode =
         new_game_mode_name_is_unique and has_game_mode_name_name;

      if (not can_create_game_mode) ImGui::BeginDisabled();

      if (ImGui::Button("Create")) {
         _edit_stack_world.apply(edits::make_add_game_mode(_game_mode_editor_new_name,
                                                           _world),
                                 _edit_context);

         _game_mode_editor_new_name = "";
      }

      if (not can_create_game_mode) ImGui::EndDisabled();

      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
         if (not new_game_mode_name_is_unique) {
            ImGui::SetTooltip("Game mode name must be unique.");
         }
         else if (not has_game_mode_name_name) {
            ImGui::SetTooltip("Game mode must have a name.");
         }
      }

      ImGui::SeparatorText("Existing Game Modes");

      for (int game_mode_index = 0; game_mode_index < _world.game_modes.size();
           ++game_mode_index) {
         const world::game_mode_description& game_mode =
            _world.game_modes[game_mode_index];

         if (ImGui::TreeNode(make_game_mode_imgui_id(game_mode_index),
                             game_mode.name.c_str())) {
            ImGui::SeparatorText("Included Layers");

            const bool is_common_game_mode = game_mode_index == 0;

            for (int layer_index = 0;
                 layer_index < _world.layer_descriptions.size(); ++layer_index) {
               const bool included_in_common = [&] {
                  for (const int game_mode_layer : _world.game_modes[0].layers) {
                     if (game_mode_layer == layer_index) return true;
                  }

                  return false;
               }();
               const auto check_is_included_in_any_game_mode = [&] {
                  for (int i = 1; i < _world.game_modes.size(); ++i) {
                     for (const int game_mode_layer : _world.game_modes[i].layers) {
                        if (game_mode_layer == layer_index) return true;
                     }
                  }

                  return false;
               };

               if (not is_common_game_mode and included_in_common) {
                  ImGui::Selectable(
                     fmt::format("{} (included from Common)",
                                 _world.layer_descriptions[layer_index].name)
                        .c_str(),
                     true, ImGuiSelectableFlags_Disabled);
               }
               else if (is_common_game_mode and check_is_included_in_any_game_mode()) {
                  ImGui::Selectable(
                     fmt::format("{} (used by Game Modes)",
                                 _world.layer_descriptions[layer_index].name)
                        .c_str(),
                     false, ImGuiSelectableFlags_Disabled);

                  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                     ImGui::SetTooltip("A layer can't be included by "
                                       "both Common and game "
                                       "modes at the same time. This "
                                       "would cause duplicate "
                                       "entities to appear ingame.");
                  }
               }
               else {
                  const bool included_in_game_mode = [&] {
                     for (const int game_mode_layer : game_mode.layers) {
                        if (game_mode_layer == layer_index) return true;
                     }

                     return false;
                  }();

                  if (ImGui::Selectable(_world.layer_descriptions[layer_index]
                                           .name.c_str(),
                                        included_in_game_mode)) {
                     if (included_in_game_mode) {
                        const int game_mode_layers_index = [&] {
                           for (int i = 0; i < game_mode.layers.size(); ++i) {
                              if (game_mode.layers[i] == layer_index) {
                                 return i;
                              }
                           }

                           std::terminate();
                        }();

                        _edit_stack_world.apply(edits::make_game_mode_unlink_layer(
                                                   game_mode_index,
                                                   game_mode_layers_index, _world),
                                                _edit_context);
                     }
                     else {
                        _edit_stack_world.apply(edits::make_game_mode_link_layer(game_mode_index,
                                                                                 layer_index,
                                                                                 _world),
                                                _edit_context);
                     }
                  }
               }
            }

            if (not is_common_game_mode) {
               ImGui::SeparatorText("Delete Game Mode");

               if (ImGui::Button("Delete", {ImGui::CalcItemWidth(), 0.0f})) {
                  _edit_stack_world.apply(edits::make_delete_game_mode(game_mode_index,
                                                                       _world),
                                          _edit_context);
               }

               ImGui::SeparatorText("Rename Game Mode");

               if (absl::InlinedVector<char, 256> name{game_mode.name.begin(),
                                                       game_mode.name.end()};
                   ImGui::InputText("Name", &name, ImGuiInputTextFlags_CallbackCharFilter,
                                    imgui_game_mode_letter_filter)) {
                  if (not name.empty() and
                      is_unique_game_mode_name({name.data(), name.size()}, _world)) {
                     _edit_stack_world.apply(edits::make_rename_game_mode(
                                                game_mode_index,
                                                {name.data(), name.size()}, _world),
                                             _edit_context);
                  }
               }
            }

            ImGui::TreePop();
         }
      }
   }

   ImGui::End();
}

}