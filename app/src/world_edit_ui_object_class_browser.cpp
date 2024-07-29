#include "world_edit.hpp"

#include "edits/creation_entity_set.hpp"
#include "edits/set_class_name.hpp"
#include "utility/string_icompare.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

constexpr float thumbnail_base_size = 128.0f;

}

void world_edit::ui_show_object_class_browser() noexcept
{
   ImGui::SetNextWindowPos({0.0f, ImGui::GetIO().DisplaySize.y},
                           ImGuiCond_FirstUseEver, {0.0f, 1.0f});
   ImGui::SetNextWindowSize({1216.0f * _display_scale, 272.0f * _display_scale},
                            ImGuiCond_FirstUseEver);

   if (ImGui::Begin("Object Class Browser", &_object_class_browser_open)) {
      if (ImGui::InputTextWithHint("Filter", "e.g. com_bldg_controlzone",
                                   &_world_explorer_class_filter)) {
         ImGui::SetNextWindowScroll({0.0f, 0.0f});
      }

      const float button_size = thumbnail_base_size * _display_scale;
      const float item_size = button_size + ImGui::GetStyle().ItemSpacing.x;
      const float window_space =
         ImGui::GetWindowWidth() - ImGui::GetStyle().ScrollbarSize;
      const ImU32 text_color = ImGui::GetColorU32(ImGuiCol_Text);
      const float text_offset = 4.0f * _display_scale;

      ImGui::BeginChild("Classes", ImGui::GetContentRegionAvail());

      _asset_libraries.odfs.view_existing([&](const std::span<const assets::stable_string> assets) noexcept {
         for (std::string_view asset : assets) {
            if (not _world_explorer_class_filter.empty() and
                not string::icontains(asset, _world_explorer_class_filter)) {
               continue;
            }

            if (ImGui::IsRectVisible({button_size, button_size})) {
               const std::optional<graphics::object_class_thumbnail> thumbnail =
                  [&]() -> std::optional<graphics::object_class_thumbnail> {
                  try {
                     return _renderer->request_object_class_thumbnail(asset);
                  }
                  catch (graphics::gpu::exception& e) {
                     handle_gpu_error(e);

                     return std::nullopt;
                  }
               }();

               if (not thumbnail) continue;

               ImGui::PushID(asset.data(), asset.data() + asset.size());

               const ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

               if (ImGui::ImageButton("##add", thumbnail->imgui_texture_id,
                                      {button_size, button_size},
                                      {thumbnail->uv_left, thumbnail->uv_top},
                                      {thumbnail->uv_right, thumbnail->uv_bottom})) {
                  if (_interaction_targets.creation_entity.is<world::object>()) {
                     world::object& object =
                        _interaction_targets.creation_entity.get<world::object>();

                     _edit_stack_world.apply(edits::make_set_class_name(&object,
                                                                        lowercase_string{asset},
                                                                        _object_classes),
                                             _edit_context);
                  }
                  else {
                     _edit_stack_world
                        .apply(edits::make_creation_entity_set(
                                  world::object{.name = "",
                                                .layer = _last_created_entities.last_layer,
                                                .class_name = lowercase_string{asset},
                                                .id = world::max_id},
                                  _object_classes),
                               _edit_context);
                     _entity_creation_context = {};
                  }
               }

               const ImVec4 label_clip{cursor_pos.x, cursor_pos.y,
                                       cursor_pos.x + button_size,
                                       cursor_pos.y + button_size};

               ImGui::GetWindowDrawList()
                  ->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                            {cursor_pos.x + text_offset, cursor_pos.y},
                            text_color, asset.data(), asset.data() + asset.size(),
                            button_size - text_offset, &label_clip);

               if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                  ImGui::TextUnformatted(asset.data(), asset.data() + asset.size());

                  ImGui::EndTooltip();
               }

               if (ImGui::BeginPopupContextItem("Class Name")) {
                  if (ImGui::MenuItem("Open .odf in Text Editor")) {
                     open_odf_in_text_editor(lowercase_string{asset});
                  }

                  if (ImGui::MenuItem("Show .odf in Explorer")) {
                     show_odf_in_explorer({lowercase_string{asset}});
                  }

                  ImGui::EndPopup();
               }

               ImGui::PopID();
            }
            else {
               ImGui::Dummy({button_size, button_size});
            }

            ImGui::SameLine();

            if (ImGui::GetCursorPosX() + item_size > window_space) {
               ImGui::NewLine();
            }
         }
      });

      ImGui::EndChild();
   }

   ImGui::End();
}

