#pragma once

#include "world_edit.hpp"

#include "edits/set_terrain.hpp"
#include "math/vector_funcs.hpp"
#include "utility/srgb_conversion.hpp"

#include "imgui.h"

#include <algorithm>
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

auto extend_terrain(const world::terrain& old_terrain, const int32 new_length,
                    const bool fill_from_edges, async::thread_pool& thread_pool)
   -> world::terrain
{
   assert(new_length > old_terrain.length);

   const int32 old_length = old_terrain.length;
   const int32 offset = (new_length - old_length) / 2;

   async::task heightmap_extend = thread_pool.exec([=, &old_terrain] {
      container::dynamic_array_2d<int16> new_heightmap{new_length, new_length};

      if (fill_from_edges) {
         for (int32 y = 0; y < new_length; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               new_heightmap[{x, y}] =
                  old_terrain.height_map[{std::clamp(x - offset, 0, old_length - 1),
                                          std::clamp(y - offset, 0, old_length - 1)}];
            }
         }
      }
      else {
         for (int32 y = 0; y < old_length; ++y) {
            for (int32 x = 0; x < old_length; ++x) {
               new_heightmap[{x + offset, y + offset}] =
                  old_terrain.height_map[{x, y}];
            }
         }
      }

      return new_heightmap;
   });

   std::array<async::task<container::dynamic_array_2d<uint8>>, world::terrain::texture_count> texture_weight_maps_extend;

   const auto extend_color_map = [=](const container::dynamic_array_2d<uint32>& old_color_map) {
      container::dynamic_array_2d<uint32> new_color_map{new_length, new_length};

      if (fill_from_edges) {
         for (int32 y = 0; y < new_length; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               new_color_map[{x, y}] =
                  old_color_map[{std::clamp(x - offset, 0, old_length - 1),
                                 std::clamp(y - offset, 0, old_length - 1)}];
            }
         }
      }
      else {
         for (uint32& v : new_color_map) v = 0xff'ff'ff'ffu;

         for (int32 y = 0; y < old_length; ++y) {
            for (int32 x = 0; x < old_length; ++x) {
               new_color_map[{x + offset, y + offset}] = old_color_map[{x, y}];
            }
         }
      }

      return new_color_map;
   };

   async::task color_map_extend = thread_pool.exec([&extend_color_map, &old_terrain] {
      return extend_color_map(old_terrain.color_map);
   });
   async::task light_map_extend = thread_pool.exec([&extend_color_map, &old_terrain] {
      return extend_color_map(old_terrain.light_map);
   });
   async::task light_map_extra_extend =
      thread_pool.exec([&extend_color_map, &old_terrain] {
         return old_terrain.light_map_extra.empty()
                   ? container::dynamic_array_2d<uint32>{}
                   : extend_color_map(old_terrain.light_map_extra);
      });

   for (int32 texture = 0; texture < world::terrain::texture_count; ++texture) {
      texture_weight_maps_extend[texture] = thread_pool.exec(
         [=, &old_texture_weight_map = old_terrain.texture_weight_maps[texture]] {
            container::dynamic_array_2d<uint8> new_weight_map{new_length, new_length};

            if (fill_from_edges) {
               for (int32 y = 0; y < new_length; ++y) {
                  for (int32 x = 0; x < new_length; ++x) {
                     new_weight_map[{x, y}] =
                        old_texture_weight_map[{std::clamp(x - offset, 0, old_length - 1),
                                                std::clamp(y - offset, 0, old_length - 1)}];
                  }
               }
            }
            else {
               if (texture == 0) {
                  for (uint8& v : new_weight_map) v = 0xffu;
               }

               for (int32 y = 0; y < old_length; ++y) {
                  for (int32 x = 0; x < old_length; ++x) {
                     new_weight_map[{x + offset, y + offset}] =
                        old_texture_weight_map[{x, y}];
                  }
               }
            }

            return new_weight_map;
         });
   }

   async::task water_map_extend = thread_pool.exec([&old_water_map = old_terrain.water_map,
                                                    old_length = old_length / 4,
                                                    new_length = new_length / 4,
                                                    fill_from_edges] {
      const int32 offset = (new_length - old_length) / 2;

      container::dynamic_array_2d<bool> new_water_map{new_length, new_length};

      if (fill_from_edges) {
         for (int32 y = 0; y < new_length; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               new_water_map[{x, y}] =
                  old_water_map[{std::clamp(x - offset, 0, old_length - 1),
                                 std::clamp(y - offset, 0, old_length - 1)}];
            }
         }
      }
      else {
         for (int32 y = 0; y < old_length; ++y) {
            for (int32 x = 0; x < old_length; ++x) {
               new_water_map[{x + offset, y + offset}] = old_water_map[{x, y}];
            }
         }
      }

      return new_water_map;
   });

   async::task foliage_map_extend = thread_pool.exec(
      [&old_foliage_map = old_terrain.foliage_map, old_length = old_length / 2,
       new_length = new_length / 2, fill_from_edges] {
         const int32 offset = (old_length - new_length) / 2;

         container::dynamic_array_2d<world::foliage_patch> new_foliage_map{new_length,
                                                                           new_length};
         if (fill_from_edges) {
            for (int32 y = 0; y < new_length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  new_foliage_map[{x, y}] =
                     old_foliage_map[{std::clamp(x - offset, 0, old_length - 1),
                                      std::clamp(y - offset, 0, old_length - 1)}];
               }
            }
         }
         else {
            for (int32 y = 0; y < old_length; ++y) {
               for (int32 x = 0; x < old_length; ++x) {
                  new_foliage_map[{x + offset, y + offset}] =
                     old_foliage_map[{x, y}];
               }
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

      .height_map = heightmap_extend.get(),
      .color_map = color_map_extend.get(),
      .light_map = light_map_extend.get(),
      .light_map_extra = light_map_extra_extend.get(),
      .texture_weight_maps =
         {texture_weight_maps_extend[0].get(), texture_weight_maps_extend[1].get(),
          texture_weight_maps_extend[2].get(), texture_weight_maps_extend[3].get(),
          texture_weight_maps_extend[4].get(), texture_weight_maps_extend[5].get(),
          texture_weight_maps_extend[6].get(), texture_weight_maps_extend[7].get(),
          texture_weight_maps_extend[8].get(), texture_weight_maps_extend[9].get(),
          texture_weight_maps_extend[10].get(), texture_weight_maps_extend[11].get(),
          texture_weight_maps_extend[12].get(), texture_weight_maps_extend[13].get(),
          texture_weight_maps_extend[14].get(), texture_weight_maps_extend[15].get()},

      .water_map = water_map_extend.get(),
      .foliage_map = foliage_map_extend.get()};

   return new_terrain;
}

}

