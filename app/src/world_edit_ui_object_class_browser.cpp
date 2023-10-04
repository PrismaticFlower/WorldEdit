#include "world_edit.hpp"

#include "edits/creation_entity_set.hpp"
#include "edits/set_value.hpp"
#include "utility/string_icompare.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

void world_edit::ui_show_object_class_browser() noexcept
{
   if (ImGui::Begin("Object Class Browser", &_object_class_browser_open)) {
      if (ImGui::InputTextWithHint("Filter", "e.g. com_bldg_controlzone",
                                   &_world_explorer_class_filter)) {
         ImGui::SetNextWindowScroll({0.0f, 0.0f});
      }

      const float button_size = 128.0f * _display_scale;
      const float item_size = button_size + ImGui::GetStyle().ItemSpacing.x;
      const float window_space =
         ImGui::GetWindowWidth() - ImGui::GetStyle().ScrollbarSize;
      const ImU32 text_color = ImGui::GetColorU32(ImGuiCol_Text);
      const float text_offset = 4.0f * _display_scale;

      ImGui::BeginChild("Classes", ImGui::GetContentRegionAvail());

      _asset_libraries.odfs.view_existing([&](const std::span<const assets::stable_string> assets) {
         _world_explorer_object_classes.reserve(assets.size());

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
                  if (_interaction_targets.creation_entity and
                      std::holds_alternative<world::object>(
                         *_interaction_targets.creation_entity)) {
                     const world::object& object =
                        std::get<world::object>(*_interaction_targets.creation_entity);

                     _edit_stack_world
                        .apply(edits::make_set_creation_value(&world::object::class_name,
                                                              lowercase_string{asset},
                                                              object.class_name),
                               _edit_context);
                  }
                  else {
                     _edit_stack_world
                        .apply(edits::make_creation_entity_set(
                                  world::object{.name = "",
                                                .layer = _last_created_entities.last_layer,
                                                .class_name = lowercase_string{asset},
                                                .id = world::max_id},
                                  _interaction_targets.creation_entity),
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

}