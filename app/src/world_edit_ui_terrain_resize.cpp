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

auto catmull_rom(const float a, const float b, const float c, const float d,
                 const float t) -> float
{
   const float t2 = t * t;
   const float t3 = t * t * t;

   return 0.5f * ((2.0f * b) + (-a + c) * t + (2.0f * a - 5.0f * b + 4.0f * c - d) * t2 +
                  (-a + 3.0f * b - 3.0f * c + d) * t3);
}

template<typename T>
auto lerp(const T a, const T b, const float t) -> T
{
   return a * (1.0f - t) + b * t;
}

auto resize_terrain(const world::terrain& old_terrain, const int32 new_length,
                    const bool maintain_world_proportions, const bool cubic_interpolation,
                    async::thread_pool& thread_pool) -> world::terrain
{
   assert(new_length != old_terrain.length);

   async::task heightmap_resize = thread_pool.exec([=, &old_terrain] {
      if (new_length > old_terrain.length) {
         if (cubic_interpolation) {
            container::dynamic_array_2d<int16>
               intermediate_heightmap{new_length, old_terrain.length};

            const int32 footprint = new_length / old_terrain.length;
            const float inv_footprint = 1.0f / footprint;

            for (int32 y = 0; y < old_terrain.length; ++y) {
               for (int32 x = 0; x < old_terrain.length; ++x) {
                  const float a =
                     old_terrain.height_map[{std::clamp(x - 1, 0, old_terrain.length - 1), y}];
                  const float b = old_terrain.height_map[{x, y}];
                  const float c =
                     old_terrain.height_map[{std::clamp(x + 1, 0, old_terrain.length - 1), y}];
                  const float d =
                     old_terrain.height_map[{std::clamp(x + 2, 0, old_terrain.length - 1), y}];

                  for (int32 i = 0; i < footprint; ++i) {
                     intermediate_heightmap[{x * footprint + i, y}] =
                        static_cast<int16>(
                           catmull_rom(a, b, c, d, i * inv_footprint) + 0.5f);
                  }
               }
            }

            container::dynamic_array_2d<int16> new_heightmap{new_length, new_length};

            const int32 y_offset = footprint - 1;

            for (int32 y = 0; y < old_terrain.length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  const float a =
                     intermediate_heightmap[{x, std::clamp(y - 1, 0, old_terrain.length - 1)}];
                  const float b = intermediate_heightmap[{x, y}];
                  const float c =
                     intermediate_heightmap[{x, std::clamp(y + 1, 0, old_terrain.length - 1)}];
                  const float d =
                     intermediate_heightmap[{x, std::clamp(y + 2, 0, old_terrain.length - 1)}];

                  for (int32 i = 0; i < footprint; ++i) {
                     new_heightmap[{x, std::clamp(y * footprint + i + y_offset, 0, new_length - 1)}] =
                        static_cast<int16>(
                           catmull_rom(a, b, c, d, i * inv_footprint) + 0.5f);
                  }
               }
            }

            for (int32 y = 0; y < y_offset; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  new_heightmap[{x, y}] = intermediate_heightmap[{x, 0}];
               }
            }

            return new_heightmap;
         }
         else {
            container::dynamic_array_2d<int16>
               intermediate_heightmap{new_length, old_terrain.length};

            const int32 footprint = new_length / old_terrain.length;
            const float inv_footprint = 1.0f / footprint;

            for (int32 y = 0; y < old_terrain.length; ++y) {
               for (int32 x = 0; x < old_terrain.length; ++x) {
                  const float a = old_terrain.height_map[{x, y}];
                  const float b =
                     old_terrain.height_map[{std::clamp(x + 1, 0, old_terrain.length - 1), y}];

                  for (int32 i = 0; i < footprint; ++i) {
                     intermediate_heightmap[{x * footprint + i, y}] =
                        static_cast<int16>(lerp(a, b, i * inv_footprint) + 0.5f);
                  }
               }
            }

            container::dynamic_array_2d<int16> new_heightmap{new_length, new_length};

            const int32 y_offset = footprint - 1;

            for (int32 y = 0; y < old_terrain.length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  const float a =
                     intermediate_heightmap[{x, std::clamp(y, 0, old_terrain.length - 1)}];
                  const float b =
                     intermediate_heightmap[{x, std::clamp(y + 1, 0, old_terrain.length - 1)}];

                  for (int32 i = 0; i < footprint; ++i) {
                     new_heightmap[{x, std::clamp(y * footprint + i + y_offset, 0, new_length - 1)}] =
                        static_cast<int16>(lerp(a, b, i * inv_footprint) + 0.5f);
                  }
               }
            }

            for (int32 y = 0; y < y_offset; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  new_heightmap[{x, y}] = intermediate_heightmap[{x, 0}];
               }
            }

            return new_heightmap;
         }
      }
      else {
         container::dynamic_array_2d<int16> intermediate_heightmap{new_length,
                                                                   old_terrain.length};

         const int32 footprint = old_terrain.length / new_length;

         for (int32 y = 0; y < old_terrain.length; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               int32 total = 0;

               for (int32 i = 0; i < footprint; ++i) {
                  total += old_terrain.height_map[{x * footprint + i, y}];
               }

               intermediate_heightmap[{x, y}] = static_cast<int16>(total / footprint);
            }
         }

         container::dynamic_array_2d<int16> new_heightmap{new_length, new_length};

         for (int32 y = 0; y < new_length; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               int32 total = 0;

               for (int32 i = 0; i < footprint; ++i) {
                  total += intermediate_heightmap[{x, y * footprint + i}];
               }

               new_heightmap[{x, y}] = static_cast<int16>(total / footprint);
            }
         }

         return new_heightmap;
      }
   });

   std::array<async::task<container::dynamic_array_2d<uint8>>, world::terrain::texture_count> texture_weight_maps_resize;

   const auto resize_color_map = [=, &old_terrain](
                                    const container::dynamic_array_2d<uint32>& old_color_map) {
      if (new_length > old_terrain.length) {
         container::dynamic_array_2d<uint32> intermediate_color_map{new_length,
                                                                    old_terrain.length};

         const int32 footprint = new_length / old_terrain.length;
         const float inv_footprint = 1.0f / footprint;

         for (int32 y = 0; y < old_terrain.length; ++y) {
            for (int32 x = 0; x < old_terrain.length; ++x) {
               const float4 a = utility::unpack_srgb_bgra(old_color_map[{x, y}]);
               const float4 b = utility::unpack_srgb_bgra(
                  old_color_map[{std::clamp(x + 1, 0, old_terrain.length - 1), y}]);

               for (int32 i = 0; i < footprint; ++i) {
                  intermediate_color_map[{x * footprint + i, y}] =
                     utility::pack_srgb_bgra(lerp(a, b, i * inv_footprint));
               }
            }
         }

         container::dynamic_array_2d<uint32> new_color_map{new_length, new_length};

         const int32 y_offset = footprint - 1;

         for (int32 y = 0; y < old_terrain.length; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               const float4 a =
                  utility::unpack_srgb_bgra(intermediate_color_map[{x, y}]);
               const float4 b = utility::unpack_srgb_bgra(
                  intermediate_color_map[{x, std::clamp(y + 1, 0, old_terrain.length - 1)}]);

               for (int32 i = 0; i < footprint; ++i) {
                  new_color_map[{x, std::clamp(y * footprint + i + y_offset, 0, new_length - 1)}] =
                     utility::pack_srgb_bgra(lerp(a, b, i * inv_footprint));
               }
            }
         }

         for (int32 y = 0; y < y_offset; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               new_color_map[{x, y}] = intermediate_color_map[{x, 0}];
            }
         }

         return new_color_map;
      }
      else {
         container::dynamic_array_2d<uint32> intermediate_color_map{new_length,
                                                                    old_terrain.length};

         const int32 footprint = old_terrain.length / new_length;
         const float inv_footprint = 1.0f / footprint;

         for (int32 y = 0; y < old_terrain.length; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               float4 total;

               for (int32 i = 0; i < footprint; ++i) {
                  total += utility::unpack_srgb_bgra(
                     old_color_map[{x * footprint + i, y}]);
               }

               intermediate_color_map[{x, y}] =
                  utility::pack_srgb_bgra(total * inv_footprint);
            }
         }

         container::dynamic_array_2d<uint32> new_color_map{new_length, new_length};

         for (int32 y = 0; y < new_length; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               float4 total;

               for (int32 i = 0; i < footprint; ++i) {
                  total += utility::unpack_srgb_bgra(
                     intermediate_color_map[{x, y * footprint + i}]);
               }

               new_color_map[{x, y}] = utility::pack_srgb_bgra(total * inv_footprint);
            }
         }

         return new_color_map;
      }
   };

   async::task color_map_resize = thread_pool.exec([&resize_color_map, &old_terrain] {
      return resize_color_map(old_terrain.color_map);
   });
   async::task light_map_resize = thread_pool.exec([&resize_color_map, &old_terrain] {
      return resize_color_map(old_terrain.light_map);
   });
   async::task light_map_extra_resize =
      thread_pool.exec([&resize_color_map, &old_terrain] {
         return old_terrain.light_map_extra.empty()
                   ? container::dynamic_array_2d<uint32>{}
                   : resize_color_map(old_terrain.light_map_extra);
      });

   for (int32 texture = 0; texture < world::terrain::texture_count; ++texture) {
      texture_weight_maps_resize[texture] = thread_pool.exec([=,
                                                              &old_texture_weight_map =
                                                                 old_terrain.texture_weight_maps[texture],
                                                              &old_terrain] {
         if (new_length > old_terrain.length) {
            container::dynamic_array_2d<uint8>
               intermediate_weight_map{new_length, old_terrain.length};

            const int32 footprint = new_length / old_terrain.length;
            const float inv_footprint = 1.0f / footprint;

            for (int32 y = 0; y < old_terrain.length; ++y) {
               for (int32 x = 0; x < old_terrain.length; ++x) {
                  const float a = old_texture_weight_map[{x, y}];
                  const float b =
                     old_texture_weight_map[{std::clamp(x + 1, 0, old_terrain.length - 1), y}];

                  for (int32 i = 0; i < footprint; ++i) {
                     intermediate_weight_map[{x * footprint + i, y}] =
                        static_cast<uint8>(lerp(a, b, i * inv_footprint) + 0.5f);
                  }
               }
            }

            container::dynamic_array_2d<uint8> new_weight_map{new_length, new_length};

            const int32 y_offset = footprint - 1;

            for (int32 y = 0; y < old_terrain.length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  const float a =
                     intermediate_weight_map[{x, std::clamp(y, 0, old_terrain.length - 1)}];
                  const float b =
                     intermediate_weight_map[{x, std::clamp(y + 1, 0,
                                                            old_terrain.length - 1)}];

                  for (int32 i = 0; i < footprint; ++i) {
                     new_weight_map[{x, std::clamp(y * footprint + i + y_offset, 0, new_length - 1)}] =
                        static_cast<uint8>(lerp(a, b, i * inv_footprint) + 0.5f);
                  }
               }
            }

            for (int32 y = 0; y < y_offset; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  new_weight_map[{x, y}] = intermediate_weight_map[{x, 0}];
               }
            }

            return new_weight_map;
         }
         else {
            container::dynamic_array_2d<uint8>
               intermediate_weight_map{new_length, old_terrain.length};

            const int32 footprint = old_terrain.length / new_length;

            for (int32 y = 0; y < old_terrain.length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  int32 total = 0;

                  for (int32 i = 0; i < footprint; ++i) {
                     total += old_texture_weight_map[{x * footprint + i, y}];
                  }

                  intermediate_weight_map[{x, y}] =
                     static_cast<uint8>(total / footprint);
               }
            }

            container::dynamic_array_2d<uint8> new_weight_map{new_length, new_length};

            for (int32 y = 0; y < new_length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  int32 total = 0;

                  for (int32 i = 0; i < footprint; ++i) {
                     total += intermediate_weight_map[{x, y * footprint + i}];
                  }

                  new_weight_map[{x, y}] = static_cast<uint8>(total / footprint);
               }
            }

            return new_weight_map;
         }
      });
   }

   async::task water_map_resize = thread_pool.exec([&old_water_map = old_terrain.water_map,
                                                    old_length = old_terrain.length / 4,
                                                    new_length = new_length / 4] {
      container::dynamic_array_2d<bool> new_water_map{new_length, new_length};

      if (new_length > old_length) {
         const int32 footprint = new_length / old_length;

         for (int32 y = 0; y < old_length; ++y) {
            for (int32 x = 0; x < old_length; ++x) {
               const bool water = old_water_map[{x, y}];

               for (int32 y_local = 0; y_local < footprint; ++y_local) {
                  for (int32 x_local = 0; x_local < footprint; ++x_local) {
                     new_water_map[{x * footprint + x_local, y * footprint + y_local}] =
                        water;
                  }
               }
            }
         }
      }
      else {
         const int32 footprint = old_length / new_length;

         for (int32 y = 0; y < new_length; ++y) {
            for (int32 x = 0; x < new_length; ++x) {
               bool water = false;

               for (int32 y_local = 0; y_local < footprint; ++y_local) {
                  for (int32 x_local = 0; x_local < footprint; ++x_local) {
                     water |=
                        old_water_map[{x * footprint + x_local, y * footprint + y_local}];
                  }
               }

               new_water_map[{x, y}] = water;
            }
         }
      }

      return new_water_map;
   });

   async::task foliage_map_resize = thread_pool.exec(
      [&old_foliage_map = old_terrain.foliage_map,
       old_length = old_terrain.length / 2, new_length = new_length / 2] {
         container::dynamic_array_2d<world::foliage_patch> new_foliage_map{new_length,
                                                                           new_length};

         if (new_length > old_length) {
            const int32 footprint = new_length / old_length;

            for (int32 y = 0; y < old_length; ++y) {
               for (int32 x = 0; x < old_length; ++x) {
                  const world::foliage_patch patch = old_foliage_map[{x, y}];

                  for (int32 y_local = 0; y_local < footprint; ++y_local) {
                     for (int32 x_local = 0; x_local < footprint; ++x_local) {
                        new_foliage_map[{x * footprint + x_local, y * footprint + y_local}] =
                           patch;
                     }
                  }
               }
            }
         }
         else {
            const int32 footprint = old_length / new_length;

            for (int32 y = 0; y < new_length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  world::foliage_patch combined_patch{};

                  for (int32 y_local = 0; y_local < footprint; ++y_local) {
                     for (int32 x_local = 0; x_local < footprint; ++x_local) {
                        const world::foliage_patch patch =
                           old_foliage_map[{x * footprint + x_local, y * footprint + y_local}];

                        combined_patch.layer0 |= patch.layer0;
                        combined_patch.layer1 |= patch.layer1;
                        combined_patch.layer2 |= patch.layer2;
                        combined_patch.layer3 |= patch.layer3;
                     }
                  }
                  new_foliage_map[{x, y}] = combined_patch;
               }
            }
         }

         return new_foliage_map;
      });

   const float world_scale =
      maintain_world_proportions
         ? static_cast<float>(old_terrain.length) / static_cast<float>(new_length)
         : 1.0f;

   world::terrain new_terrain{
      .version = old_terrain.version,
      .length = new_length,

      .height_scale = old_terrain.height_scale,
      .grid_scale = old_terrain.grid_scale * world_scale,
      .active_flags = old_terrain.active_flags,
      .prelit = old_terrain.prelit,

      .water_settings = old_terrain.water_settings,

      .texture_names = old_terrain.texture_names,
      .texture_scales = old_terrain.texture_scales,
      .texture_axes = old_terrain.texture_axes,
      .detail_texture_name = old_terrain.detail_texture_name,

      .height_map = heightmap_resize.get(),
      .color_map = color_map_resize.get(),
      .light_map = light_map_resize.get(),
      .light_map_extra = light_map_extra_resize.get(),
      .texture_weight_maps =
         {texture_weight_maps_resize[0].get(), texture_weight_maps_resize[1].get(),
          texture_weight_maps_resize[2].get(), texture_weight_maps_resize[3].get(),
          texture_weight_maps_resize[4].get(), texture_weight_maps_resize[5].get(),
          texture_weight_maps_resize[6].get(), texture_weight_maps_resize[7].get(),
          texture_weight_maps_resize[8].get(), texture_weight_maps_resize[9].get(),
          texture_weight_maps_resize[10].get(), texture_weight_maps_resize[11].get(),
          texture_weight_maps_resize[12].get(), texture_weight_maps_resize[13].get(),
          texture_weight_maps_resize[14].get(), texture_weight_maps_resize[15].get()},

      .water_map = water_map_resize.get(),
      .foliage_map = foliage_map_resize.get()};

   return new_terrain;
}

}

