#include "world_edit.hpp"

#include "edits/add_block.hpp"
#include "edits/set_block.hpp"

#include "math/intersectors.hpp"
#include "math/plane_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "utility/srgb_conversion.hpp"

#include "world/blocks/highlight_surface.hpp"
#include "world/blocks/raycast.hpp"

#include <algorithm>
#include <numbers>

#include <imgui.h>

using namespace std::literals;

namespace we {

namespace {

constexpr std::array<const char*, 13> alignment_names =
   {"0.0625", "0.125", "0.25", "0.5",  "1.0",   "2.0",  "4.0",
    "8.0",    "16.0",  "32.0", "64.0", "128.0", "256.0"};

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

   bool selection_open = true;

   if (ImGui::Begin("Blocks", &_block_editor_open, ImGuiWindowFlags_NoCollapse)) {
      ImGui::SeparatorText("Block Tools");

      if (ImGui::Button("Draw Block", {ImGui::CalcItemWidth(), 0.0f})) {
         _block_editor_context.activate_tool = block_edit_tool::draw;
      }

      ImGui::BeginGroup();

      _block_editor_config.xz_alignment_exponent =
         std::clamp(_block_editor_config.xz_alignment_exponent, -4, 8);
      _block_editor_config.y_alignment_exponent =
         std::clamp(_block_editor_config.y_alignment_exponent, -4, 8);

      ImGui::SliderInt("Draw XZ Alignment",
                       &_block_editor_config.xz_alignment_exponent, -4, 8,
                       alignment_names[_block_editor_config.xz_alignment_exponent + 4],
                       ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);

      ImGui::SliderInt("Draw Y Alignment",
                       &_block_editor_config.y_alignment_exponent, -4, 8,
                       alignment_names[_block_editor_config.y_alignment_exponent + 4],
                       ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);

      ImGui::EndGroup();

      ImGui::SetItemTooltip(
         "Hold Ctrl when drawing blocks to enable snapping to alignment.");

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
   }

   ImGui::End();

   if (not selection_open) _interaction_targets.selection.clear();

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Block Editing");

      ImGui::Text("Draw Block");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Block Editing", "Draw Block")));

      ImGui::Text("Rotate Texture");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Block Editing", "Rotate Texture")));

      ImGui::Text("Scale Texture");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Block Editing", "Scale Texture")));

      ImGui::End();
   }

   switch (std::exchange(_block_editor_context.activate_tool, block_edit_tool::none)) {
   case block_edit_tool::none: {
   } break;
   case block_edit_tool::draw: {
      _block_editor_context.tool = block_edit_tool::draw;
      _block_editor_context.tool_click = false;
      _block_editor_context.draw_block = {};
   } break;
   case block_edit_tool::rotate_texture: {
      _block_editor_context.tool_click = false;
      _block_editor_context.tool = block_edit_tool::rotate_texture;
   } break;
   case block_edit_tool::scale_texture: {
      _block_editor_context.tool_click = false;
      _block_editor_context.tool = block_edit_tool::scale_texture;
   } break;
   }

   if (_block_editor_context.tool == block_edit_tool::draw) {
      const bool click = std::exchange(_block_editor_context.tool_click, false);
      const bool align = (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) or
                          ImGui::IsKeyDown(ImGuiKey_RightCtrl)) and
                         not ImGui::GetIO().WantCaptureKeyboard;
      float3 cursor_positionWS = _cursor_positionWS;

