#include "world_edit.hpp"

#include "edits/imgui_ext.hpp"
#include "edits/set_value.hpp"

namespace we {

void world_edit::ui_show_world_config_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({512.0f * _display_scale, 256.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({400.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("World Configuration", &_world_layers_editor_open)) {
      ImGui::Checkbox("Save in BF1 Format", &_world.configuration.save_bf1_format,
                      _edit_stack_world, _edit_context);

      ImGui::SetItemTooltip(
         "Save world in formats friendly to BF1's toolchain. Terrain "
         "version is controlled seperately.");

      ImGui::Checkbox("Save Effects", &_world.configuration.save_effects,
                      _edit_stack_world, _edit_context);

      ImGui::SetItemTooltip(
         "Save the world .fx file. Note that this won't disable "
         "edit controls for values from the .fx file.");

      ImGui::SeparatorText("Blocks Mode");

      if (ImGui::BeginTable("Blocks Mode", 2,
                            ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoSavedSettings,
                            {ImGui::CalcItemWidth(), 0.0f})) {
         ImGui::TableNextColumn();

         if (ImGui::RadioButton("WorldEdit Munged",
                                not _world.configuration.save_blocks_into_layer)) {
            _edit_stack_world.apply(edits::make_set_value(&_world.configuration.save_blocks_into_layer,
                                                          false),
                                    _edit_context, {.closed = true});
         }

         ImGui::SetItemTooltip(
            "Blocks will only appear ingame when munged through "
            "WorldEdit. Blocks will have uncompresseds UVs on PC.");

         ImGui::TableNextColumn();

         if (ImGui::RadioButton("Saved to Layer",
                                _world.configuration.save_blocks_into_layer)) {
            _edit_stack_world.apply(edits::make_set_value(&_world.configuration.save_blocks_into_layer,
                                                          true),
                                    _edit_context, {.closed = true});
         }

         ImGui::SetItemTooltip(
            "Blocks will be saved into a special hidden layer named "
            "'{WORLD}_WE_blocks'. A set of ODFs and MSHs for the layer "
            "will be saved into a 'blocks\\' folder relative to the "
            "world.");

         ImGui::EndTable();
      }

      ImGui::Checkbox("Save Lights References",
                      &_world.configuration.save_lights_references,
                      _edit_stack_world, _edit_context);

      ImGui::SetItemTooltip("Save LightName in .wld/.lyr files.");

      ImGui::Checkbox("Save Sky Reference", &_world.configuration.save_sky_reference,
                      _edit_stack_world, _edit_context);

      ImGui::SetItemTooltip("Save SkyName in the .wld file.");
   }

   ImGui::End();
}

}