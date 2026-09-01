#include "world_edit.hpp"

#include "edits/set_terrain_area.hpp"

#include "math/scalar_funcs.hpp"

#include "imgui.h"

#include <algorithm>
#include <numbers>

namespace we {

void world_edit::ui_show_terrain_blur_height_map() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   bool open = _terrain_edit_tool == terrain_edit_tool::blur_height_map;

   if (ImGui::Begin("Blur Heightmap", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      bool terrain_needs_update = ImGui::IsWindowAppearing();

      if (_terrain_blur_height_map_context.height_map.s_width() !=
             _world.terrain.length or
          _terrain_blur_height_map_context.height_map.s_height() !=
             _world.terrain.length) {
         _terrain_blur_height_map_context.height_map = _world.terrain.height_map;
      }

      terrain_needs_update |=
         ImGui::SliderInt("Radius", &_terrain_blur_height_map_config.radius, 1,
                          _world.terrain.length / 2, "%d",
                          ImGuiSliderFlags_AlwaysClamp);

      if (terrain_needs_update) {
         container::dynamic_array_2d<int16> x_height_map{_world.terrain.length,
                                                         _world.terrain.length};
         container::dynamic_array_2d<int16> y_height_map{_world.terrain.length,
                                                         _world.terrain.length};

         const int32 radius = _terrain_blur_height_map_config.radius;

         for (int32 y = 0; y < _world.terrain.length; ++y) {
            for (int32 x = 0; x < _world.terrain.length; ++x) {
               int32 sum = 0;

               for (int32 sample = -radius; sample <= radius; ++sample) {
                  sum +=
                     _terrain_blur_height_map_context
                        .height_map[{std::clamp(x + sample, 0, _world.terrain.length - 1), y}];
               }

               x_height_map[{x, y}] = static_cast<int16>(sum / (radius * 2 + 1));
            }
         }

         for (int32 y = 0; y < _world.terrain.length; ++y) {
            for (int32 x = 0; x < _world.terrain.length; ++x) {
               int32 sum = 0;

               for (int32 sample = -radius; sample <= radius; ++sample) {
                  sum +=
                     x_height_map[{x, std::clamp(y + sample, 0, _world.terrain.length - 1)}];
               }

               y_height_map[{x, y}] = static_cast<int16>(sum / (radius * 2 + 1));
            }
         }

         _edit_stack_world.apply(edits::make_set_terrain_area(0, 0, std::move(y_height_map)),
                                 _edit_context);
      }

      if (ImGui::Button("Done", {ImGui::CalcItemWidth(), 0.0f})) {
         open = false;
         _edit_stack_world.close_last();
      }
   }

   if (not open) {
      set_terrain_edit_tool(terrain_edit_tool::none);
   }

   ImGui::End();
}

}