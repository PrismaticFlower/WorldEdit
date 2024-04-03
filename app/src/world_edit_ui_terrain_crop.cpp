#pragma once

#include "world_edit.hpp"

#include "edits/set_terrain.hpp"
#include "math/vector_funcs.hpp"
#include "utility/srgb_conversion.hpp"

#include "imgui.h"

#include <array>
#include <bit>

namespace we {

namespace {

auto get_size_name(const int32 length_bit) -> const char*
{
   if (length_bit == 5) return "32 x 32";
   if (length_bit == 6) return "64 x 64";
   if (length_bit == 7) return "128 x 128";
   if (length_bit == 8) return "256 x 256";
   if (length_bit == 9) return "512 x 512";
   if (length_bit == 10) return "1024 x 1024";

   return "";
}

auto crop_terrain(const world::terrain& old_terrain, const int32 new_length,
                  async::thread_pool& thread_pool) -> world::terrain
{
   assert(new_length < old_terrain.length);

   const int32 offset = (old_terrain.length - new_length) / 2;

   async::task heightmap_crop = thread_pool.exec([=, &old_terrain] {
      container::dynamic_array_2d<int16> new_heightmap{new_length, new_length};

      for (int32 y = 0; y < new_length; ++y) {
         for (int32 x = 0; x < new_length; ++x) {
            new_heightmap[{x, y}] = old_terrain.height_map[{x + offset, y + offset}];
         }
      }

      return new_heightmap;
   });

   std::array<async::task<container::dynamic_array_2d<uint8>>, world::terrain::texture_count> texture_weight_maps_crop;

   const auto crop_color_map = [=](const container::dynamic_array_2d<uint32>& old_color_map) {
      container::dynamic_array_2d<uint32> new_color_map{new_length, new_length};

      for (int32 y = 0; y < new_length; ++y) {
         for (int32 x = 0; x < new_length; ++x) {
            new_color_map[{x, y}] = old_color_map[{x + offset, y + offset}];
         }
      }

      return new_color_map;
   };

   async::task color_map_crop = thread_pool.exec([&crop_color_map, &old_terrain] {
      return crop_color_map(old_terrain.color_map);
   });
   async::task light_map_crop = thread_pool.exec([&crop_color_map, &old_terrain] {
      return crop_color_map(old_terrain.light_map);
   });
   async::task light_map_extra_crop =
      thread_pool.exec([&crop_color_map, &old_terrain] {
         return old_terrain.light_map_extra.empty()
                   ? container::dynamic_array_2d<uint32>{}
                   : crop_color_map(old_terrain.light_map_extra);
      });

   for (int32 texture = 0; texture < world::terrain::texture_count; ++texture) {
      texture_weight_maps_crop[texture] = thread_pool.exec(
         [=, &old_texture_weight_map = old_terrain.texture_weight_maps[texture]] {
            container::dynamic_array_2d<uint8> new_weight_map{new_length, new_length};

            for (int32 y = 0; y < new_length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  new_weight_map[{x, y}] =
                     old_texture_weight_map[{x + offset, y + offset}];
               }
            }

            return new_weight_map;
         });
   }

   async::task water_map_crop = thread_pool.exec([&old_water_map = old_terrain.water_map,
                                                  old_length = old_terrain.length / 4,
                                                  new_length = new_length / 4] {
      const int32 offset = (old_length - new_length) / 2;

      container::dynamic_array_2d<bool> new_water_map{new_length, new_length};

      for (int32 y = 0; y < new_length; ++y) {
         for (int32 x = 0; x < new_length; ++x) {
            new_water_map[{x, y}] = old_water_map[{x + offset, y + offset}];
         }
      }

      return new_water_map;
   });

   async::task foliage_map_crop = thread_pool.exec([&old_foliage_map =
                                                       old_terrain.foliage_map,
                                                    old_length = old_terrain.length / 2,
                                                    new_length = new_length / 2] {
      const int32 offset = (old_length - new_length) / 2;

      container::dynamic_array_2d<world::foliage_patch> new_foliage_map{new_length,
                                                                        new_length};

      for (int32 y = 0; y < new_length; ++y) {
         for (int32 x = 0; x < new_length; ++x) {
            new_foliage_map[{x, y}] = old_foliage_map[{x + offset, y + offset}];
         }
      }

      return new_foliage_map;
   });

