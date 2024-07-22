#include "world_edit.hpp"

#include "edits/set_terrain_area.hpp"

#include "imgui.h"

namespace we {

void world_edit::ui_show_terrain_light_baker() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   bool open = _terrain_edit_tool == terrain_edit_tool::light_baker;

   if (ImGui::Begin("Bake Terrain Lighting", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SeparatorText("Bake Settings");

      world::terrain_light_map_baker_config& config = _terrain_light_baker_config;

      ImGui::Checkbox("Object Shadows", &config.include_object_shadows);

      ImGui::SetItemTooltip(
         "Bake Shadows from objects (in the active layers) into the terrain. "
         "Applies to static lights and Ambient Occlusion.");

      ImGui::Checkbox("Ambient Occlusion", &config.ambient_occlusion);

      ImGui::SetItemTooltip(
         "Bake Ambient Occlusion into the terrain. Increases bake "
         "times by a lot and may not always be artistically desirable.");

      ImGui::DragInt("Ambient Occlusion Samples", &config.ambient_occlusion_samples,
                     1.0f, 1, 1024, "%d", ImGuiSliderFlags_AlwaysClamp);

      ImGui::SetItemTooltip(
         "Lower this to get much, much faster Ambient Occlusion bakes "
         "at the expense of quality.");

      ImGui::Checkbox("Supersample", &config.supersample);

      ImGui::SetItemTooltip(
         "Sample the lighting multiple times for each terrain grid point. "
         "Increases quality and bake time.");

      ImGui::Separator();

      if (ImGui::Button("Bake!", {ImGui::CalcItemWidth(), 0.0f})) {
         _terrain_light_map_baking = true;
         _terrain_light_map_baker.emplace(_world, _object_classes,
                                          _world_layers_draw_mask,
                                          *_thread_pool, config);
      }
   }

   if (not open) _terrain_edit_tool = terrain_edit_tool::none;

   ImGui::End();
}

void world_edit::ui_show_terrain_light_bake_progress() noexcept
{
   if (not _terrain_light_map_baker) return;

   ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
                           ImGuiCond_Appearing, {0.5f, 0.5f});
   ImGui::SetNextWindowSize({400.0f * _display_scale, 64.0f * _display_scale});

   if (ImGui::Begin("Baking Lighting...", nullptr,
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
      ImGui::ProgressBar(_terrain_light_map_baker->progress());
   }

   ImGui::End();

   if (_terrain_light_map_baker->ready()) {
      container::dynamic_array_2d<uint32> light_map =
         _terrain_light_map_baker->light_map();

      _terrain_light_map_baking = false;
      _terrain_light_map_baker = std::nullopt;

      if (light_map.s_width() != _world.terrain.length) {
         return;
      }

      _edit_stack_world.apply(edits::make_set_terrain_area_light_map(0, 0, std::move(light_map)),
                              _edit_context, {.closed = true});
   }
}

}