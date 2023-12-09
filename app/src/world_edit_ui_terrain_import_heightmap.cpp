#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_terrain.hpp"
#include "edits/set_terrain_area.hpp"
#include "edits/set_value.hpp"
#include "utility/file_pickers.hpp"
#include "world/utility/load_terrain_heightmap.hpp"

#include "imgui.h"

namespace we {

void world_edit::ui_show_terrain_import_height_map() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   if (ImGui::Begin("Import Heightmap", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      if (ImGui::Button("Import Heightmap", {ImGui::CalcItemWidth(), 0.0f})) {
         static constexpr GUID load_heightmap_picker_guid = {0xe9ec2fc8,
                                                             0x774c,
                                                             0x4004,
                                                             {0xaa, 0xa6, 0xe4,
                                                              0x55, 0x1e, 0x62,
                                                              0x3d, 0x63}};

         auto path = utility::show_file_open_picker(
            {.title = L"Import Heightmap",
             .ok_button_label = L"Import",
             .filters = {utility::file_picker_filter{.name = L"Texture",
                                                     .filter = L"*.tga;*.png"}},
             .picker_guid = load_heightmap_picker_guid,
             .window = _window,
             .must_exist = true});

         if (path) {
            _terrain_import_heightmap_context.loaded_heightmap = {};
            _terrain_import_heightmap_context.error_message.clear();

            try {
               _terrain_import_heightmap_context.loaded_heightmap =
                  world::load_heightmap(*path);
               _terrain_import_heightmap_context.heightmap_peak_height = 25.5f;
               _terrain_import_heightmap_context.heightmap_terrain_world_size =
                  _terrain_import_heightmap_context.loaded_heightmap.shape()[0] * 8.0f;
            }
            catch (world::heightmap_load_error& e) {
               _terrain_import_heightmap_context.error_message = e.what();
            }
         }
      }

      if (not _terrain_import_heightmap_context.error_message.empty()) {
         ImGui::SeparatorText("Heightmap Import Error");

         ImGui::TextWrapped(_terrain_import_heightmap_context.error_message.c_str());
      }

      ImGui::BeginDisabled(_terrain_import_heightmap_context.loaded_heightmap.empty());

      ImGui::SeparatorText("Heightmap Import Options");

      if (ImGui::DragFloat("Heightmap Peak Height",
                           &_terrain_import_heightmap_context.heightmap_peak_height,
                           1.0f, 1.0f, 1e10f)) {
         _terrain_import_heightmap_context.heightmap_peak_height =
            std::max(_terrain_import_heightmap_context.heightmap_peak_height, 1.0f);
      }

      if (ImGui::DragFloat("Heightmap World Size",
                           &_terrain_import_heightmap_context.heightmap_terrain_world_size,
                           1.0f, 1.0f, 1e10f)) {
         _terrain_import_heightmap_context.heightmap_terrain_world_size =
            std::max(_terrain_import_heightmap_context.heightmap_terrain_world_size,
                     1.0f);
      }

      ImGui::Checkbox("Start From Bottom",
                      &_terrain_import_heightmap_context.start_from_bottom);

      ImGui::SetItemTooltip("Import the heightmap at the bottom of the "
                            "terrain instead of the midpoint.");

      ImGui::EndDisabled();

      ImGui::Separator();

      if (ImGui::Button("Done", {ImGui::CalcItemWidth(), 0.0f})) {
         _terrain_import_heightmap_open = false;
         _terrain_import_heightmap_context = {};
         _edit_stack_world.close_last();
      }

      if (not _terrain_import_heightmap_context.loaded_heightmap.empty()) {
         const int32 new_length = static_cast<int32>(
            _terrain_import_heightmap_context.loaded_heightmap.shape()[0]);

         if (_world.terrain.length != new_length) {
            world::terrain terrain{
               .version = _world.terrain.version,
               .length = new_length,
               .active_flags = _world.terrain.active_flags,
               .water_settings = _world.terrain.water_settings,
               .texture_names = _world.terrain.texture_names,
               .texture_scales = _world.terrain.texture_scales,
               .texture_axes = _world.terrain.texture_axes,
               .detail_texture_name = _world.terrain.detail_texture_name,
            };

            for (uint8& weight : terrain.texture_weight_maps[0]) weight = 0xff;
            for (uint32& color : terrain.color_map) color = 0xff'ff'ff'ffu;

            _edit_stack_world.apply(edits::make_set_terrain(std::move(terrain)),
                                    _edit_context);
         }

         edits::bundle_vector bundle;

         container::dynamic_array_2d<int16> height_map{new_length, new_length};

         if (_terrain_import_heightmap_context.start_from_bottom) {
            for (int32 y = 0; y < new_length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  height_map[{x, y}] = static_cast<int16>(
                     -32768 +
                     _terrain_import_heightmap_context.loaded_heightmap[{x, y}]);
               }
            }
         }
         else {
            for (int32 y = 0; y < new_length; ++y) {
               for (int32 x = 0; x < new_length; ++x) {
                  height_map[{x, y}] = static_cast<int16>(
                     _terrain_import_heightmap_context.loaded_heightmap[{x, y}]);
               }
            }
         }

         bundle.push_back(edits::make_set_terrain_area(0, 0, std::move(height_map)));
         bundle.push_back(edits::make_set_global_value(
            &world::world::terrain, &world::terrain::height_scale,
            _terrain_import_heightmap_context.heightmap_peak_height / 255.0f,
            _world.terrain.height_scale));
         bundle.push_back(edits::make_set_global_value(
            &world::world::terrain, &world::terrain::grid_scale,
            _terrain_import_heightmap_context.heightmap_terrain_world_size / new_length,
            _world.terrain.grid_scale));

         _edit_stack_world.apply(edits::make_bundle(std::move(bundle)), _edit_context);
      }
   }

   ImGui::End();
}

}