   world::terrain new_terrain{
      .version = old_terrain.version,
      .length = new_length,

      .height_scale = old_terrain.height_scale,
      .grid_scale = old_terrain.grid_scale,
      .active_flags = old_terrain.active_flags,
      .prelit = old_terrain.prelit,

      .water_settings = old_terrain.water_settings,

      .texture_names = old_terrain.texture_names,
      .texture_scales = old_terrain.texture_scales,
      .texture_axes = old_terrain.texture_axes,
      .detail_texture_name = old_terrain.detail_texture_name,

      .height_map = heightmap_crop.get(),
      .color_map = color_map_crop.get(),
      .light_map = light_map_crop.get(),
      .light_map_extra = light_map_extra_crop.get(),
      .texture_weight_maps =
         {texture_weight_maps_crop[0].get(), texture_weight_maps_crop[1].get(),
          texture_weight_maps_crop[2].get(), texture_weight_maps_crop[3].get(),
          texture_weight_maps_crop[4].get(), texture_weight_maps_crop[5].get(),
          texture_weight_maps_crop[6].get(), texture_weight_maps_crop[7].get(),
          texture_weight_maps_crop[8].get(), texture_weight_maps_crop[9].get(),
          texture_weight_maps_crop[10].get(), texture_weight_maps_crop[11].get(),
          texture_weight_maps_crop[12].get(), texture_weight_maps_crop[13].get(),
          texture_weight_maps_crop[14].get(), texture_weight_maps_crop[15].get()},

      .water_map = water_map_crop.get(),
      .foliage_map = foliage_map_crop.get()};

   return new_terrain;
}

}

void world_edit::ui_show_terrain_crop() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   bool open = _terrain_edit_tool == terrain_edit_tool::crop;

   if (not std::has_single_bit(static_cast<uint32>(_world.terrain.length))) {
      ImGui::Begin("Crop Terrain Error", &open, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::Text("Can not crop non power of 2 terrain.");

      ImGui::End();

      if (not open) _terrain_edit_tool = terrain_edit_tool::none;

      return;
   }

   if (_world.terrain.length == 32) {
      ImGui::Begin("Crop Terrain Error", &open, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::Text(
         "Terrain is already at the smallest size WorldEdit will allow.");

      ImGui::End();

      if (not open) _terrain_edit_tool = terrain_edit_tool::none;

      return;
   }

   if (ImGui::Begin("Crop Terrain", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      if (_terrain_crop_context.new_length == 0) {
         _terrain_crop_context.new_length = _world.terrain.length;
      }

      constexpr int32 min_bit = 5; // 2^5= 32
      const int32 max_bit =
         std::countr_zero(static_cast<uint32>(_world.terrain.length));

      int32 length_bit = std::clamp(std::countr_zero(static_cast<uint32>(
                                       _terrain_crop_context.new_length)),
                                    min_bit, max_bit);

      if (ImGui::SliderInt("New Length", &length_bit, min_bit, max_bit,
                           get_size_name(length_bit), ImGuiSliderFlags_AlwaysClamp)) {
         _terrain_crop_context.new_length = 1 << length_bit;
      }

      ImGui::BeginDisabled(_terrain_crop_context.new_length >= _world.terrain.length);

      if (ImGui::Button("Crop", {ImGui::CalcItemWidth(), 0.0f})) {
         _edit_stack_world.apply(edits::make_set_terrain(
                                    crop_terrain(_world.terrain,
                                                 _terrain_crop_context.new_length,
                                                 *_thread_pool)),
                                 _edit_context, {.closed = true});

         open = false;
      }

      ImGui::EndDisabled();
   }

   if (not open) _terrain_edit_tool = terrain_edit_tool::none;

   ImGui::End();
}

}