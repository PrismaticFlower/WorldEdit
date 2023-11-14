#include "edits/set_terrain_area.hpp"
#include "edits/set_value.hpp"
#include "math/vector_funcs.hpp"
#include "world/utility/raycast_terrain.hpp"
#include "world_edit.hpp"

#include "imgui.h"

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

      ImGui::SeparatorText("Brush Settings");

      if (_terrain_editor_config.brush_mode == terrain_brush_mode::overwrite) {
         ImGui::DragScalar("Overwrite Value", ImGuiDataType_S16,
                           &_terrain_editor_config.brush_overwrite_value);
      }

      ImGui::DragInt("Brush Radius", &_terrain_editor_config.brush_radius, 1.0f,
                     0, 64, "%d",
                     ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);

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

   graphics::camera_ray ray =
      make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                      {ImGui::GetMainViewport()->Size.x,
                       ImGui::GetMainViewport()->Size.y});

   float hit_distance = 0.0f;

   if (_terrain_editor_context.brush_active) {
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
   }

   if (_terrain_editor_context.brush_active) {
      const int32 terrain_half_length = _world.terrain.length / 2;

      int32 x = static_cast<int32>(terrain_point.x) + terrain_half_length;
      int32 y = static_cast<int32>(terrain_point.y) + terrain_half_length - 1;

      int32 left = std::clamp(x - _terrain_editor_config.brush_radius, 0,
                              _world.terrain.length - 1);
      int32 top = std::clamp(y - _terrain_editor_config.brush_radius, 0,
                             _world.terrain.length - 1);
      int32 right = std::clamp(x + _terrain_editor_config.brush_radius + 1, 0,
                               _world.terrain.length);
      int32 bottom = std::clamp(y + _terrain_editor_config.brush_radius + 1, 0,
                                _world.terrain.length);

      container::dynamic_array_2d<int16> area{right - left, bottom - top};

      if (_terrain_editor_config.brush_mode == terrain_brush_mode::overwrite) {
         for (int16& v : area) v = _terrain_editor_config.brush_overwrite_value;
      }

      _edit_stack_world.apply(edits::make_set_terrain_area(left, top, std::move(area)),
                              _edit_context);
   }

   const float radius = static_cast<float>(_terrain_editor_config.brush_radius);

   if (radius == 0.0f) {
      const float3 point = get_position(terrain_point, _world.terrain);

      _tool_visualizers.add_line_overlay(point,
                                         point + float3{0.0f, _world.terrain.grid_scale,
                                                        0.0f},
                                         0xffffffff);
   }

   for (float y = -radius; y < radius; ++y) {
      for (float x = -radius; x < radius; ++x) {
         const std::array vertices{
            get_position(terrain_point + float2{0.0f + x, 0.0f + y}, _world.terrain),
            get_position(terrain_point + float2{1.0f + x, 0.0f + y}, _world.terrain),
            get_position(terrain_point + float2{1.0f + x, 1.0f + y}, _world.terrain),
            get_position(terrain_point + float2{0.0f + x, 1.0f + y}, _world.terrain)};

         _tool_visualizers.add_line_overlay(vertices[0], vertices[1], 0xffffffff);
         // _tool_visualizers.add_line_overlay(vertices[1], vertices[2], 0xffffffff);
         // _tool_visualizers.add_line_overlay(vertices[2], vertices[3], 0xffffffff);
         _tool_visualizers.add_line_overlay(vertices[3], vertices[0], 0xffffffff);
      }
   }

   for (float y = -radius; y < radius; ++y) {
      _tool_visualizers.add_line_overlay(
         get_position(terrain_point + float2{radius, y}, _world.terrain),
         get_position(terrain_point + float2{radius, y + 1.0f}, _world.terrain),
         0xffffffff);
   }

   for (float x = -radius; x < radius; ++x) {
      _tool_visualizers.add_line_overlay(
         get_position(terrain_point + float2{x, radius}, _world.terrain),
         get_position(terrain_point + float2{x + 1.0f, radius}, _world.terrain),
         0xffffffff);
   }
}

}