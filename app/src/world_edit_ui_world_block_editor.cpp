#include "world_edit.hpp"

#include "edits/add_block.hpp"
#include "edits/imgui_ext.hpp"
#include "edits/set_block.hpp"

#include "math/curves.hpp"
#include "math/intersectors.hpp"
#include "math/iq_intersectors.hpp"
#include "math/plane_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"

#include "world/blocks/utility/accessors.hpp"
#include "world/blocks/utility/bounding_box.hpp"
#include "world/blocks/utility/find.hpp"
#include "world/blocks/utility/highlight_block.hpp"
#include "world/blocks/utility/highlight_surface.hpp"
#include "world/blocks/utility/raycast.hpp"
#include "world/blocks/utility/snapping.hpp"

#include <algorithm>
#include <numbers>

#include "imgui_ext.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we {

namespace {

constexpr std::array<const char*, 13> alignment_names =
   {"0.0625", "0.125", "0.25", "0.5",  "1.0",   "2.0",  "4.0",
    "8.0",    "16.0",  "32.0", "64.0", "128.0", "256.0"};

constexpr uint32 cursor_plane_axis_color_x = 0x7f'e5'40'40;
constexpr uint32 cursor_plane_axis_color_y = 0x7f'40'60'e5;
constexpr uint32 cursor_plane_axis_color_z = 0x7f'30'e5'30;

auto texture_mode_name(world::block_texture_mode mode) noexcept -> const char*
{
   // clang-format off
   switch (mode) {
   case world::block_texture_mode::world_space_auto:  return "World Space Auto";
   case world::block_texture_mode::world_space_zy:    return "World Space ZY";
   case world::block_texture_mode::world_space_xz:    return "World Space XZ";
   case world::block_texture_mode::world_space_xy:    return "World Space XY";
   case world::block_texture_mode::local_space_auto:  return "Local Space Auto";
   case world::block_texture_mode::local_space_zy:    return "Local Space ZY";
   case world::block_texture_mode::local_space_xz:    return "Local Space XZ";
   case world::block_texture_mode::local_space_xy:    return "Local Space XY";
   case world::block_texture_mode::unwrapped:         return "Unwrapped";
   }
   // clang-format on

   return "<unknown>";
}

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

auto block_type_name(draw_block_type type) noexcept -> const char*
{

   // clang-format off
   switch (type) {
   case draw_block_type::box:               return "Box";
   case draw_block_type::ramp:              return "Ramp";
   case draw_block_type::quad:              return "Quadrilateral";
   case draw_block_type::cylinder:          return "Cylinder";
   case draw_block_type::stairway:          return "Stairs";
   case draw_block_type::stairway_floating: return "Stairs (Floating)";
   case draw_block_type::cone:              return "Cone";
   case draw_block_type::hemisphere:        return "Hemisphere";
   case draw_block_type::pyramid:           return "Pyramid";
   case draw_block_type::ring:              return "Ring";
   case draw_block_type::beveled_box:       return "Beveled Box";
   case draw_block_type::curve:             return "Curve";
   case draw_block_type::arch:              return "Arch";
   case draw_block_type::terrain_cut_box:   return "Terrain Cutter (Box)";
   }
   // clang-format on

   std::unreachable();
}

}

