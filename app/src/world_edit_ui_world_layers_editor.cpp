#include "world_edit.hpp"

#include "edits/add_layer.hpp"
#include "edits/delete_layer.hpp"
#include "edits/rename_layer.hpp"
#include "imgui_ext.hpp"
#include "utility/string_icompare.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

bool is_unique_layer_name(std::string_view name, const world::world& world) noexcept
{
   for (const auto& desc : world.layer_descriptions) {
      if (string::iequals(name, desc.name)) return false;
   }

   return true;
};

int imgui_layer_letter_filter(ImGuiInputTextCallbackData* data) noexcept
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

}

void world_edit::ui_show_world_layers_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({400.0f * _display_scale, 256.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({400.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("Layers Editor", &_world_layers_editor_open)) {
      ImGui::SeparatorText("Create New Layer");

      ImGui::InputTextWithHint("##create", "New Layer Name", &_layer_editor_new_name,
                               ImGuiInputTextFlags_CallbackCharFilter,
                               imgui_layer_letter_filter);
      ImGui::SameLine();

      const bool new_layer_name_is_unique =
         is_unique_layer_name(_layer_editor_new_name, _world);
      const bool has_new_layer_name = not _layer_editor_new_name.empty();
      const bool space_for_layer = _world.layer_descriptions.size() != world::max_layers;
      const bool can_create_layer =
         new_layer_name_is_unique and has_new_layer_name and space_for_layer;

      if (not can_create_layer) ImGui::BeginDisabled();

      if (ImGui::Button("Create")) {
         _edit_stack_world.apply(edits::make_add_layer(std::move(_layer_editor_new_name),
                                                       _world),
                                 _edit_context);

         _layer_editor_new_name = "";
      }

      if (not can_create_layer) ImGui::EndDisabled();

      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
         if (not new_layer_name_is_unique) {
            ImGui::SetTooltip("Layer name must be unique.");
         }
         else if (not has_new_layer_name) {
            ImGui::SetTooltip("Layer must have a name.");
         }
         else if (not space_for_layer) {
            ImGui::SetTooltip("WorldEdit only supports up to 64 layers.");
         }
      }

      ImGui::SeparatorText("Existing Layers");

      for (int i = 0; i < _world.layer_descriptions.size(); ++i) {
         ImGui::PushID(i);

         const bool base_layer = i == 0;

         if (base_layer) ImGui::BeginDisabled();

         if (absl::InlinedVector<char, 256>
                name{_world.layer_descriptions[i].name.begin(),
                     _world.layer_descriptions[i].name.end()};
             ImGui::InputText("##layer", &name, ImGuiInputTextFlags_CallbackCharFilter,
                              imgui_layer_letter_filter)) {
            if (not name.empty() and
                is_unique_layer_name({name.data(), name.size()}, _world)) {
               _edit_stack_world
                  .apply(edits::make_rename_layer(i, {name.data(), name.size()}, _world),
                         _edit_context);
            }
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (base_layer) ImGui::EndDisabled();

         if (base_layer and ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("The base layer can not be renamed.");
         }

         ImGui::SameLine();

         if (base_layer) ImGui::BeginDisabled();

         if (ImGui::Button("Delete")) {
            _edit_stack_world.apply(edits::make_delete_layer(i, _world), _edit_context);
         }

         if (base_layer) ImGui::EndDisabled();

         if (base_layer and ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("The base layer can not be deleted.");
         }

         ImGui::PopID();
      }
   }

   ImGui::End();
}

}