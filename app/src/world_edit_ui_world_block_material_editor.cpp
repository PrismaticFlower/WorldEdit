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
      if (ImGui::BeginChild("Materials", {160.0f * _display_scale, 0.0f},
                            ImGuiChildFlags_ResizeX)) {

         ImGui::BeginChild("##scrolling",
                           {0.0f, ImGui::GetContentRegionAvail().y -
                                     ImGui::GetTextLineHeightWithSpacing() -
                                     ImGui::GetStyle().ItemSpacing.y * 2.0f});

         const float thumbnail_size = 64.0f * _display_scale;
         const ImVec2 text_offset = {thumbnail_size +
                                        ImGui::GetStyle().ItemSpacing.x,
                                     floorf((thumbnail_size - ImGui::GetFontSize()) *
                                            0.5f)};
         const float button_width = ImGui::GetContentRegionAvail().x;
         const float button_height =
            thumbnail_size + ImGui::GetStyle().ItemInnerSpacing.y;

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
                  _block_material_editor_context.selected_index = material_index;
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

         ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

         ImGui::InputTextWithHint("##name", "filter e.g. tiles",
                                  &_block_material_editor_config.filter);
      }

      ImGui::EndChild();

      ImGui::SameLine();

      if (ImGui::BeginChild("##selected") and _block_material_editor_context.selected_index <
                                                 _world.blocks.materials.size()) {
         world::block_material& material =
            _world.blocks.materials[_block_material_editor_context.selected_index];

         ImGui::InputText("Name", &material.name, _edit_stack_world, _edit_context);

         if (absl::InlinedVector<char, 256> diffuse_map{material.diffuse_map.begin(),
                                                        material.diffuse_map.end()};
             ImGui::InputText("Diffuse Map", &diffuse_map)) {
            _edit_stack_world.apply(edits::make_set_block_material(
                                       &material.diffuse_map,
                                       std::string{diffuse_map.begin(),
                                                   diffuse_map.end()},
                                       _block_material_editor_context.selected_index,
                                       &_world.blocks.materials_dirty),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

         if (absl::InlinedVector<char, 256> normal_map{material.normal_map.begin(),
                                                       material.normal_map.end()};
             ImGui::InputText("Normal Map", &normal_map)) {
            _edit_stack_world.apply(edits::make_set_block_material(
                                       &material.normal_map,
                                       std::string{normal_map.begin(),
                                                   normal_map.end()},
                                       _block_material_editor_context.selected_index,
                                       &_world.blocks.materials_dirty),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

         if (absl::InlinedVector<char, 256> detail_map{material.detail_map.begin(),
                                                       material.detail_map.end()};
             ImGui::InputText("Detail Map", &detail_map)) {
            _edit_stack_world.apply(edits::make_set_block_material(
                                       &material.detail_map,
                                       std::string{detail_map.begin(),
                                                   detail_map.end()},
                                       _block_material_editor_context.selected_index,
                                       &_world.blocks.materials_dirty),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

         if (absl::InlinedVector<char, 256> env_map{material.env_map.begin(),
                                                    material.env_map.end()};
             ImGui::InputText("Env Map", &env_map)) {
            _edit_stack_world.apply(edits::make_set_block_material(
                                       &material.env_map,
                                       std::string{env_map.begin(), env_map.end()},
                                       _block_material_editor_context.selected_index,
                                       &_world.blocks.materials_dirty),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

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

         if (ImGui::BeginCombo("Foley FX Group",
                               foley_group_name(material.foley_group))) {
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
                  _edit_stack_world.apply(edits::make_set_value(&material.foley_group,
                                                                group),
                                          _edit_context, {.closed = true});
               }
            }

            ImGui::EndCombo();
         }

         ImGui::SetItemTooltip("Not all foley groups may be availible, "
                               "depending on the sound .lvl your map uses.");
      }

      ImGui::EndChild();
   }

   ImGui::End();
}

}