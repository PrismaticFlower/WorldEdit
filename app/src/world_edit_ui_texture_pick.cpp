#include "world_edit.hpp"

#include "edits/set_value.hpp"

#include <algorithm>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

auto world_edit::ui_texture_pick_widget_untracked(const char* label,
                                                  const char* c_str_texture,
                                                  const char* preview_override) noexcept
   -> std::optional<std::string>
{
   const float tiny_preview_size = ImGui::GetFrameHeight();
   const float combo_width =
      ImGui::CalcItemWidth() - ImGui::GetStyle().ItemInnerSpacing.x - tiny_preview_size;
   const std::string_view texture = c_str_texture;

   std::optional<std::string> picked_texture;

   ImGui::PushID(label);

   ImGui::SetNextItemWidth(combo_width);

   if (ImGui::BeginCombo("##combo", preview_override ? preview_override : c_str_texture,
                         ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoArrowButton)) {
      if (ImGui::IsWindowAppearing()) {
         _texture_pick_filter = texture;
         _texture_pick_keyboard_hover = -1;

         ImGui::SetKeyboardFocusHere();
      }

      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

      if (ImGui::InputText("##name", &_texture_pick_filter,
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
         picked_texture = _texture_pick_filter;

         ImGui::CloseCurrentPopup();
      }

      if (ImGui::IsItemFocused()) _texture_pick_keyboard_hover = -1;

      bool new_keyboard_input = false;

      if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
         _texture_pick_keyboard_hover -= 1;
         new_keyboard_input = true;
      }

      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
         _texture_pick_keyboard_hover += 1;
         new_keyboard_input = true;
      }

      const float thumbnail_size =
         graphics::renderer::thumbnail_base_length * _display_scale;
      const ImVec2 text_offset = {thumbnail_size + ImGui::GetStyle().ItemSpacing.x,
                                  floorf((thumbnail_size - ImGui::GetFontSize()) * 0.5f)};
      const float text_wrap_length =
         ImGui::GetContentRegionAvail().x - text_offset.x -
         ImGui::GetStyle().FramePadding.x * 2.0f - ImGui::GetStyle().ScrollbarSize;
      const ImVec2 button_size = {ImGui::GetContentRegionAvail().x,
                                  thumbnail_size +
                                     ImGui::GetStyle().FramePadding.x};

      ImGui::BeginChild("##scrolling",
                        {0.0f, ImGui::GetMainViewport()->WorkSize.y / 2.0f});

      _asset_libraries.textures.view_existing([&](const std::span<const assets::stable_string> assets) noexcept {
         int32 asset_index = 0;

         for (const std::string_view asset : assets) {
            if (not _texture_pick_filter.empty() and
                not string::icontains(asset, _texture_pick_filter)) {
               continue;
            }

            const bool is_hovered_from_keyboard =
               _texture_pick_keyboard_hover == asset_index;

            if (new_keyboard_input and is_hovered_from_keyboard) {
               ImGui::SetScrollHereY();
               ImGui::SetKeyboardFocusHere();
            }

            if (ImGui::IsRectVisible({thumbnail_size, thumbnail_size})) {
               ImGui::PushID(asset.data(), asset.data() + asset.size());

               const ImVec2 image_cursor_pos = {
                  ImGui::GetCursorScreenPos().x + ImGui::GetStyle().FramePadding.x,
                  ImGui::GetCursorScreenPos().y + ImGui::GetStyle().FramePadding.y};

               const bool pick_with_keyboard =
                  is_hovered_from_keyboard and ImGui::IsKeyPressed(ImGuiKey_Enter);

               if (ImGui::Selectable("##pick",
                                     is_hovered_from_keyboard or
                                        string::iequals(asset, texture),
                                     is_hovered_from_keyboard
                                        ? ImGuiSelectableFlags_Highlight
                                        : ImGuiSelectableFlags_None,
                                     button_size) or
                   pick_with_keyboard) {
                  picked_texture.emplace(asset);

                  ImGui::CloseCurrentPopup();
               }

               const std::optional<graphics::object_class_thumbnail> thumbnail =
                  [&]() -> std::optional<graphics::object_class_thumbnail> {
                  try {
                     return _renderer->request_texture_thumbnail(asset);
                  }
                  catch (graphics::gpu::exception& e) {
                     handle_gpu_error(e);

                     return std::nullopt;
                  }
               }();

               if (thumbnail) {
                  ImGui::GetWindowDrawList()
                     ->AddImage(thumbnail->imgui_texture_id, image_cursor_pos,
                                {image_cursor_pos.x + thumbnail_size,
                                 image_cursor_pos.y + thumbnail_size},
                                {thumbnail->uv_left, thumbnail->uv_top},
                                {thumbnail->uv_right, thumbnail->uv_bottom});
               }

               if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                  ImGui::TextUnformatted(asset.data(), asset.data() + asset.size());

                  ImGui::EndTooltip();
               }

               const ImVec2 text_cursor_pos = {image_cursor_pos.x + text_offset.x,
                                               image_cursor_pos.y + text_offset.y};

               ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(),
                                                   ImGui::GetFontSize(), text_cursor_pos,
                                                   ImGui::GetColorU32(ImGuiCol_Text),
                                                   asset.data(),
                                                   asset.data() + asset.size(),
                                                   text_wrap_length);

               ImGui::PopID();
            }
            else {
               ImGui::Dummy(button_size);
            }

            asset_index += 1;
         }

         _texture_pick_keyboard_hover =
            std::clamp(_texture_pick_keyboard_hover, -1, asset_index);
      });

      ImGui::EndChild();

      ImGui::EndCombo();
   };

   ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

   ImGui::Image(_renderer->request_imgui_texture_id(texture,
                                                    graphics::fallback_imgui_texture::missing_diffuse),
                {tiny_preview_size, tiny_preview_size}, {0.0f, 0.0f}, {1.0f, 1.0f});

   ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

   ImGui::TextUnformatted(label);

   ImGui::PopID();

   return picked_texture;
}

bool world_edit::ui_texture_pick_widget(const char* label, std::string* texture) noexcept
{
   assert(_edit_context.is_memory_valid(texture));

   std::optional<std::string> picked_texture =
      ui_texture_pick_widget_untracked(label, texture->c_str());

   if (picked_texture) {
      _edit_stack_world.apply(edits::make_set_value(texture, std::move(*picked_texture)),
                              _edit_context, {.closed = true});

      return true;
   }

   return false;
}
}
