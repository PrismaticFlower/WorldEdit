#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_terrain_area.hpp"
#include "utility/file_pickers.hpp"
#include "world/utility/load_terrain_map.hpp"

#include "imgui.h"

namespace we {

void world_edit::ui_show_terrain_import_color_map() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({250.0f * _display_scale, -1.0f},
                                       {250.0f * _display_scale, -1.0f});

   bool open = _terrain_edit_tool == terrain_edit_tool::import_color_map;

   if (ImGui::Begin("Import Color Map", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Checkbox("As Light Map", &_terrain_import_color_map_context.as_light_map);

      ImGui::SetItemTooltip("Import the image as the terrain's light map "
                            "instead of as it's color map.");

      if (ImGui::Button("Import Color Map", {ImGui::GetContentRegionAvail().x, 0.0f})) {
         static constexpr GUID load_color_map_picker_guid = {0x2130c6b2,
                                                             0xb393,
                                                             0x4763,
                                                             {0xbc, 0x1e, 0x9f,
                                                              0xa4, 0x6e, 0xe5,
                                                              0x4a, 0x5a}};

         auto path = utility::show_file_open_picker(
            {.title = L"Import Color Map",
             .ok_button_label = L"Import",
             .filters = {utility::file_picker_filter{.name = L"Texture",
                                                     .filter = L"*.tga;*.png"}},
             .picker_guid = load_color_map_picker_guid,
             .window = _window,
             .must_exist = true});

         if (path) {
            _terrain_import_color_map_context.error_message.clear();

            try {
               container::dynamic_array_2d<uint32> loaded_color_map =
                  world::load_color_map(*path);

               const std::ptrdiff_t imported_length = loaded_color_map.s_width();

               if (imported_length != _world.terrain.length) {
                  throw world::terrain_map_load_error{fmt::format(
                     "Imported texture weight map length ({0} x {0}) "
                     "does not match terrain length ({1} x {1}).",
                     imported_length, _world.terrain.length)};
               }

               if (_terrain_import_color_map_context.as_light_map) {
                  _edit_stack_world.apply(edits::make_set_terrain_area_light_map(
                                             0, 0, std::move(loaded_color_map)),
                                          _edit_context, {.closed = true});
               }
               else {
                  _edit_stack_world.apply(edits::make_set_terrain_area_color_map(
                                             0, 0, std::move(loaded_color_map)),
                                          _edit_context, {.closed = true});
               }

               open = false;
            }
            catch (world::terrain_map_load_error& e) {
               _terrain_import_texture_weight_map_context.error_message = e.what();
            }
         }
      }

      if (not _terrain_import_texture_weight_map_context.error_message.empty()) {
         ImGui::SeparatorText("Import Error");

         ImGui::TextWrapped(
            _terrain_import_texture_weight_map_context.error_message.c_str());
      }
   }

   if (not open) _terrain_edit_tool = terrain_edit_tool::none;

   ImGui::End();
}

}