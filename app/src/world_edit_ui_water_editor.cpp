#include "world_edit.hpp"

#include "edits/set_terrain_area.hpp"
#include "math/vector_funcs.hpp"

#include <imgui.h>

namespace we {

void world_edit::ui_show_water_editor() noexcept
{
   const int32 water_map_length = _world.terrain.length / 4;

   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 620.0f * _display_scale},
                                       {std::numeric_limits<float>::max(),
                                        620.0f * _display_scale});

   if (ImGui::Begin("Water Editor", &_water_editor_open)) {
      ImGui::SeparatorText("Brush Mode");

      if (ImGui::Selectable("Paint", _water_editor_config.brush_mode ==
                                        water_brush_mode::paint)) {
         _water_editor_config.brush_mode = water_brush_mode::paint;
      }

      if (ImGui::Selectable("Erase", _water_editor_config.brush_mode ==
                                        water_brush_mode::erase)) {
         _water_editor_config.brush_mode = water_brush_mode::erase;
      }

      ImGui::SeparatorText("Brush Settings");

      if (int32 size = std::max(_water_editor_config.brush_size_x,
                                _water_editor_config.brush_size_y);
          ImGui::SliderInt("Size", &size, 0, water_map_length / 2, "%d",
                           ImGuiSliderFlags_AlwaysClamp)) {
         _water_editor_config.brush_size_x = size;
         _water_editor_config.brush_size_y = size;
      }

      ImGui::SliderInt("Size (X)", &_water_editor_config.brush_size_x, 0,
                       water_map_length / 2, "%d", ImGuiSliderFlags_AlwaysClamp);

      ImGui::SliderInt("Size (Y)", &_water_editor_config.brush_size_y, 0,
                       water_map_length / 2, "%d", ImGuiSliderFlags_AlwaysClamp);
   }

   ImGui::End();

   if (not _water_editor_open) return;

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Water Editing");

      ImGui::Text("Cycle Brush Mode");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Water Editing", "Cycle Brush Mode")));

      ImGui::Text("Increase Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Water Editing", "Increase Brush Size")));

      ImGui::Text("Decrease Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Decrease Brush Size")));

      ImGui::End();
   }

   graphics::camera_ray ray =
      make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                      {ImGui::GetMainViewport()->Size.x,
                       ImGui::GetMainViewport()->Size.y});

   const float hit_distance = -(dot(ray.origin, float3{0.0f, 1.0f, 0.0f}) -
                                _world.terrain.water_settings.height) /
                              dot(ray.direction, float3{0.0f, 1.0f, 0.0f});

   if (hit_distance < 0.0f) return;

   const float3 cursor_positionWS = ray.origin + ray.direction * hit_distance;

   const float water_grid_scale = _world.terrain.grid_scale * 4.0f;

   const float2 water_point =
      floor(float2{cursor_positionWS.x, cursor_positionWS.z} / water_grid_scale) +
      (water_map_length / 2.0f);

   const int32 water_x = static_cast<int32>(water_point.x);
   const int32 water_y = static_cast<int32>(water_point.y);

   const int32 brush_size_x = _water_editor_config.brush_size_x;
   const int32 brush_size_y = _water_editor_config.brush_size_y;

   if (_water_editor_context.brush_held != _water_editor_context.brush_active) {
      _water_editor_context.brush_active = _water_editor_context.brush_held;
      _edit_stack_world.close_last();
   }

   if (_water_editor_context.brush_active) {
      int32 left = std::clamp(water_x - brush_size_x, 0, water_map_length - 1);
      int32 top = std::clamp(water_y - brush_size_y, 0, water_map_length - 1);
      int32 right = std::clamp(water_x + brush_size_x + 1, 0, water_map_length);
      int32 bottom = std::clamp(water_y + brush_size_y + 1, 0, water_map_length);

      if (left >= right or top >= bottom) return;

      container::dynamic_array_2d<bool> area{right - left, bottom - top};

      if (_water_editor_config.brush_mode == water_brush_mode::paint) {
         for (bool& v : area) v = true;
      }
      else if (_water_editor_config.brush_mode == water_brush_mode::erase) {
         for (bool& v : area) v = false;
      }

      _edit_stack_world.apply(edits::make_set_terrain_area_water_map(left, top,
                                                                     std::move(area)),
                              _edit_context);
   }

   const auto get_position = [&](int32 x, int32 y) {
      return float3{(x - (water_map_length / 2.0f)) * water_grid_scale,
                    _world.terrain.water_settings.height,
                    (y - (water_map_length / 2.0f)) * water_grid_scale};
   };

   for (int32 y = -brush_size_y; y <= brush_size_y + 1; ++y) {
      _tool_visualizers.add_line_overlay(get_position(water_x - brush_size_x,
                                                      water_y + y),
                                         get_position(water_x + brush_size_x + 1,
                                                      water_y + y),
                                         0xff'ff'ff'ffu);
   }

   for (int32 x = -brush_size_x; x <= brush_size_x + 1; ++x) {
      _tool_visualizers.add_line_overlay(get_position(water_x + x, water_y - brush_size_y),
                                         get_position(water_x + x,
                                                      water_y + brush_size_y + 1),
                                         0xff'ff'ff'ffu);
   }
}

}