void world_edit::ui_show_block_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({520.0f * _display_scale, 620.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("Blocks", &_block_editor_open, ImGuiWindowFlags_NoCollapse)) {
      ImGui::SeparatorText("Block Tools");

      if (ImGui::Button("Draw Block", {ImGui::CalcItemWidth(), 0.0f})) {
         _block_editor_context.activate_tool = block_edit_tool::draw;
      }

      if (ImGui::BeginCombo("Draw Shape", block_type_name(_block_editor_config.draw_type),
                            ImGuiComboFlags_HeightLargest)) {
         for (const draw_block_type type : {
                 draw_block_type::box,
                 draw_block_type::ramp,
                 draw_block_type::quad,
                 draw_block_type::cylinder,
                 draw_block_type::stairway,
                 draw_block_type::stairway_floating,
                 draw_block_type::cone,
                 draw_block_type::hemisphere,
                 draw_block_type::pyramid,
                 draw_block_type::ring,
                 draw_block_type::beveled_box,
                 draw_block_type::curve,
                 draw_block_type::arch,
                 draw_block_type::terrain_cut_box,
              }) {

            if (ImGui::Selectable(block_type_name(type),
                                  type == _block_editor_config.draw_type)) {
               _block_editor_config.draw_type = type;
            }
         }

         ImGui::EndCombo();
      }

      ImGui::Checkbox("Enable Alignment", &_block_editor_config.enable_alignment);
      ImGui::SameLine();
      ImGui::Checkbox("Enable Snapping", &_block_editor_config.enable_snapping);

      ImGui::BeginDisabled(not _block_editor_config.enable_alignment);

      _block_editor_config.xz_alignment_exponent =
         std::clamp(_block_editor_config.xz_alignment_exponent, -4, 8);
      _block_editor_config.y_alignment_exponent =
         std::clamp(_block_editor_config.y_alignment_exponent, -4, 8);

      ImGui::SliderInt("Draw XZ Alignment",
                       &_block_editor_config.xz_alignment_exponent, -4, 8,
                       alignment_names[_block_editor_config.xz_alignment_exponent + 4],
                       ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);

      ImGui::SetItemTooltip("Alignment Along the XZ-Axes when drawing.");

      ImGui::SliderInt("Draw Y Alignment",
                       &_block_editor_config.y_alignment_exponent, -4, 8,
                       alignment_names[_block_editor_config.y_alignment_exponent + 4],
                       ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);

      ImGui::SetItemTooltip("Alignment Along the Y-Axis when drawing.");

      ImGui::EndDisabled();

      ImGui::BeginDisabled(not _block_editor_config.enable_snapping);

      const ImVec2 cursor_pos = ImGui::GetCursorPos();

      ImGui::LabelText("Snapping Mode", "");

      ImGui::SetCursorPos(cursor_pos);

      if (ImGui::RadioButton("Even", not _block_editor_config.snapping_odd)) {
         _block_editor_config.snapping_odd = false;
      }

      ImGui::SetItemTooltip(
         "Create an even number of snapping points along edges.");

      ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

      if (ImGui::RadioButton("Odd", _block_editor_config.snapping_odd)) {
         _block_editor_config.snapping_odd = true;
      }

      ImGui::SetItemTooltip(
         "Create an odd number of snapping points along edges.");

      ImGui::EndDisabled();

      _block_editor_config.new_block_layer =
         std::clamp(_block_editor_config.new_block_layer, int8{0},
                    static_cast<int8>(_world.layer_descriptions.size()));

      if (ImGui::BeginCombo("Draw Layer",
                            _world
                               .layer_descriptions[_block_editor_config.new_block_layer]
                               .name.c_str())) {
         for (int8 i = 0; i < std::ssize(_world.layer_descriptions); ++i) {
            if (ImGui::Selectable(_world.layer_descriptions[i].name.c_str(),
                                  _block_editor_config.new_block_layer == i)) {
               _block_editor_config.new_block_layer = i;
            }
         }

         ImGui::EndCombo();
      }

      ImGui::SetItemTooltip(
         "Layers for blocks are just for in the editor. Ingame all "
         "blocks are effectively in the base layer.");

      ImGui::Separator();

      if (ImGui::Button("Tweak Block", {ImGui::CalcItemWidth(), 0.0f})) {
         _block_editor_context.activate_tool = block_edit_tool::tweak_block;
      }

      if (ImGui::Button("Set Texture Mode", {ImGui::CalcItemWidth(), 0.0f})) {
         _block_editor_context.activate_tool = block_edit_tool::set_texture_mode;
      }

      if (ImGui::BeginCombo("Texture Mode",
                            texture_mode_name(_block_editor_config.texture_mode),
                            ImGuiComboFlags_HeightLarge)) {
         for (world::block_texture_mode mode : {
                 world::block_texture_mode::world_space_auto,

                 world::block_texture_mode::world_space_zy,
                 world::block_texture_mode::world_space_xz,
                 world::block_texture_mode::world_space_xy,

                 world::block_texture_mode::local_space_auto,

                 world::block_texture_mode::local_space_zy,
                 world::block_texture_mode::local_space_xz,
                 world::block_texture_mode::local_space_xy,

                 world::block_texture_mode::unwrapped,
              }) {
            if (ImGui::Selectable(texture_mode_name(mode),
                                  _block_editor_config.texture_mode == mode)) {
               _block_editor_config.texture_mode = mode;
            }
         }

         ImGui::EndCombo();
      }

      ImGui::Separator();

      if (ImGui::Button("Rotate Texture", {ImGui::CalcItemWidth(), 0.0f})) {
         _block_editor_context.activate_tool = block_edit_tool::rotate_texture;
      }

      if (ImGui::Button("Scale Texture", {ImGui::CalcItemWidth(), 0.0f})) {
         _block_editor_context.activate_tool = block_edit_tool::scale_texture;
      }

      if (ImGui::BeginTable("##scale_options", 2, ImGuiTableFlags_SizingStretchSame,
                            {ImGui::CalcItemWidth(), 0.0f})) {
         ImGui::TableNextRow();

         ImGui::TableNextColumn();
         ImGui::Checkbox("Scale U", &_block_editor_config.scale_texture_u);

         ImGui::TableNextColumn();
         ImGui::Checkbox("Scale V", &_block_editor_config.scale_texture_v);

         ImGui::EndTable();
      }

      if (ImGui::Button("Offset Texture", {ImGui::CalcItemWidth(), 0.0f})) {
         _block_editor_context.activate_tool = block_edit_tool::offset_texture;
      }

      ImGui::Separator();

      if (ImGui::Button("Paint Material", {ImGui::CalcItemWidth(), 0.0f})) {
         _block_editor_context.activate_tool = block_edit_tool::paint_material;
      }

      const std::string& material_name =
         _world.blocks.materials[_block_editor_config.paint_material_index].name;

      if (ImGui::BeginCombo("Material",
                            material_name.empty() ? "<unnamed material>"
                                                  : material_name.c_str(),
                            ImGuiComboFlags_HeightLargest |
                               ImGuiComboFlags_NoArrowButton)) {
         ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

         ImGui::InputTextWithHint("##name", "filter e.g. tiles",
                                  &_block_editor_config.paint_material_filter);

         const float thumbnail_size = 64.0f * _display_scale;
         const float visible_thumbnails = 12.0f;
         const ImVec2 text_offset = {thumbnail_size +
                                        ImGui::GetStyle().ItemSpacing.x,
                                     floorf((thumbnail_size - ImGui::GetFontSize()) *
                                            0.5f)};
         const float button_width = ImGui::GetContentRegionAvail().x;
         const float button_height =
            thumbnail_size + ImGui::GetStyle().ItemInnerSpacing.y;

         ImGui::BeginChild("Classes", {0.0f, (thumbnail_size +
                                              ImGui::GetStyle().ItemSpacing.y) *
                                                visible_thumbnails});

         for (uint32 material_index = 0;
              material_index < _world.blocks.materials.size(); ++material_index) {
            const world::block_material& material =
               _world.blocks.materials[material_index];

            if (not _block_editor_config.paint_material_filter.empty() and
                not string::icontains(material.name,
                                      _block_editor_config.paint_material_filter)) {
               continue;
            }

            if (ImGui::IsRectVisible({button_width, button_height})) {
               ImGui::PushID(material_index);

               const ImVec2 image_cursor_pos = ImGui::GetCursorScreenPos();

               if (ImGui::Selectable("##pick",
                                     _block_editor_config.paint_material_index == material_index,
                                     ImGuiSelectableFlags_None,
                                     {button_width, button_height})) {
                  _block_editor_config.paint_material_index =
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

      if (ImGui::BeginChild("##material", {}, ImGuiChildFlags_Borders)) {
         const uint32 material_index = _block_editor_config.paint_material_index;
         world::block_material& material = _world.blocks.materials[material_index];

         ImGui::InputText("Name", &material.name, _edit_stack_world, _edit_context);

         ui_block_texture_pick_widget("Diffuse Map", &material.diffuse_map,
                                      material_index);
         ui_block_texture_pick_widget("Normal Map", &material.normal_map, material_index);
         ui_block_texture_pick_widget("Detail Map", &material.detail_map, material_index);
         ui_block_texture_pick_widget("Env Map", &material.env_map, material_index);

         if (std::array<uint8, 2> detail_tiling = material.detail_tiling;
             ImGui::DragScalarN("Detail Tiling", ImGuiDataType_U8,
                                detail_tiling.data(), 2)) {
            _edit_stack_world
               .apply(edits::make_set_block_material(&material.detail_tiling,
                                                     detail_tiling, material_index,
                                                     &_world.blocks.materials_dirty),
                      _edit_context);
         }

         if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

         if (bool tile_normal_map = material.tile_normal_map;
             ImGui::Checkbox("Tile Normal Map", &tile_normal_map)) {
            _edit_stack_world
               .apply(edits::make_set_block_material(&material.tile_normal_map,
                                                     tile_normal_map, material_index,
                                                     &_world.blocks.materials_dirty),
                      _edit_context, {.closed = true});
         }

         if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

         if (bool specular_lighting = material.specular_lighting;
             ImGui::Checkbox("Specular Lighting", &specular_lighting)) {
            _edit_stack_world
               .apply(edits::make_set_block_material(&material.specular_lighting,
                                                     specular_lighting, material_index,
                                                     &_world.blocks.materials_dirty),
                      _edit_context, {.closed = true});
         }

         if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();

         if (float3 specular_color = material.specular_color;
             ImGui::ColorEdit3("Specular Lighting", &specular_color.x)) {
            _edit_stack_world
               .apply(edits::make_set_block_material(&material.specular_color,
                                                     specular_color, material_index,
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

   const float blocks_editor_width = ImGui::GetWindowWidth();

   ImGui::End();

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      if (_block_editor_context.tool == block_edit_tool::draw) {
         ImGui::SeparatorText("Block Drawing");

         ImGui::Text("Toggle Cursor Alignment");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Block Editing (Drawing)",
                                   "Toggle Cursor Alignment")));

         ImGui::Text("Toggle Cursor Snapping");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Block Editing (Drawing)",
                                   "Toggle Cursor Snapping")));

         ImGui::Text("Toggle X Draw Plane");
         ImGui::BulletText(
            get_display_string(_hotkeys.query_binding("Block Editing (Drawing)",
                                                      "Toggle X Draw Plane")));

         ImGui::Text("Toggle Y Draw Plane");
         ImGui::BulletText(
            get_display_string(_hotkeys.query_binding("Block Editing (Drawing)",
                                                      "Toggle Y Draw Plane")));

         ImGui::Text("Toggle Z Draw Plane");
         ImGui::BulletText(
            get_display_string(_hotkeys.query_binding("Block Editing (Drawing)",
                                                      "Toggle Z Draw Plane")));
      }
      else {
         ImGui::SeparatorText("Block Editing");

         ImGui::Text("Draw Block");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Block Editing", "Draw Block")));

         ImGui::Text("Resize Block");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Block Editing", "Resize Block")));

         ImGui::Text("Paint Material");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Block Editing", "Paint Material")));

         ImGui::Text("Set Texture Mode");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Block Editing", "Set Texture Mode")));

         ImGui::Text("Rotate Texture");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Block Editing", "Rotate Texture")));

         ImGui::Text("Scale Texture");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Block Editing", "Scale Texture")));

         ImGui::Text("Offset Texture");
         ImGui::BulletText(get_display_string(
            _hotkeys.query_binding("Block Editing", "Offset Texture")));
      }

      ImGui::End();
   }

   if (ImGui::BeginPopup("Block Quick Tools", ImGuiWindowFlags_NoTitleBar)) {
      if (ImGui::MenuItem("Draw Box")) {
         _block_editor_config.draw_type = draw_block_type::box;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Ramp")) {
         _block_editor_config.draw_type = draw_block_type::ramp;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Quadrilateral")) {
         _block_editor_config.draw_type = draw_block_type::quad;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Cylinder")) {
         _block_editor_config.draw_type = draw_block_type::cylinder;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Stairs")) {
         _block_editor_config.draw_type = draw_block_type::stairway;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Stairs (Floating)")) {
         _block_editor_config.draw_type = draw_block_type::stairway_floating;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Cone")) {
         _block_editor_config.draw_type = draw_block_type::cone;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Hemisphere")) {
         _block_editor_config.draw_type = draw_block_type::hemisphere;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Pyramid")) {
         _block_editor_config.draw_type = draw_block_type::pyramid;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Ring")) {
         _block_editor_config.draw_type = draw_block_type::ring;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Beveled Box")) {
         _block_editor_config.draw_type = draw_block_type::beveled_box;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Curve")) {
         _block_editor_config.draw_type = draw_block_type::curve;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Arch")) {
         _block_editor_config.draw_type = draw_block_type::arch;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      if (ImGui::MenuItem("Draw Terrain Cutter (Box)")) {
         _block_editor_config.draw_type = draw_block_type::terrain_cut_box;
         _block_editor_context = {.activate_tool = block_edit_tool::draw};
      }

      ImGui::EndPopup();
   }

   switch (std::exchange(_block_editor_context.activate_tool, block_edit_tool::none)) {
   case block_edit_tool::none: {
   } break;
   case block_edit_tool::draw: {
      _block_editor_context.tool = block_edit_tool::draw;
      _block_editor_context.tool_click = false;
      _block_editor_context.tool_ctrl_click = false;
      _block_editor_context.draw_block = {};
   } break;
   case block_edit_tool::rotate_texture: {
      _block_editor_context.tool_click = false;
      _block_editor_context.tool_ctrl_click = false;
      _block_editor_context.tool = block_edit_tool::rotate_texture;
   } break;
   case block_edit_tool::scale_texture: {
      _block_editor_context.tool_click = false;
      _block_editor_context.tool_ctrl_click = false;
      _block_editor_context.tool = block_edit_tool::scale_texture;
   } break;
   case block_edit_tool::paint_material: {
      _block_editor_context.tool_click = false;
      _block_editor_context.tool_ctrl_click = false;
      _block_editor_context.tool = block_edit_tool::paint_material;
   } break;
   case block_edit_tool::set_texture_mode: {
      _block_editor_context.tool_click = false;
      _block_editor_context.tool_ctrl_click = false;
      _block_editor_context.tool = block_edit_tool::set_texture_mode;
   } break;
   case block_edit_tool::offset_texture: {
      _block_editor_context.tool_click = false;
      _block_editor_context.tool_ctrl_click = false;
      _block_editor_context.tool = block_edit_tool::offset_texture;
      _block_editor_context.offset_texture = {};
   } break;
   case block_edit_tool::tweak_block: {
      _block_editor_context.tool_click = false;
      _block_editor_context.tool_ctrl_click = false;
      _block_editor_context.tool = block_edit_tool::tweak_block;
      _block_editor_context.resize_block = {};
   } break;
   }

   if (_block_editor_context.tool == block_edit_tool::draw) {
      // Parameters Window

      const ImVec2 parameters_window_pos = {tool_window_start_x * _display_scale +
                                               blocks_editor_width +
                                               ImGui::GetStyle().ItemSpacing.x,
                                            32.0f * _display_scale};

      switch (_block_editor_config.draw_type) {
      case draw_block_type::box:
      case draw_block_type::ramp:
         break;
      case draw_block_type::quad: {
         ImGui::SetNextWindowPos(parameters_window_pos, ImGuiCond_Once);

         if (ImGui::Begin("Quadrilateral Parameters###draw_block_paramaters",
                          nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SeparatorText("Triangulation");

            if (ImGui::RadioButton("Longest Diagonal",
                                   _block_editor_config.quad_split ==
                                      draw_block_quad_split::longest)) {
               _block_editor_config.quad_split = draw_block_quad_split::longest;
            }

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

            if (ImGui::RadioButton("Shortest Diagonal",
                                   _block_editor_config.quad_split ==
                                      draw_block_quad_split::shortest)) {
               _block_editor_config.quad_split = draw_block_quad_split::shortest;
            }
         }

         ImGui::End();
      } break;
      case draw_block_type::cylinder: {
         ImGui::SetNextWindowPos(parameters_window_pos, ImGuiCond_Once);

         if (ImGui::Begin("Cylinder Parameters###draw_block_paramaters",
                          nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            const uint16 min_segments = 3;
            const uint16 max_segments = 256;

            ImGui::SliderScalar("Segments", ImGuiDataType_U16,
                                &_block_editor_config.cylinder.segments,
                                &min_segments, &max_segments);
            ImGui::Checkbox("Flat Shading", &_block_editor_config.cylinder.flat_shading);
            ImGui::DragFloat("Texture Loops",
                             &_block_editor_config.cylinder.texture_loops);

            ImGui::SetItemTooltip(
               "How many times the texture wraps around the cylinder in the "
               "Unwrapped Texture Mode.");

            _block_editor_config.cylinder.segments =
               std::max(_block_editor_config.cylinder.segments, min_segments);
         }

         ImGui::End();
      } break;
      case draw_block_type::stairway:
      case draw_block_type::stairway_floating: {
         ImGui::SetNextWindowPos(parameters_window_pos, ImGuiCond_Once);

         if (ImGui::Begin("Stairway Parameters###draw_block_paramaters",
                          nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::DragFloat("Step Height", &_block_editor_config.step_height,
                             0.125f * 0.25f, 0.125f, 1.0f, "%.3f",
                             ImGuiSliderFlags_NoRoundToFormat);

            _block_editor_config.step_height =
               std::max(_block_editor_config.step_height, 0.015625f);

            ImGui::DragFloat("First Step Offset",
                             &_block_editor_config.first_step_offset, 0.125f);
         }

         ImGui::End();
      } break;
      case draw_block_type::cone: {
         ImGui::SetNextWindowPos(parameters_window_pos, ImGuiCond_Once);

         if (ImGui::Begin("Cone Parameters###draw_block_paramaters", nullptr,
                          ImGuiWindowFlags_AlwaysAutoResize)) {
            const uint16 min_segments = 3;
            const uint16 max_segments = 256;

            ImGui::SliderScalar("Segments", ImGuiDataType_U16,
                                &_block_editor_config.cone.segments,
                                &min_segments, &max_segments);
            ImGui::Checkbox("Flat Shading", &_block_editor_config.cone.flat_shading);

            ImGui::SetItemTooltip(
               "How many times the texture wraps around the ring in the "
               "Unwrapped Texture Mode.");

            _block_editor_config.cone.segments =
               std::max(_block_editor_config.cone.segments, min_segments);
         }

         ImGui::End();
      } break;
      case draw_block_type::hemisphere:
      case draw_block_type::pyramid:
         break;
      case draw_block_type::ring: {
         ImGui::SetNextWindowPos(parameters_window_pos, ImGuiCond_Once);

         if (ImGui::Begin("Beveled Box Parameters###draw_block_paramaters",
                          nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            const uint16 min_segments = 3;
            const uint16 max_segments = 256;

            ImGui::SliderScalar("Segments", ImGuiDataType_U16,
                                &_block_editor_config.ring.segments,
                                &min_segments, &max_segments);
            ImGui::Checkbox("Flat Shading", &_block_editor_config.ring.flat_shading);
            ImGui::DragFloat("Texture Loops", &_block_editor_config.ring.texture_loops);

            ImGui::SetItemTooltip(
               "How many times the texture wraps around the ring in the "
               "Unwrapped Texture Mode.");

            _block_editor_config.ring.segments =
               std::max(_block_editor_config.ring.segments, min_segments);
         }

         ImGui::End();
      } break;
      case draw_block_type::beveled_box: {
         ImGui::SetNextWindowPos(parameters_window_pos, ImGuiCond_Once);

         if (ImGui::Begin("Ring Parameters###draw_block_paramaters", nullptr,
                          ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::DragFloat("Bevel Amount",
                             &_block_editor_config.beveled_box.amount, 0.0625f,
                             0.0f, 1e10f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::Checkbox("Bevel Top", &_block_editor_config.beveled_box.bevel_top);
            ImGui::Checkbox("Bevel Sides", &_block_editor_config.beveled_box.bevel_sides);
            ImGui::Checkbox("Bevel Bottom",
                            &_block_editor_config.beveled_box.bevel_bottom);
         }

         ImGui::End();
      } break;
      case draw_block_type::curve: {
         ImGui::SetNextWindowPos(parameters_window_pos, ImGuiCond_Once);

         if (ImGui::Begin("Curve Parameters###draw_block_paramaters", nullptr,
                          ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::DragFloat("Width", &_block_editor_config.curve.width, 1.0f,
                             0.0f, 1e10f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::DragFloat("Height", &_block_editor_config.curve.height, 1.0f,
                             0.0f, 1e10f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

            const uint16 min_segments = 3;
            const uint16 max_segments = 256;

            ImGui::SliderScalar("Segments", ImGuiDataType_U16,
                                &_block_editor_config.curve.segments,
                                &min_segments, &max_segments);

            ImGui::DragFloat("Texture Loops", &_block_editor_config.curve.texture_loops);

            _block_editor_config.curve.segments =
               std::max(_block_editor_config.curve.segments, min_segments);
         }

         ImGui::End();
      } break;
      case draw_block_type::arch: {
         ImGui::SetNextWindowPos(parameters_window_pos, ImGuiCond_Once);

         if (ImGui::Begin("Arch Parameters###draw_block_paramaters", nullptr,
                          ImGuiWindowFlags_AlwaysAutoResize)) {
            const uint16 min_segments = 1;
            const uint16 max_segments = 64;

            ImGui::DragFloat("Crown Length", &_block_editor_config.arch.crown_length,
                             0.5f, 0.0f, 1e10f);
            ImGui::DragFloat("Crown Height", &_block_editor_config.arch.crown_height,
                             0.125f, 0.0f, 1e10f);
            ImGui::DragFloat("Curve Height", &_block_editor_config.arch.curve_height,
                             0.5f, 0.0f, 1e10f);
            ImGui::DragFloat("Span Length", &_block_editor_config.arch.span_length,
                             0.5f, 0.0f, 1e10f);
            ImGui::SliderScalar("Segments", ImGuiDataType_U16,
                                &_block_editor_config.arch.segments,
                                &min_segments, &max_segments);

            _block_editor_config.arch.segments =
               std::max(_block_editor_config.arch.segments, min_segments);
         }

         ImGui::End();
      } break;
      case draw_block_type::terrain_cut_box:
         break;
      }

      // Cursor Input, Snapping and Alignment

      const bool click = std::exchange(_block_editor_context.tool_click, false);
      const bool align = _block_editor_config.enable_alignment;
      const bool snap = _block_editor_config.enable_snapping;
      float3 cursor_positionWS = _cursor_positionWS;

      if (_block_editor_context.draw_block.height_plane) {
         const graphics::camera_ray rayWS =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         const float4 draw_planeWS =
            make_plane_from_point({0.0f, *_block_editor_context.draw_block.height_plane,
                                   0.0f},
                                  float3{0.0f, 1.0f, 0.0f});

         if (float hit = intersect_plane(rayWS.origin, rayWS.direction, draw_planeWS);
             hit > 0.0f) {
            cursor_positionWS = rayWS.origin + rayWS.direction * hit;
         }
         else {
            cursor_positionWS.y = *_block_editor_context.draw_block.height_plane;
         }
      }

      const float3 alignment =
         {std::exp2f(static_cast<float>(_block_editor_config.xz_alignment_exponent)),
          std::exp2f(static_cast<float>(_block_editor_config.y_alignment_exponent)),
          std::exp2f(static_cast<float>(_block_editor_config.xz_alignment_exponent))};

      if (std::optional<float3> snapped_positionWS =
             snap ? world::get_snapped_position(
                       cursor_positionWS, _world.blocks,
                       {
                          .snap_radius = alignment.x * 0.5f,
                          .snap_odd = _block_editor_config.snapping_odd,
                          .filter_id = _block_editor_context.draw_block.block_id,
                          .active_layers = _world_layers_hit_mask,
                       },
                       _tool_visualizers,
                       {
                          .snapped = _settings.graphics.snapping_snapped_color,
                          .corner = _settings.graphics.snapping_corner_color,
                          .edge = _settings.graphics.snapping_edge_color,
                       })
                  : std::nullopt;
          snapped_positionWS) {
         cursor_positionWS = *snapped_positionWS;
      }
      else if (align) {
         cursor_positionWS = round(cursor_positionWS / alignment) * alignment;
      }

      if (_block_editor_context.draw_block.cursor_plane !=
          draw_block_cursor_plane::none) {
         const graphics::camera_ray rayWS =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         uint32 axis_color = 0x30'ff'ff'ff;
         float3 plane_axisWS;

         switch (_block_editor_context.draw_block.cursor_plane) {
         case draw_block_cursor_plane::none:
            break;
         case draw_block_cursor_plane::x:
            axis_color = cursor_plane_axis_color_x;
            plane_axisWS = float3{1.0f, 0.0f, 0.0f};
            break;
         case draw_block_cursor_plane::y:
            axis_color = cursor_plane_axis_color_y;
            plane_axisWS = float3{0.0f, 1.0f, 0.0f};
            break;
         case draw_block_cursor_plane::z:
            axis_color = cursor_plane_axis_color_z;
            plane_axisWS = float3{0.0f, 0.0f, 1.0f};
            break;
         }

         const float3 plane_positionWS =
            _block_editor_context.draw_block.cursor_plane_positionWS;
         const float3 eye_directionWS = plane_positionWS - _camera.position();
         const float3 plane_tangentWS = cross(plane_axisWS, eye_directionWS);
         const float3 plane_normalWS =
            normalize(cross(plane_axisWS, plane_tangentWS));

         const float4 planeWS =
            make_plane_from_point(plane_positionWS, plane_normalWS);

         if (float hit = intersect_plane(rayWS.origin, rayWS.direction, planeWS);
             hit > 0.0f) {
            float3 hit_positionWS = rayWS.origin + rayWS.direction * hit;

            if (align) {
               hit_positionWS = round(hit_positionWS / alignment) * alignment;
            }

            switch (_block_editor_context.draw_block.cursor_plane) {
            case draw_block_cursor_plane::none:
               break;
            case draw_block_cursor_plane::x:
               cursor_positionWS = {hit_positionWS.x, plane_positionWS.y,
                                    plane_positionWS.z};
               break;
            case draw_block_cursor_plane::y:
               cursor_positionWS = {plane_positionWS.x, hit_positionWS.y,
                                    plane_positionWS.z};
               break;
            case draw_block_cursor_plane::z:
               cursor_positionWS = {plane_positionWS.x, plane_positionWS.y,
                                    hit_positionWS.z};
               break;
            }

            _tool_visualizers.add_line(cursor_positionWS, plane_positionWS,
                                       0xff'ff'ff'ff);
         }

         if (dot(cursor_positionWS, plane_axisWS) < 0.0f) {
            _tool_visualizers.add_line(cursor_positionWS,
                                       cursor_positionWS -
                                          plane_axisWS * _camera.far_clip(),
                                       axis_color);
            _tool_visualizers.add_line(plane_positionWS,
                                       plane_positionWS +
                                          plane_axisWS * _camera.far_clip(),
                                       axis_color);
         }
         else {
            _tool_visualizers.add_line(plane_positionWS,
                                       plane_positionWS -
                                          plane_axisWS * _camera.far_clip(),
                                       axis_color);
            _tool_visualizers.add_line(cursor_positionWS,
                                       cursor_positionWS +
                                          plane_axisWS * _camera.far_clip(),
                                       axis_color);
         }

         if (click) {
            _block_editor_context.draw_block.cursor_plane_positionWS = cursor_positionWS;
         }
      }

      switch (_block_editor_context.draw_block.toggle_plane) {
      case draw_block_cursor_plane::none:
         break;
      case draw_block_cursor_plane::x: {
         _block_editor_context.draw_block.toggle_plane = draw_block_cursor_plane::none;

         if (_block_editor_context.draw_block.cursor_plane ==
             draw_block_cursor_plane::x) {
            _block_editor_context.draw_block.cursor_plane =
               draw_block_cursor_plane::none;
         }
         else {
            _block_editor_context.draw_block.cursor_plane =
               draw_block_cursor_plane::x;
            _block_editor_context.draw_block.cursor_plane_positionWS = cursor_positionWS;
         }
      } break;
      case draw_block_cursor_plane::y: {
         _block_editor_context.draw_block.toggle_plane = draw_block_cursor_plane::none;

         if (_block_editor_context.draw_block.cursor_plane ==
             draw_block_cursor_plane::y) {
            _block_editor_context.draw_block.cursor_plane =
               draw_block_cursor_plane::none;
         }
         else {
            _block_editor_context.draw_block.cursor_plane =
               draw_block_cursor_plane::y;
            _block_editor_context.draw_block.cursor_plane_positionWS = cursor_positionWS;
         }
      } break;
      case draw_block_cursor_plane::z: {
         _block_editor_context.draw_block.toggle_plane = draw_block_cursor_plane::none;

         if (_block_editor_context.draw_block.cursor_plane ==
             draw_block_cursor_plane::z) {
            _block_editor_context.draw_block.cursor_plane =
               draw_block_cursor_plane::none;
         }
         else {
            _block_editor_context.draw_block.cursor_plane =
               draw_block_cursor_plane::z;
            _block_editor_context.draw_block.cursor_plane_positionWS = cursor_positionWS;
         }
      } break;
      }

      const uint32 line_color =
         utility::pack_srgb_bgra({_settings.graphics.creation_color, 1.0f});

      _tool_visualizers.add_mini_grid({
         .positionWS = cursor_positionWS,
         .size = alignment.x,
         .divisions = 3.0f,
         .color = _settings.graphics.block_draw_grid_color,
      });

      // Main Draw Block Switch
      //
      // When the cursor is clicked in the start step the step is changed to the
      // first step of the currently set draw type.
      //
      // When the cursor is clicked in these states they move onto the next state for their
      // shape. The final step clears the draw block context, putting the step back to start.

      switch (_block_editor_context.draw_block.step) {
      case draw_block_step::start: {
         if (align) {
            _tool_visualizers.add_line(cursor_positionWS -
                                          float3{alignment.x, 0.0f, 0.0f},
                                       cursor_positionWS +
                                          float3{alignment.x, 0.0f, 0.0f},
                                       line_color);
            _tool_visualizers.add_line(cursor_positionWS -
                                          float3{0.0f, alignment.y, 0.0f},
                                       cursor_positionWS +
                                          float3{0.0f, alignment.y, 0.0f},
                                       line_color);
            _tool_visualizers.add_line(cursor_positionWS -
                                          float3{0.0f, 0.0f, alignment.z},
                                       cursor_positionWS +
                                          float3{0.0f, 0.0f, alignment.z},
                                       line_color);
         }

         if (click) {
            _block_editor_context.draw_block.height_plane = cursor_positionWS.y;

            switch (_block_editor_config.draw_type) {
            case draw_block_type::box: {
               _block_editor_context.draw_block.box.start = cursor_positionWS;
               _block_editor_context.draw_block.step = draw_block_step::box_depth;
            } break;
            case draw_block_type::ramp: {
               _block_editor_context.draw_block.ramp.start = cursor_positionWS;
               _block_editor_context.draw_block.step = draw_block_step::ramp_width;
            } break;
            case draw_block_type::quad: {
               _block_editor_context.draw_block.height_plane = std::nullopt;
               _block_editor_context.draw_block.quad.vertices[0] = cursor_positionWS;
               _block_editor_context.draw_block.step = draw_block_step::quad_v1;
            } break;
            case draw_block_type::cylinder: {
               const uint8 material_index = _block_editor_config.paint_material_index;
               const world::block_custom_id id =
                  _world.blocks.next_id.custom.aquire();

               _block_editor_context.draw_block.index =
                  static_cast<uint32>(_world.blocks.custom.size());
               _block_editor_context.draw_block.block_id = id;

               _block_editor_context.draw_block.cylinder.start = cursor_positionWS;
               _block_editor_context.draw_block.step = draw_block_step::cylinder_radius;

               if (_world.blocks.custom.size() < world::max_blocks) {
                  _edit_stack_world.apply(
                     edits::make_add_block(
                        world::block_description_custom{
                           .position = cursor_positionWS,
                           .mesh_description =
                              world::block_custom_mesh_description_cylinder{
                                 .size = {1.0f, 0.0f, 1.0f},
                                 .segments = _block_editor_config.cylinder.segments,
                                 .flat_shading = _block_editor_config.cylinder.flat_shading,
                                 .texture_loops = _block_editor_config.cylinder.texture_loops,
                              },
                           .surface_materials = {material_index, material_index, material_index},
                           .surface_texture_mode = {world::block_texture_mode::world_space_auto,
                                                    world::block_texture_mode::world_space_auto,
                                                    world::block_texture_mode::unwrapped}},
                        _block_editor_config.new_block_layer, id),
                     _edit_context);
               }
               else {
                  MessageBoxA(_window,
                              fmt::format("Max Custom Blocks ({}) Reached", world::max_blocks)
                                 .c_str(),
                              "Limit Reached", MB_OK);

                  _block_editor_context.tool = block_edit_tool::none;
               }

            } break;
            case draw_block_type::stairway: {
               _block_editor_context.draw_block.stairway.start = cursor_positionWS;
               _block_editor_context.draw_block.step = draw_block_step::stairway_width;
            } break;
            case draw_block_type::stairway_floating: {
               _block_editor_context.draw_block.stairway.start = cursor_positionWS;
               _block_editor_context.draw_block.step =
                  draw_block_step::stairway_floating_width;
            } break;
            case draw_block_type::cone: {
               const uint8 material_index = _block_editor_config.paint_material_index;
               const world::block_custom_id id =
                  _world.blocks.next_id.custom.aquire();

               _block_editor_context.draw_block.index =
                  static_cast<uint32>(_world.blocks.custom.size());
               _block_editor_context.draw_block.block_id = id;

               _block_editor_context.draw_block.cone.start = cursor_positionWS;
               _block_editor_context.draw_block.step = draw_block_step::cone_radius;

               if (_world.blocks.custom.size() < world::max_blocks) {
                  _edit_stack_world.apply(
                     edits::make_add_block(
                        world::block_description_custom{
                           .position = cursor_positionWS +
                                       float3{0.0f, alignment.y / 2.0f, 0.0f},
                           .mesh_description =
                              world::block_custom_mesh_description_cone{
                                 .size = {1.0f, alignment.y / 2.0f, 1.0f},
                                 .segments = _block_editor_config.cone.segments,
                                 .flat_shading = _block_editor_config.cone.flat_shading,
                              },
                           .surface_materials = {material_index, material_index},
                           .surface_texture_mode = {world::block_texture_mode::world_space_auto,
                                                    world::block_texture_mode::unwrapped}},
                        _block_editor_config.new_block_layer, id),
                     _edit_context);
               }
               else {
                  MessageBoxA(_window,
                              fmt::format("Max Custom Blocks ({}) Reached", world::max_blocks)
                                 .c_str(),
                              "Limit Reached", MB_OK);

                  _block_editor_context.tool = block_edit_tool::none;
               }

            } break;
            case draw_block_type::hemisphere: {
               const uint8 material_index = _block_editor_config.paint_material_index;
               const world::block_hemisphere_id id =
                  _world.blocks.next_id.hemispheres.aquire();

               _block_editor_context.draw_block.index =
                  static_cast<uint32>(_world.blocks.hemispheres.size());
               _block_editor_context.draw_block.block_id = id;

               _block_editor_context.draw_block.hemisphere.start = cursor_positionWS;
               _block_editor_context.draw_block.step =
                  draw_block_step::hemisphere_radius;

               if (_world.blocks.hemispheres.size() < world::max_blocks) {
                  _edit_stack_world.apply(
                     edits::make_add_block(
                        world::block_description_hemisphere{
                           .position = cursor_positionWS,
                           .size = {1.0f, 1.0f, 1.0f},
                           .surface_materials = {material_index, material_index},
                           .surface_texture_mode = {world::block_texture_mode::world_space_auto,
                                                    world::block_texture_mode::unwrapped}},
                        _block_editor_config.new_block_layer, id),
                     _edit_context);
               }
               else {
                  MessageBoxA(_window,
                              fmt::format("Max hemispheres ({}) Reached", world::max_blocks)
                                 .c_str(),
                              "Limit Reached", MB_OK);

                  _block_editor_context.tool = block_edit_tool::none;
               }

            } break;
            case draw_block_type::pyramid: {
               _block_editor_context.draw_block.pyramid.start = cursor_positionWS;
               _block_editor_context.draw_block.step = draw_block_step::pyramid_depth;
            } break;
            case draw_block_type::ring: {
               _block_editor_context.draw_block.ring.start = cursor_positionWS;
               _block_editor_context.draw_block.step =
                  draw_block_step::ring_inner_radius;
            } break;
            case draw_block_type::beveled_box: {
               _block_editor_context.draw_block.beveled_box.start = cursor_positionWS;
               _block_editor_context.draw_block.step =
                  draw_block_step::beveled_box_depth;
            } break;
            case draw_block_type::curve: {
               _block_editor_context.draw_block.height_plane = std::nullopt;
               _block_editor_context.draw_block.curve.p0 = cursor_positionWS;
               _block_editor_context.draw_block.step = draw_block_step::curve_p3;
            } break;
            case draw_block_type::arch: {
               _block_editor_context.draw_block.arch.start = cursor_positionWS;
               _block_editor_context.draw_block.step = draw_block_step::arch_length;
            } break;
            case draw_block_type::terrain_cut_box: {
               _block_editor_context.draw_block.terrain_cut_box.start =
                  cursor_positionWS;
               _block_editor_context.draw_block.step =
                  draw_block_step::terrain_cut_box_depth;
            } break;
            }
         }
      } break;
      case draw_block_step::box_depth: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.box.start,
                              {cursor_positionWS.x,
                               _block_editor_context.draw_block.box.start.y,
                               cursor_positionWS.z},
                              line_color);

         if (click) {
            _block_editor_context.draw_block.box.depth_x = cursor_positionWS.x;
            _block_editor_context.draw_block.box.depth_z = cursor_positionWS.z;
            _block_editor_context.draw_block.step = draw_block_step::box_width;
         }

      } break;
      case draw_block_step::box_width: {
         const float3 draw_block_start = _block_editor_context.draw_block.box.start;
         const float3 draw_block_depth = {_block_editor_context.draw_block.box.depth_x,
                                          draw_block_start.y,
                                          _block_editor_context.draw_block.box.depth_z};

         const float3 cursor_direction =
            normalize(cursor_positionWS - draw_block_depth);

         const float3 extend_normal =
            normalize(float3{draw_block_depth.z, 0.0f, draw_block_depth.x} -
                      float3{draw_block_start.z, 0.0f, draw_block_start.x}) *
            float3{-1.0, 0.0f, 1.0};

         float rotation_angle = std::atan2(draw_block_start.x - draw_block_depth.x,
                                           draw_block_start.z - draw_block_depth.z);

         if (draw_block_start.z - draw_block_depth.z < 0.0f) {
            rotation_angle += std::numbers::pi_v<float>;
         }

         const quaternion rotation =
            make_quat_from_euler({0.0f, rotation_angle, 0.0f});
         const quaternion inv_rotation = conjugate(rotation);

         const float normal_sign =
            dot(cursor_direction, extend_normal) < 0.0f ? -1.0f : 1.0f;

         const float cursor_distance =
            std::fabs((inv_rotation * draw_block_depth).x -
                      (inv_rotation * cursor_positionWS).x);

         const float3 draw_block_width =
            draw_block_depth + extend_normal * cursor_distance * normal_sign;

         _tool_visualizers.add_line_overlay(draw_block_start, draw_block_depth,
                                            line_color);
         _tool_visualizers.add_line_overlay(draw_block_depth, draw_block_width,
                                            line_color);

         if (click) {
            const world::block_box_id id = _world.blocks.next_id.boxes.aquire();

            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.box.width_x = draw_block_width.x;
            _block_editor_context.draw_block.box.width_z = draw_block_width.z;
            _block_editor_context.draw_block.box.rotation = rotation;
            _block_editor_context.draw_block.step = draw_block_step::box_height;
            _block_editor_context.draw_block.index =
               static_cast<uint32>(_world.blocks.boxes.size());
            _block_editor_context.draw_block.block_id = id;

            const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
            std::array<float3, 2> cornersLS{};

            for (std::size_t i = 0; i < cornersLS.size(); ++i) {
               cornersLS[i] = inv_rotation * cornersWS[i];
            }

            const float3 block_max = max(cornersLS[0], cornersLS[1]);
            const float3 block_min = min(cornersLS[0], cornersLS[1]);

            const float3 size = float3{std::fabs(block_max.x - block_min.x), 0.0f,
                                       std::fabs(block_max.z - block_min.z)} /
                                2.0f;

            const float3 position = (draw_block_start + draw_block_width) / 2.0f;

            const uint8 material_index = _block_editor_config.paint_material_index;

            if (_world.blocks.boxes.size() < world::max_blocks) {
               _edit_stack_world
                  .apply(edits::make_add_block(
                            world::block_description_box{
                               .rotation = rotation,
                               .position = position,
                               .size = size,
                               .surface_materials = {material_index, material_index,
                                                     material_index, material_index,
                                                     material_index, material_index},
                            },
                            _block_editor_config.new_block_layer, id),
                         _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Boxes ({}) Reached", world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }
         }
      } break;
      case draw_block_step::box_height: {
         const float3 draw_block_start = _block_editor_context.draw_block.box.start;
         const float3 draw_block_depth = {_block_editor_context.draw_block.box.depth_x,
                                          draw_block_start.y,
                                          _block_editor_context.draw_block.box.depth_z};
         const float3 draw_block_width = {_block_editor_context.draw_block.box.width_x,
                                          draw_block_start.y,
                                          _block_editor_context.draw_block.box.width_z};

         const quaternion rotation = _block_editor_context.draw_block.box.rotation;
         const quaternion inv_rotation = conjugate(rotation);

         const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
         std::array<float3, 2> cornersLS{};

         for (std::size_t i = 0; i < cornersLS.size(); ++i) {
            cornersLS[i] = inv_rotation * cornersWS[i];
         }

         const float3 block_max = max(cornersLS[0], cornersLS[1]);
         const float3 block_min = min(cornersLS[0], cornersLS[1]);

         const float4 height_plane =
            make_plane_from_point(draw_block_width,
                                  normalize(draw_block_width - _camera.position()));

         graphics::camera_ray ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = 0.0f;
             intersect_aabb(inv_rotation * ray.origin,
                            1.0f / (inv_rotation * ray.direction),
                            {.min = {block_min.x, -FLT_MAX, block_min.z},
                             .max = {block_max.x, FLT_MAX, block_max.z}},
                            FLT_MAX, hit) and
             hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = ray.origin + hit * ray.direction;
         }

         const float unaligned_box_height = cursor_position.y - draw_block_start.y;
         const float box_height =
            align ? std::round(unaligned_box_height / alignment.y) * alignment.y
                  : unaligned_box_height;

         const float3 position = {(draw_block_start.x + draw_block_width.x) / 2.0f,
                                  draw_block_start.y + (box_height / 2.0f),
                                  (draw_block_start.z + draw_block_width.z) / 2.0f};
         const float3 size = abs(float3{block_max.x - block_min.x, box_height,
                                        block_max.z - block_min.z}) /
                             2.0f;

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.boxes.size() and
             _world.blocks.boxes.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world.apply(edits::make_set_block_box_metrics(index, rotation,
                                                                      position, size),
                                    _edit_context, {.transparent = true});
         }

         _tool_visualizers.add_line_overlay(position -
                                               float3{0.0f, box_height * 0.5f, 0.0f},
                                            position +
                                               float3{0.0f, box_height * 0.5f, 0.0f},
                                            line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::ramp_width: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.ramp.start,
                              {cursor_positionWS.x,
                               _block_editor_context.draw_block.ramp.start.y,
                               cursor_positionWS.z},
                              line_color);

         if (click) {
            _block_editor_context.draw_block.ramp.width_x = cursor_positionWS.x;
            _block_editor_context.draw_block.ramp.width_z = cursor_positionWS.z;
            _block_editor_context.draw_block.step = draw_block_step::ramp_length;
         }

      } break;
      case draw_block_step::ramp_length: {
         const float3 draw_block_start = _block_editor_context.draw_block.ramp.start;
         const float3 draw_block_width = {_block_editor_context.draw_block.ramp.width_x,
                                          draw_block_start.y,
                                          _block_editor_context.draw_block.ramp.width_z};

         const float3 cursor_direction =
            normalize(cursor_positionWS - draw_block_width);

         const float3 extend_normal =
            normalize(float3{draw_block_width.z, 0.0f, draw_block_width.x} -
                      float3{draw_block_start.z, 0.0f, draw_block_start.x}) *
            float3{-1.0, 0.0f, 1.0};

         const float normal_sign =
            dot(cursor_direction, extend_normal) < 0.0f ? -1.0f : 1.0f;

         float rotation_angle = std::atan2(draw_block_start.x - draw_block_width.x,
                                           draw_block_start.z - draw_block_width.z);

         rotation_angle += std::numbers::pi_v<float> * 0.5f;

         if (normal_sign > 0.0f) rotation_angle += std::numbers::pi_v<float>;

         const quaternion rotation =
            make_quat_from_euler({0.0f, rotation_angle, 0.0f});
         const quaternion inv_rotation = conjugate(rotation);

         const float cursor_distance =
            std::fabs((inv_rotation * draw_block_width).z -
                      (inv_rotation * cursor_positionWS).z);

         const float3 draw_block_length =
            draw_block_width + extend_normal * cursor_distance * normal_sign;

         _tool_visualizers.add_line_overlay(draw_block_start, draw_block_width,
                                            line_color);
         _tool_visualizers.add_line_overlay(draw_block_width, draw_block_length,
                                            line_color);

         if (click) {
            const world::block_ramp_id id = _world.blocks.next_id.ramps.aquire();

            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.ramp.length_x = draw_block_length.x;
            _block_editor_context.draw_block.ramp.length_z = draw_block_length.z;
            _block_editor_context.draw_block.ramp.rotation = rotation;
            _block_editor_context.draw_block.step = draw_block_step::ramp_height;
            _block_editor_context.draw_block.index =
               static_cast<uint32>(_world.blocks.ramps.size());
            _block_editor_context.draw_block.block_id = id;

            const std::array<float3, 2> cornersWS{draw_block_start, draw_block_length};
            std::array<float3, 2> cornersLS{};

            for (std::size_t i = 0; i < cornersLS.size(); ++i) {
               cornersLS[i] = inv_rotation * cornersWS[i];
            }

            const float3 block_max = max(cornersLS[0], cornersLS[1]);
            const float3 block_min = min(cornersLS[0], cornersLS[1]);

            const float3 size = float3{std::fabs(block_max.x - block_min.x), 0.0f,
                                       std::fabs(block_max.z - block_min.z)} /
                                2.0f;

            const float3 position = (draw_block_start + draw_block_length) / 2.0f;

            const uint8 material_index = _block_editor_config.paint_material_index;

            if (_world.blocks.ramps.size() < world::max_blocks) {
               _edit_stack_world.apply(edits::make_add_block(
                                          world::block_description_ramp{
                                             .rotation = rotation,
                                             .position = position,
                                             .size = size,
                                             .surface_materials = {material_index, material_index,
                                                                   material_index, material_index,
                                                                   material_index},
                                          },
                                          _block_editor_config.new_block_layer, id),
                                       _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Ramps ({}) Reached", world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }
         }
      } break;
      case draw_block_step::ramp_height: {
         const float3 draw_block_start = _block_editor_context.draw_block.ramp.start;
         const float3 draw_block_width = {_block_editor_context.draw_block.ramp.width_x,
                                          draw_block_start.y,
                                          _block_editor_context.draw_block.ramp.width_z};
         const float3 draw_block_length = {_block_editor_context.draw_block.ramp.length_x,
                                           draw_block_start.y,
                                           _block_editor_context.draw_block.ramp.length_z};

         quaternion rotation = _block_editor_context.draw_block.ramp.rotation;
         quaternion inv_rotation = conjugate(rotation);

         const std::array<float3, 2> cornersWS{draw_block_start, draw_block_length};
         std::array<float3, 2> cornersLS{};

         for (std::size_t i = 0; i < cornersLS.size(); ++i) {
            cornersLS[i] = inv_rotation * cornersWS[i];
         }

         const float3 block_max = max(cornersLS[0], cornersLS[1]);
         const float3 block_min = min(cornersLS[0], cornersLS[1]);

         const float4 height_plane =
            make_plane_from_point(draw_block_width,
                                  normalize(draw_block_width - _camera.position()));

         graphics::camera_ray ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = 0.0f;
             intersect_aabb(inv_rotation * ray.origin,
                            1.0f / (inv_rotation * ray.direction),
                            {.min = {block_min.x, -FLT_MAX, block_min.z},
                             .max = {block_max.x, FLT_MAX, block_max.z}},
                            FLT_MAX, hit) and
             hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = ray.origin + hit * ray.direction;
         }

         const float unaligned_ramp_height =
            cursor_position.y - draw_block_start.y;
         const float ramp_height =
            align ? std::round(unaligned_ramp_height / alignment.y) * alignment.y
                  : unaligned_ramp_height;

         const float3 position = {(draw_block_start.x + draw_block_length.x) / 2.0f,
                                  draw_block_start.y + (ramp_height / 2.0f),
                                  (draw_block_start.z + draw_block_length.z) / 2.0f};
         const float3 size = abs(float3{block_max.x - block_min.x, ramp_height,
                                        block_max.z - block_min.z}) /
                             2.0f;

         if (ramp_height < 0.0f) {
            rotation = normalize(rotation * quaternion{0.0f, 0.0f, 1.0f, 0.0f});
            inv_rotation = conjugate(rotation);
         }

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.ramps.size() and
             _world.blocks.ramps.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world.apply(edits::make_set_block_ramp_metrics(index, rotation,
                                                                       position, size),
                                    _edit_context, {.transparent = true});
         }

         _tool_visualizers
            .add_line_overlay(position - float3{0.0f, ramp_height * 0.5f, 0.0f},
                              position + float3{0.0f, ramp_height * 0.5f, 0.0f},
                              line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::quad_v1: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.quad.vertices[0],
                              cursor_positionWS, line_color);

         if (click) {
            _block_editor_context.draw_block.quad.vertices[1] = cursor_positionWS;
            _block_editor_context.draw_block.step = draw_block_step::quad_v2;
         }

      } break;
      case draw_block_step::quad_v2: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.quad.vertices[0],
                              _block_editor_context.draw_block.quad.vertices[1],
                              line_color);
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.quad.vertices[1],
                              cursor_positionWS, line_color);

         if (click) {
            _block_editor_context.draw_block.quad.vertices[2] = cursor_positionWS;
            _block_editor_context.draw_block.step = draw_block_step::quad_v3;
         }

      } break;
      case draw_block_step::quad_v3: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.quad.vertices[0],
                              _block_editor_context.draw_block.quad.vertices[1],
                              line_color);
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.quad.vertices[1],
                              _block_editor_context.draw_block.quad.vertices[2],
                              line_color);
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.quad.vertices[2],
                              cursor_positionWS, line_color);
         _tool_visualizers
            .add_line_overlay(cursor_positionWS,
                              _block_editor_context.draw_block.quad.vertices[0],
                              line_color);

         if (click) {
            std::array<float3, 4> vertices = {
               _block_editor_context.draw_block.quad.vertices[0],
               _block_editor_context.draw_block.quad.vertices[1],
               _block_editor_context.draw_block.quad.vertices[2],
               cursor_positionWS,
            };

            // Orientate vertices quad to face towards the camera.
            if (dot(normalize(cross(vertices[1] - vertices[0], vertices[2] - vertices[0])),
                    _camera.forward()) > 0.0f) {
               vertices = {
                  vertices[3],
                  vertices[2],
                  vertices[1],
                  vertices[0],
               };
            }

            world::block_quad_split quad_split = world::block_quad_split::regular;

            if (_block_editor_config.quad_split == draw_block_quad_split::longest) {
               quad_split = distance(vertices[0], vertices[2]) >
                                  distance(vertices[1], vertices[3])
                               ? world::block_quad_split::regular
                               : world::block_quad_split::alternate;
            }
            else {
               quad_split = distance(vertices[0], vertices[2]) <
                                  distance(vertices[1], vertices[3])
                               ? world::block_quad_split::regular
                               : world::block_quad_split::alternate;
            }

            _edit_stack_world
               .apply(edits::make_add_block(
                         world::block_description_quad{
                            .vertices = vertices,
                            .quad_split = quad_split,
                            .surface_materials = {_block_editor_config.paint_material_index},
                         },
                         _block_editor_config.new_block_layer,
                         _world.blocks.next_id.quads.aquire()),
                      _edit_context);

            _block_editor_context.draw_block.step = draw_block_step::start;
         }

      } break;
      case draw_block_step::cylinder_radius: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.cylinder.start;

         _tool_visualizers.add_line_overlay(draw_block_start,
                                            {cursor_positionWS.x,
                                             draw_block_start.y,
                                             cursor_positionWS.z},
                                            line_color);

         const float radius =
            distance(float2{draw_block_start.x, draw_block_start.z},
                     float2{cursor_positionWS.x, cursor_positionWS.z});

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.custom.size() and
             _world.blocks.custom.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world
               .apply(edits::make_set_block_custom_metrics(
                         index, {}, draw_block_start,
                         world::block_custom_mesh_description_cylinder{
                            .size = {radius, 0.0f, radius},
                            .segments = _block_editor_config.cylinder.segments,
                            .flat_shading = _block_editor_config.cylinder.flat_shading,
                            .texture_loops = _block_editor_config.cylinder.texture_loops,
                         }),
                      _edit_context, {.transparent = true});
         }

         if (click) {
            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.cylinder.radius = radius;
            _block_editor_context.draw_block.step = draw_block_step::cylinder_height;
         }
      } break;
      case draw_block_step::cylinder_height: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.cylinder.start;
         const float radius = _block_editor_context.draw_block.cylinder.radius;

         graphics::camera_ray rayWS =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = iCylinderInfinite(rayWS.origin, rayWS.direction, draw_block_start,
                                           float3{0.0f, 1.0f, 0.0f}, radius)
                            .x;
             hit >= 0.0f and hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = rayWS.origin + hit * rayWS.direction;
         }

         const float unaligned_cylinder_height =
            cursor_position.y - draw_block_start.y;
         const float cylinder_height =
            align
               ? std::round(unaligned_cylinder_height / alignment.y) * alignment.y
               : unaligned_cylinder_height;

         const float3 position = {draw_block_start.x,
                                  draw_block_start.y + (cylinder_height / 2.0f),
                                  draw_block_start.z};
         const float3 size = abs(float3{radius, cylinder_height / 2.0f, radius});

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.custom.size() and
             _world.blocks.custom.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world
               .apply(edits::make_set_block_custom_metrics(
                         index, {}, position,
                         world::block_custom_mesh_description_cylinder{
                            .size = size,
                            .segments = _block_editor_config.cylinder.segments,
                            .flat_shading = _block_editor_config.cylinder.flat_shading,
                            .texture_loops = _block_editor_config.cylinder.texture_loops,
                         }),
                      _edit_context, {.transparent = true});
         }

         _tool_visualizers.add_line_overlay(
            position - float3{0.0f, cylinder_height * 0.5f, 0.0f},
            position + float3{0.0f, cylinder_height * 0.5f, 0.0f}, line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::stairway_width: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.stairway.start,
                              {cursor_positionWS.x,
                               _block_editor_context.draw_block.stairway.start.y,
                               cursor_positionWS.z},
                              line_color);

         if (click) {
            _block_editor_context.draw_block.stairway.width_x =
               cursor_positionWS.x;
            _block_editor_context.draw_block.stairway.width_z =
               cursor_positionWS.z;
            _block_editor_context.draw_block.step = draw_block_step::stairway_length;
         }

      } break;
      case draw_block_step::stairway_length: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.stairway.start;
         const float3 draw_block_width =
            {_block_editor_context.draw_block.stairway.width_x, draw_block_start.y,
             _block_editor_context.draw_block.stairway.width_z};

         const float3 cursor_direction =
            normalize(cursor_positionWS - draw_block_width);

         const float3 extend_normal =
            normalize(float3{draw_block_width.z, 0.0f, draw_block_width.x} -
                      float3{draw_block_start.z, 0.0f, draw_block_start.x}) *
            float3{-1.0, 0.0f, 1.0};

         const float normal_sign =
            dot(cursor_direction, extend_normal) < 0.0f ? -1.0f : 1.0f;

         float rotation_angle = std::atan2(draw_block_start.x - draw_block_width.x,
                                           draw_block_start.z - draw_block_width.z);

         rotation_angle += std::numbers::pi_v<float> * 0.5f;

         if (normal_sign > 0.0f) rotation_angle += std::numbers::pi_v<float>;

         const quaternion rotation =
            make_quat_from_euler({0.0f, rotation_angle, 0.0f});
         const quaternion inv_rotation = conjugate(rotation);

         const float cursor_distance =
            std::fabs((inv_rotation * draw_block_width).z -
                      (inv_rotation * cursor_positionWS).z);

         const float3 draw_block_length =
            draw_block_width + extend_normal * cursor_distance * normal_sign;

         _tool_visualizers.add_line_overlay(draw_block_start, draw_block_width,
                                            line_color);
         _tool_visualizers.add_line_overlay(draw_block_width, draw_block_length,
                                            line_color);

         if (click) {
            const world::block_custom_id id = _world.blocks.next_id.custom.aquire();

            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.stairway.length_x =
               draw_block_length.x;
            _block_editor_context.draw_block.stairway.length_z =
               draw_block_length.z;
            _block_editor_context.draw_block.stairway.rotation = rotation;
            _block_editor_context.draw_block.step = draw_block_step::stairway_height;
            _block_editor_context.draw_block.index =
               static_cast<uint32>(_world.blocks.custom.size());
            _block_editor_context.draw_block.block_id = id;

            const std::array<float3, 2> cornersWS{draw_block_start, draw_block_length};
            std::array<float3, 2> cornersLS{};

            for (std::size_t i = 0; i < cornersLS.size(); ++i) {
               cornersLS[i] = inv_rotation * cornersWS[i];
            }

            const float3 block_max = max(cornersLS[0], cornersLS[1]);
            const float3 block_min = min(cornersLS[0], cornersLS[1]);

            const float3 size = float3{std::fabs(block_max.x - block_min.x), 0.0f,
                                       std::fabs(block_max.z - block_min.z)} /
                                2.0f;

            const float3 position = (draw_block_start + draw_block_length) / 2.0f;

            const uint8 material_index = _block_editor_config.paint_material_index;

            if (_world.blocks.custom.size() < world::max_blocks) {
               _edit_stack_world.apply(
                  edits::make_add_block(
                     world::block_description_custom{
                        .rotation = rotation,
                        .position = position,
                        .mesh_description =
                           world::block_custom_mesh_description_stairway{
                              .size = size,
                              .step_height = _block_editor_config.step_height,
                              .first_step_offset = _block_editor_config.first_step_offset,
                           },
                        .surface_materials = {material_index, material_index, material_index,
                                              material_index, material_index},
                     },
                     _block_editor_config.new_block_layer, id),
                  _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Custom Blocks ({}) Reached", world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }
         }
      } break;
      case draw_block_step::stairway_height: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.stairway.start;
         const float3 draw_block_width =
            {_block_editor_context.draw_block.stairway.width_x, draw_block_start.y,
             _block_editor_context.draw_block.stairway.width_z};
         const float3 draw_block_length =
            {_block_editor_context.draw_block.stairway.length_x,
             draw_block_start.y, _block_editor_context.draw_block.stairway.length_z};

         quaternion rotation = _block_editor_context.draw_block.stairway.rotation;
         quaternion inv_rotation = conjugate(rotation);

         const std::array<float3, 2> cornersWS{draw_block_start, draw_block_length};
         std::array<float3, 2> cornersLS{};

         for (std::size_t i = 0; i < cornersLS.size(); ++i) {
            cornersLS[i] = inv_rotation * cornersWS[i];
         }

         const float3 block_max = max(cornersLS[0], cornersLS[1]);
         const float3 block_min = min(cornersLS[0], cornersLS[1]);

         const float4 height_plane =
            make_plane_from_point(draw_block_width,
                                  normalize(draw_block_width - _camera.position()));

         graphics::camera_ray ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = 0.0f;
             intersect_aabb(inv_rotation * ray.origin,
                            1.0f / (inv_rotation * ray.direction),
                            {.min = {block_min.x, -FLT_MAX, block_min.z},
                             .max = {block_max.x, FLT_MAX, block_max.z}},
                            FLT_MAX, hit) and
             hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = ray.origin + hit * ray.direction;
         }

         const float unaligned_stairway_height =
            cursor_position.y - draw_block_start.y;
         const float stairway_height =
            align
               ? std::round(unaligned_stairway_height / alignment.y) * alignment.y
               : unaligned_stairway_height;

         float3 position = {(draw_block_start.x + draw_block_length.x) / 2.0f,
                            draw_block_start.y,
                            (draw_block_start.z + draw_block_length.z) / 2.0f};
         const float3 size = abs(float3{block_max.x - block_min.x, stairway_height,
                                        block_max.z - block_min.z});

         if (stairway_height < 0.0f) {
            position.y += stairway_height;
         }
         else {
            rotation = normalize(rotation * quaternion{0.0f, 0.0f, 1.0f, 0.0f});
            inv_rotation = conjugate(rotation);
         }

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.custom.size() and
             _world.blocks.custom.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                       index, rotation, position,
                                       world::block_custom_mesh_description_stairway{
                                          .size = size,
                                          .step_height = _block_editor_config.step_height,
                                          .first_step_offset =
                                             _block_editor_config.first_step_offset,
                                       }),
                                    _edit_context, {.transparent = true});
         }

         _tool_visualizers.add_line_overlay(position,
                                            position +
                                               float3{0.0f, stairway_height * 0.5f, 0.0f},
                                            line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::stairway_floating_width: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.stairway.start,
                              {cursor_positionWS.x,
                               _block_editor_context.draw_block.stairway.start.y,
                               cursor_positionWS.z},
                              line_color);

         if (click) {
            _block_editor_context.draw_block.stairway.width_x =
               cursor_positionWS.x;
            _block_editor_context.draw_block.stairway.width_z =
               cursor_positionWS.z;
            _block_editor_context.draw_block.step =
               draw_block_step::stairway_floating_length;
         }

      } break;
      case draw_block_step::stairway_floating_length: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.stairway.start;
         const float3 draw_block_width =
            {_block_editor_context.draw_block.stairway.width_x, draw_block_start.y,
             _block_editor_context.draw_block.stairway.width_z};

         const float3 cursor_direction =
            normalize(cursor_positionWS - draw_block_width);

         const float3 extend_normal =
            normalize(float3{draw_block_width.z, 0.0f, draw_block_width.x} -
                      float3{draw_block_start.z, 0.0f, draw_block_start.x}) *
            float3{-1.0, 0.0f, 1.0};

         const float normal_sign =
            dot(cursor_direction, extend_normal) < 0.0f ? -1.0f : 1.0f;

         float rotation_angle = std::atan2(draw_block_start.x - draw_block_width.x,
                                           draw_block_start.z - draw_block_width.z);

         rotation_angle += std::numbers::pi_v<float> * 0.5f;

         if (normal_sign > 0.0f) rotation_angle += std::numbers::pi_v<float>;

         const quaternion rotation =
            make_quat_from_euler({0.0f, rotation_angle, 0.0f});
         const quaternion inv_rotation = conjugate(rotation);

         const float cursor_distance =
            std::fabs((inv_rotation * draw_block_width).z -
                      (inv_rotation * cursor_positionWS).z);

         const float3 draw_block_length =
            draw_block_width + extend_normal * cursor_distance * normal_sign;

         _tool_visualizers.add_line_overlay(draw_block_start, draw_block_width,
                                            line_color);
         _tool_visualizers.add_line_overlay(draw_block_width, draw_block_length,
                                            line_color);

         if (click) {
            const world::block_custom_id id = _world.blocks.next_id.custom.aquire();

            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.stairway.length_x =
               draw_block_length.x;
            _block_editor_context.draw_block.stairway.length_z =
               draw_block_length.z;
            _block_editor_context.draw_block.stairway.rotation = rotation;
            _block_editor_context.draw_block.step =
               draw_block_step::stairway_floating_height;
            _block_editor_context.draw_block.index =
               static_cast<uint32>(_world.blocks.custom.size());
            _block_editor_context.draw_block.block_id = id;

            const std::array<float3, 2> cornersWS{draw_block_start, draw_block_length};
            std::array<float3, 2> cornersLS{};

            for (std::size_t i = 0; i < cornersLS.size(); ++i) {
               cornersLS[i] = inv_rotation * cornersWS[i];
            }

            const float3 block_max = max(cornersLS[0], cornersLS[1]);
            const float3 block_min = min(cornersLS[0], cornersLS[1]);

            const float3 size = float3{std::fabs(block_max.x - block_min.x), 0.0f,
                                       std::fabs(block_max.z - block_min.z)} /
                                2.0f;

            const float3 position = (draw_block_start + draw_block_length) / 2.0f;

            const uint8 material_index = _block_editor_config.paint_material_index;

            if (_world.blocks.custom.size() < world::max_blocks) {
               _edit_stack_world.apply(
                  edits::make_add_block(
                     world::block_description_custom{
                        .rotation = rotation,
                        .position = position,
                        .mesh_description =
                           world::block_custom_mesh_description_stairway_floating{
                              .size = size,
                              .step_height = _block_editor_config.step_height,
                              .first_step_offset = _block_editor_config.first_step_offset,
                           },
                        .surface_materials = {material_index, material_index, material_index,
                                              material_index, material_index},
                     },
                     _block_editor_config.new_block_layer, id),
                  _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Custom Blocks ({}) Reached", world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }
         }
      } break;
      case draw_block_step::stairway_floating_height: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.stairway.start;
         const float3 draw_block_width =
            {_block_editor_context.draw_block.stairway.width_x, draw_block_start.y,
             _block_editor_context.draw_block.stairway.width_z};
         const float3 draw_block_length =
            {_block_editor_context.draw_block.stairway.length_x,
             draw_block_start.y, _block_editor_context.draw_block.stairway.length_z};

         quaternion rotation = _block_editor_context.draw_block.stairway.rotation;
         quaternion inv_rotation = conjugate(rotation);

         const std::array<float3, 2> cornersWS{draw_block_start, draw_block_length};
         std::array<float3, 2> cornersLS{};

         for (std::size_t i = 0; i < cornersLS.size(); ++i) {
            cornersLS[i] = inv_rotation * cornersWS[i];
         }

         const float3 block_max = max(cornersLS[0], cornersLS[1]);
         const float3 block_min = min(cornersLS[0], cornersLS[1]);

         const float4 height_plane =
            make_plane_from_point(draw_block_width,
                                  normalize(draw_block_width - _camera.position()));

         graphics::camera_ray ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = 0.0f;
             intersect_aabb(inv_rotation * ray.origin,
                            1.0f / (inv_rotation * ray.direction),
                            {.min = {block_min.x, -FLT_MAX, block_min.z},
                             .max = {block_max.x, FLT_MAX, block_max.z}},
                            FLT_MAX, hit) and
             hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = ray.origin + hit * ray.direction;
         }

         const float unaligned_stairway_height =
            cursor_position.y - draw_block_start.y;
         const float stairway_height =
            align
               ? std::round(unaligned_stairway_height / alignment.y) * alignment.y
               : unaligned_stairway_height;

         float3 position = {(draw_block_start.x + draw_block_length.x) / 2.0f,
                            draw_block_start.y,
                            (draw_block_start.z + draw_block_length.z) / 2.0f};
         const float3 size = abs(float3{block_max.x - block_min.x, stairway_height,
                                        block_max.z - block_min.z});

         if (stairway_height < 0.0f) {
            position.y += stairway_height;
         }
         else {
            rotation = normalize(rotation * quaternion{0.0f, 0.0f, 1.0f, 0.0f});
            inv_rotation = conjugate(rotation);
         }

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.custom.size() and
             _world.blocks.custom.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                       index, rotation, position,
                                       world::block_custom_mesh_description_stairway_floating{
                                          .size = size,
                                          .step_height = _block_editor_config.step_height,
                                          .first_step_offset =
                                             _block_editor_config.first_step_offset,
                                       }),
                                    _edit_context, {.transparent = true});
         }

         _tool_visualizers.add_line_overlay(position,
                                            position +
                                               float3{0.0f, stairway_height * 0.5f, 0.0f},
                                            line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::cone_radius: {
         const float3 draw_block_start = _block_editor_context.draw_block.cone.start;

         _tool_visualizers.add_line_overlay(draw_block_start,
                                            {cursor_positionWS.x,
                                             draw_block_start.y,
                                             cursor_positionWS.z},
                                            line_color);

         const float radius =
            distance(float2{draw_block_start.x, draw_block_start.z},
                     float2{cursor_positionWS.x, cursor_positionWS.z});

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.custom.size() and
             _world.blocks.custom.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world
               .apply(edits::make_set_block_custom_metrics(
                         index, {},
                         draw_block_start + float3{0.0f, alignment.y / 2.0f, 0.0f},
                         world::block_custom_mesh_description_cone{
                            .size = {radius, alignment.y / 2.0f, radius},
                            .segments = _block_editor_config.cone.segments,
                            .flat_shading = _block_editor_config.cone.flat_shading,
                         }),
                      _edit_context, {.transparent = true});
         }

         if (click) {
            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.cone.radius = radius;
            _block_editor_context.draw_block.step = draw_block_step::cone_height;
         }
      } break;
      case draw_block_step::cone_height: {
         const float3 draw_block_start = _block_editor_context.draw_block.cone.start;
         const float radius = _block_editor_context.draw_block.cone.radius;

         graphics::camera_ray rayWS =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = iCylinderInfinite(rayWS.origin, rayWS.direction, draw_block_start,
                                           float3{0.0f, 1.0f, 0.0f}, radius)
                            .x;
             hit >= 0.0f and hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = rayWS.origin + hit * rayWS.direction;
         }

         const float unaligned_cone_height =
            cursor_position.y - draw_block_start.y;
         const float cone_height =
            align ? std::round(unaligned_cone_height / alignment.y) * alignment.y
                  : unaligned_cone_height;

         const quaternion rotation = cone_height < 0.0f
                                        ? quaternion{0.0f, 1.0f, 0.0f, 0.0f}
                                        : quaternion{1.0f, 0.0f, 0.0f, 0.0f};
         const float3 position = {draw_block_start.x,
                                  draw_block_start.y + (cone_height / 2.0f),
                                  draw_block_start.z};
         const float3 size = abs(float3{radius, cone_height / 2.0f, radius});

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.custom.size() and
             _world.blocks.custom.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                       index, rotation, position,
                                       world::block_custom_mesh_description_cone{
                                          .size = size,
                                          .segments = _block_editor_config.cone.segments,
                                          .flat_shading =
                                             _block_editor_config.cone.flat_shading,
                                       }),
                                    _edit_context, {.transparent = true});
         }

         _tool_visualizers
            .add_line_overlay(position - float3{0.0f, cone_height * 0.5f, 0.0f},
                              position + float3{0.0f, cone_height * 0.5f, 0.0f},
                              line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::hemisphere_radius: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.hemisphere.start;

         _tool_visualizers.add_line_overlay(draw_block_start,
                                            {cursor_positionWS.x,
                                             draw_block_start.y,
                                             cursor_positionWS.z},
                                            line_color);

         const float radius =
            distance(float2{draw_block_start.x, draw_block_start.z},
                     float2{cursor_positionWS.x, cursor_positionWS.z});

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.hemispheres.size() and
             _world.blocks.hemispheres.ids[index] ==
                _block_editor_context.draw_block.block_id) {
            _edit_stack_world.apply(
               edits::make_set_block_hemisphere_metrics(index, {}, draw_block_start,
                                                        {radius, radius, radius}),
               _edit_context, {.transparent = true});
         }

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::pyramid_depth: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.pyramid.start,
                              {cursor_positionWS.x,
                               _block_editor_context.draw_block.pyramid.start.y,
                               cursor_positionWS.z},
                              line_color);

         if (click) {
            _block_editor_context.draw_block.pyramid.depth_x = cursor_positionWS.x;
            _block_editor_context.draw_block.pyramid.depth_z = cursor_positionWS.z;
            _block_editor_context.draw_block.step = draw_block_step::pyramid_width;
         }

      } break;
      case draw_block_step::pyramid_width: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.pyramid.start;
         const float3 draw_block_depth =
            {_block_editor_context.draw_block.pyramid.depth_x, draw_block_start.y,
             _block_editor_context.draw_block.pyramid.depth_z};

         const float3 cursor_direction =
            normalize(cursor_positionWS - draw_block_depth);

         const float3 extend_normal =
            normalize(float3{draw_block_depth.z, 0.0f, draw_block_depth.x} -
                      float3{draw_block_start.z, 0.0f, draw_block_start.x}) *
            float3{-1.0, 0.0f, 1.0};

         float rotation_angle = std::atan2(draw_block_start.x - draw_block_depth.x,
                                           draw_block_start.z - draw_block_depth.z);

         if (draw_block_start.z - draw_block_depth.z < 0.0f) {
            rotation_angle += std::numbers::pi_v<float>;
         }

         const quaternion rotation =
            make_quat_from_euler({0.0f, rotation_angle, 0.0f});
         const quaternion inv_rotation = conjugate(rotation);

         const float normal_sign =
            dot(cursor_direction, extend_normal) < 0.0f ? -1.0f : 1.0f;

         const float cursor_distance =
            std::fabs((inv_rotation * draw_block_depth).x -
                      (inv_rotation * cursor_positionWS).x);

         const float3 draw_block_width =
            draw_block_depth + extend_normal * cursor_distance * normal_sign;

         _tool_visualizers.add_line_overlay(draw_block_start, draw_block_depth,
                                            line_color);
         _tool_visualizers.add_line_overlay(draw_block_depth, draw_block_width,
                                            line_color);

         if (click) {
            const world::block_pyramid_id id =
               _world.blocks.next_id.pyramids.aquire();

            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.pyramid.width_x = draw_block_width.x;
            _block_editor_context.draw_block.pyramid.width_z = draw_block_width.z;
            _block_editor_context.draw_block.pyramid.rotation = rotation;
            _block_editor_context.draw_block.step = draw_block_step::pyramid_height;
            _block_editor_context.draw_block.index =
               static_cast<uint32>(_world.blocks.pyramids.size());
            _block_editor_context.draw_block.block_id = id;

            const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
            std::array<float3, 2> cornersLS{};

            for (std::size_t i = 0; i < cornersLS.size(); ++i) {
               cornersLS[i] = inv_rotation * cornersWS[i];
            }

            const float3 block_max = max(cornersLS[0], cornersLS[1]);
            const float3 block_min = min(cornersLS[0], cornersLS[1]);

            const float3 size = float3{std::fabs(block_max.x - block_min.x), 0.0f,
                                       std::fabs(block_max.z - block_min.z)} /
                                2.0f;

            const float3 position = (draw_block_start + draw_block_width) / 2.0f;

            const uint8 material_index = _block_editor_config.paint_material_index;

            if (_world.blocks.pyramids.size() < world::max_blocks) {
               _edit_stack_world.apply(edits::make_add_block(
                                          world::block_description_pyramid{
                                             .rotation = rotation,
                                             .position = position,
                                             .size = size,
                                             .surface_materials = {material_index, material_index,
                                                                   material_index, material_index,
                                                                   material_index},
                                          },
                                          _block_editor_config.new_block_layer, id),
                                       _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Pyramids ({}) Reached", world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }
         }
      } break;
      case draw_block_step::pyramid_height: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.pyramid.start;
         const float3 draw_block_depth =
            {_block_editor_context.draw_block.pyramid.depth_x, draw_block_start.y,
             _block_editor_context.draw_block.pyramid.depth_z};
         const float3 draw_block_width =
            {_block_editor_context.draw_block.pyramid.width_x, draw_block_start.y,
             _block_editor_context.draw_block.pyramid.width_z};

         const quaternion rotation = _block_editor_context.draw_block.pyramid.rotation;
         const quaternion inv_rotation = conjugate(rotation);

         const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
         std::array<float3, 2> cornersLS{};

         for (std::size_t i = 0; i < cornersLS.size(); ++i) {
            cornersLS[i] = inv_rotation * cornersWS[i];
         }

         const float3 block_max = max(cornersLS[0], cornersLS[1]);
         const float3 block_min = min(cornersLS[0], cornersLS[1]);

         const float4 height_plane =
            make_plane_from_point(draw_block_width,
                                  normalize(draw_block_width - _camera.position()));

         graphics::camera_ray ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = 0.0f;
             intersect_aabb(inv_rotation * ray.origin,
                            1.0f / (inv_rotation * ray.direction),
                            {.min = {block_min.x, -FLT_MAX, block_min.z},
                             .max = {block_max.x, FLT_MAX, block_max.z}},
                            FLT_MAX, hit) and
             hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = ray.origin + hit * ray.direction;
         }

         const float unaligned_pyramid_height =
            cursor_position.y - draw_block_start.y;
         const float pyramid_height =
            align
               ? std::round(unaligned_pyramid_height / alignment.y) * alignment.y
               : unaligned_pyramid_height;

         const float3 position = {(draw_block_start.x + draw_block_width.x) / 2.0f,
                                  draw_block_start.y + (pyramid_height / 2.0f),
                                  (draw_block_start.z + draw_block_width.z) / 2.0f};
         const float3 size = abs(float3{block_max.x - block_min.x, pyramid_height,
                                        block_max.z - block_min.z}) /
                             2.0f;

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.pyramids.size() and
             _world.blocks.pyramids.ids[index] ==
                _block_editor_context.draw_block.block_id) {
            _edit_stack_world.apply(edits::make_set_block_pyramid_metrics(index, rotation,
                                                                          position, size),
                                    _edit_context, {.transparent = true});
         }

         _tool_visualizers
            .add_line_overlay(position - float3{0.0f, pyramid_height * 0.5f, 0.0f},
                              position + float3{0.0f, pyramid_height * 0.5f, 0.0f},
                              line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::ring_inner_radius: {
         const float3 positionWS = _block_editor_context.draw_block.ring.start;

         const float inner_radius =
            distance(float2{positionWS.x, positionWS.z},
                     float2{cursor_positionWS.x, cursor_positionWS.z});

         if (click) {
            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.ring.inner_radius = inner_radius;
            _block_editor_context.draw_block.step = draw_block_step::ring_outer_radius;
         }

         const float pi2 = std::numbers::pi_v<float> * 2.0f;
         const int segments = _block_editor_config.ring.segments;

         for (int i = 0; i < segments; ++i) {
            const float segment_begin = (i / static_cast<float>(segments)) * pi2;
            const float segment_end = ((i + 1) / static_cast<float>(segments)) * pi2;

            const float3 beginLS = {
               cosf(segment_begin),
               0.0f,
               sinf(segment_begin),
            };
            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            _tool_visualizers.add_line_overlay(positionWS + beginLS * inner_radius,
                                               positionWS + endLS * inner_radius,
                                               line_color);
         }
      } break;
      case draw_block_step::ring_outer_radius: {
         const float3 positionWS = _block_editor_context.draw_block.ring.start;
         const float inner_radius = _block_editor_context.draw_block.ring.inner_radius;

         const float cursor_radius =
            distance(float2{positionWS.x, positionWS.z},
                     float2{cursor_positionWS.x, cursor_positionWS.z});
         const float outer_radius =
            std::max((cursor_radius - inner_radius) / 2.0f, 0.0f);

         if (click) {
            if (_world.blocks.custom.size() < world::max_blocks) {
               const uint8 material_index = _block_editor_config.paint_material_index;
               const world::block_custom_id id =
                  _world.blocks.next_id.custom.aquire();

               _block_editor_context.draw_block.index =
                  static_cast<uint32>(_world.blocks.custom.size());
               _block_editor_context.draw_block.block_id = id;

               _edit_stack_world.apply(
                  edits::make_add_block(
                     world::block_description_custom{
                        .position = positionWS,
                        .mesh_description =
                           world::block_custom_mesh_description_ring{
                              .inner_radius = inner_radius,
                              .outer_radius = outer_radius,
                              .height = 1.0f,
                              .segments = _block_editor_config.ring.segments,
                              .flat_shading = _block_editor_config.ring.flat_shading,
                              .texture_loops = _block_editor_config.ring.texture_loops,
                           },
                        .surface_materials = {material_index, material_index,
                                              material_index, material_index},
                        .surface_texture_mode = {world::block_texture_mode::world_space_auto,
                                                 world::block_texture_mode::world_space_auto,
                                                 world::block_texture_mode::unwrapped,
                                                 world::block_texture_mode::unwrapped}},
                     _block_editor_config.new_block_layer, id),
                  _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Custom Blocks ({}) Reached", world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }

            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.ring.outer_radius = outer_radius;
            _block_editor_context.draw_block.step = draw_block_step::ring_height;
         }

         const float pi2 = std::numbers::pi_v<float> * 2.0f;
         const float ring_radius = inner_radius + outer_radius * 2.0f;
         const int segments = _block_editor_config.ring.segments;

         for (int i = 0; i < segments; ++i) {
            const float segment_begin = (i / static_cast<float>(segments)) * pi2;
            const float segment_end = ((i + 1) / static_cast<float>(segments)) * pi2;

            const float3 beginLS = {
               cosf(segment_begin),
               0.0f,
               sinf(segment_begin),
            };
            const float3 endLS = {
               cosf(segment_end),
               0.0f,
               sinf(segment_end),
            };

            const float3 offsetWS = positionWS + float3{0.0f, 0.001f, 0.0f};

            const float3 v0 = endLS * inner_radius + offsetWS;
            const float3 v1 = endLS * ring_radius + offsetWS;
            const float3 v2 = beginLS * ring_radius + offsetWS;
            const float3 v3 = beginLS * inner_radius + offsetWS;

            _tool_visualizers.add_line_overlay(v0, v3, line_color);
            _tool_visualizers.add_line_overlay(v1, v2, line_color);

            _tool_visualizers.add_triangle_additive(v0, v1, v2,
                                                    line_color & 0x20'ff'ff'ff);
            _tool_visualizers.add_triangle_additive(v0, v2, v3,
                                                    line_color & 0x20'ff'ff'ff);
         }
      } break;
      case draw_block_step::ring_height: {
         const float3 draw_block_start = _block_editor_context.draw_block.ring.start;
         const float inner_radius = _block_editor_context.draw_block.ring.inner_radius;
         const float outer_radius = _block_editor_context.draw_block.ring.outer_radius;

         graphics::camera_ray rayWS =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = iCylinderInfinite(rayWS.origin, rayWS.direction,
                                           draw_block_start, float3{0.0f, 1.0f, 0.0f},
                                           inner_radius + outer_radius * 2.0f)
                            .x;
             hit >= 0.0f and hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = rayWS.origin + hit * rayWS.direction;
         }

         const float unaligned_ring_height =
            cursor_position.y - draw_block_start.y;
         const float ring_height =
            align ? std::round(unaligned_ring_height / alignment.y) * alignment.y
                  : unaligned_ring_height;

         const float3 position = {draw_block_start.x,
                                  draw_block_start.y + (ring_height / 2.0f),
                                  draw_block_start.z};

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.custom.size() and
             _world.blocks.custom.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world
               .apply(edits::make_set_block_custom_metrics(
                         index, {}, position,
                         world::block_custom_mesh_description_ring{
                            .inner_radius = inner_radius,
                            .outer_radius = outer_radius,
                            .height = ring_height / 2.0f,
                            .segments = _block_editor_config.ring.segments,
                            .flat_shading = _block_editor_config.ring.flat_shading,
                            .texture_loops = _block_editor_config.ring.texture_loops,
                         }),
                      _edit_context, {.transparent = true});
         }

         _tool_visualizers
            .add_line_overlay(position - float3{0.0f, ring_height * 0.5f, 0.0f},
                              position + float3{0.0f, ring_height * 0.5f, 0.0f},
                              line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::beveled_box_depth: {
         _tool_visualizers.add_line_overlay(
            _block_editor_context.draw_block.beveled_box.start,
            {cursor_positionWS.x,
             _block_editor_context.draw_block.beveled_box.start.y,
             cursor_positionWS.z},
            line_color);

         if (click) {
            _block_editor_context.draw_block.beveled_box.depth_x =
               cursor_positionWS.x;
            _block_editor_context.draw_block.beveled_box.depth_z =
               cursor_positionWS.z;
            _block_editor_context.draw_block.step = draw_block_step::beveled_box_width;
         }

      } break;
      case draw_block_step::beveled_box_width: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.beveled_box.start;
         const float3 draw_block_depth =
            {_block_editor_context.draw_block.beveled_box.depth_x,
             draw_block_start.y, _block_editor_context.draw_block.beveled_box.depth_z};

         const float3 cursor_direction =
            normalize(cursor_positionWS - draw_block_depth);

         const float3 extend_normal =
            normalize(float3{draw_block_depth.z, 0.0f, draw_block_depth.x} -
                      float3{draw_block_start.z, 0.0f, draw_block_start.x}) *
            float3{-1.0, 0.0f, 1.0};

         float rotation_angle = std::atan2(draw_block_start.x - draw_block_depth.x,
                                           draw_block_start.z - draw_block_depth.z);

         if (draw_block_start.z - draw_block_depth.z < 0.0f) {
            rotation_angle += std::numbers::pi_v<float>;
         }

         const quaternion rotation =
            make_quat_from_euler({0.0f, rotation_angle, 0.0f});
         const quaternion inv_rotation = conjugate(rotation);

         const float normal_sign =
            dot(cursor_direction, extend_normal) < 0.0f ? -1.0f : 1.0f;

         const float cursor_distance =
            std::fabs((inv_rotation * draw_block_depth).x -
                      (inv_rotation * cursor_positionWS).x);

         const float3 draw_block_width =
            draw_block_depth + extend_normal * cursor_distance * normal_sign;

         _tool_visualizers.add_line_overlay(draw_block_start, draw_block_depth,
                                            line_color);
         _tool_visualizers.add_line_overlay(draw_block_depth, draw_block_width,
                                            line_color);

         if (click) {
            const world::block_custom_id id = _world.blocks.next_id.custom.aquire();

            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.beveled_box.width_x =
               draw_block_width.x;
            _block_editor_context.draw_block.beveled_box.width_z =
               draw_block_width.z;
            _block_editor_context.draw_block.beveled_box.rotation = rotation;
            _block_editor_context.draw_block.step = draw_block_step::beveled_box_height;
            _block_editor_context.draw_block.index =
               static_cast<uint32>(_world.blocks.custom.size());
            _block_editor_context.draw_block.block_id = id;

            const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
            std::array<float3, 2> cornersLS{};

            for (std::size_t i = 0; i < cornersLS.size(); ++i) {
               cornersLS[i] = inv_rotation * cornersWS[i];
            }

            const float3 block_max = max(cornersLS[0], cornersLS[1]);
            const float3 block_min = min(cornersLS[0], cornersLS[1]);

            const float3 size = float3{std::fabs(block_max.x - block_min.x), 0.0f,
                                       std::fabs(block_max.z - block_min.z)} /
                                2.0f;

            const float3 position = (draw_block_start + draw_block_width) / 2.0f;

            const uint8 material_index = _block_editor_config.paint_material_index;

            if (_world.blocks.custom.size() < world::max_blocks) {
               _edit_stack_world.apply(
                  edits::make_add_block(
                     world::block_description_custom{
                        .rotation = rotation,
                        .position = position,

                        .mesh_description =
                           world::block_custom_mesh_description_beveled_box{
                              .size = size,
                              .amount = _block_editor_config.beveled_box.amount,
                              .bevel_top = _block_editor_config.beveled_box.bevel_top,
                              .bevel_sides = _block_editor_config.beveled_box.bevel_sides,
                              .bevel_bottom = _block_editor_config.beveled_box.bevel_bottom,
                           },
                        .surface_materials = {material_index, material_index,
                                              material_index, material_index,
                                              material_index, material_index},
                     },
                     _block_editor_config.new_block_layer, id),
                  _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Custom Blocks ({}) Reached", world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }
         }
      } break;
      case draw_block_step::beveled_box_height: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.beveled_box.start;
         const float3 draw_block_depth =
            {_block_editor_context.draw_block.beveled_box.depth_x,
             draw_block_start.y, _block_editor_context.draw_block.beveled_box.depth_z};
         const float3 draw_block_width =
            {_block_editor_context.draw_block.beveled_box.width_x,
             draw_block_start.y, _block_editor_context.draw_block.beveled_box.width_z};

         const quaternion rotation =
            _block_editor_context.draw_block.beveled_box.rotation;
         const quaternion inv_rotation = conjugate(rotation);

         const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
         std::array<float3, 2> cornersLS{};

         for (std::size_t i = 0; i < cornersLS.size(); ++i) {
            cornersLS[i] = inv_rotation * cornersWS[i];
         }

         const float3 block_max = max(cornersLS[0], cornersLS[1]);
         const float3 block_min = min(cornersLS[0], cornersLS[1]);

         const float4 height_plane =
            make_plane_from_point(draw_block_width,
                                  normalize(draw_block_width - _camera.position()));

         graphics::camera_ray ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = 0.0f;
             intersect_aabb(inv_rotation * ray.origin,
                            1.0f / (inv_rotation * ray.direction),
                            {.min = {block_min.x, -FLT_MAX, block_min.z},
                             .max = {block_max.x, FLT_MAX, block_max.z}},
                            FLT_MAX, hit) and
             hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = ray.origin + hit * ray.direction;
         }

         const float unaligned_beveled_box_height =
            cursor_position.y - draw_block_start.y;
         const float beveled_box_height =
            align ? std::round(unaligned_beveled_box_height / alignment.y) *
                       alignment.y
                  : unaligned_beveled_box_height;

         const float3 position = {(draw_block_start.x + draw_block_width.x) / 2.0f,
                                  draw_block_start.y + (beveled_box_height / 2.0f),
                                  (draw_block_start.z + draw_block_width.z) / 2.0f};
         const float3 size = abs(float3{block_max.x - block_min.x, beveled_box_height,
                                        block_max.z - block_min.z}) /
                             2.0f;

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.custom.size() and
             _world.blocks.custom.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world
               .apply(edits::make_set_block_custom_metrics(
                         index, rotation, position,
                         world::block_custom_mesh_description_beveled_box{
                            .size = size,
                            .amount = _block_editor_config.beveled_box.amount,
                            .bevel_top = _block_editor_config.beveled_box.bevel_top,
                            .bevel_sides = _block_editor_config.beveled_box.bevel_sides,
                            .bevel_bottom = _block_editor_config.beveled_box.bevel_bottom,
                         }),
                      _edit_context, {.transparent = true});
         }

         _tool_visualizers.add_line_overlay(
            position - float3{0.0f, beveled_box_height * 0.5f, 0.0f},
            position + float3{0.0f, beveled_box_height * 0.5f, 0.0f}, line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::curve_p3: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.curve.p0,
                              cursor_positionWS, line_color & 0x7f'ff'ff'ffu);

         if (click) {
            _block_editor_context.draw_block.curve.p3 = cursor_positionWS;
            _block_editor_context.draw_block.curve.p1 =
               _block_editor_context.draw_block.curve.p0;
            _block_editor_context.draw_block.curve.p2 =
               _block_editor_context.draw_block.curve.p3;
            _block_editor_context.draw_block.step = draw_block_step::curve_finalize;
         }
      } break;
      case draw_block_step::curve_finalize: {
         _gizmos.gizmo_position({.name = "Curve Finalize",
                                 .instance = 0,
                                 .alignment = alignment.x},
                                _block_editor_context.draw_block.curve.p1);
         _gizmos.gizmo_position({.name = "Curve Finalize",
                                 .instance = 1,
                                 .alignment = alignment.x},
                                _block_editor_context.draw_block.curve.p2);

         if (click) {
            const world::block_custom_id id = _world.blocks.next_id.custom.aquire();
            const uint8 material_index = _block_editor_config.paint_material_index;

            const float3 position = (_block_editor_context.draw_block.curve.p0 +
                                     _block_editor_context.draw_block.curve.p1 +
                                     _block_editor_context.draw_block.curve.p2 +
                                     _block_editor_context.draw_block.curve.p3) /
                                    4.0f;

            if (_world.blocks.custom.size() < world::max_blocks) {
               _edit_stack_world.apply(
                  edits::make_add_block(
                     world::block_description_custom{
                        .position = position,

                        .mesh_description =
                           world::block_custom_mesh_description_curve{
                              .width = _block_editor_config.curve.width,
                              .height = _block_editor_config.curve.height,
                              .segments = _block_editor_config.curve.segments,
                              .texture_loops = _block_editor_config.curve.texture_loops,
                              .p0 = _block_editor_context.draw_block.curve.p0 - position,
                              .p1 = _block_editor_context.draw_block.curve.p1 - position,
                              .p2 = _block_editor_context.draw_block.curve.p2 - position,
                              .p3 = _block_editor_context.draw_block.curve.p3 - position,
                           },
                        .surface_materials = {material_index, material_index,
                                              material_index, material_index,
                                              material_index, material_index},
                        .surface_texture_mode = {world::block_texture_mode::unwrapped,
                                                 world::block_texture_mode::unwrapped,
                                                 world::block_texture_mode::unwrapped,
                                                 world::block_texture_mode::unwrapped,
                                                 world::block_texture_mode::unwrapped,
                                                 world::block_texture_mode::unwrapped},
                     },
                     _block_editor_config.new_block_layer, id),
                  _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Custom Blocks ({}) Reached", world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }

            _block_editor_context.draw_block = {};
         }

         {

            const float3& p0 = _block_editor_context.draw_block.curve.p0;
            const float3& p1 = _block_editor_context.draw_block.curve.p1;
            const float3& p2 = _block_editor_context.draw_block.curve.p2;
            const float3& p3 = _block_editor_context.draw_block.curve.p3;

            const float segments_flt = _block_editor_config.curve.segments;
            const float height = _block_editor_config.curve.height;
            const float half_width = _block_editor_config.curve.width / 2.0f;

            for (int i = 0; i < _block_editor_config.curve.segments; ++i) {
               const float3 start = cubic_bezier(p0, p1, p2, p3, i / segments_flt);
               const float3 start_tangent =
                  cubic_bezier_tangent(p0, p1, p2, p3,
                                       std::max(std::min(i / segments_flt, 1.0f - FLT_EPSILON),
                                                FLT_EPSILON));
               const float3 start_normal =
                  cross(start_tangent, cross(float3{0.0f, 1.0f, 0.0f}, start_tangent));

               const float3 start_x_axis =
                  normalize(cross(start_tangent, start_normal));
               const float3 start_y_axis =
                  normalize(cross(start_x_axis, start_tangent));
               const float3 start_z_axis = start_tangent;

               const float3 start_top = start + start_y_axis * height;
               const float3 start_bottom = start;

               const float3 start_top_left = start_top - start_x_axis * half_width;
               const float3 start_top_right = start_top + start_x_axis * half_width;
               const float3 start_bottom_left = start_bottom - start_x_axis * half_width;
               const float3 start_bottom_right =
                  start_bottom + start_x_axis * half_width;

               const float3 end = cubic_bezier(p0, p1, p2, p3, (i + 1) / segments_flt);
               const float3 end_tangent =
                  cubic_bezier_tangent(p0, p1, p2, p3,
                                       std::max(std::min((i + 1) / segments_flt,
                                                         1.0f - FLT_EPSILON),
                                                FLT_EPSILON));
               const float3 end_normal =
                  cross(end_tangent, cross(float3{0.0f, 1.0f, 0.0f}, end_tangent));

               const float3 end_x_axis = normalize(cross(end_tangent, end_normal));
               const float3 end_y_axis = normalize(cross(end_x_axis, end_tangent));
               const float3 end_z_axis = end_tangent;

               const float3 end_top = end + end_y_axis * height;
               const float3 end_bottom = end;

               const float3 end_top_left = end_top - end_x_axis * half_width;
               const float3 end_top_right = end_top + end_x_axis * half_width;
               const float3 end_bottom_left = end_bottom - end_x_axis * half_width;
               const float3 end_bottom_right = end_bottom + end_x_axis * half_width;

               _tool_visualizers.add_line_overlay(start, end, line_color);
               _tool_visualizers.add_line(start_top_left, end_top_left,
                                          0x7f'ff'ff'ffu & line_color);
               _tool_visualizers.add_line(start_top_right, end_top_right,
                                          0x7f'ff'ff'ffu & line_color);
               _tool_visualizers.add_line(start_bottom_left, end_bottom_left,
                                          0x7f'ff'ff'ffu & line_color);
               _tool_visualizers.add_line(start_bottom_right, end_bottom_right,
                                          0x7f'ff'ff'ffu & line_color);

               _tool_visualizers.add_triangle_additive(start_top_left,
                                                       start_top_right, end_top_right,
                                                       0x3f'ff'ff'ffu & line_color);
               _tool_visualizers.add_triangle_additive(start_top_left,
                                                       end_top_right, end_top_left,
                                                       0x3f'ff'ff'ffu & line_color);

               _tool_visualizers.add_triangle_additive(end_bottom_right,
                                                       start_bottom_right,
                                                       start_bottom_left,
                                                       0x3f'ff'ff'ffu & line_color);
               _tool_visualizers.add_triangle_additive(end_bottom_left, end_bottom_right,
                                                       start_bottom_left,
                                                       0x3f'ff'ff'ffu & line_color);

               _tool_visualizers.add_triangle_additive(start_top_left, end_top_left,
                                                       end_bottom_left,
                                                       0x3f'ff'ff'ffu & line_color);
               _tool_visualizers.add_triangle_additive(start_top_left, end_bottom_left,
                                                       start_bottom_left,
                                                       0x3f'ff'ff'ffu & line_color);

               _tool_visualizers.add_triangle_additive(end_bottom_right,
                                                       end_top_right, start_top_right,
                                                       0x3f'ff'ff'ffu & line_color);
               _tool_visualizers.add_triangle_additive(start_bottom_right,
                                                       end_bottom_right, start_top_right,
                                                       0x3f'ff'ff'ffu & line_color);

               if (i == 0) {
                  _tool_visualizers.add_triangle_additive(start_bottom_right,
                                                          start_top_right, start_top_left,
                                                          0x3f'ff'ff'ffu & line_color);
                  _tool_visualizers.add_triangle_additive(start_bottom_left,
                                                          start_bottom_right,
                                                          start_top_left,
                                                          0x3f'ff'ff'ffu & line_color);

                  _tool_visualizers.add_line(start_top_left, start_top_right,
                                             0x7f'ff'ff'ffu & line_color);
                  _tool_visualizers.add_line(start_top_right, start_bottom_right,
                                             0x7f'ff'ff'ffu & line_color);
                  _tool_visualizers.add_line(start_bottom_right, start_bottom_left,
                                             0x7f'ff'ff'ffu & line_color);
                  _tool_visualizers.add_line(start_bottom_left, start_top_left,
                                             0x7f'ff'ff'ffu & line_color);
               }
               else if (i == (_block_editor_config.curve.segments - 1)) {
                  _tool_visualizers.add_triangle_additive(end_top_left, end_top_right,
                                                          end_bottom_right,
                                                          0x3f'ff'ff'ffu & line_color);
                  _tool_visualizers.add_triangle_additive(end_top_left, end_bottom_right,
                                                          end_bottom_left,
                                                          0x3f'ff'ff'ffu & line_color);

                  _tool_visualizers.add_line(end_top_left, end_top_right,
                                             0x7f'ff'ff'ffu & line_color);
                  _tool_visualizers.add_line(end_top_right, end_bottom_right,
                                             0x7f'ff'ff'ffu & line_color);
                  _tool_visualizers.add_line(end_bottom_right, end_bottom_left,
                                             0x7f'ff'ff'ffu & line_color);
                  _tool_visualizers.add_line(end_bottom_left, end_top_left,
                                             0x7f'ff'ff'ffu & line_color);
               }
            }

            _tool_visualizers
               .add_line_overlay(_block_editor_context.draw_block.curve.p0,
                                 _block_editor_context.draw_block.curve.p1,
                                 line_color);
            _tool_visualizers
               .add_line_overlay(_block_editor_context.draw_block.curve.p3,
                                 _block_editor_context.draw_block.curve.p2,
                                 line_color);
         }

      } break;
      case draw_block_step::arch_length: {
         _tool_visualizers
            .add_line_overlay(_block_editor_context.draw_block.arch.start,
                              {cursor_positionWS.x,
                               _block_editor_context.draw_block.arch.start.y,
                               cursor_positionWS.z},
                              line_color);

         if (click) {
            _block_editor_context.draw_block.arch.length_x = cursor_positionWS.x;
            _block_editor_context.draw_block.arch.length_z = cursor_positionWS.z;
            _block_editor_context.draw_block.step = draw_block_step::arch_depth;
         }

      } break;
      case draw_block_step::arch_depth: {
         const float3 draw_block_start = _block_editor_context.draw_block.arch.start;
         const float3 draw_block_length = {_block_editor_context.draw_block.arch.length_x,
                                           draw_block_start.y,
                                           _block_editor_context.draw_block.arch.length_z};

         const float3 cursor_direction =
            normalize(cursor_positionWS - draw_block_length);

         const float3 extend_normal =
            normalize(float3{draw_block_length.z, 0.0f, draw_block_length.x} -
                      float3{draw_block_start.z, 0.0f, draw_block_start.x}) *
            float3{-1.0, 0.0f, 1.0};

         const float normal_sign =
            dot(cursor_direction, extend_normal) < 0.0f ? -1.0f : 1.0f;

         float rotation_angle =
            std::atan2(draw_block_start.x - draw_block_length.x,
                       draw_block_start.z - draw_block_length.z);

         rotation_angle += std::numbers::pi_v<float> * 0.5f;

         if (normal_sign > 0.0f) rotation_angle += std::numbers::pi_v<float>;

         const quaternion rotation =
            make_quat_from_euler({0.0f, rotation_angle, 0.0f});
         const quaternion inv_rotation = conjugate(rotation);

         const float cursor_distance =
            std::fabs((inv_rotation * draw_block_length).z -
                      (inv_rotation * cursor_positionWS).z);

         const float3 draw_block_depth =
            draw_block_length + extend_normal * cursor_distance * normal_sign;

         _tool_visualizers.add_line_overlay(draw_block_start, draw_block_length,
                                            line_color);
         _tool_visualizers.add_line_overlay(draw_block_length, draw_block_depth,
                                            line_color);

         if (click) {
            const world::block_custom_id id = _world.blocks.next_id.custom.aquire();

            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.arch.depth_x = draw_block_depth.x;
            _block_editor_context.draw_block.arch.depth_z = draw_block_depth.z;
            _block_editor_context.draw_block.arch.rotation = rotation;
            _block_editor_context.draw_block.step = draw_block_step::arch_height;
            _block_editor_context.draw_block.index =
               static_cast<uint32>(_world.blocks.custom.size());
            _block_editor_context.draw_block.block_id = id;

            const std::array<float3, 2> cornersWS{draw_block_start, draw_block_depth};
            std::array<float3, 2> cornersLS{};

            for (std::size_t i = 0; i < cornersLS.size(); ++i) {
               cornersLS[i] = inv_rotation * cornersWS[i];
            }

            const float3 block_max = max(cornersLS[0], cornersLS[1]);
            const float3 block_min = min(cornersLS[0], cornersLS[1]);

            const float3 size = float3{std::fabs(block_max.x - block_min.x), 0.0f,
                                       std::fabs(block_max.z - block_min.z)} /
                                2.0f;

            const float3 position = (draw_block_start + draw_block_depth) / 2.0f;

            const uint8 material_index = _block_editor_config.paint_material_index;

            if (_world.blocks.custom.size() < world::max_blocks) {
               _edit_stack_world.apply(
                  edits::make_add_block(
                     world::block_description_custom{
                        .rotation = rotation,
                        .position = position,
                        .mesh_description =
                           world::block_custom_mesh_description_arch{
                              .size = size,
                              .crown_length = _block_editor_config.arch.crown_length,
                              .crown_height = _block_editor_config.arch.crown_height,
                              .curve_height = _block_editor_config.arch.curve_height,
                              .span_length = _block_editor_config.arch.span_length,
                              .segments = _block_editor_config.arch.segments,
                           },
                        .surface_materials = {material_index, material_index,
                                              material_index, material_index,
                                              material_index, material_index},
                        .surface_texture_mode =
                           {world::block_texture_mode::world_space_auto,
                            world::block_texture_mode::world_space_auto,
                            world::block_texture_mode::world_space_auto,
                            world::block_texture_mode::unwrapped,
                            world::block_texture_mode::world_space_auto,
                            world::block_texture_mode::world_space_auto}},
                     _block_editor_config.new_block_layer, id),
                  _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Custom Blocks ({}) Reached", world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }
         }
      } break;
      case draw_block_step::arch_height: {
         const float3 draw_block_start = _block_editor_context.draw_block.arch.start;
         const float3 draw_block_length = {_block_editor_context.draw_block.arch.length_x,
                                           draw_block_start.y,
                                           _block_editor_context.draw_block.arch.length_z};
         const float3 draw_block_depth = {_block_editor_context.draw_block.arch.depth_x,
                                          draw_block_start.y,
                                          _block_editor_context.draw_block.arch.depth_z};

         quaternion rotation = _block_editor_context.draw_block.arch.rotation;
         quaternion inv_rotation = conjugate(rotation);

         const std::array<float3, 2> cornersWS{draw_block_start, draw_block_depth};
         std::array<float3, 2> cornersLS{};

         for (std::size_t i = 0; i < cornersLS.size(); ++i) {
            cornersLS[i] = inv_rotation * cornersWS[i];
         }

         const float3 block_max = max(cornersLS[0], cornersLS[1]);
         const float3 block_min = min(cornersLS[0], cornersLS[1]);

         const float4 height_plane =
            make_plane_from_point(draw_block_length,
                                  normalize(draw_block_length - _camera.position()));

         graphics::camera_ray ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = 0.0f;
             intersect_aabb(inv_rotation * ray.origin,
                            1.0f / (inv_rotation * ray.direction),
                            {.min = {block_min.x, -FLT_MAX, block_min.z},
                             .max = {block_max.x, FLT_MAX, block_max.z}},
                            FLT_MAX, hit) and
             hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = ray.origin + hit * ray.direction;
         }

         const float unaligned_arch_height =
            cursor_position.y - draw_block_start.y;
         const float arch_height =
            align ? std::round(unaligned_arch_height / alignment.y) * alignment.y
                  : unaligned_arch_height;

         const float3 position = {(draw_block_start.x + draw_block_depth.x) / 2.0f,
                                  draw_block_start.y + (arch_height / 2.0f),
                                  (draw_block_start.z + draw_block_depth.z) / 2.0f};
         const float3 size = abs(float3{block_max.x - block_min.x, arch_height,
                                        block_max.z - block_min.z}) /
                             2.0f;

         if (arch_height < 0.0f) {
            rotation = normalize(rotation * quaternion{0.0f, 0.0f, 1.0f, 0.0f});
            inv_rotation = conjugate(rotation);
         }

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.custom.size() and
             _world.blocks.custom.ids[index] == _block_editor_context.draw_block.block_id) {
            _edit_stack_world
               .apply(edits::make_set_block_custom_metrics(
                         index, rotation, position,
                         world::block_custom_mesh_description_arch{
                            .size = size,
                            .crown_length = _block_editor_config.arch.crown_length,
                            .crown_height = _block_editor_config.arch.crown_height,
                            .curve_height = _block_editor_config.arch.curve_height,
                            .span_length = _block_editor_config.arch.span_length,
                            .segments = _block_editor_config.arch.segments,
                         }),
                      _edit_context, {.transparent = true});
         }

         _tool_visualizers
            .add_line_overlay(position - float3{0.0f, arch_height * 0.5f, 0.0f},
                              position + float3{0.0f, arch_height * 0.5f, 0.0f},
                              line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      case draw_block_step::terrain_cut_box_depth: {
         _tool_visualizers.add_line_overlay(
            _block_editor_context.draw_block.terrain_cut_box.start,
            {cursor_positionWS.x,
             _block_editor_context.draw_block.terrain_cut_box.start.y,
             cursor_positionWS.z},
            line_color);

         if (click) {
            _block_editor_context.draw_block.terrain_cut_box.depth_x =
               cursor_positionWS.x;
            _block_editor_context.draw_block.terrain_cut_box.depth_z =
               cursor_positionWS.z;
            _block_editor_context.draw_block.step =
               draw_block_step::terrain_cut_box_width;
         }

      } break;
      case draw_block_step::terrain_cut_box_width: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.terrain_cut_box.start;
         const float3 draw_block_depth =
            {_block_editor_context.draw_block.terrain_cut_box.depth_x,
             draw_block_start.y,
             _block_editor_context.draw_block.terrain_cut_box.depth_z};

         const float3 cursor_direction =
            normalize(cursor_positionWS - draw_block_depth);

         const float3 extend_normal =
            normalize(float3{draw_block_depth.z, 0.0f, draw_block_depth.x} -
                      float3{draw_block_start.z, 0.0f, draw_block_start.x}) *
            float3{-1.0, 0.0f, 1.0};

         float rotation_angle = std::atan2(draw_block_start.x - draw_block_depth.x,
                                           draw_block_start.z - draw_block_depth.z);

         if (draw_block_start.z - draw_block_depth.z < 0.0f) {
            rotation_angle += std::numbers::pi_v<float>;
         }

         const quaternion rotation =
            make_quat_from_euler({0.0f, rotation_angle, 0.0f});
         const quaternion inv_rotation = conjugate(rotation);

         const float normal_sign =
            dot(cursor_direction, extend_normal) < 0.0f ? -1.0f : 1.0f;

         const float cursor_distance =
            std::fabs((inv_rotation * draw_block_depth).x -
                      (inv_rotation * cursor_positionWS).x);

         const float3 draw_block_width =
            draw_block_depth + extend_normal * cursor_distance * normal_sign;

         _tool_visualizers.add_line_overlay(draw_block_start, draw_block_depth,
                                            line_color);
         _tool_visualizers.add_line_overlay(draw_block_depth, draw_block_width,
                                            line_color);

         if (click) {
            const world::block_terrain_cut_box_id id =
               _world.blocks.next_id.terrain_cut_boxes.aquire();

            _block_editor_context.draw_block.height_plane = std::nullopt;
            _block_editor_context.draw_block.terrain_cut_box.width_x =
               draw_block_width.x;
            _block_editor_context.draw_block.terrain_cut_box.width_z =
               draw_block_width.z;
            _block_editor_context.draw_block.terrain_cut_box.rotation = rotation;
            _block_editor_context.draw_block.step =
               draw_block_step::terrain_cut_box_height;
            _block_editor_context.draw_block.index =
               static_cast<uint32>(_world.blocks.terrain_cut_boxes.size());
            _block_editor_context.draw_block.block_id = id;

            const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
            std::array<float3, 2> cornersLS{};

            for (std::size_t i = 0; i < cornersLS.size(); ++i) {
               cornersLS[i] = inv_rotation * cornersWS[i];
            }

            const float3 block_max = max(cornersLS[0], cornersLS[1]);
            const float3 block_min = min(cornersLS[0], cornersLS[1]);

            const float3 size = float3{std::fabs(block_max.x - block_min.x), 2.0f,
                                       std::fabs(block_max.z - block_min.z)} /
                                2.0f;

            const float3 position = (draw_block_start + draw_block_width) / 2.0f;

            if (_world.blocks.custom.size() < world::max_blocks) {
               _edit_stack_world.apply(edits::make_add_block(
                                          world::block_description_terrain_cut_box{
                                             .rotation = rotation,
                                             .position = position,
                                             .size = size,
                                          },
                                          _block_editor_config.new_block_layer, id),
                                       _edit_context);
            }
            else {
               MessageBoxA(_window,
                           fmt::format("Max Terrain Cutter Blocks ({}) Reached",
                                       world::max_blocks)
                              .c_str(),
                           "Limit Reached", MB_OK);

               _block_editor_context.tool = block_edit_tool::none;
            }
         }
      } break;
      case draw_block_step::terrain_cut_box_height: {
         const float3 draw_block_start =
            _block_editor_context.draw_block.terrain_cut_box.start;
         const float3 draw_block_depth =
            {_block_editor_context.draw_block.terrain_cut_box.depth_x,
             draw_block_start.y,
             _block_editor_context.draw_block.terrain_cut_box.depth_z};
         const float3 draw_block_width =
            {_block_editor_context.draw_block.terrain_cut_box.width_x,
             draw_block_start.y,
             _block_editor_context.draw_block.terrain_cut_box.width_z};

         const quaternion rotation =
            _block_editor_context.draw_block.terrain_cut_box.rotation;
         const quaternion inv_rotation = conjugate(rotation);

         const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
         std::array<float3, 2> cornersLS{};

         for (std::size_t i = 0; i < cornersLS.size(); ++i) {
            cornersLS[i] = inv_rotation * cornersWS[i];
         }

         const float3 block_max = max(cornersLS[0], cornersLS[1]);
         const float3 block_min = min(cornersLS[0], cornersLS[1]);

         const float4 height_plane =
            make_plane_from_point(draw_block_width,
                                  normalize(draw_block_width - _camera.position()));

         graphics::camera_ray ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = cursor_positionWS;

         if (float hit = 0.0f;
             intersect_aabb(inv_rotation * ray.origin,
                            1.0f / (inv_rotation * ray.direction),
                            {.min = {block_min.x, -FLT_MAX, block_min.z},
                             .max = {block_max.x, FLT_MAX, block_max.z}},
                            FLT_MAX, hit) and
             hit < distance(_camera.position(), cursor_positionWS)) {
            cursor_position = ray.origin + hit * ray.direction;
         }

         const float unaligned_terrain_cut_box_height =
            cursor_position.y - draw_block_start.y;
         const float terrain_cut_box_height =
            align ? std::round(unaligned_terrain_cut_box_height / alignment.y) *
                       alignment.y
                  : unaligned_terrain_cut_box_height;

         const float3 position = {(draw_block_start.x + draw_block_width.x) / 2.0f,
                                  draw_block_start.y + (terrain_cut_box_height / 2.0f),
                                  (draw_block_start.z + draw_block_width.z) / 2.0f};
         const float3 size =
            abs(float3{block_max.x - block_min.x, terrain_cut_box_height,
                       block_max.z - block_min.z}) /
            2.0f;

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.terrain_cut_boxes.size() and
             _world.blocks.terrain_cut_boxes.ids[index] ==
                _block_editor_context.draw_block.block_id) {
            _edit_stack_world.apply(edits::make_set_block_terrain_cut_box_metrics(
                                       index, rotation, position, size),
                                    _edit_context, {.transparent = true});
         }

         _tool_visualizers.add_line_overlay(
            position - float3{0.0f, terrain_cut_box_height * 0.5f, 0.0f},
            position + float3{0.0f, terrain_cut_box_height * 0.5f, 0.0f}, line_color);

         if (click) {
            _block_editor_context.draw_block = {};

            _edit_stack_world.close_last();
         }
      } break;
      }
   }
   else if (_block_editor_context.tool == block_edit_tool::rotate_texture) {
      const bool click = std::exchange(_block_editor_context.tool_click, false);

      const graphics::camera_ray rayWS =
         make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                         {ImGui::GetMainViewport()->Size.x,
                          ImGui::GetMainViewport()->Size.y});

      if (std::optional<world::raycast_block_result> hit =
             world::raycast(rayWS.origin, rayWS.direction, _world_layers_hit_mask,
                            _world.blocks, _world_blocks_bvh_library);
          hit) {
         if (click) {
            world::block_texture_rotation new_rotation =
               world::get_block_surface_texture_rotation(_world.blocks,
                                                         hit->id.type(), hit->index,
                                                         hit->surface_index);

            switch (new_rotation) {
            case world::block_texture_rotation::d0:
               new_rotation = world::block_texture_rotation::d90;
               break;
            case world::block_texture_rotation::d90:
               new_rotation = world::block_texture_rotation::d180;
               break;
            case world::block_texture_rotation::d180:
               new_rotation = world::block_texture_rotation::d270;
               break;
            case world::block_texture_rotation::d270:
               new_rotation = world::block_texture_rotation::d0;
               break;
            }

            _edit_stack_world.apply(
               edits::make_set_block_surface(
                  &world::get_block_surface_texture_rotation(_world.blocks,
                                                             hit->id.type(), hit->index,
                                                             hit->surface_index),
                  new_rotation, hit->index,
                  &world::get_dirty_tracker(_world.blocks, hit->id.type())),
               _edit_context, {.closed = true});
         }

         world::highlight_surface(_world.blocks, hit->id.type(), hit->index,
                                  hit->surface_index, _tool_visualizers);
      }
   }
   else if (_block_editor_context.tool == block_edit_tool::scale_texture) {
      const bool click_enlarge =
         std::exchange(_block_editor_context.tool_click, false);
      const bool click_shrink =
         std::exchange(_block_editor_context.tool_ctrl_click, false);

      const graphics::camera_ray rayWS =
         make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                         {ImGui::GetMainViewport()->Size.x,
                          ImGui::GetMainViewport()->Size.y});

      if (std::optional<world::raycast_block_result> hit =
             world::raycast(rayWS.origin, rayWS.direction, _world_layers_hit_mask,
                            _world.blocks, _world_blocks_bvh_library);
          hit) {
         if (click_enlarge or click_shrink) {
            std::array<int8, 2> new_scale =
               world::get_block_surface_texture_scale(_world.blocks,
                                                      hit->id.type(), hit->index,
                                                      hit->surface_index);

            if (click_shrink) {
               if (_block_editor_config.scale_texture_u) new_scale[0] += 1;
               if (_block_editor_config.scale_texture_v) new_scale[1] += 1;
            }
            else {
               if (_block_editor_config.scale_texture_u) new_scale[0] -= 1;
               if (_block_editor_config.scale_texture_v) new_scale[1] -= 1;
            }

            new_scale[0] = std::clamp(new_scale[0], world::block_min_texture_scale,
                                      world::block_max_texture_scale);
            new_scale[1] = std::clamp(new_scale[1], world::block_min_texture_scale,
                                      world::block_max_texture_scale);

            _edit_stack_world.apply(
               edits::make_set_block_surface(
                  &world::get_block_surface_texture_scale(_world.blocks,
                                                          hit->id.type(), hit->index,
                                                          hit->surface_index),
                  new_scale, hit->index,
                  &world::get_dirty_tracker(_world.blocks, hit->id.type())),
               _edit_context, {.closed = true});
         }

         world::highlight_surface(_world.blocks, hit->id.type(), hit->index,
                                  hit->surface_index, _tool_visualizers);
      }
   }
   else if (_block_editor_context.tool == block_edit_tool::paint_material) {
      const bool click = std::exchange(_block_editor_context.tool_click, false);

      const graphics::camera_ray rayWS =
         make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                         {ImGui::GetMainViewport()->Size.x,
                          ImGui::GetMainViewport()->Size.y});

      if (std::optional<world::raycast_block_result> hit =
             world::raycast(rayWS.origin, rayWS.direction, _world_layers_hit_mask,
                            _world.blocks, _world_blocks_bvh_library);
          hit) {
         if (click) {
            _edit_stack_world.apply(
               edits::make_set_block_surface(
                  &world::get_block_surface_material(_world.blocks, hit->id.type(),
                                                     hit->index, hit->surface_index),
                  _block_editor_config.paint_material_index, hit->index,
                  &world::get_dirty_tracker(_world.blocks, hit->id.type())),
               _edit_context, {.closed = true});
         }

         world::highlight_surface(_world.blocks, hit->id.type(), hit->index,
                                  hit->surface_index, _tool_visualizers);
      }
   }
   else if (_block_editor_context.tool == block_edit_tool::set_texture_mode) {
      const bool click = std::exchange(_block_editor_context.tool_click, false);

      const graphics::camera_ray rayWS =
         make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                         {ImGui::GetMainViewport()->Size.x,
                          ImGui::GetMainViewport()->Size.y});

      if (std::optional<world::raycast_block_result> hit =
             world::raycast(rayWS.origin, rayWS.direction, _world_layers_hit_mask,
                            _world.blocks, _world_blocks_bvh_library);
          hit) {
         if (click) {
            _edit_stack_world.apply(
               edits::make_set_block_surface(
                  &world::get_block_surface_texture_mode(_world.blocks, hit->id.type(),
                                                         hit->index, hit->surface_index),
                  _block_editor_config.texture_mode, hit->index,
                  &world::get_dirty_tracker(_world.blocks, hit->id.type())),
               _edit_context, {.closed = true});
         }

         world::highlight_surface(_world.blocks, hit->id.type(), hit->index,
                                  hit->surface_index, _tool_visualizers);
      }
   }
   else if (_block_editor_context.tool == block_edit_tool::offset_texture) {
      const bool click = std::exchange(_block_editor_context.tool_click, false);

      if (not ImGui::GetIO().WantCaptureMouse) {
         const graphics::camera_ray rayWS =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         if (std::optional<world::raycast_block_result> hit =
                world::raycast(rayWS.origin, rayWS.direction, _world_layers_hit_mask,
                               _world.blocks, _world_blocks_bvh_library);
             hit) {
            if (click) {
               _block_editor_context.offset_texture.block_id = hit->id;
               _block_editor_context.offset_texture.surface_index = hit->surface_index;
            }

            world::highlight_surface(_world.blocks, hit->id.type(), hit->index,
                                     hit->surface_index, _tool_visualizers);
         }
         else if (click) {
            _block_editor_context.offset_texture.block_id = world::block_id::none;
            _block_editor_context.offset_texture.surface_index = 0;
         }
      }

      if (std::optional<uint32> selected_index =
             world::find_block(_world.blocks,
                               _block_editor_context.offset_texture.block_id);
          selected_index) {
         ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);

         if (ImGui::Begin("Blocks Texture Offset", nullptr,
                          ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize)) {
            std::array<std::uint16_t, 2>& texture_offset = world::get_block_surface_texture_offset(
               _world.blocks, _block_editor_context.offset_texture.block_id.type(),
               *selected_index, _block_editor_context.offset_texture.surface_index);

            float u = texture_offset[0] / 8192.0f;
            float v = texture_offset[1] / 8192.0f;

            ImGui::BeginGroup();

            ImGui::DragFloat("U Normalized Offset", &u, 1.0f / 128.0f, 0.0f,
                             1.0f, nullptr,
                             ImGuiSliderFlags_AlwaysClamp |
                                ImGuiSliderFlags_NoRoundToFormat);

            ImGui::DragFloat("V Normalized Offset", &v, 1.0f / 128.0f, 0.0f,
                             1.0f, nullptr,
                             ImGuiSliderFlags_AlwaysClamp |
                                ImGuiSliderFlags_NoRoundToFormat);

            ImGui::EndGroup();

            if (ImGui::IsItemEdited()) {
               _edit_stack_world.apply(
                  edits::make_set_block_surface(
                     &texture_offset,
                     std::array{static_cast<uint16>(u * 8192.0f),
                                static_cast<uint16>(v * 8192.0f)},
                     *selected_index,
                     &world::get_dirty_tracker(_world.blocks,
                                               _block_editor_context
                                                  .offset_texture.block_id.type())),
                  _edit_context);
            }

            if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();
         }

         ImGui::End();
      }
   }
   else if (_block_editor_context.tool == block_edit_tool::tweak_block) {
      const bool click = std::exchange(_block_editor_context.tool_click, false);

      const graphics::camera_ray rayWS =
         make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                         {ImGui::GetMainViewport()->Size.x,
                          ImGui::GetMainViewport()->Size.y});

      if (not _gizmos.want_capture_mouse()) {
         if (std::optional<world::raycast_block_result> hit =
                world::raycast(rayWS.origin, rayWS.direction, _world_layers_hit_mask,
                               _world.blocks, _world_blocks_bvh_library);
             hit) {
            if (click) _block_editor_context.resize_block.block_id = hit->id;

            world::highlight_block(_world.blocks, hit->id.type(), hit->index,
                                   _tool_visualizers);
         }
         else if (click) {
            _block_editor_context.resize_block.block_id = world::block_id::none;
         }
      }

      if (std::optional<uint32> selected_index =
             world::find_block(_world.blocks, _block_editor_context.resize_block.block_id);
          selected_index) {
         switch (_block_editor_context.resize_block.block_id.type()) {
         case world::block_type::box: {
            const world::block_description_box& box =
               _world.blocks.boxes.description[*selected_index];

            float3 size = box.size;
            float3 positionWS = box.position;

            if (_gizmos.gizmo_size({.name = "Resize Block (Box)",
                                    .instance = *selected_index,
                                    .alignment = _editor_grid_size,
                                    .gizmo_rotation = box.rotation},
                                   positionWS, size)) {
               _edit_stack_world.apply(edits::make_set_block_box_metrics(*selected_index,
                                                                         box.rotation, positionWS,
                                                                         size),
                                       _edit_context);
            }
         } break;
         case world::block_type::ramp: {
            const world::block_description_ramp& ramp =
               _world.blocks.ramps.description[*selected_index];

            float3 size = ramp.size;
            float3 positionWS = ramp.position;

            if (_gizmos.gizmo_size({.name = "Resize Block (Ramp)",
                                    .instance = *selected_index,
                                    .alignment = _editor_grid_size,
                                    .gizmo_rotation = ramp.rotation},
                                   positionWS, size)) {
               _edit_stack_world.apply(edits::make_set_block_ramp_metrics(*selected_index,
                                                                          ramp.rotation, positionWS,
                                                                          size),
                                       _edit_context);
            }
         } break;
         case world::block_type::quad: {
            const world::block_description_quad& quad =
               _world.blocks.quads.description[*selected_index];

            std::array<float3, 4> new_vertices = quad.vertices;

            bool edited = false;

            edited |= _gizmos.gizmo_position(
               {
                  .name = "Resize Block (Quad, V0)",
                  .instance = static_cast<int64>(_world.blocks.quads.ids[*selected_index]),
                  .alignment = _editor_grid_size,
               },
               new_vertices[0]);
            edited |= _gizmos.gizmo_position(
               {
                  .name = "Resize Block (Quad, V1)",
                  .instance = static_cast<int64>(_world.blocks.quads.ids[*selected_index]),
                  .alignment = _editor_grid_size,
               },
               new_vertices[1]);
            edited |= _gizmos.gizmo_position(
               {
                  .name = "Resize Block (Quad, V2)",
                  .instance = static_cast<int64>(_world.blocks.quads.ids[*selected_index]),
                  .alignment = _editor_grid_size,
               },
               new_vertices[2]);
            edited |= _gizmos.gizmo_position(
               {
                  .name = "Resize Block (Quad, V3)",
                  .instance = static_cast<int64>(_world.blocks.quads.ids[*selected_index]),
                  .alignment = _editor_grid_size,
               },
               new_vertices[3]);

            if (edited) {
               _edit_stack_world.apply(edits::make_set_block_quad_metrics(*selected_index,
                                                                          new_vertices),
                                       _edit_context);
            }
         } break;
         case world::block_type::custom: {
            const world::block_description_custom& block =
               _world.blocks.custom.description[*selected_index];

            switch (block.mesh_description.type) {
            case world::block_custom_mesh_type::stairway: {
               const world::block_custom_mesh_description_stairway& stairway =
                  block.mesh_description.stairway;

               float3 size = stairway.size;
               size.y += stairway.first_step_offset;
               size /= 2.0f;

               float3 positionWS =
                  block.rotation * (conjugate(block.rotation) * block.position +
                                    float3{0.0f, size.y, 0.0f});

               if (_gizmos.gizmo_size({.name = "Resize Block (Stairway)",
                                       .instance = *selected_index,
                                       .alignment = _editor_grid_size,
                                       .gizmo_rotation = block.rotation},
                                      positionWS, size)) {
                  positionWS = block.rotation * (conjugate(block.rotation) * positionWS -
                                                 float3{0.0f, size.y, 0.0f});
                  size *= 2.0f;
                  size.y -= stairway.first_step_offset;

                  _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                             *selected_index, block.rotation, positionWS,
                                             world::block_custom_mesh_description_stairway{
                                                .size = size,
                                                .step_height = stairway.step_height,
                                                .first_step_offset = stairway.first_step_offset,
                                             }),
                                          _edit_context);
               }
            } break;
            case world::block_custom_mesh_type::stairway_floating: {
               const world::block_custom_mesh_description_stairway_floating& stairway =
                  block.mesh_description.stairway_floating;

               float3 size = stairway.size;
               size.y += stairway.first_step_offset;
               size /= 2.0f;

               float3 positionWS =
                  block.rotation * (conjugate(block.rotation) * block.position +
                                    float3{0.0f, size.y, 0.0f});

               if (_gizmos.gizmo_size({.name =
                                          "Resize Block (Floating Stairway)",
                                       .instance = *selected_index,
                                       .alignment = _editor_grid_size,
                                       .gizmo_rotation = block.rotation},
                                      positionWS, size)) {
                  positionWS = block.rotation * (conjugate(block.rotation) * positionWS -
                                                 float3{0.0f, size.y, 0.0f});
                  size *= 2.0f;
                  size.y -= stairway.first_step_offset;

                  _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                             *selected_index, block.rotation, positionWS,
                                             world::block_custom_mesh_description_stairway_floating{
                                                .size = size,
                                                .step_height = stairway.step_height,
                                                .first_step_offset = stairway.first_step_offset,
                                             }),
                                          _edit_context);
               }
            } break;
            case world::block_custom_mesh_type::ring: {
               const world::block_custom_mesh_description_ring& ring =
                  block.mesh_description.ring;

               float3 positionWS = block.position;

               float new_inner_radius = ring.inner_radius;
               float new_outer_radius = ring.outer_radius;
               float new_height = ring.height;

               if (_gizmos.gizmo_ring_size(
                      {
                         .name = "Resize Block (Ring)",
                         .alignment = _editor_grid_size,
                         .gizmo_rotation = block.rotation,
                      },
                      positionWS, new_inner_radius, new_outer_radius, new_height)) {
                  _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                             *selected_index, block.rotation, positionWS,
                                             world::block_custom_mesh_description_ring{
                                                .inner_radius = new_inner_radius,
                                                .outer_radius = new_outer_radius,
                                                .height = new_height,
                                                .segments = ring.segments,
                                                .flat_shading = ring.flat_shading,
                                             }),
                                          _edit_context);
               }
            } break;
            case world::block_custom_mesh_type::beveled_box: {
               const world::block_custom_mesh_description_beveled_box& box =
                  block.mesh_description.beveled_box;

               float3 size = box.size;
               float3 positionWS = block.position;

               if (_gizmos.gizmo_size(
                      {
                         .name = "Resize Block (Beveled Box)",
                         .alignment = _editor_grid_size,
                         .gizmo_rotation = block.rotation,
                      },
                      positionWS, size)) {
                  world::block_custom_mesh_description_beveled_box new_box = box;

                  new_box.size = size;

                  _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                             *selected_index, block.rotation,
                                             positionWS, new_box),
                                          _edit_context);
               }
            } break;
            case world::block_custom_mesh_type::curve: {
               const world::block_custom_mesh_description_curve& curve =
                  block.mesh_description.curve;

               const int64 gizmo_instance =
                  static_cast<int64>(_world.blocks.custom.ids[*selected_index]);

               float3 p0WS = block.rotation * curve.p0 + block.position;
               float3 p1WS = block.rotation * curve.p1 + block.position;
               float3 p2WS = block.rotation * curve.p2 + block.position;
               float3 p3WS = block.rotation * curve.p3 + block.position;

               bool edited = false;

               edited |= _gizmos.gizmo_position(
                  {
                     .name = "Resize Block (Curve), P0",
                     .instance = gizmo_instance,
                     .alignment = _editor_grid_size,
                  },
                  p0WS);
               edited |= _gizmos.gizmo_position(
                  {
                     .name = "Resize Block (Curve), P1",
                     .instance = gizmo_instance,
                     .alignment = _editor_grid_size,
                  },
                  p1WS);
               edited |= _gizmos.gizmo_position(
                  {
                     .name = "Resize Block (Curve), P2",
                     .instance = gizmo_instance,
                     .alignment = _editor_grid_size,
                  },
                  p2WS);
               edited |= _gizmos.gizmo_position(
                  {
                     .name = "Resize Block (Curve), P3",
                     .instance = gizmo_instance,
                     .alignment = _editor_grid_size,
                  },
                  p3WS);

               _tool_visualizers.add_line(p0WS, p1WS, 0x7f'ff'ff'ff);
               _tool_visualizers.add_line(p1WS, p2WS, 0x7f'ff'ff'ff);
               _tool_visualizers.add_line(p2WS, p3WS, 0x7f'ff'ff'ff);
               _tool_visualizers.add_line(p0WS, p1WS, 0xff'ff'ff'ff);

               if (edited) {
                  world::block_custom_mesh_description_curve new_curve = curve;

                  new_curve.p0 = conjugate(block.rotation) * (p0WS - block.position);
                  new_curve.p1 = conjugate(block.rotation) * (p1WS - block.position);
                  new_curve.p2 = conjugate(block.rotation) * (p2WS - block.position);
                  new_curve.p3 = conjugate(block.rotation) * (p3WS - block.position);

                  _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                             *selected_index, block.rotation,
                                             block.position, new_curve),
                                          _edit_context);
               }
            } break;
            case world::block_custom_mesh_type::cylinder: {
               const world::block_custom_mesh_description_cylinder& cylinder =
                  block.mesh_description.cylinder;

               float3 size = cylinder.size;
               float3 positionWS = block.position;

               if (_gizmos.gizmo_size(
                      {
                         .name = "Resize Block (Cylinder)",
                         .alignment = _editor_grid_size,
                         .gizmo_rotation = block.rotation,
                      },
                      positionWS, size)) {
                  world::block_custom_mesh_description_cylinder new_cylinder = cylinder;

                  new_cylinder.size = size;

                  _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                             *selected_index, block.rotation,
                                             positionWS, new_cylinder),
                                          _edit_context);
               }
            } break;
            case world::block_custom_mesh_type::cone: {
               const world::block_custom_mesh_description_cone& cone =
                  block.mesh_description.cone;

               float3 size = cone.size;
               float3 positionWS = block.position;

               if (_gizmos.gizmo_size(
                      {
                         .name = "Resize Block (Cone)",
                         .alignment = _editor_grid_size,
                         .gizmo_rotation = block.rotation,
                      },
                      positionWS, size)) {
                  world::block_custom_mesh_description_cone new_cone = cone;

                  new_cone.size = size;

                  _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                             *selected_index, block.rotation,
                                             positionWS, new_cone),
                                          _edit_context);
               }
            } break;
            case world::block_custom_mesh_type::arch: {
               const world::block_custom_mesh_description_arch& arch =
                  block.mesh_description.arch;

               float3 size = arch.size;
               float3 positionWS = block.position;

               if (_gizmos.gizmo_size(
                      {
                         .name = "Resize Block (Arch)",
                         .alignment = _editor_grid_size,
                         .gizmo_rotation = block.rotation,
                      },
                      positionWS, size)) {
                  world::block_custom_mesh_description_arch new_arch = arch;

                  new_arch.size = size;

                  _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                             *selected_index, block.rotation,
                                             positionWS, new_arch),
                                          _edit_context);
               }
            } break;
            }

         } break;
         case world::block_type::hemisphere: {
            const world::block_description_hemisphere& hemisphere =
               _world.blocks.hemispheres.description[*selected_index];

            float3 size = hemisphere.size;
            size.y /= 2.0f;

            float3 positionWS = hemisphere.rotation *
                                (conjugate(hemisphere.rotation) * hemisphere.position +
                                 float3{0.0f, size.y, 0.0f});

            if (_gizmos.gizmo_size({.name = "Resize Block (Hemisphere)",
                                    .instance = *selected_index,
                                    .alignment = _editor_grid_size,
                                    .gizmo_rotation = hemisphere.rotation},
                                   positionWS, size)) {
               positionWS = hemisphere.rotation *
                            (conjugate(hemisphere.rotation) * positionWS -
                             float3{0.0f, size.y, 0.0f});
               size.y *= 2.0f;

               _edit_stack_world.apply(edits::make_set_block_hemisphere_metrics(
                                          *selected_index, hemisphere.rotation,
                                          positionWS, size),
                                       _edit_context);
            }
         } break;
         case world::block_type::pyramid: {
            const world::block_description_pyramid& pyramid =
               _world.blocks.pyramids.description[*selected_index];

            float3 size = pyramid.size;
            float3 positionWS = pyramid.position;

            if (_gizmos.gizmo_size({.name = "Resize Block (Pyramid)",
                                    .instance = *selected_index,
                                    .alignment = _editor_grid_size,
                                    .gizmo_rotation = pyramid.rotation},
                                   positionWS, size)) {
               _edit_stack_world
                  .apply(edits::make_set_block_pyramid_metrics(*selected_index,
                                                               pyramid.rotation,
                                                               positionWS, size),
                         _edit_context);
            }
         } break;
         case world::block_type::terrain_cut_box: {
            const world::block_description_terrain_cut_box& terrain_cut_box =
               _world.blocks.terrain_cut_boxes.description[*selected_index];

            float3 size = terrain_cut_box.size;
            float3 positionWS = terrain_cut_box.position;

            if (_gizmos.gizmo_size({.name = "Resize Block (Terrain Cut Box)",
                                    .instance = *selected_index,
                                    .alignment = _editor_grid_size,
                                    .gizmo_rotation = terrain_cut_box.rotation},
                                   positionWS, size)) {
               _edit_stack_world.apply(edits::make_set_block_terrain_cut_box_metrics(
                                          *selected_index, terrain_cut_box.rotation,
                                          positionWS, size),
                                       _edit_context);
            }
         } break;
         }

         if (_gizmos.can_close_last_edit()) _edit_stack_world.close_last();

         ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);

         if (ImGui::Begin("Tweak Block", nullptr,
                          ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize)) {
            switch (_block_editor_context.resize_block.block_id.type()) {
            case world::block_type::box: {
               world::block_description_box box =
                  _world.blocks.boxes.description[*selected_index];

               ImGui::BeginGroup();

               ImGui::DragQuat("Rotation", &box.rotation);
               ImGui::DragFloat3("Position", &box.position);
               ImGui::DragFloat3("Size", &box.size, 1.0f, 0.0f, 1e10f);

               ImGui::EndGroup();

               if (ImGui::IsItemEdited()) {
                  _edit_stack_world.apply(edits::make_set_block_box_metrics(*selected_index,
                                                                            box.rotation,
                                                                            box.position,
                                                                            box.size),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();
            } break;
            case world::block_type::ramp: {
               world::block_description_ramp ramp =
                  _world.blocks.ramps.description[*selected_index];

               ImGui::BeginGroup();

               ImGui::DragQuat("Rotation", &ramp.rotation);
               ImGui::DragFloat3("Position", &ramp.position);
               ImGui::DragFloat3("Size", &ramp.size, 1.0f, 0.0f, 1e10f);

               ImGui::EndGroup();

               if (ImGui::IsItemEdited()) {
                  _edit_stack_world.apply(edits::make_set_block_ramp_metrics(
                                             *selected_index, ramp.rotation,
                                             ramp.position, ramp.size),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();
            } break;
            case world::block_type::quad: {
               world::block_description_quad quad =
                  _world.blocks.quads.description[*selected_index];

               ImGui::BeginGroup();

               ImGui::DragFloat3("V0", &quad.vertices[0]);
               ImGui::DragFloat3("V1", &quad.vertices[1]);
               ImGui::DragFloat3("V2", &quad.vertices[2]);
               ImGui::DragFloat3("V3", &quad.vertices[3]);

               ImGui::EndGroup();

               if (ImGui::IsItemEdited()) {
                  _edit_stack_world
                     .apply(edits::make_set_block_quad_metrics(*selected_index,
                                                               quad.vertices),
                            _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();
            } break;
            case world::block_type::custom: {
               world::block_description_custom block =
                  _world.blocks.custom.description[*selected_index];

               bool checkbox_clicked = false;

               ImGui::BeginGroup();

               ImGui::DragQuat("Rotation", &block.rotation);
               ImGui::DragFloat3("Position", &block.position);

               switch (block.mesh_description.type) {
               case world::block_custom_mesh_type::stairway: {
                  world::block_custom_mesh_description_stairway& stairway =
                     block.mesh_description.stairway;

                  ImGui::DragFloat3("Size", &stairway.size, 1.0f,
                                    stairway.step_height, 1e10f);

                  ImGui::Separator();

                  ImGui::DragFloat("Step Height", &stairway.step_height,
                                   0.125f * 0.25f, 0.125f, 1.0f, "%.3f",
                                   ImGuiSliderFlags_NoRoundToFormat);
                  ImGui::DragFloat("First Step Offset",
                                   &stairway.first_step_offset, 0.125f);

                  stairway.size.y = std::max(stairway.size.y, stairway.step_height);
               } break;
               case world::block_custom_mesh_type::stairway_floating: {
                  world::block_custom_mesh_description_stairway_floating& stairway =
                     block.mesh_description.stairway_floating;

                  ImGui::DragFloat3("Size", &stairway.size, 1.0f,
                                    stairway.step_height, 1e10f);

                  ImGui::Separator();

                  ImGui::DragFloat("Step Height", &stairway.step_height,
                                   0.125f * 0.25f, 0.125f, 1.0f, "%.3f",
                                   ImGuiSliderFlags_NoRoundToFormat);
                  ImGui::DragFloat("First Step Offset",
                                   &stairway.first_step_offset, 0.125f);

                  stairway.size.y = std::max(stairway.size.y, stairway.step_height);
               } break;
               case world::block_custom_mesh_type::ring: {
                  world::block_custom_mesh_description_ring& ring =
                     block.mesh_description.ring;

                  const uint16 min_segments = 3;
                  const uint16 max_segments = 256;

                  ImGui::Separator();

                  ImGui::DragFloat("Inner Radius", &ring.inner_radius, 1.0f,
                                   0.0f, 1e10f);
                  ImGui::DragFloat("Outer Radius", &ring.outer_radius, 1.0f,
                                   0.0f, 1e10f);
                  ImGui::DragFloat("Height", &ring.height, 1.0f, 0.0f, 1e10f);

                  ImGui::Separator();

                  ImGui::SliderScalar("Segments", ImGuiDataType_U16, &ring.segments,
                                      &min_segments, &max_segments);
                  checkbox_clicked |=
                     ImGui::Checkbox("Flat Shading", &ring.flat_shading);
                  ImGui::DragFloat("Texture Loops", &ring.texture_loops);

                  ImGui::SetItemTooltip(
                     "How many times the texture wraps around the ring in the "
                     "Unwrapped Texture Mode.");
               } break;
               case world::block_custom_mesh_type::beveled_box: {
                  world::block_custom_mesh_description_beveled_box& box =
                     block.mesh_description.beveled_box;

                  ImGui::DragFloat3("Size", &box.size, 1.0f, 0.0f, 1e10f);

                  ImGui::Separator();

                  ImGui::DragFloat("Bevel Amount", &box.amount, 0.0625f, 0.0f,
                                   std::min(std::min(box.size.x, box.size.y),
                                            box.size.z));
                  checkbox_clicked |= ImGui::Checkbox("Bevel Top", &box.bevel_top);
                  checkbox_clicked |= ImGui::Checkbox("Bevel Sides", &box.bevel_sides);
                  checkbox_clicked |=
                     ImGui::Checkbox("Bevel Bottom", &box.bevel_bottom);
               } break;
               case world::block_custom_mesh_type::curve: {
                  world::block_custom_mesh_description_curve& curve =
                     block.mesh_description.curve;

                  const uint16 min_segments = 3;
                  const uint16 max_segments = 256;

                  ImGui::Separator();

                  ImGui::DragFloat3("P0", &curve.p0);
                  ImGui::DragFloat3("P1", &curve.p1);
                  ImGui::DragFloat3("P2", &curve.p2);
                  ImGui::DragFloat3("P3", &curve.p3);

                  ImGui::Separator();

                  ImGui::DragFloat("Width", &curve.width, 1.0f, 0.0f, 1e10f);
                  ImGui::DragFloat("Height", &curve.height, 1.0f, 0.0f, 1e10f);
                  ImGui::SliderScalar("Segments", ImGuiDataType_U16, &curve.segments,
                                      &min_segments, &max_segments);
                  ImGui::DragFloat("Texture Loops", &curve.texture_loops);

                  curve.segments = std::max(curve.segments, min_segments);
               } break;
               case world::block_custom_mesh_type::cylinder: {
                  world::block_custom_mesh_description_cylinder& cylinder =
                     block.mesh_description.cylinder;

                  ImGui::DragFloat3("Size", &cylinder.size, 1.0f, 0.0f, 1e10f);

                  ImGui::Separator();

                  const uint16 min_segments = 3;
                  const uint16 max_segments = 256;

                  ImGui::SliderScalar("Segments", ImGuiDataType_U16, &cylinder.segments,
                                      &min_segments, &max_segments);
                  checkbox_clicked |=
                     ImGui::Checkbox("Flat Shading", &cylinder.flat_shading);
                  ImGui::DragFloat("Texture Loops", &cylinder.texture_loops);

                  cylinder.segments = std::max(cylinder.segments, min_segments);
               } break;
               case world::block_custom_mesh_type::cone: {
                  world::block_custom_mesh_description_cone& cone =
                     block.mesh_description.cone;

                  const uint16 min_segments = 3;
                  const uint16 max_segments = 256;

                  ImGui::DragFloat3("Size", &cone.size, 1.0f, 0.0f, 1e10f);

                  ImGui::Separator();

                  ImGui::SliderScalar("Segments", ImGuiDataType_U16, &cone.segments,
                                      &min_segments, &max_segments);
                  checkbox_clicked |=
                     ImGui::Checkbox("Flat Shading", &cone.flat_shading);

                  cone.segments = std::max(cone.segments, min_segments);
               } break;
               case world::block_custom_mesh_type::arch: {
                  world::block_custom_mesh_description_arch& arch =
                     block.mesh_description.arch;

                  const uint16 min_segments = 1;
                  const uint16 max_segments = 64;

                  ImGui::DragFloat3("Size", &arch.size, 1.0f, 0.0f, 1e10f);

                  ImGui::Separator();

                  ImGui::DragFloat("Crown Length", &arch.crown_length, 0.5f,
                                   0.0f, arch.span_length);
                  ImGui::DragFloat("Crown Height", &arch.crown_height, 0.125f,
                                   0.0f, arch.size.y * 2.0f - arch.curve_height);
                  ImGui::DragFloat("Curve Height", &arch.curve_height, 0.5f,
                                   0.0f, arch.size.y * 2.0f);
                  ImGui::DragFloat("Span Length", &arch.span_length, 0.5f, 0.0f,
                                   arch.size.x * 2.0f);
                  ImGui::SliderScalar("Segments", ImGuiDataType_U16, &arch.segments,
                                      &min_segments, &max_segments);

                  arch.segments = std::max(arch.segments, min_segments);
               } break;
               }

               ImGui::EndGroup();

               if (ImGui::IsItemEdited() or checkbox_clicked) {
                  _edit_stack_world.apply(edits::make_set_block_custom_metrics(
                                             *selected_index, block.rotation,
                                             block.position, block.mesh_description),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated() or checkbox_clicked) {
                  _edit_stack_world.close_last();
               }
            } break;
            case world::block_type::hemisphere: {
               world::block_description_hemisphere hemisphere =
                  _world.blocks.hemispheres.description[*selected_index];

               ImGui::BeginGroup();

               ImGui::DragQuat("Rotation", &hemisphere.rotation);
               ImGui::DragFloat3("Position", &hemisphere.position);
               ImGui::DragFloat3("Size", &hemisphere.size, 1.0f, 0.0f, 1e10f);

               ImGui::EndGroup();

               if (ImGui::IsItemEdited()) {
                  _edit_stack_world.apply(edits::make_set_block_hemisphere_metrics(
                                             *selected_index, hemisphere.rotation,
                                             hemisphere.position, hemisphere.size),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();
            } break;
            case world::block_type::pyramid: {
               world::block_description_pyramid pyramid =
                  _world.blocks.pyramids.description[*selected_index];

               ImGui::BeginGroup();

               ImGui::DragQuat("Rotation", &pyramid.rotation);
               ImGui::DragFloat3("Position", &pyramid.position);
               ImGui::DragFloat3("Size", &pyramid.size, 1.0f, 0.0f, 1e10f);

               ImGui::EndGroup();

               if (ImGui::IsItemEdited()) {
                  _edit_stack_world.apply(edits::make_set_block_hemisphere_metrics(
                                             *selected_index, pyramid.rotation,
                                             pyramid.position, pyramid.size),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();
            } break;
            case world::block_type::terrain_cut_box: {
               world::block_description_terrain_cut_box terrain_cut_box =
                  _world.blocks.terrain_cut_boxes.description[*selected_index];
               ;

               ImGui::BeginGroup();

               ImGui::DragQuat("Rotation", &terrain_cut_box.rotation);
               ImGui::DragFloat3("Position", &terrain_cut_box.position);
               ImGui::DragFloat3("Size", &terrain_cut_box.size, 1.0f, 0.0f, 1e10f);

               ImGui::EndGroup();

               if (ImGui::IsItemEdited()) {
                  _edit_stack_world.apply(edits::make_set_block_terrain_cut_box_metrics(
                                             *selected_index, terrain_cut_box.rotation,
                                             terrain_cut_box.position,
                                             terrain_cut_box.size),
                                          _edit_context);
               }

               if (ImGui::IsItemDeactivated()) _edit_stack_world.close_last();
            } break;
            }
         }

         ImGui::End();
      }
   }
}
}