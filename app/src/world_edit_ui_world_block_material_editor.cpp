#include "world_edit.hpp"

#include "edits/imgui_ext.hpp"
#include "edits/set_block.hpp"

#include "utility/string_icompare.hpp"

#include "imgui_ext.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

auto foley_group_name(world::block_foley_group group) noexcept -> const char*
{
   // clang-format off
   switch (group) {
   case world::block_foley_group::stone:   return "Stone";
   case world::block_foley_group::dirt:    return "Dirt";
   case world::block_foley_group::grass:   return "Grass";
   case world::block_foley_group::metal:   return "Metal";
   case world::block_foley_group::snow:    return "Snow";
   case world::block_foley_group::terrain: return "Terrain";
   case world::block_foley_group::water:   return "Water";
   case world::block_foley_group::wood:    return "Wood";
   }
   // clang-format on

   return "<unknown>";
}

}

void world_edit::ui_show_block_material_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({520.0f * _display_scale, 620.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("Block Materials", &_block_material_editor_open,
                    ImGuiWindowFlags_NoCollapse)) {
      const std::string& material_name =
         _world.blocks.materials[_block_material_editor_context.selected_index].name;

      if (ImGui::BeginCombo("Material",
                            material_name.empty() ? "<unnamed material>"
                                                  : material_name.c_str(),
                            ImGuiComboFlags_HeightLargest |
                               ImGuiComboFlags_NoArrowButton)) {
         ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

         ImGui::InputTextWithHint("##name", "filter e.g. tiles",
                                  &_block_material_editor_config.filter);

         const float thumbnail_size = 64.0f * _display_scale;
         const float visible_thumbnails = 12.0f;
         const ImVec2 text_offset = {thumbnail_size +
                                        ImGui::GetStyle().ItemSpacing.x,
                                     floorf((thumbnail_size - ImGui::GetFontSize()) *
                                            0.5f)};
         const float button_width = ImGui::GetContentRegionAvail().x;
         const float button_height =
            thumbnail_size + ImGui::GetStyle().ItemInnerSpacing.y;

         ImGui::BeginChild("##scrolling", {0.0f, (thumbnail_size +
                                                  ImGui::GetStyle().ItemSpacing.y) *
                                                    visible_thumbnails});

         for (uint32 material_index = 0;
              material_index < _world.blocks.materials.size(); ++material_index) {
            const world::block_material& material =
               _world.blocks.materials[material_index];

            if (not _block_material_editor_config.filter.empty() and
                not string::icontains(material.name,
                                      _block_material_editor_config.filter)) {
               continue;
            }

            if (ImGui::IsRectVisible({button_width, button_height})) {
               ImGui::PushID(material_index);

               const ImVec2 image_cursor_pos = ImGui::GetCursorScreenPos();

               if (ImGui::Selectable("##pick",
                                     _block_material_editor_context.selected_index == material_index,
                                     ImGuiSelectableFlags_None,
                                     {button_width, button_height})) {
                  _block_material_editor_context.selected_index =
                     static_cast<uint8>(material_index);

                  ImGui::CloseCurrentPopup();
               }

               const ImTextureID texture =
                  _renderer->request_imgui_texture_id(material.diffuse_map,
                                                      graphics::fallback_imgui_texture::missing_diffuse);

               ImGui::GetWindowDrawList()->AddImage(
                  texture,
                  {image_cursor_pos.x + ImGui::GetStyle().ItemInnerSpacing.x,
                   image_cursor_pos.y + ImGui::GetStyle().ItemInnerSpacing.y},
                  {image_cursor_pos.x + thumbnail_size,
                   image_cursor_pos.y + thumbnail_size});

               const ImVec2 next_line_cursor = ImGui::GetCursorPos();

               if (not material.name.empty()) {
                  const ImVec2 text_cursor_pos = {image_cursor_pos.x +
                                                     text_offset.x,
                                                  image_cursor_pos.y +
                                                     text_offset.y};

                  ImGui::GetWindowDrawList()->AddText(text_cursor_pos,
                                                      ImGui::GetColorU32(ImGuiCol_Text),
                                                      material.name.data(),
                                                      material.name.data() +
                                                         material.name.size());

                  ImGui::SetItemTooltip(material.name.c_str());
               }

               ImGui::PopID();
            }
            else {
               ImGui::Dummy({button_width, thumbnail_size});
            }
         }

         ImGui::EndChild();

         ImGui::EndCombo();
      }

      if (_block_material_editor_context.selected_index >=
          _world.blocks.materials.size()) {
         _block_material_editor_context.selected_index = 0;
      }

      ImGui::BeginChild("Material", {}, ImGuiChildFlags_Border);

      world::block_material& material =
         _world.blocks.materials[_block_material_editor_context.selected_index];

      ImGui::InputText("Name", &material.name, _edit_stack_world, _edit_context);

      ui_block_texture_pick_widget("Diffuse Map", &material.diffuse_map,
                                   _block_material_editor_context.selected_index);
      ui_block_texture_pick_widget("Normal Map", &material.normal_map,
                                   _block_material_editor_context.selected_index);
      ui_block_texture_pick_widget("Detail Map", &material.detail_map,
                                   _block_material_editor_context.selected_index);
      ui_block_texture_pick_widget("Env Map", &material.env_map,
                                   _block_material_editor_context.selected_index);

      if (std::array<uint8, 2> detail_tiling = material.detail_tiling;
          ImGui::DragScalarN("Detail Tiling", ImGuiDataType_U8,
                             detail_tiling.data(), 2)) {
         _edit_stack_world.apply(edits::make_set_block_material(
                                    &material.detail_tiling, detail_tiling,
                                    _block_material_editor_context.selected_index,
                                    &_world.blocks.materials_dirty),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

      if (bool tile_normal_map = material.tile_normal_map;
          ImGui::Checkbox("Tile Normal Map", &tile_normal_map)) {
         _edit_stack_world.apply(edits::make_set_block_material(
                                    &material.tile_normal_map, tile_normal_map,
                                    _block_material_editor_context.selected_index,
                                    &_world.blocks.materials_dirty),
                                 _edit_context, {.closed = true});
      }

      if (bool specular_lighting = material.specular_lighting;
          ImGui::Checkbox("Specular Lighting", &specular_lighting)) {
         _edit_stack_world.apply(edits::make_set_block_material(
                                    &material.specular_lighting, specular_lighting,
                                    _block_material_editor_context.selected_index,
                                    &_world.blocks.materials_dirty),
                                 _edit_context, {.closed = true});
      }

      if (float3 specular_color = material.specular_color;
          ImGui::ColorEdit3("Specular Lighting", &specular_color.x)) {
         _edit_stack_world.apply(edits::make_set_block_material(
                                    &material.specular_color, specular_color,
                                    _block_material_editor_context.selected_index,
                                    &_world.blocks.materials_dirty),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

      if (ImGui::BeginCombo("Foley FX Group", foley_group_name(material.foley_group))) {
         for (world::block_foley_group group : {
                 world::block_foley_group::stone,
                 world::block_foley_group::dirt,
                 world::block_foley_group::grass,
                 world::block_foley_group::metal,
                 world::block_foley_group::snow,
                 world::block_foley_group::terrain,
                 world::block_foley_group::water,
                 world::block_foley_group::wood,
              }) {
            if (ImGui::Selectable(foley_group_name(group),
                                  material.foley_group == group)) {
               _edit_stack_world.apply(edits::make_set_value(&material.foley_group, group),
                                       _edit_context, {.closed = true});
            }
         }

         ImGui::EndCombo();
      }

      ImGui::SetItemTooltip("Not all foley groups may be availible, "
                            "depending on the sound .lvl your map uses.");

      ImGui::EndChild();
   }

   ImGui::End();
}

bool world_edit::ui_block_texture_pick_widget(const char* label, std::string* texture,
                                              uint32 material_index) noexcept
{
   assert(_edit_context.is_memory_valid(texture));
   assert(material_index < world::max_block_materials);

   bool modified = false;

   const float tiny_preview_size = ImGui::GetFrameHeight();
   const float combo_width =
      ImGui::CalcItemWidth() - ImGui::GetStyle().ItemInnerSpacing.x - tiny_preview_size;

   ImGui::PushID(texture->c_str());

   ImGui::SetNextItemWidth(combo_width);

   if (ImGui::BeginCombo("##combo", texture->c_str(),
                         ImGuiComboFlags_HeightLargest | ImGuiComboFlags_NoArrowButton)) {
      if (ImGui::IsWindowAppearing()) {
         _block_texture_pick_filter = *texture;
         _block_texture_pick_keyboard_hover = -1;

         ImGui::SetKeyboardFocusHere();
      }

      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

      if (ImGui::InputText("##name", &_block_texture_pick_filter,
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
         _edit_stack_world.apply(edits::make_set_block_material(texture, _block_texture_pick_filter,
                                                                material_index,
                                                                &_world.blocks.materials_dirty),
                                 _edit_context, {.closed = true});

         modified = true;

         ImGui::CloseCurrentPopup();
      }

      if (ImGui::IsItemFocused()) _block_texture_pick_keyboard_hover = -1;

      bool new_keyboard_input = false;

      if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
         _block_texture_pick_keyboard_hover -= 1;
         new_keyboard_input = true;
      }

      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
         _block_texture_pick_keyboard_hover += 1;
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
            if (not _block_texture_pick_filter.empty() and
                not string::icontains(asset, _block_texture_pick_filter)) {
               continue;
            }

            const bool is_hovered_from_keyboard =
               _block_texture_pick_keyboard_hover == asset_index;

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
                                        string::iequals(asset, *texture),
                                     is_hovered_from_keyboard
                                        ? ImGuiSelectableFlags_Highlight
                                        : ImGuiSelectableFlags_None,
                                     button_size) or
                   pick_with_keyboard) {
                  _edit_stack_world.apply(edits::make_set_block_material(
                                             texture, std::string{asset}, material_index,
                                             &_world.blocks.materials_dirty),
                                          _edit_context, {.closed = true});

                  modified = true;

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

         _block_texture_pick_keyboard_hover =
            std::clamp(_block_texture_pick_keyboard_hover, -1, asset_index);
      });

      ImGui::EndChild();

      ImGui::EndCombo();
   };

   ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

   ImGui::Image(_renderer->request_imgui_texture_id(*texture,
                                                    graphics::fallback_imgui_texture::missing_diffuse),
                {tiny_preview_size, tiny_preview_size}, {0.0f, 0.0f}, {1.0f, 1.0f});

   ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

   ImGui::TextUnformatted(label);

   ImGui::PopID();

   return modified;
}
}