#include "world_edit.hpp"

#include "edits/set_terrain_area.hpp"
#include "edits/set_value.hpp"
#include "math/vector_funcs.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/raycast_terrain.hpp"

#include "imgui.h"
#include "imgui_ext.hpp"

#include <numbers>

namespace we {

namespace {

template<typename Value>
auto make_set_terrain_value(Value world::terrain::*value_member_ptr,
                            Value new_value, Value original_value)
   -> std::unique_ptr<edits::edit<world::edit_context>>
{
   return std::make_unique<edits::set_global_value<world::terrain, Value>>(
      &world::world::terrain, value_member_ptr, std::move(new_value),
      std::move(original_value));
}

template<typename Container, typename Value>
auto make_set_terrain_value_indexed(Container world::terrain::*value_member_ptr,
                                    uint32 index, Value new_value, Value original_value)
   -> std::unique_ptr<edits::edit<world::edit_context>>
{
   return std::make_unique<edits::set_global_value_indexed<world::terrain, Container, Value>>(
      &world::world::terrain, value_member_ptr, index, std::move(new_value),
      std::move(original_value));
}

auto get_position(int32 x, int32 y, const world::terrain& terrain) noexcept -> float3
{
   const int32 terrain_half_length = terrain.length / 2;

   return float3{(x - terrain_half_length) * terrain.grid_scale,
                 terrain.height_map[{std::clamp(x, 0, terrain.length - 1),
                                     std::clamp(y, 0, terrain.length - 1)}] *
                    terrain.height_scale,
                 (y - terrain_half_length + 1) * terrain.grid_scale};
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
   case terrain_brush_falloff::smooth:
      return std::clamp(1.0f - (normalized_distance * normalized_distance), 0.0f, 1.0f);
   case terrain_brush_falloff::sine:
      return (sinf(std::numbers::pi_v<float> * (normalized_distance + 0.5f))) * 0.5f + 0.5f;
   }

   std::unreachable();
}

auto brush_visualizer_color(int32 x, int32 y, float2 centre, float radius,
                            terrain_brush_falloff falloff) -> uint32
{
   const uint32 weight =
      static_cast<uint32>(brush_weight(x, y, centre, radius, falloff) * 255.0f + 0.5f);

   return (weight << 24u) | 0x00ffffff;
}

struct texture_axis_name {
   world::texture_axis axis = {};
   const char* name = nullptr;
};

constexpr std::array<texture_axis_name, 12> texture_axis_names = {
   texture_axis_name{world::texture_axis::xz, "XZ"},
   texture_axis_name{world::texture_axis::xy, "XY"},
   texture_axis_name{world::texture_axis::yz, "YZ"},
   texture_axis_name{world::texture_axis::zx, "ZX"},
   texture_axis_name{world::texture_axis::yx, "YX"},
   texture_axis_name{world::texture_axis::zy, "ZY"},
   texture_axis_name{world::texture_axis::negative_xz, "Negative XZ"},
   texture_axis_name{world::texture_axis::negative_xy, "Negative XY"},
   texture_axis_name{world::texture_axis::negative_yz, "Negative YZ"},
   texture_axis_name{world::texture_axis::negative_zx, "Negative ZX"},
   texture_axis_name{world::texture_axis::negative_yx, "Negative YX"},
   texture_axis_name{world::texture_axis::negative_zy, "Negative ZY"},
};

}