void world_edit::ui_show_terrain_extend() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   bool open = _terrain_edit_tool == terrain_edit_tool::extend;

   if (not std::has_single_bit(static_cast<uint32>(_world.terrain.length))) {
      ImGui::Begin("Extend Terrain Error", &open, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::Text("Can not extend non power of 2 terrain.");

      if (not open) _terrain_edit_tool = terrain_edit_tool::none;

      ImGui::End();

      return;
   }

   if (_world.terrain.length == 1024) {
      ImGui::Begin("Extend Terrain Error", &open, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::Text("Terrain is already at the max size.");

      if (not open) _terrain_edit_tool = terrain_edit_tool::none;

      ImGui::End();

      return;
   }

   if (ImGui::Begin("Extend Terrain", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      if (_terrain_extend_context.new_length == 0) {
         _terrain_extend_context.new_length = _world.terrain.length;
      }

      const int32 min_bit =
         std::countr_zero(static_cast<uint32>(_world.terrain.length)); // 2^5= 32
      constexpr int32 max_bit = 10; // 2^10= 1024

      int32 length_bit = std::clamp(std::countr_zero(static_cast<uint32>(
                                       _terrain_extend_context.new_length)),
                                    min_bit, max_bit);

      if (ImGui::SliderInt("New Length", &length_bit, min_bit, max_bit,
                           get_size_name(length_bit), ImGuiSliderFlags_AlwaysClamp)) {
         _terrain_extend_context.new_length = 1 << length_bit;
      }

      ImGui::Checkbox("Fill From Edges", &_terrain_extend_context.fill_from_edges);

      ImGui::SetItemTooltip("When enabled fill the new terrain area using the "
                            "edges of the existing terrain area.");

      ImGui::BeginDisabled(_terrain_extend_context.new_length <= _world.terrain.length);

      if (ImGui::Button("Extend", {ImGui::CalcItemWidth(), 0.0f})) {
         _edit_stack_world.apply(edits::make_set_terrain(
                                    extend_terrain(_world.terrain,
                                                   _terrain_extend_context.new_length,
                                                   _terrain_extend_context.fill_from_edges,
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