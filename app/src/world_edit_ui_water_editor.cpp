#include "world_edit.hpp"

#include "edits/imgui_ext.hpp"
#include "edits/set_terrain_area.hpp"
#include "math/vector_funcs.hpp"
#include "utility/string_icompare.hpp"

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

   bool open = _terrain_edit_tool == terrain_edit_tool::water_editor;

   if (ImGui::Begin("Water Editor", &open)) {
      if (ImGui::CollapsingHeader("Edit", ImGuiTreeNodeFlags_DefaultOpen)) {
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

         ImGui::SeparatorText("Tools");

         ImGui::Selectable("Flood Fill", &_water_editor_context.flood_fill_active);

         if (ImGui::Selectable("Clear World")) {
            _edit_stack_world.apply(edits::make_set_terrain_area_water_map(
                                       0, 0,
                                       container::dynamic_array_2d<bool>{water_map_length,
                                                                         water_map_length}),
                                    _edit_context);
         }
      }

      if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
         ImGui::Checkbox("Enabled", &_world.terrain.active_flags.water,
                         _edit_stack_world, _edit_context);

         ImGui::DragFloat("Height", &_world.terrain.water_settings.height,
                          _edit_stack_world, _edit_context, 0.125f);

         world::water& water = _world.effects.water;

         ImGui::SeparatorText("Mesh");

         ImGui::SetItemTooltip("Settings affecting the water mesh.");

         ImGui::DragInt2("Patch Divisions", &water.patch_divisions_pc,
                         _edit_stack_world, _edit_context, 1.0f, 1, 256, "%d",
                         ImGuiSliderFlags_AlwaysClamp);

         ImGui::SliderInt("LOD Decimation", &water.lod_decimation_pc,
                          _edit_stack_world, _edit_context, 1, 16, "%d",
                          ImGuiSliderFlags_AlwaysClamp);

         ImGui::BeginDisabled(water.ocean_enable_pc);

         ImGui::Checkbox("Oscillation Enable", &water.oscillation_enable_pc,
                         _edit_stack_world, _edit_context);

         ImGui::EndDisabled();

         ImGui::Checkbox("Disable LowRes", &water.disable_low_res_pc,
                         _edit_stack_world, _edit_context);

         ImGui::DragFloat("Far Scene Range", &water.far_scene_range_pc,
                          _edit_stack_world, _edit_context, 1.0f, 0.0f, 1e10f,
                          water.far_scene_range_pc == 0.0f ? "Infinite" : "%.1f");

         ImGui::SeparatorText("Colors");

         ImGui::ColorEdit4("Refraction Color", &water.refraction_color_pc,
                           _edit_stack_world, _edit_context);

         ImGui::ColorEdit4("Reflection Color", &water.reflection_color_pc,
                           _edit_stack_world, _edit_context);

         ImGui::ColorEdit4("Underwater Color", &water.underwater_color_pc,
                           _edit_stack_world, _edit_context);

         ImGui::ColorEdit4("Water Ring Color", &water.water_ring_color_pc,
                           _edit_stack_world, _edit_context);

         ImGui::ColorEdit4("Water Wake Color", &water.water_wake_color_pc,
                           _edit_stack_world, _edit_context);

         ImGui::ColorEdit4("Water Splash Color", &water.water_splash_color_pc,
                           _edit_stack_world, _edit_context);

         ImGui::SeparatorText("Textures");

         ImGui::DragFloat2("Tile", &water.tile_pc, _edit_stack_world,
                           _edit_context, 0.25f);

         ImGui::DragFloat2("Velocity", &water.velocity_pc, _edit_stack_world,
                           _edit_context, 0.0125f);

         ImGui::InputTextAutoComplete(
            "Main Texture", &water.main_texture_pc, _edit_stack_world,
            _edit_context, [&]() noexcept {
               std::array<std::string_view, 6> entries;
               std::size_t matching_count = 0;

               _asset_libraries.textures.view_existing(
                  [&](const std::span<const assets::stable_string> assets) noexcept {
                     for (const std::string_view asset : assets) {
                        if (matching_count == entries.size()) break;
                        if (not string::icontains(asset, water.main_texture_pc)) {
                           continue;
                        }

                        entries[matching_count] = asset;

                        ++matching_count;
                     }
                  });

               return entries;
            });

         ImGui::BeginDisabled(water.ocean_enable_pc);

         ImGui::InputText("Normal Map Textures Prefix",
                          &water.normal_map_textures_pc.prefix,
                          _edit_stack_world, _edit_context);

         ImGui::SliderInt("Normal Map Textures Count",
                          &water.normal_map_textures_pc.count, _edit_stack_world,
                          _edit_context, 1, 50, "%d", ImGuiSliderFlags_AlwaysClamp);

         ImGui::DragFloat("Normal Map Textures Framerate",
                          &water.normal_map_textures_pc.framerate,
                          _edit_stack_world, _edit_context, 0.25f, 1.0f, 250.0f);

         ImGui::InputText("Bump Map Textures Prefix",
                          &water.bump_map_textures_pc.prefix, _edit_stack_world,
                          _edit_context);

         ImGui::SliderInt("Bump Map Textures Count",
                          &water.bump_map_textures_pc.count, _edit_stack_world,
                          _edit_context, 1, 50, "%d", ImGuiSliderFlags_AlwaysClamp);

         ImGui::DragFloat("Bump Map Textures Framerate",
                          &water.bump_map_textures_pc.framerate,
                          _edit_stack_world, _edit_context, 0.25f, 1.0f, 250.0f);

         ImGui::InputText("Specular Mask Textures Prefix",
                          &water.specular_mask_textures_pc.prefix,
                          _edit_stack_world, _edit_context);

         ImGui::SliderInt("Specular Mask Textures Count",
                          &water.specular_mask_textures_pc.count, _edit_stack_world,
                          _edit_context, 1, 50, "%d", ImGuiSliderFlags_AlwaysClamp);

         ImGui::DragFloat("Specular Mask Textures Framerate",
                          &water.specular_mask_textures_pc.framerate,
                          _edit_stack_world, _edit_context, 0.25f, 1.0f, 250.0f);

         ImGui::DragFloat2("Specular Mask Tile", &water.specular_mask_tile_pc,
                           _edit_stack_world, _edit_context, 0.25f);

         ImGui::DragFloat2("Specular Mask Scroll Speed",
                           &water.specular_mask_scroll_speed_pc,
                           _edit_stack_world, _edit_context, 0.0125f);

         ImGui::EndDisabled();

         ImGui::SeparatorText("Reflections");

         ImGui::BeginDisabled(water.ocean_enable_pc);

         ImGui::DragFloatRange2("Fresnel Min-Max", &water.fresnel_min_max_pc.x,
                                &water.fresnel_min_max_pc.y, _edit_stack_world,
                                _edit_context, 0.025f, 0.0f, 1.0f, "Min: %.3f",
                                "Max: %.3f", ImGuiSliderFlags_AlwaysClamp);

         ImGui::EndDisabled();

         ImGui::SeparatorText("Ocean");

         ImGui::Checkbox("Ocean Enable", &water.ocean_enable_pc,
                         _edit_stack_world, _edit_context);

         ImGui::BeginDisabled(not water.ocean_enable_pc);

         ImGui::DragFloat2("Wind Direction", &water.wind_direction_pc,
                           _edit_stack_world, _edit_context, 0.0125f);

         ImGui::DragFloat("Wind Speed", &water.wind_speed_pc, _edit_stack_world,
                          _edit_context, 0.5f);

         ImGui::DragFloat("Phillips Constant", &water.phillips_constant_pc,
                          _edit_stack_world, _edit_context, 0.000001f, 0.0f,
                          0.0f, "%.6f");

         ImGui::InputTextAutoComplete(
            "Foam Texture", &water.foam_texture_pc, _edit_stack_world,
            _edit_context, [&]() noexcept {
               std::array<std::string_view, 6> entries;
               std::size_t matching_count = 0;

               _asset_libraries.textures.view_existing(
                  [&](const std::span<const assets::stable_string> assets) noexcept {
                     for (const std::string_view asset : assets) {
                        if (matching_count == entries.size()) break;
                        if (not string::icontains(asset, water.foam_texture_pc)) {
                           continue;
                        }

                        entries[matching_count] = asset;

                        ++matching_count;
                     }
                  });

               return entries;
            });

         ImGui::DragFloat2("Foam Tile", &water.foam_tile_pc, _edit_stack_world,
                           _edit_context, 0.25f);

         ImGui::EndDisabled();
      }
   }

   ImGui::End();

   if (not open) {
      _terrain_edit_tool = terrain_edit_tool::none;

      return;
   }

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Water Editing");

      ImGui::Text("Cycle Brush Mode");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Water Editing", "Cycle Brush Mode")));

      ImGui::Text("Flood Fill");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Water Editing", "Flood Fill")));

      ImGui::Text("Increase Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Water Editing", "Increase Brush Size")));

      ImGui::Text("Decrease Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Water Editing", "Decrease Brush Size")));

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

   if (_water_editor_context.brush_held != _water_editor_context.brush_active) {
      _water_editor_context.brush_active = _water_editor_context.brush_held;

      if (_water_editor_context.flooded) {
         _water_editor_context.flood_fill_active = false;
      }

      _water_editor_context.flooded = false;

      _edit_stack_world.close_last();
   }

   if (_water_editor_context.flood_fill_active) {
      if (_water_editor_context.brush_active and not _water_editor_context.flooded) {
         if (water_x < 0 or water_x >= water_map_length) return;
         if (water_y < 0 or water_y >= water_map_length) return;

         container::dynamic_array_2d<bool> underwater{water_map_length, water_map_length};

         for (int32 y = 0; y < water_map_length; ++y) {
            for (int32 x = 0; x < water_map_length; ++x) {
               for (int32 ter_y = -1; ter_y <= 4; ++ter_y) {
                  for (int32 ter_x = -1; ter_x <= 4; ++ter_x) {
                     const float terrain_height =
                        _world.terrain.height_map[{
                           std::clamp(x * 4 + ter_x, 0, _world.terrain.length - 1),
                           std::clamp(y * 4 + ter_y, 0, _world.terrain.length - 1)}] *
                        _world.terrain.height_scale;

                     underwater[{x, y}] |=
                        terrain_height < _world.terrain.water_settings.height;
                  }
               }
            }
         }

         if (not underwater[{water_x, water_y}]) return;

         struct fill_span {
            int32 x1;
            int32 x2;
            int32 y;
            int32 y_dir;
         };

         container::dynamic_array_2d<bool> filled{water_map_length, water_map_length};

         std::vector<fill_span> spans;
         spans.reserve(water_map_length);

         spans.push_back({water_x, water_x, water_y, 1});
         spans.push_back({water_x, water_x, water_y - 1, -1});

         const auto should_fill = [&](const int32 x, const int32 y) {
            if (x < 0 or x >= water_map_length) return false;
            if (y < 0 or y >= water_map_length) return false;

            return underwater[{x, y}] and not filled[{x, y}];
         };

         // Paul S. Heckbert's Seed Fill Algorithm
         while (not spans.empty()) {
            auto [x1, x2, y, y_dir] = spans.back();
            spans.pop_back();

            int32 x = x1;

            if (should_fill(x, y)) {
               while (should_fill(x - 1, y)) {
                  filled[{x - 1, y}] = true;
                  x = x - 1;
               }

               if (x < x1) {
                  spans.push_back({x, x1 - 1, y - y_dir, -y_dir});
               }
            }

            while (x1 <= x2) {
               while (should_fill(x1, y)) {
                  filled[{x1, y}] = true;
                  x1 = x1 + 1;
               }

               if (x1 > x) {
                  spans.push_back({x, x1 - 1, y + y_dir, y_dir});
               }

               if (x1 - 1 > x2) {
                  spans.push_back({x2 + 1, x1 - 1, y - y_dir, -y_dir});
               }

               x1 = x1 + 1;

               while (x1 < x2 and not should_fill(x1, y)) {
                  x1 = x1 + 1;
               }

               x = x1;
            }
         }

         for (int32 y = 0; y < water_map_length; ++y) {
            for (int32 x = 0; x < water_map_length; ++x) {
               filled[{x, y}] |= _world.terrain.water_map[{x, y}];
            }
         }

         _edit_stack_world.apply(edits::make_set_terrain_area_water_map(0, 0,
                                                                        std::move(filled)),
                                 _edit_context);

         _water_editor_context.flooded = true;
      }

      if (not _water_editor_context.flooded) {
         _tool_visualizers.add_line_overlay(cursor_positionWS,
                                            cursor_positionWS +
                                               float3{0.0f, water_grid_scale, 0.0f},
                                            0xff'ff'ff'ffu);
      }
   }
   else {
      const int32 brush_size_x = _water_editor_config.brush_size_x;
      const int32 brush_size_y = _water_editor_config.brush_size_y;

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
}