void world_edit::ui_show_terrain_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 620.0f * _display_scale},
                                       {std::numeric_limits<float>::max(),
                                        620.0f * _display_scale});

   if (ImGui::Begin("Terrain Editor", &_terrain_editor_open)) {
      ImGui::SeparatorText("Edit Target");

      if (ImGui::BeginTable("Edit Target", 3,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Height", _terrain_editor_config.edit_target ==
                                            terrain_edit_target::height)) {
            _terrain_editor_config.edit_target = terrain_edit_target::height;
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Texture", _terrain_editor_config.edit_target ==
                                             terrain_edit_target::texture)) {
            _terrain_editor_config.edit_target = terrain_edit_target::texture;
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Colour", _terrain_editor_config.edit_target ==
                                            terrain_edit_target::color)) {
            _terrain_editor_config.edit_target = terrain_edit_target::color;
         }

         ImGui::EndTable();
      }

      if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
         terrain_editor_config::height_config& config = _terrain_editor_config.height;

         ImGui::SeparatorText("Brush Mode");

         if (ImGui::Selectable("Raise", config.brush_mode == terrain_brush_mode::raise)) {
            config.brush_mode = terrain_brush_mode::raise;
         }

         if (ImGui::Selectable("Lower", config.brush_mode == terrain_brush_mode::lower)) {
            config.brush_mode = terrain_brush_mode::lower;
         }

         if (ImGui::Selectable("Overwrite", config.brush_mode ==
                                               terrain_brush_mode::overwrite)) {
            config.brush_mode = terrain_brush_mode::overwrite;
         }

         if (ImGui::Selectable("Pull Towards", config.brush_mode ==
                                                  terrain_brush_mode::pull_towards)) {
            config.brush_mode = terrain_brush_mode::pull_towards;
         }

         if (ImGui::Selectable("Blend", config.brush_mode == terrain_brush_mode::blend)) {
            config.brush_mode = terrain_brush_mode::blend;
         }

         ImGui::SeparatorText("Brush Falloff");

         if (ImGui::Selectable("None", config.brush_falloff ==
                                          terrain_brush_falloff::none)) {
            config.brush_falloff = terrain_brush_falloff::none;
         }

         if (ImGui::Selectable("Linear", config.brush_falloff ==
                                            terrain_brush_falloff::linear)) {
            config.brush_falloff = terrain_brush_falloff::linear;
         }

         if (ImGui::Selectable("Smooth", config.brush_falloff ==
                                            terrain_brush_falloff::smooth)) {
            config.brush_falloff = terrain_brush_falloff::smooth;
         }

         if (ImGui::Selectable("Sine", config.brush_falloff ==
                                          terrain_brush_falloff::sine)) {
            config.brush_falloff = terrain_brush_falloff::sine;
         }

         ImGui::SeparatorText("Brush Settings");

         if (config.brush_mode == terrain_brush_mode::pull_towards or
             config.brush_mode == terrain_brush_mode::overwrite) {
            if (float height = config.brush_height * _world.terrain.height_scale;
                ImGui::DragFloat("Height", &height, _world.terrain.height_scale,
                                 -32768.0f * _world.terrain.height_scale,
                                 32767.0f * _world.terrain.height_scale, "%.3f",
                                 ImGuiSliderFlags_AlwaysClamp |
                                    ImGuiSliderFlags_NoRoundToFormat)) {
               config.brush_height = std::trunc(height / _world.terrain.height_scale);
            }
         }

         ImGui::SliderInt("Size", &_terrain_editor_config.brush_size, 0,
                          _world.terrain.length / 2, "%d",
                          ImGuiSliderFlags_AlwaysClamp);

         if (config.brush_mode == terrain_brush_mode::pull_towards or
             config.brush_mode == terrain_brush_mode::blend) {
            ImGui::SliderFloat("Speed", &config.brush_speed, 0.125f, 1.0f,
                               "%.2f", ImGuiSliderFlags_NoRoundToFormat);
         }

         if (config.brush_mode == terrain_brush_mode::raise or
             config.brush_mode == terrain_brush_mode::lower) {
            ImGui::DragFloat("Rate", &config.brush_rate, 0.02f, 0.1f, 10.0f);
         }
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::texture) {
         terrain_editor_config::texture_config& config = _terrain_editor_config.texture;

         ImGui::SeparatorText("Brush Mode");

         if (ImGui::Selectable("Paint", config.brush_mode ==
                                           terrain_texture_brush_mode::paint)) {
            config.brush_mode = terrain_texture_brush_mode::paint;
         }

         if (ImGui::Selectable("Spray", config.brush_mode ==
                                           terrain_texture_brush_mode::spray)) {
            config.brush_mode = terrain_texture_brush_mode::spray;
         }

         if (ImGui::Selectable("Erase", config.brush_mode ==
                                           terrain_texture_brush_mode::erase)) {
            config.brush_mode = terrain_texture_brush_mode::erase;
         }

         if (ImGui::Selectable("Soften", config.brush_mode ==
                                            terrain_texture_brush_mode::soften)) {
            config.brush_mode = terrain_texture_brush_mode::soften;
         }

         ImGui::SeparatorText("Brush Falloff");

         if (ImGui::Selectable("None", config.brush_falloff ==
                                          terrain_brush_falloff::none)) {
            config.brush_falloff = terrain_brush_falloff::none;
         }

         if (ImGui::Selectable("Linear", config.brush_falloff ==
                                            terrain_brush_falloff::linear)) {
            config.brush_falloff = terrain_brush_falloff::linear;
         }

         if (ImGui::Selectable("Smooth", config.brush_falloff ==
                                            terrain_brush_falloff::smooth)) {
            config.brush_falloff = terrain_brush_falloff::smooth;
         }

         if (ImGui::Selectable("Sine", config.brush_falloff ==
                                          terrain_brush_falloff::sine)) {
            config.brush_falloff = terrain_brush_falloff::sine;
         }

         ImGui::SeparatorText("Brush Settings");

         ImGui::SliderInt("Size", &_terrain_editor_config.brush_size, 0,
                          _world.terrain.length / 2, "%d",
                          ImGuiSliderFlags_AlwaysClamp);

         if (config.brush_mode == terrain_texture_brush_mode::paint) {
            ImGui::SliderFloat("Texture Weight", &config.brush_texture_weight,
                               0.0f, 255.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
         }

         if (config.brush_mode == terrain_texture_brush_mode::spray or
             config.brush_mode == terrain_texture_brush_mode::erase) {
            ImGui::DragFloat("Rate", &config.brush_rate, 0.05f, 0.1f, 10.0f);
         }

         ImGui::SeparatorText("Textures");

         const std::array<void*, world::terrain::texture_count> texture_ids =
            _renderer->terrain_texture_ids();

         for (uint32 i = 0; i < world::terrain::texture_count; ++i) {
            const float size = 64.0f * _display_scale;

            ImGui::PushID(i);

            const ImVec2 cursor_position = ImGui::GetCursorPos();

            if (ImGui::Selectable("##select", config.edit_texture == i,
                                  ImGuiSelectableFlags_None, {size, size})) {
               config.edit_texture = i;
            }

            ImGui::SetCursorPos(cursor_position);
            ImGui::Image(texture_ids[i], {size, size});

            if (ImGui::IsItemHovered()) {
               ImGui::SetTooltip("%u - %s", i,
                                 _world.terrain.texture_names[i].c_str());
            }

            ImGui::PopID();

            if ((i + 1) % 4 != 0) {
               ImGui::SameLine();
            }
         }

         ImGui::Separator();

         const uint32 texture = _terrain_editor_config.texture.edit_texture;

         auto texture_name_auto_complete = [&] {
            std::array<std::string_view, 6> entries;
            std::size_t matching_count = 0;

            _asset_libraries.textures.view_existing(
               [&](const std::span<const assets::stable_string> assets) noexcept {
                  for (const std::string_view asset : assets) {
                     if (matching_count == entries.size()) break;
                     if (not string::icontains(asset,
                                               _world.terrain.texture_names[texture])) {
                        continue;
                     }

                     entries[matching_count] = asset;

                     ++matching_count;
                  }
               });

            return entries;
         };

         if (absl::InlinedVector<char, 256> texture_name =
                {_world.terrain.texture_names[texture].begin(),
                 _world.terrain.texture_names[texture].end()};
             ImGui::InputTextAutoComplete(
                "Name", &texture_name,
                [](void* callback) {
                   return (*static_cast<decltype(texture_name_auto_complete)*>(
                      callback))();
                },
                &texture_name_auto_complete)) {
            _edit_stack_world.apply(make_set_terrain_value_indexed(
                                       &world::terrain::texture_names, texture,
                                       std::string{texture_name.begin(),
                                                   texture_name.end()},
                                       _world.terrain.texture_names[texture]),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (ImGui::BeginCombo("Axis Mapping", [&] {
                for (auto [axis, name] : texture_axis_names) {
                   if (axis == _world.terrain.texture_axes[texture])
                      return name;
                }

                return "";
             }())) {

            for (auto [axis, name] : texture_axis_names) {
               if (ImGui::Selectable(name, axis == _world.terrain.texture_axes[texture])) {
                  _edit_stack_world.apply(make_set_terrain_value_indexed(
                                             &world::terrain::texture_axes, texture, axis,
                                             _world.terrain.texture_axes[texture]),
                                          _edit_context, {.closed = true});
               }
            }

            ImGui::EndCombo();
         }

         if (float scale = 1.0f / _world.terrain.texture_scales[texture];
             ImGui::DragFloat("Scale", &scale, 1.0f)) {
            scale = std::max(scale, 1.0f);

            _edit_stack_world.apply(make_set_terrain_value_indexed(
                                       &world::terrain::texture_scales, texture,
                                       1.0f / scale,
                                       _world.terrain.texture_scales[texture]),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
         terrain_editor_config::color_config& config = _terrain_editor_config.color;

         ImGui::SeparatorText("Brush Mode");

         if (ImGui::Selectable("Paint", config.brush_mode ==
                                           terrain_color_brush_mode::paint)) {
            config.brush_mode = terrain_color_brush_mode::paint;
         }

         if (ImGui::Selectable("Spray", config.brush_mode ==
                                           terrain_color_brush_mode::spray)) {
            config.brush_mode = terrain_color_brush_mode::spray;
         }

         if (ImGui::Selectable("Blur", config.brush_mode ==
                                          terrain_color_brush_mode::blur)) {
            config.brush_mode = terrain_color_brush_mode::blur;
         }

         ImGui::SeparatorText("Brush Falloff");

         if (ImGui::Selectable("None", config.brush_falloff ==
                                          terrain_brush_falloff::none)) {
            config.brush_falloff = terrain_brush_falloff::none;
         }

         if (ImGui::Selectable("Linear", config.brush_falloff ==
                                            terrain_brush_falloff::linear)) {
            config.brush_falloff = terrain_brush_falloff::linear;
         }

         if (ImGui::Selectable("Smooth", config.brush_falloff ==
                                            terrain_brush_falloff::smooth)) {
            config.brush_falloff = terrain_brush_falloff::smooth;
         }

         if (ImGui::Selectable("Sine", config.brush_falloff ==
                                          terrain_brush_falloff::sine)) {
            config.brush_falloff = terrain_brush_falloff::sine;
         }

         ImGui::SeparatorText("Brush Settings");

         ImGui::SliderInt("Size", &_terrain_editor_config.brush_size, 0,
                          _world.terrain.length / 2, "%d",
                          ImGuiSliderFlags_AlwaysClamp);

         if (config.brush_mode == terrain_color_brush_mode::paint or
             config.brush_mode == terrain_color_brush_mode::spray) {
            ImGui::ColorEdit3("Colour", &config.brush_color.x);
         }

         if (config.brush_mode == terrain_color_brush_mode::spray) {
            ImGui::DragFloat("Rate", &config.brush_rate, 0.05f, 0.1f, 10.0f);
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

      auto detail_texture_auto_complete = [&] {
         std::array<std::string_view, 6> entries;
         std::size_t matching_count = 0;

         _asset_libraries.textures.view_existing(
            [&](const std::span<const assets::stable_string> assets) noexcept {
               for (const std::string_view asset : assets) {
                  if (matching_count == entries.size()) break;
                  if (not string::icontains(asset, _world.terrain.detail_texture_name)) {
                     continue;
                  }

                  entries[matching_count] = asset;

                  ++matching_count;
               }
            });

         return entries;
      };

      if (absl::InlinedVector<char, 256> detail_texture =
             {_world.terrain.detail_texture_name.begin(),
              _world.terrain.detail_texture_name.end()};
          ImGui::InputTextAutoComplete(
             "Detail Texture", &detail_texture,
             [](void* callback) {
                return (*static_cast<decltype(detail_texture_auto_complete)*>(
                   callback))();
             },
             &detail_texture_auto_complete)) {
         _edit_stack_world
            .apply(make_set_terrain_value(&world::terrain::detail_texture_name,
                                          std::string{detail_texture.begin(),
                                                      detail_texture.end()},
                                          _world.terrain.detail_texture_name),
                   _edit_context);
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
         _edit_stack_world.close_last();
      }

      ImGui::SeparatorText("Advanced");

      ImGui::Text("Edit these if you're the adventurous sort or "
                  "just know what "
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

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Terrain Editing");

      ImGui::Text("Cycle Brush Mode");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Cycle Brush Mode")));

      ImGui::Text("Cycle Brush Falloff");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Cycle Brush Falloff")));

      ImGui::Text("Increase Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Increase Brush Size")));

      ImGui::Text("Decrease Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Decrease Brush Size")));

      ImGui::End();
   }

   graphics::camera_ray ray =
      make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                      {ImGui::GetMainViewport()->Size.x,
                       ImGui::GetMainViewport()->Size.y});

   float hit_distance = 0.0f;

   if (_terrain_editor_context.brush_active and
       _terrain_editor_config.edit_target == terrain_edit_target::height and
       _terrain_editor_config.height.brush_mode == terrain_brush_mode::overwrite) {
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

   terrain_point = round(terrain_point / _world.terrain.grid_scale) +
                   (_world.terrain.length / 2.0f) - float2{0.0f, 1.0f};

   const int32 terrain_x = static_cast<int32>(terrain_point.x);
   const int32 terrain_y = static_cast<int32>(terrain_point.y);

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

   const float brush_radius = static_cast<float>(_terrain_editor_config.brush_size);

   if (_terrain_editor_context.brush_active) {
      const float delta_time =
         std::chrono::duration<float>(
            std::chrono::steady_clock::now() -
            std::exchange(_terrain_editor_context.last_brush_update,
                          std::chrono::steady_clock::now()))
            .count();

      int32 left = std::clamp(terrain_x - _terrain_editor_config.brush_size, 0,
                              _world.terrain.length - 1);
      int32 top = std::clamp(terrain_y - _terrain_editor_config.brush_size, 0,
                             _world.terrain.length - 1);
      int32 right = std::clamp(terrain_x + _terrain_editor_config.brush_size + 1,
                               0, _world.terrain.length);
      int32 bottom = std::clamp(terrain_y + _terrain_editor_config.brush_size + 1,
                                0, _world.terrain.length);

      if (left == right or top == bottom) return;

      if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
         const terrain_editor_config::height_config& config =
            _terrain_editor_config.height;

         container::dynamic_array_2d<int16> area{right - left, bottom - top};

         for (int32 y = top; y < bottom; ++y) {
            for (int32 x = left; x < right; ++x) {
               area[{x - left, y - top}] = _world.terrain.height_map[{x, y}];
            }
         }

         if (config.brush_mode == terrain_brush_mode::raise) {
            const float height_increase =
               (config.brush_rate / _world.terrain.height_scale) * delta_time;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  int16& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  v = static_cast<int16>(
                     std::min(v + (height_increase * weight), 32767.0f));
               }
            }
         }
         else if (config.brush_mode == terrain_brush_mode::lower) {
            const float height_decrease =
               (config.brush_rate / _world.terrain.height_scale) * delta_time;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  int16& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  v = static_cast<int16>(
                     std::max(v - (height_decrease * weight), -32768.0f));
               }
            }
         }
         else if (config.brush_mode == terrain_brush_mode::overwrite) {
            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  int16& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  if (weight <= 0.0f) continue;

                  v = static_cast<int16>(config.brush_height * weight);
               }
            }
         }
         else if (config.brush_mode == terrain_brush_mode::pull_towards) {
            const float time_weight =
               std::clamp(delta_time * config.brush_speed, 0.0f, 1.0f);
            const float target_height = config.brush_height;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  int16& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  v = static_cast<int16>(std::lerp(static_cast<float>(v),
                                                   target_height * weight, time_weight));
               }
            }
         }
         else if (config.brush_mode == terrain_brush_mode::blend) {
            int64 total_height = 0;
            float samples = 0.0f;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  if (weight <= 0.0f) continue;

                  total_height += area[{x - left, y - top}];
                  samples += 1.0f;
               }
            }

            const float time_weight =
               std::clamp(delta_time * config.brush_speed, 0.0f, 1.0f);
            const float target_height = total_height / samples;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  int16& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  v = static_cast<int16>(
                     std::lerp(static_cast<float>(v), target_height,
                               std::lerp(0.0f, time_weight, weight)));
               }
            }
         }

         _edit_stack_world.apply(edits::make_set_terrain_area(left, top,
                                                              std::move(area)),
                                 _edit_context);
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::texture) {
         const terrain_editor_config::texture_config& config =
            _terrain_editor_config.texture;
         const uint32 texture = config.edit_texture;

         container::dynamic_array_2d<uint8> area{right - left, bottom - top};

         for (int32 y = top; y < bottom; ++y) {
            for (int32 x = left; x < right; ++x) {
               area[{x - left, y - top}] =
                  _world.terrain.texture_weight_maps[texture][{x, y}];
            }
         }

         if (config.brush_mode == terrain_texture_brush_mode::paint) {
            const uint8 max_value = static_cast<uint8>(config.brush_texture_weight);

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  uint8& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  if (weight <= 0.0f) continue;

                  v = std::clamp(std::max(static_cast<uint8>(
                                             config.brush_texture_weight * weight + 0.5f),
                                          v),
                                 uint8{0}, max_value);
               }
            }
         }
         else if (config.brush_mode == terrain_texture_brush_mode::spray) {
            const float weight_increase = config.brush_rate * delta_time * 255.0f;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  uint8& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  v = static_cast<uint8>(
                     std::min(v + (weight_increase * weight), 255.0f));
               }
            }
         }
         else if (config.brush_mode == terrain_texture_brush_mode::erase) {
            const float height_decrease = config.brush_rate * delta_time * 255.0f;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  uint8& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  v = static_cast<uint8>(std::max(v - (height_decrease * weight), 0.0f));
               }
            }
         }
         else if (config.brush_mode == terrain_texture_brush_mode::soften) {
            uint64 total_texture_weight = 0;
            float samples = 0.0;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  if (weight <= 0.0f) continue;

                  total_texture_weight += area[{x - left, y - top}];
                  samples += 1.0f;
               }
            }

            const float target_texture_weight = total_texture_weight / samples;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  uint8& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  if (weight <= 0.0f) continue;

                  v = static_cast<uint8>(std::lerp(v, target_texture_weight, weight));
               }
            }
         }

         _edit_stack_world.apply(edits::make_set_terrain_area(left, top, texture,
                                                              std::move(area)),
                                 _edit_context);
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
         const terrain_editor_config::color_config& config =
            _terrain_editor_config.color;

         container::dynamic_array_2d<uint32> area{right - left, bottom - top};

         for (int32 y = top; y < bottom; ++y) {
            for (int32 x = left; x < right; ++x) {
               area[{x - left, y - top}] = _world.terrain.color_map[{x, y}];
            }
         }

         if (config.brush_mode == terrain_color_brush_mode::paint) {
            const float4 brush_color = {config.brush_color, 1.0f};

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  uint32& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  if (weight <= 0.0f) continue;

                  float4 color = utility::unpack_srgb_bgra(v);

                  color = weight * brush_color + (1.0f - weight) * color;

                  color.w = 1.0f;

                  v = utility::pack_srgb_bgra(color);
               }
            }
         }
         else if (config.brush_mode == terrain_color_brush_mode::spray) {
            const float4 brush_color = {config.brush_color, 1.0f};
            const float time_weight = config.brush_rate * delta_time;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  uint32& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff) *
                                       time_weight;

                  if (weight <= 0.0f) continue;

                  float4 color = utility::unpack_srgb_bgra(v);

                  color = weight * brush_color + (1.0f - weight) * color;

                  color.w = 1.0f;

                  v = utility::pack_srgb_bgra(color);
               }
            }
         }
         else if (config.brush_mode == terrain_color_brush_mode::blur) {
            float4 total_color;
            float samples = 0.0f;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  if (weight <= 0.0f) continue;

                  total_color +=
                     utility::unpack_srgb_bgra(area[{x - left, y - top}]);
                  samples += 1.0f;
               }
            }

            const float4 target_color = total_color / samples;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  uint32& v = area[{x - left, y - top}];

                  const float weight = brush_weight(x, y, terrain_point, brush_radius,
                                                    config.brush_falloff);

                  if (weight <= 0.0f) continue;

                  float4 color = utility::unpack_srgb_bgra(v);

                  color = weight * target_color + (1.0f - weight) * color;

                  color.w = 1.0f;

                  v = utility::pack_srgb_bgra(color);
               }
            }
         }

         _edit_stack_world.apply(edits::make_set_terrain_area_color_map(left, top,
                                                                        std::move(area)),
                                 _edit_context);
      }
   }

   const terrain_brush_falloff brush_falloff = [&] {
      if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
         return _terrain_editor_config.height.brush_falloff;
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::texture) {
         return _terrain_editor_config.texture.brush_falloff;
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
         return _terrain_editor_config.color.brush_falloff;
      }

      return terrain_brush_falloff::none;
   }();

   if (const int32 size = _terrain_editor_config.brush_size; size == 0) {
      const float3 point = get_position(terrain_x, terrain_y, _world.terrain);
      const uint32 color = brush_visualizer_color(terrain_x, terrain_y, terrain_point,
                                                  brush_radius, brush_falloff);

      _tool_visualizers.add_line_overlay(point, color,
                                         point + float3{0.0f, _world.terrain.grid_scale,
                                                        0.0f},
                                         color);
   }
   else if (size <= 8) {
      for (int32 y = -size; y < size; ++y) {
         for (int32 x = -size; x < size; ++x) {
            const std::array vertices{
               get_position(terrain_x + x + 0, terrain_y + y + 0, _world.terrain),
               get_position(terrain_x + x + 1, terrain_y + y + 0, _world.terrain),
               get_position(terrain_x + x + 1, terrain_y + y + 1, _world.terrain),
               get_position(terrain_x + x + 0, terrain_y + y + 1, _world.terrain)};

            const std::array colors{
               brush_visualizer_color(terrain_x + x + 0, terrain_y + y + 0,
                                      terrain_point, brush_radius, brush_falloff),
               brush_visualizer_color(terrain_x + x + 1, terrain_y + y + 0,
                                      terrain_point, brush_radius, brush_falloff),
               brush_visualizer_color(terrain_x + x + 1, terrain_y + y + 1,
                                      terrain_point, brush_radius, brush_falloff),
               brush_visualizer_color(terrain_x + x + 0, terrain_y + y + 1,
                                      terrain_point, brush_radius, brush_falloff)};

            _tool_visualizers.add_line_overlay(vertices[0], colors[0],
                                               vertices[1], colors[1]);
            _tool_visualizers.add_line_overlay(vertices[3], colors[3],
                                               vertices[0], colors[0]);
         }
      }

      for (int32 y = -size; y < size; ++y) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_x + size, terrain_y + y, _world.terrain),
            brush_visualizer_color(terrain_x + size, terrain_y + y,
                                   terrain_point, brush_radius, brush_falloff),
            get_position(terrain_x + size, terrain_y + y + 1, _world.terrain),
            brush_visualizer_color(terrain_x + size, terrain_y + y + 1,
                                   terrain_point, brush_radius, brush_falloff));
      }

      for (int32 x = -size; x < size; ++x) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_x + x, terrain_y + size, _world.terrain),
            brush_visualizer_color(terrain_x + x, terrain_y + size,
                                   terrain_point, brush_radius, brush_falloff),
            get_position(terrain_x + x + 1, terrain_y + size, _world.terrain),
            brush_visualizer_color(terrain_x + x + 1, terrain_y + size,
                                   terrain_point, brush_radius, brush_falloff));
      }
   }
   else {
      for (int32 y = -size; y < size; ++y) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_x, terrain_y + y, _world.terrain),
            brush_visualizer_color(terrain_x, terrain_y + y, terrain_point,
                                   brush_radius, brush_falloff),
            get_position(terrain_x, terrain_y + y + 1, _world.terrain),
            brush_visualizer_color(terrain_x, terrain_y + y + 1, terrain_point,
                                   brush_radius, brush_falloff));
      }

      for (int32 x = -size; x < size; ++x) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_x + x, terrain_y, _world.terrain),
            brush_visualizer_color(terrain_x + x, terrain_y, terrain_point,
                                   brush_radius, brush_falloff),
            get_position(terrain_x + x + 1, terrain_y, _world.terrain),
            brush_visualizer_color(terrain_x + x + 1, terrain_y, terrain_point,
                                   brush_radius, brush_falloff));
      }
   }
}
}