      if (_block_editor_context.draw_block.step != draw_block_step::start) {
         if (cursor_positionWS.y < _block_editor_context.draw_block.height) {
            const graphics::camera_ray rayWS =
               make_camera_ray(_camera,
                               {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                               {ImGui::GetMainViewport()->Size.x,
                                ImGui::GetMainViewport()->Size.y});

            const float4 draw_planeWS =
               make_plane_from_point({0.0f, _block_editor_context.draw_block.height, 0.0f},
                                     float3{0.0f, 1.0f, 0.0f});

            if (float hit = intersect_plane(rayWS.origin, rayWS.direction, draw_planeWS);
                hit > 0.0f) {
               cursor_positionWS = rayWS.origin + rayWS.direction * hit;
            }
         }
      }

      const float3 alignment =
         {std::exp2f(static_cast<float>(_block_editor_config.xz_alignment_exponent)),
          std::exp2f(static_cast<float>(_block_editor_config.y_alignment_exponent)),
          std::exp2f(static_cast<float>(_block_editor_config.xz_alignment_exponent))};
      const float3 aligned_cursorWS =
         align ? round(cursor_positionWS / alignment) * alignment : cursor_positionWS;

      const uint32 line_color =
         utility::pack_srgb_bgra({_settings.graphics.creation_color, 1.0f});

      const world::tool_visualizers_mini_grid xz_grid_desc = {
         .positionWS = aligned_cursorWS,
         .size = alignment.x,
         .divisions = 3.0f,
         .color = float3{1.0f, 1.0f, 1.0f},
      };

      switch (_block_editor_context.draw_block.step) {
      case draw_block_step::start: {
         if (align) {
            _tool_visualizers.add_line(aligned_cursorWS - float3{alignment.x, 0.0f, 0.0f},
                                       aligned_cursorWS + float3{alignment.x, 0.0f, 0.0f},
                                       line_color);
            _tool_visualizers.add_line(aligned_cursorWS - float3{0.0f, alignment.y, 0.0f},
                                       aligned_cursorWS + float3{0.0f, alignment.y, 0.0f},
                                       line_color);
            _tool_visualizers.add_line(aligned_cursorWS -
                                          float3{0.0f, 0.0f, alignment.z},
                                       aligned_cursorWS +
                                          float3{0.0f, 0.0f, alignment.z},
                                       line_color);
         }

         _tool_visualizers.add_mini_grid(xz_grid_desc);

         if (click) {
            _block_editor_context.draw_block.height = aligned_cursorWS.y;
            _block_editor_context.draw_block.start = aligned_cursorWS;
            _block_editor_context.draw_block.step = draw_block_step::box_depth;
         }
      } break;
      case draw_block_step::box_depth: {
         _tool_visualizers.add_line_overlay(_block_editor_context.draw_block.start,
                                            aligned_cursorWS, line_color);
         _tool_visualizers.add_mini_grid(xz_grid_desc);

         if (click) {
            _block_editor_context.draw_block.depth = aligned_cursorWS;
            _block_editor_context.draw_block.step = draw_block_step::box_width;
         }

      } break;
      case draw_block_step::box_width: {
         const float3 draw_block_start = _block_editor_context.draw_block.start;
         const float3 draw_block_depth = _block_editor_context.draw_block.depth;

         const float3 cursor_direction =
            normalize(aligned_cursorWS - draw_block_depth);

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
                      (inv_rotation * aligned_cursorWS).x);

         const float3 draw_block_width =
            draw_block_depth + extend_normal * cursor_distance * normal_sign;

         _tool_visualizers.add_line_overlay(draw_block_start, draw_block_depth,
                                            line_color);
         _tool_visualizers.add_line_overlay(draw_block_depth, draw_block_width,
                                            line_color);
         _tool_visualizers.add_mini_grid(xz_grid_desc);

         if (click) {
            const world::block_box_id id = _world.blocks.next_id.boxes.aquire();

            _block_editor_context.draw_block.width = draw_block_width;
            _block_editor_context.draw_block.rotation_angle = rotation_angle;
            _block_editor_context.draw_block.step = draw_block_step::box_height;
            _block_editor_context.draw_block.index =
               static_cast<uint32>(_world.blocks.boxes.size());
            _block_editor_context.draw_block.box_id = id;

            const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
            std::array<float3, 2> cornersOS{};

            for (std::size_t i = 0; i < cornersOS.size(); ++i) {
               cornersOS[i] = inv_rotation * cornersWS[i];
            }

            const float3 block_max = max(cornersOS[0], cornersOS[1]);
            const float3 block_min = min(cornersOS[0], cornersOS[1]);

            const float3 size = float3{std::fabs(block_max.x - block_min.x), 0.0f,
                                       std::fabs(block_max.z - block_min.z)} /
                                2.0f;

            const float3 position = (draw_block_start + draw_block_width) / 2.0f;

            _edit_stack_world.apply(
               edits::make_add_block(world::block_description_box{.rotation = rotation,
                                                                  .position = position,
                                                                  .size = size},
                                     id),
               _edit_context);
         }
      } break;
      case draw_block_step::box_height: {
         const float3 draw_block_start = _block_editor_context.draw_block.start;
         const float3 draw_block_depth = _block_editor_context.draw_block.depth;
         const float3 draw_block_width = _block_editor_context.draw_block.width;
         const float draw_block_rotation_angle =
            _block_editor_context.draw_block.rotation_angle;

         const quaternion rotation =
            make_quat_from_euler({0.0f, draw_block_rotation_angle, 0.0f});
         const quaternion inv_rotation = conjugate(rotation);

         const std::array<float3, 2> cornersWS{draw_block_start, draw_block_width};
         std::array<float3, 2> cornersOS{};

         for (std::size_t i = 0; i < cornersOS.size(); ++i) {
            cornersOS[i] = inv_rotation * cornersWS[i];
         }

         const float3 block_max = max(cornersOS[0], cornersOS[1]);
         const float3 block_min = min(cornersOS[0], cornersOS[1]);

         const float4 height_plane =
            make_plane_from_point(draw_block_width,
                                  normalize(draw_block_width - _camera.position()));

         graphics::camera_ray ray =
            make_camera_ray(_camera,
                            {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                            {ImGui::GetMainViewport()->Size.x,
                             ImGui::GetMainViewport()->Size.y});

         float3 cursor_position = aligned_cursorWS;

         if (float hit = 0.0f;
             intersect_aabb(inv_rotation * ray.origin,
                            1.0f / (inv_rotation * ray.direction),
                            {.min = {block_min.x, draw_block_start.y, block_min.z},
                             .max = {block_max.x, FLT_MAX, block_max.z}},
                            FLT_MAX, hit) and
             hit < distance(_camera.position(), aligned_cursorWS)) {
            cursor_position = ray.origin + hit * ray.direction;
         }

         const float unaligned_box_height =
            std::max(cursor_position.y - draw_block_width.y, 0.0f);
         const float box_height =
            align ? std::round(unaligned_box_height / alignment.y) * alignment.y
                  : unaligned_box_height;
         const float3 draw_block_height =
            draw_block_width + float3{0.0f, box_height, 0.0f};

         const float3 position = (draw_block_start + draw_block_height) / 2.0f;

         const float3 size = float3{std::fabs(block_max.x - block_min.x), box_height,
                                    std::fabs(block_max.z - block_min.z)} /
                             2.0f;

         if (const uint32 index = _block_editor_context.draw_block.index;
             index < _world.blocks.boxes.size() and
             _world.blocks.boxes.ids[index] == _block_editor_context.draw_block.box_id) {
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
      }
   }
   else if (_block_editor_context.tool == block_edit_tool::rotate_texture) {
      const bool click = std::exchange(_block_editor_context.tool_click, false);

      const graphics::camera_ray rayWS =
         make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                         {ImGui::GetMainViewport()->Size.x,
                          ImGui::GetMainViewport()->Size.y});

      if (std::optional<world::raycast_block_result> hit =
             world::raycast(rayWS.origin, rayWS.direction, _world.blocks.boxes);
          hit) {
         if (click) {
            world::block_texture_rotation new_rotation = {};

            switch (_world.blocks.boxes.description[hit->index]
                       .surface_texture_rotation[hit->surface_index]) {
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

            _edit_stack_world
               .apply(edits::make_set_block_surface(
                         &_world.blocks.boxes.description[hit->index]
                             .surface_texture_rotation[hit->surface_index],
                         new_rotation, hit->index, &_world.blocks.boxes.dirty),
                      _edit_context, {.closed = true});
         }

         world::highlight_surface(_world.blocks.boxes.description[hit->index],
                                  hit->surface_index, _tool_visualizers);
      }
   }
   else if (_block_editor_context.tool == block_edit_tool::scale_texture) {
      const bool click = std::exchange(_block_editor_context.tool_click, false);
      const bool shrink = (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) or
                           ImGui::IsKeyDown(ImGuiKey_RightCtrl)) and
                          not ImGui::GetIO().WantCaptureKeyboard;

      const graphics::camera_ray rayWS =
         make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                         {ImGui::GetMainViewport()->Size.x,
                          ImGui::GetMainViewport()->Size.y});

      if (std::optional<world::raycast_block_result> hit =
             world::raycast(rayWS.origin, rayWS.direction, _world.blocks.boxes);
          hit) {
         if (click) {
            std::array<int8, 2> new_scale =
               _world.blocks.boxes.description[hit->index].surface_texture_scale[hit->surface_index];

            if (shrink) {
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

            _edit_stack_world
               .apply(edits::make_set_block_surface(
                         &_world.blocks.boxes.description[hit->index]
                             .surface_texture_scale[hit->surface_index],
                         new_scale, hit->index, &_world.blocks.boxes.dirty),
                      _edit_context, {.closed = true});
         }

         world::highlight_surface(_world.blocks.boxes.description[hit->index],
                                  hit->surface_index, _tool_visualizers);
      }
   }
}

}