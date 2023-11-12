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
      ImGui::SeparatorText("Brush");

      ImGui::DragFloat("Brush Radius", &_terrain_editor_config.brush_radius,
                       1.0f, 0.0f, 64.0f, "%.0f",
                       ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);

      _terrain_editor_config.brush_radius =
         std::trunc(_terrain_editor_config.brush_radius);

      ImGui::SeparatorText("Settings");

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

   static uint32 set_length = 0;
   static int16 set_value = 0;
   static uint32 set_x = 0;
   static uint32 set_y = 0;

   ImGui::DragScalar("Set Length", ImGuiDataType_U32, &set_length);
   ImGui::DragScalar("Set Value", ImGuiDataType_S16, &set_value);
   ImGui::DragScalar("X", ImGuiDataType_U32, &set_x);
   ImGui::DragScalar("Y", ImGuiDataType_U32, &set_y);

   if (ImGui::Button("Set")) {
      world::dirty_rect rect{.left = set_x,
                             .top = set_y,
                             .right = set_x + set_length,
                             .bottom = set_y + set_length};

      for (uint32 y = rect.top; y < rect.bottom; ++y) {
         for (uint32 x = rect.left; x < rect.right; ++x) {
            _world.terrain.height_map[{x, y}] = set_value;
         }
      }

      _world.terrain.height_map_dirty.add(rect);
   }

   if (not _terrain_editor_open) return;

   if (_terrain_editor_context.brush_active) {
      graphics::camera_ray ray =
         make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                         {ImGui::GetMainViewport()->Size.x,
                          ImGui::GetMainViewport()->Size.y});

      float hit_distance = 0.0f;

      if (auto hit = world::raycast(ray.origin, ray.direction, _world.terrain); hit) {
         hit_distance = *hit;
      }

      float3 cursor_positionWS = ray.origin + ray.direction * hit_distance;

      float2 terrain_point{cursor_positionWS.x, cursor_positionWS.z};

      terrain_point /= _world.terrain.grid_scale;
      terrain_point = round(terrain_point);

      if (_terrain_editor_context.brush_painting) {
      }

      const float radius = _terrain_editor_config.brush_radius;

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

}