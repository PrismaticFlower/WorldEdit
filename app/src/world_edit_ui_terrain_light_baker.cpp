#include "world_edit.hpp"

#include "edits/set_terrain_area.hpp"
#include "edits/set_value.hpp"

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

      ImGui::DragInt("Triangle Samples", &config.triangle_samples, 1.0f, 1,
                     1024, "%d", ImGuiSliderFlags_AlwaysClamp);

      ImGui::SetItemTooltip(
         "Number of samples to take per terrain triangle. "
         "Bake time will scale almost linearly with this. Reduce to low counts "
         "for very fast preview bakes, increase as high as you can stand for "
         "your \"final\" bake.");

      ImGui::Checkbox("Object Shadows", &config.include_object_shadows);

      ImGui::SetItemTooltip(
         "Bake Shadows from objects (in the active layers) into the terrain. "
         "Applies to static lights and Ambient Occlusion.");

      ImGui::Checkbox("Block Shadows", &config.include_block_shadows);

      ImGui::SetItemTooltip(
         "Bake Shadows from blocks (in the active layers) into the terrain. "
         "Applies to static lights and Ambient Occlusion.");

      ImGui::Checkbox("Ambient Occlusion", &config.ambient_occlusion);

      ImGui::SetItemTooltip(
         "Bake Ambient Occlusion into the terrain. Increases bake "
         "times by a lot and may not always be artistically desirable.");

      ImGui::DragInt("Ambient Occlusion Samples", &config.ambient_occlusion_samples,
                     1.0f, 1, 1024, "%d", ImGuiSliderFlags_AlwaysClamp);

      ImGui::SetItemTooltip(
         "Lower this to get faster Ambient Occlusion bakes "
         "at the expense of quality. Increase it to boost the quality.");

      ImGui::Checkbox("Bake PS2 Light Map", &config.bake_ps2_light_map);

      ImGui::SetItemTooltip(
         "Bake the extra PS2 light map. Only needed when targetting the PS2. "
         "Significantly increases bake time.");

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

      switch (_terrain_light_map_baker->progress_status()) {
      case world::terrain_light_map_baker_status::sampling: {
         ImGui::ProgressBar(_terrain_light_map_baker->sampling_progress(),
                            {-1.0f, 0.0f}, "Sampling lighting...");
      } break;
      case world::terrain_light_map_baker_status::filtering: {
         ImGui::ProgressBar(-0.5f * (float)ImGui::GetTime(), {-1.0f, 0.0f},
                            "Filtering lighting...");
      } break;
      case world::terrain_light_map_baker_status::sampling_ps2: {
         ImGui::ProgressBar(_terrain_light_map_baker->sampling_ps2_progress(),
                            {-1.0f, 0.0f}, "Sampling PS2 lighting...");
      } break;
      case world::terrain_light_map_baker_status::filtering_ps2: {
         ImGui::ProgressBar(-0.5f * (float)ImGui::GetTime(), {-1.0f, 0.0f},
                            "Filtering PS2 lighting...");
      } break;
      }
   }

   ImGui::End();

   if (_terrain_light_map_baker->ready()) {
      container::dynamic_array_2d<uint32> light_map =
         _terrain_light_map_baker->light_map();
      container::dynamic_array_2d<uint32> light_map_dynamic_ps2 =
         _terrain_light_map_baker->light_map_dynamic_ps2();

      _terrain_light_map_baking = false;
      _terrain_light_map_baker = std::nullopt;

      if (light_map.s_width() != _world.terrain.length) {
         return;
      }

      _edit_stack_world.apply(edits::make_set_terrain_area_light_map(0, 0, std::move(light_map)),
                              _edit_context, {.closed = true});

      if (not light_map_dynamic_ps2.empty()) {
         _edit_stack_world.apply(edits::make_set_value(&_world.terrain.light_map_extra,
                                                       std::move(light_map_dynamic_ps2)),
                                 _edit_context,
                                 {.closed = true, .transparent = true});
      }
   }
}

}