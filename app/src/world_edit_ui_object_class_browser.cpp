#include "world_edit.hpp"

#include "edits/creation_entity_set.hpp"
#include "edits/set_class_name.hpp"
#include "utility/string_icompare.hpp"

#include <algorithm>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

void show_tree_branch(const assets::library_tree_branch& branch,
                      std::vector<uint32>& traversal_stack,
                      std::vector<uint32>& selected_stack,
                      std::string& selected_name, bool& found_root) noexcept
{
   if (not found_root and branch.directories.size() == 1) {
      traversal_stack.push_back(0);

      show_tree_branch(branch.directories[0], traversal_stack, selected_stack,
                       selected_name, found_root);

      traversal_stack.pop_back();

      return;
   }

   const bool is_selected = traversal_stack == selected_stack and
                            string::iequals(selected_name, branch.name);

   ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;

   if (branch.directories.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
   if (is_selected) flags |= ImGuiTreeNodeFlags_Selected;

   found_root |= not branch.directories.empty();

   if (ImGui::TreeNodeEx(branch.name.c_str(), flags)) {
      if (ImGui::IsItemClicked() or ImGui::IsItemToggledOpen()) {
         if (is_selected and not ImGui::IsItemToggledOpen()) {
            selected_stack.clear();
            selected_name.clear();
         }
         else {
            selected_stack = traversal_stack;
            selected_name = branch.name;
         }
      }

      for (uint32 i = 0; i < branch.directories.size(); ++i) {
         traversal_stack.push_back(i);

         show_tree_branch(branch.directories[i], traversal_stack,
                          selected_stack, selected_name, found_root);

         traversal_stack.pop_back();
      }

      ImGui::TreePop();
   }
}

auto walk_to_selected_branch(const assets::library_tree& tree,
                             const std::vector<uint32>& selected_stack,
                             const std::string_view selected_name) noexcept
   -> const assets::library_tree_branch*
{
   if (selected_stack.empty()) return nullptr;
   if (selected_stack.front() >= tree.directories.size()) return nullptr;

   const assets::library_tree_branch* branch =
      &tree.directories[selected_stack.front()];

   for (uint32 iter_index = 1; iter_index < selected_stack.size(); ++iter_index) {
      const uint32 selected_index = selected_stack[iter_index];

      if (selected_index >= branch->directories.size()) return nullptr;

      branch = &branch->directories[selected_index];
   }

   return string::iequals(branch->name, selected_name) ? branch : nullptr;
}

}

