#include "edits/set_terrain_area.hpp"
#include "edits/set_value.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/raycast_terrain.hpp"
#include "world_edit.hpp"

#include "imgui.h"

#include <numbers>

namespace we {

namespace {

template<typename Value>
auto make_set_terrain_value(Value world::terrain::*value_member_ptr,
                            Value new_value, Value original_value)
   -> std::unique_ptr<edits::set_global_value<world::terrain, Value>>
{
   return std::make_unique<edits::set_global_value<world::terrain, Value>>(
      &world::world::terrain, value_member_ptr, std::move(new_value),
      std::move(original_value));
}

auto get_position(const float2 point, const world::terrain& terrain) noexcept -> float3
{
   const int32 terrain_half_length = terrain.length / 2;

   return float3{(point.x) * terrain.grid_scale,
                 terrain.height_map[{std::clamp(static_cast<int32>(point.x) + terrain_half_length,
                                                0, terrain.length - 1),
                                     std::clamp(static_cast<int32>(point.y) + terrain_half_length - 1,
                                                0, terrain.length - 1)}] *
                    terrain.height_scale,
                 (point.y) * terrain.grid_scale};
}

auto brush_weight(int32 x, int32 y, float2 centre, float radius,
                  terrain_brush_falloff falloff) noexcept -> float
{
   const float distance =
      we::distance(float2{static_cast<float>(x), static_cast<float>(y)}, centre);
   const float normalized_distance = std::clamp(distance / radius, 0.0f, 1.0f);

   switch (falloff) {
   case terrain_brush_falloff::none:
      return 1.0f;
   case terrain_brush_falloff::linear:
      return std::clamp(1.0f - normalized_distance, 0.0f, 1.0f);
   case terrain_brush_falloff::inverse_sq:
      return std::clamp(1.0f - (normalized_distance * normalized_distance), 0.0f, 1.0f);
   case terrain_brush_falloff::sine:
      return (sinf(std::numbers::pi_v<float> * (normalized_distance + 0.5f))) * 0.5f + 0.5f;
   }

   std::unreachable();
}

}

void world_edit::ui_show_terrain_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 620.0f * _display_scale},
                                       {std::numeric_limits<float>::max(),
                                        620.0f * _display_scale});

   if (ImGui::Begin("Terrain Editor", &_terrain_editor_open)) {
      ImGui::SeparatorText("Brush Mode");

      if (ImGui::Selectable("Overwrite", _terrain_editor_config.brush_mode ==
                                            terrain_brush_mode::overwrite)) {
         _terrain_editor_config.brush_mode = terrain_brush_mode::overwrite;
      }

      if (ImGui::Selectable("Pull Towards", _terrain_editor_config.brush_mode ==
                                               terrain_brush_mode::pull_towards)) {
         _terrain_editor_config.brush_mode = terrain_brush_mode::pull_towards;
      }

      if (ImGui::Selectable("Raise", _terrain_editor_config.brush_mode ==
                                        terrain_brush_mode::raise)) {
         _terrain_editor_config.brush_mode = terrain_brush_mode::raise;
      }

      if (ImGui::Selectable("Lower", _terrain_editor_config.brush_mode ==
                                        terrain_brush_mode::lower)) {
         _terrain_editor_config.brush_mode = terrain_brush_mode::lower;
      }

      if (ImGui::Selectable("Blend", _terrain_editor_config.brush_mode ==
                                        terrain_brush_mode::blend)) {
         _terrain_editor_config.brush_mode = terrain_brush_mode::blend;
      }

      ImGui::SeparatorText("Brush Falloff");

      if (ImGui::Selectable("None", _terrain_editor_config.brush_falloff ==
                                       terrain_brush_falloff::none)) {
         _terrain_editor_config.brush_falloff = terrain_brush_falloff::none;
      }

      if (ImGui::Selectable("Linear", _terrain_editor_config.brush_falloff ==
                                         terrain_brush_falloff::linear)) {
         _terrain_editor_config.brush_falloff = terrain_brush_falloff::linear;
      }

      if (ImGui::Selectable("Inverse Square", _terrain_editor_config.brush_falloff ==
                                                 terrain_brush_falloff::inverse_sq)) {
         _terrain_editor_config.brush_falloff = terrain_brush_falloff::inverse_sq;
      }

      if (ImGui::Selectable("Sine", _terrain_editor_config.brush_falloff ==
                                       terrain_brush_falloff::sine)) {
         _terrain_editor_config.brush_falloff = terrain_brush_falloff::sine;
      }

      ImGui::SeparatorText("Brush Settings");

      if (_terrain_editor_config.brush_mode == terrain_brush_mode::pull_towards or
          _terrain_editor_config.brush_mode == terrain_brush_mode::overwrite) {
         if (float height = _terrain_editor_config.brush_height * _world.terrain.height_scale;
             ImGui::DragFloat("Height", &height, _world.terrain.height_scale,
                              -32768.0f * _world.terrain.height_scale,
                              32767.0f * _world.terrain.height_scale, "%.3f",
                              ImGuiSliderFlags_AlwaysClamp |
                                 ImGuiSliderFlags_NoRoundToFormat)) {
            _terrain_editor_config.brush_height =
               std::trunc(height / _world.terrain.height_scale);
         }
      }

      ImGui::SliderInt("Size", &_terrain_editor_config.brush_size, 0,
                       _world.terrain.length / 2, "%d", ImGuiSliderFlags_AlwaysClamp);

      if (_terrain_editor_config.brush_mode == terrain_brush_mode::pull_towards or
          _terrain_editor_config.brush_mode == terrain_brush_mode::blend) {
         ImGui::SliderFloat("Speed", &_terrain_editor_config.brush_speed,
                            0.125f, 1.0f, "%.2f");
      }

      if (_terrain_editor_config.brush_mode == terrain_brush_mode::raise or
          _terrain_editor_config.brush_mode == terrain_brush_mode::lower) {
         if (float rate = _terrain_editor_config.brush_rate * _world.terrain.height_scale;
             ImGui::DragFloat("Rate", &rate, 0.02f, 0.1f, 10.0f)) {
            _terrain_editor_config.brush_rate =
               static_cast<int16>(rate / _world.terrain.height_scale);
         }
      }

      ImGui::SeparatorText("Terrain Settings");

      if (bool active = _world.terrain.active_flags.terrain;
          ImGui::Checkbox("Terrain Enabled", &active)) {
         world::active_flags flags = _world.terrain.active_flags;

         flags.terrain = active;

         _edit_stack_world.apply(make_set_terrain_value(&world::terrain::active_flags,
                                                        flags, _world.terrain.active_flags),
                                 _edit_context, {.closed = true});
      }

      ImGui::SeparatorText("Advanced");

      ImGui::Text("Edit these if you're the adventurous sort or just know what "
                  "you're doing.");

      if (float height_scale = _world.terrain.height_scale;
          ImGui::DragFloat("Height Scale", &height_scale, 0.0025f)) {
         _edit_stack_world.apply(make_set_terrain_value(&world::terrain::height_scale,
                                                        std::max(static_cast<float>(height_scale),
                                                                 0.0f),
                                                        _world.terrain.height_scale),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) _edit_stack_world.close_last();

      if (float grid_scale = _world.terrain.grid_scale;
          ImGui::DragFloat("Grid Scale", &grid_scale, 1.0f, 1.0f, 16777216.0f)) {
         _edit_stack_world.apply(make_set_terrain_value(&world::terrain::grid_scale,
                                                        std::max(static_cast<float>(grid_scale),
                                                                 1.0f),
                                                        _world.terrain.grid_scale),
                                 _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) _edit_stack_world.close_last();
   }

   ImGui::End();

   if (not _terrain_editor_open) return;

   ImGui::Begin("Hotkeys");

   ImGui::SeparatorText("Terrain Editing");

   ImGui::Text("Increase Brush Size");
   ImGui::BulletText(get_display_string(
      _hotkeys.query_binding("Terrain Editing", "Increase Brush Size")));

   ImGui::Text("Decrease Brush Size");
   ImGui::BulletText(get_display_string(
      _hotkeys.query_binding("Terrain Editing", "Decrease Brush Size")));

   ImGui::End();

   graphics::camera_ray ray =
      make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                      {ImGui::GetMainViewport()->Size.x,
                       ImGui::GetMainViewport()->Size.y});

   float hit_distance = 0.0f;

   if (_terrain_editor_context.brush_active and
       _terrain_editor_config.brush_mode == terrain_brush_mode::overwrite) {
      if (float hit = -(dot(ray.origin, float3{0.0f, 1.0f, 0.0f}) -
                        _terrain_editor_context.brush_plane_height) /
                      dot(ray.direction, float3{0.0f, 1.0f, 0.0f});
          hit > 0.0f) {
         hit_distance = hit;
      }
   }
   else {
      if (auto hit = world::raycast(ray.origin, ray.direction, _world.terrain); hit) {
         hit_distance = *hit;
      }
   }

   float3 cursor_positionWS = ray.origin + ray.direction * hit_distance;

   float2 terrain_point{cursor_positionWS.x, cursor_positionWS.z};

   terrain_point /= _world.terrain.grid_scale;
   terrain_point = round(terrain_point);

   if (_terrain_editor_context.brush_held != _terrain_editor_context.brush_active) {
      _terrain_editor_context.brush_active = _terrain_editor_context.brush_held;

      if (not _terrain_editor_context.brush_held) {
         _terrain_editor_context = {};
         _edit_stack_world.close_last();
      }
      else {
         _terrain_editor_context.brush_plane_height = cursor_positionWS.y;
         _terrain_editor_context.last_brush_update = std::chrono::steady_clock::now();
      }
   }

   if (_terrain_editor_context.brush_active) {
      const float delta_time =
         std::chrono::duration<float>(
            std::chrono::steady_clock::now() -
            std::exchange(_terrain_editor_context.last_brush_update,
                          std::chrono::steady_clock::now()))
            .count();

      const int32 terrain_half_length = _world.terrain.length / 2;

      int32 terrain_x = static_cast<int32>(terrain_point.x) + terrain_half_length;
      int32 terrain_y = static_cast<int32>(terrain_point.y) + terrain_half_length - 1;

      int32 left = std::clamp(terrain_x - _terrain_editor_config.brush_size, 0,
                              _world.terrain.length - 1);
      int32 top = std::clamp(terrain_y - _terrain_editor_config.brush_size, 0,
                             _world.terrain.length - 1);
      int32 right = std::clamp(terrain_x + _terrain_editor_config.brush_size + 1,
                               0, _world.terrain.length);
      int32 bottom = std::clamp(terrain_y + _terrain_editor_config.brush_size + 1,
                                0, _world.terrain.length);

      if (left == right or top == bottom) return;

      container::dynamic_array_2d<int16> area{right - left, bottom - top};

      for (int32 y = top; y < bottom; ++y) {
         for (int32 x = left; x < right; ++x) {
            area[{x - left, y - top}] = _world.terrain.height_map[{x, y}];
         }
      }

      const float2 brush_center = {static_cast<float>(terrain_x),
                                   static_cast<float>(terrain_y)};
      const float brush_radius = static_cast<float>(_terrain_editor_config.brush_size);

      if (_terrain_editor_config.brush_mode == terrain_brush_mode::overwrite) {
         for (int32 y = top; y < bottom; ++y) {
            for (int32 x = left; x < right; ++x) {
               int16& v = area[{x - left, y - top}];

               const float weight =
                  brush_weight(x, y, brush_center, brush_radius,
                               _terrain_editor_config.brush_falloff);

               v = static_cast<int16>(_terrain_editor_config.brush_height * weight);
            }
         }
      }
      else if (_terrain_editor_config.brush_mode == terrain_brush_mode::pull_towards) {
         const float time_weight =
            std::clamp(delta_time * _terrain_editor_config.brush_speed, 0.0f, 1.0f);
         const float target_height = _terrain_editor_config.brush_height;

         for (int32 y = top; y < bottom; ++y) {
            for (int32 x = left; x < right; ++x) {
               int16& v = area[{x - left, y - top}];

               const float weight =
                  brush_weight(x, y, brush_center, brush_radius,
                               _terrain_editor_config.brush_falloff);

               v = static_cast<int16>(std::lerp(static_cast<float>(v),
                                                target_height * weight, time_weight));
            }
         }
      }
      else if (_terrain_editor_config.brush_mode == terrain_brush_mode::raise) {
         const float height_increase = _terrain_editor_config.brush_rate * delta_time;

         for (int32 y = top; y < bottom; ++y) {
            for (int32 x = left; x < right; ++x) {
               int16& v = area[{x - left, y - top}];

               const float weight =
                  brush_weight(x, y, brush_center, brush_radius,
                               _terrain_editor_config.brush_falloff);

               v = static_cast<int16>(std::min(v + (height_increase * weight), 32767.0f));
            }
         }
      }
      else if (_terrain_editor_config.brush_mode == terrain_brush_mode::lower) {
         const float height_decrease = _terrain_editor_config.brush_rate * delta_time;

         for (int32 y = top; y < bottom; ++y) {
            for (int32 x = left; x < right; ++x) {
               int16& v = area[{x - left, y - top}];

               const float weight =
                  brush_weight(x, y, brush_center, brush_radius,
                               _terrain_editor_config.brush_falloff);

               v = static_cast<int16>(
                  std::max(v - (height_decrease * weight), -32768.0f));
            }
         }
      }
      else if (_terrain_editor_config.brush_mode == terrain_brush_mode::blend) {
         int64 average_height = 0;

         for (int16& v : area) average_height += v;

         average_height /= area.ssize();

         const float time_weight =
            std::clamp(delta_time * _terrain_editor_config.brush_speed, 0.0f, 1.0f);
         const float target_height = static_cast<float>(average_height);

         for (int32 y = top; y < bottom; ++y) {
            for (int32 x = left; x < right; ++x) {
               int16& v = area[{x - left, y - top}];

               const float weight =
                  brush_weight(x, y, brush_center, brush_radius,
                               _terrain_editor_config.brush_falloff);

               v = static_cast<int16>(std::lerp(static_cast<float>(v), target_height,
                                                std::lerp(0.0f, time_weight, weight)));
            }
         }
      }

      _edit_stack_world.apply(edits::make_set_terrain_area(left, top, std::move(area)),
                              _edit_context);
   }

   if (_terrain_editor_config.brush_size <= 8) {
      const float size = static_cast<float>(_terrain_editor_config.brush_size);

      if (size == 0.0f) {
         const float3 point = get_position(terrain_point, _world.terrain);

         _tool_visualizers.add_line_overlay(point,
                                            point + float3{0.0f, _world.terrain.grid_scale,
                                                           0.0f},
                                            0xffffffff);
      }

      for (float y = -size; y < size; ++y) {
         for (float x = -size; x < size; ++x) {
            const std::array vertices{
               get_position(terrain_point + float2{0.0f + x, 0.0f + y}, _world.terrain),
               get_position(terrain_point + float2{1.0f + x, 0.0f + y}, _world.terrain),
               get_position(terrain_point + float2{1.0f + x, 1.0f + y}, _world.terrain),
               get_position(terrain_point + float2{0.0f + x, 1.0f + y}, _world.terrain)};

            _tool_visualizers.add_line_overlay(vertices[0], vertices[1], 0xffffffff);
            _tool_visualizers.add_line_overlay(vertices[3], vertices[0], 0xffffffff);
         }
      }

      for (float y = -size; y < size; ++y) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_point + float2{size, y}, _world.terrain),
            get_position(terrain_point + float2{size, y + 1.0f}, _world.terrain),
            0xffffffff);
      }

      for (float x = -size; x < size; ++x) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_point + float2{x, size}, _world.terrain),
            get_position(terrain_point + float2{x + 1.0f, size}, _world.terrain),
            0xffffffff);
      }
   }
   else {
      const float radius = _terrain_editor_config.brush_size * _world.terrain.grid_scale;

      const float3 point = get_position(terrain_point, _world.terrain);

      _tool_visualizers.add_line_overlay(point,
                                         point + float3{0.0f, _world.terrain.grid_scale,
                                                        0.0f},
                                         0xffffffff);

      _tool_visualizers.add_line_overlay(point + float3{1.0f, 0.0f, 0.0f} * radius,
                                         point + float3{-1.0f, 0.0f, 0.0f} * radius,
                                         0xffffffff);
      _tool_visualizers.add_line_overlay(point + float3{0.0f, 0.0f, 1.0f} * radius,
                                         point + float3{0.0f, 0.0f, -1.0f} * radius,
                                         0xffffffff);
   }
}

}