bool world_edit::ui_object_class_pick_widget(world::object* object) noexcept
{
   assert(_edit_context.is_memory_valid(object));

   bool modified = false;

   if (ImGui::BeginCombo("Class Name", object->class_name.c_str(),
                         ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoArrowButton)) {
      if (ImGui::IsWindowAppearing()) {
         _object_class_pick_filter = object->class_name;

         ImGui::SetKeyboardFocusHere();
      }

      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

      if (ImGui::InputText("##name", &_object_class_pick_filter,
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
         _edit_stack_world.apply(edits::make_set_class_name(object,
                                                            lowercase_string{_object_class_pick_filter},
                                                            _object_classes),
                                 _edit_context);

         modified = true;

         ImGui::CloseCurrentPopup();
      }

      const float thumbnail_size = thumbnail_base_size * 0.5f * _display_scale;
      const float visible_classes = 8.0f;
      const ImVec2 text_offset = {thumbnail_size + ImGui::GetStyle().ItemSpacing.x,
                                  floorf((thumbnail_size - ImGui::GetFontSize()) * 0.5f)};
      const float button_width = ImGui::GetContentRegionAvail().x;

      ImGui::BeginChild("Classes",
                        {0.0f, (thumbnail_size + ImGui::GetStyle().ItemSpacing.y) *
                                  visible_classes});

      _asset_libraries.odfs.view_existing([&](const std::span<const assets::stable_string> assets) noexcept {
         for (const std::string_view asset : assets) {
            if (not _object_class_pick_filter.empty() and
                not string::icontains(asset, _object_class_pick_filter)) {
               continue;
            }

            if (ImGui::IsRectVisible({thumbnail_size, thumbnail_size})) {
               const std::optional<graphics::object_class_thumbnail> thumbnail =
                  [&]() -> std::optional<graphics::object_class_thumbnail> {
                  try {
                     return _renderer->request_object_class_thumbnail(asset);
                  }
                  catch (graphics::gpu::exception& e) {
                     handle_gpu_error(e);

                     return std::nullopt;
                  }
               }();

               if (not thumbnail) continue;

               ImGui::PushID(asset.data(), asset.data() + asset.size());

               const ImVec2 image_cursor_pos = ImGui::GetCursorScreenPos();

               if (ImGui::Selectable("##pick", string::iequals(asset, object->class_name),
                                     ImGuiSelectableFlags_None,
                                     {button_width, thumbnail_size})) {
                  _edit_stack_world.apply(edits::make_set_class_name(object,
                                                                     lowercase_string{asset},
                                                                     _object_classes),
                                          _edit_context);

                  modified = true;

                  ImGui::CloseCurrentPopup();
               }

               ImGui::GetWindowDrawList()
                  ->AddImage(thumbnail->imgui_texture_id, image_cursor_pos,
                             {image_cursor_pos.x + thumbnail_size,
                              image_cursor_pos.y + thumbnail_size},
                             {thumbnail->uv_left, thumbnail->uv_top},
                             {thumbnail->uv_right, thumbnail->uv_bottom});

               if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                  ImGui::TextUnformatted(asset.data(), asset.data() + asset.size());

                  ImGui::EndTooltip();
               }

               const ImVec2 text_cursor_pos = {image_cursor_pos.x + text_offset.x,
                                               image_cursor_pos.y + text_offset.y};

               ImGui::GetWindowDrawList()->AddText(text_cursor_pos,
                                                   ImGui::GetColorU32(ImGuiCol_Text),
                                                   asset.data(),
                                                   asset.data() + asset.size());

               ImGui::PopID();
            }
            else {
               ImGui::Dummy({button_width, thumbnail_size});
            }
         }
      });

      ImGui::EndChild();

      ImGui::EndCombo();
   };

   return modified;
}

}