void world_edit::ui_show_object_class_browser() noexcept
{
   ImGui::SetNextWindowPos({0.0f, ImGui::GetIO().DisplaySize.y},
                           ImGuiCond_FirstUseEver, {0.0f, 1.0f});
   ImGui::SetNextWindowSize({1216.0f * _display_scale, 272.0f * _display_scale},
                            ImGuiCond_FirstUseEver);

   if (ImGui::Begin("Object Class Browser", &_object_class_browser_open)) {
      class_browser_context& context = _class_browser_context;

      if (ImGui::BeginChild("##directories", {176.0f * _display_scale, 0.0f},
                            ImGuiChildFlags_ResizeX)) {
         ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize());
         ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign,
                             {ImGui::GetStyle().FramePadding.x /
                                 ImGui::GetContentRegionAvail().x,
                              0.0f});

         if (ImGui::Selectable("<All>", context.selected_stack.empty())) {
            context.selected_stack.clear();
            context.selected_name.clear();
         }

         ImGui::PopStyleVar();

         _asset_libraries.odfs.view_tree([&](const assets::library_tree& tree) noexcept {
            context.traversal_stack.clear();

            for (uint32 i = 0; i < std::size(tree.directories); ++i) {
               bool found_root = false;

               context.traversal_stack.push_back(i);

               show_tree_branch(tree.directories[i], context.traversal_stack,
                                context.selected_stack, context.selected_name,
                                found_root);

               context.traversal_stack.pop_back();
            }
         });

         ImGui::PopStyleVar();
      }

      ImGui::EndChild();

      ImGui::SameLine();

      ImGui::BeginChild("##classes");

      if (ImGui::InputTextWithHint("Filter", "e.g. com_bldg_controlzone",
                                   &context.filter)) {
         ImGui::SetNextWindowScroll({0.0f, 0.0f});
      }

      ImGui::BeginChild("Classes", ImGui::GetContentRegionAvail());

      const float button_size =
         graphics::renderer::thumbnail_base_length * _display_scale;
      const float item_size = button_size + ImGui::GetStyle().ItemSpacing.x;
      const float window_space =
         ImGui::GetWindowWidth() - ImGui::GetStyle().ScrollbarSize;
      const ImU32 text_color = ImGui::GetColorU32(ImGuiCol_Text);
      const float text_offset = 4.0f * _display_scale;

      if (not context.selected_stack.empty()) {
         _asset_libraries.odfs.view_tree([&](const assets::library_tree& tree) noexcept {
            const assets::library_tree_branch* selected_branch =
               walk_to_selected_branch(tree, context.selected_stack,
                                       context.selected_name);

            if (not selected_branch) {
               context.selected_stack.clear();
               context.selected_name.clear();

               return;
            }

            context.branch_stack.push_back(selected_branch);

            while (not context.branch_stack.empty()) {
               const assets::library_tree_branch* branch =
                  context.branch_stack.back();

               context.branch_stack.pop_back();

               for (const lowercase_string& asset : branch->assets) {
                  if (not context.filter.empty() and
                      not string::icontains(asset, context.filter)) {
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

                           _edit_stack_world
                              .apply(edits::make_set_class_name(&object,
                                                                lowercase_string{asset},
                                                                _object_classes),
                                     _edit_context);
                        }
                        else {
                           _edit_stack_world.apply(
                              edits::make_creation_entity_set(
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
                                  {cursor_pos.x + text_offset, cursor_pos.y}, text_color,
                                  asset.data(), asset.data() + asset.size(),
                                  button_size - text_offset, &label_clip);

                     if (ImGui::IsItemHovered() and ImGui::BeginTooltip()) {
                        ImGui::TextUnformatted(asset.data(),
                                               asset.data() + asset.size());

                        ImGui::EndTooltip();
                     }

                     if (ImGui::BeginPopupContextItem("Class Name")) {
                        if (ImGui::MenuItem("Open .odf in Text Editor")) {
                           open_odf_in_text_editor(asset);
                        }

                        if (ImGui::MenuItem("Show .odf in Explorer")) {
                           show_odf_in_explorer(asset);
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

               for (std::ptrdiff_t i = std::ssize(branch->directories) - 1;
                    i >= 0; --i) {
                  context.branch_stack.push_back(&branch->directories[i]);
               }
            }
         });
      }
      else {
         _asset_libraries.odfs.view_existing([&](const std::span<const assets::stable_string> assets) noexcept {
            for (std::string_view asset : assets) {
               if (not context.filter.empty() and
                   not string::icontains(asset, context.filter)) {
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

                        _edit_stack_world
                           .apply(edits::make_set_class_name(&object,
                                                             lowercase_string{asset},
                                                             _object_classes),
                                  _edit_context);
                     }
                     else {
                        _edit_stack_world.apply(
                           edits::make_creation_entity_set(
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
                               {cursor_pos.x + text_offset, cursor_pos.y}, text_color,
                               asset.data(), asset.data() + asset.size(),
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
      }

      ImGui::EndChild();

      ImGui::EndChild();
   }

   ImGui::End();
}

auto world_edit::ui_object_class_pick_widget_untracked(const lowercase_string& class_name,
                                                       const char* preview_override) noexcept
   -> std::optional<lowercase_string>
{
   std::optional<lowercase_string> picked;

   if (ImGui::BeginCombo("Class Name",
                         preview_override ? preview_override : class_name.c_str(),
                         ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoArrowButton)) {
      if (ImGui::IsWindowAppearing()) {
         _object_class_pick_filter = class_name;
         _object_class_pick_keyboard_hover = -1;

         ImGui::SetKeyboardFocusHere();
      }

      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

      if (ImGui::InputText("##name", &_object_class_pick_filter,
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
         picked.emplace(_object_class_pick_filter);

         ImGui::CloseCurrentPopup();
      }

      if (ImGui::IsItemFocused()) _object_class_pick_keyboard_hover = -1;

      bool new_keyboard_input = false;

      if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
         _object_class_pick_keyboard_hover -= 1;
         new_keyboard_input = true;
      }

      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
         _object_class_pick_keyboard_hover += 1;
         new_keyboard_input = true;
      }

      const float thumbnail_size =
         graphics::renderer::thumbnail_base_length * 0.5f * _display_scale;
      const float visible_classes = 8.0f;
      const ImVec2 text_offset = {thumbnail_size + ImGui::GetStyle().ItemSpacing.x,
                                  floorf((thumbnail_size - ImGui::GetFontSize()) * 0.5f)};
      const float button_width = ImGui::GetContentRegionAvail().x;

      ImGui::BeginChild("Classes",
                        {0.0f, (thumbnail_size + ImGui::GetStyle().ItemSpacing.y) *
                                  visible_classes});

      _asset_libraries.odfs.view_existing([&](const std::span<const assets::stable_string> assets) noexcept {
         int32 asset_index = 0;

         for (const std::string_view asset : assets) {
            if (not _object_class_pick_filter.empty() and
                not string::icontains(asset, _object_class_pick_filter)) {
               continue;
            }

            const bool is_hovered_from_keyboard =
               _object_class_pick_keyboard_hover == asset_index;

            if (new_keyboard_input and is_hovered_from_keyboard) {
               ImGui::SetScrollHereY();
               ImGui::SetKeyboardFocusHere();
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

               const bool pick_with_keyboard =
                  is_hovered_from_keyboard and ImGui::IsKeyPressed(ImGuiKey_Enter);

               if (ImGui::Selectable("##pick",
                                     is_hovered_from_keyboard or
                                        string::iequals(asset, class_name),
                                     is_hovered_from_keyboard
                                        ? ImGuiSelectableFlags_Highlight
                                        : ImGuiSelectableFlags_None,
                                     {button_width, thumbnail_size}) or
                   pick_with_keyboard) {
                  picked.emplace(asset);

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

            asset_index += 1;
         }

         _object_class_pick_keyboard_hover =
            std::clamp(_object_class_pick_keyboard_hover, -1, asset_index);
      });

      ImGui::EndChild();

      ImGui::EndCombo();
   };

   return picked;
}

bool world_edit::ui_object_class_pick_widget(world::object* object) noexcept
{
   assert(_edit_context.is_memory_valid(object));

   std::optional<lowercase_string> new_class_name =
      ui_object_class_pick_widget_untracked(object->class_name);

   if (new_class_name) {
      _edit_stack_world.apply(edits::make_set_class_name(object, std::move(*new_class_name),
                                                         _object_classes),
                              _edit_context);

      return true;
   }

   return false;
}

}