void world_edit::ui_show_terrain_resize() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   if (not std::has_single_bit(static_cast<uint32>(_world.terrain.length))) {
      ImGui::Begin("Resize Terrain Error", &_terrain_resize_open,
                   ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::Text("Can not resize non power of 2 terrain.");

      ImGui::End();

      return;
   }

   if (ImGui::Begin("Resize Terrain", &_terrain_resize_open,
                    ImGuiWindowFlags_AlwaysAutoResize)) {
      if (_terrain_resize_context.new_length == 0) {
         _terrain_resize_context.new_length = _world.terrain.length;
      }

      constexpr int32 min_bit = 5;  // 2^5= 32
      constexpr int32 max_bit = 10; // 2^10= 1024

      int32 length_bit = std::clamp(std::countr_zero(static_cast<uint32>(
                                       _terrain_resize_context.new_length)),
                                    min_bit, max_bit);

      if (ImGui::SliderInt("New Length", &length_bit, min_bit, max_bit,
                           get_size_name(length_bit), ImGuiSliderFlags_AlwaysClamp)) {
         _terrain_resize_context.new_length = 1 << length_bit;
      }

      ImGui::Checkbox("Maintain World Proportions",
                      &_terrain_resize_context.maintain_world_proportions);

      ImGui::SetItemTooltip(
         "When enabled the resized the terrain's height and grid scale will be "
         "adjusted "
         "to keep the terrain roughly the same size and shape in the "
         "world.");

      ImGui::BeginDisabled(_terrain_resize_context.new_length < _world.terrain.length);

      ImGui::Checkbox("Cubic Interpolation",
                      &_terrain_resize_context.cubic_interpolation);

      ImGui::EndDisabled();

      if (_terrain_resize_context.new_length < _world.terrain.length) {
         ImGui::SetItemTooltip("Cubic interpolation is only available when "
                               "upscaling the terrain.");
      }
      else {
         ImGui::SetItemTooltip(
            "When enabled uses cubic interpolation when resizing the terrain "
            "heightmap. "
            "The upscaled terrain can appear smoother than the original but "
            "may "
            "also have ringing artifacts.");
      }

      ImGui::BeginDisabled(_terrain_resize_context.new_length == _world.terrain.length);

      if (ImGui::Button("Resize", {ImGui::CalcItemWidth(), 0.0f})) {
         _edit_stack_world.apply(edits::make_set_terrain(
                                    resize_terrain(_world.terrain,
                                                   _terrain_resize_context.new_length,
                                                   _terrain_resize_context.maintain_world_proportions,
                                                   _terrain_resize_context.cubic_interpolation,
                                                   *_thread_pool)),
                                 _edit_context, {.closed = true});

         _terrain_resize_open = false;
      }

      ImGui::EndDisabled();
   }

   ImGui::End();
}

}