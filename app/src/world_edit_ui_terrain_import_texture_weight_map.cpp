#include "world_edit.hpp"

#include "edits/bundle.hpp"
#include "edits/set_terrain.hpp"
#include "edits/set_terrain_area.hpp"
#include "edits/set_value.hpp"
#include "utility/file_pickers.hpp"
#include "world/utility/load_terrain_map.hpp"

#include "imgui.h"

namespace we {

void world_edit::ui_show_terrain_import_texture_weight_map() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});

   bool open = _terrain_edit_tool == terrain_edit_tool::import_texture_weight_map;

   if (ImGui::Begin("Import Texture Weight Map", &open,
                    ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SeparatorText("Texture");

      for (int i = 0; i < world::terrain::texture_count; ++i) {
         const float size = 64.0f * _display_scale;

         ImGui::PushID(i);

         const ImVec2 cursor_position = ImGui::GetCursorPos();

         if (ImGui::Selectable("##select",
                               _terrain_import_texture_weight_map_context.weight_map_index == i,
                               ImGuiSelectableFlags_None, {size, size})) {
            _terrain_import_texture_weight_map_context.weight_map_index = i;
         }

         const uint32 texture_id =
            _renderer->request_imgui_texture_id(_world.terrain.texture_names[i],
                                                graphics::fallback_imgui_texture::missing_diffuse);

         ImGui::SetCursorPos(cursor_position);
         ImGui::Image(texture_id, {size, size});

         if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%u - %s", i, _world.terrain.texture_names[i].c_str());
         }

         ImGui::PopID();

         if ((i + 1) % 4 != 0) {
            ImGui::SameLine();
         }
      }

      ImGui::Separator();

      const int weight_map_index =
         _terrain_import_texture_weight_map_context.weight_map_index;

      ImGui::BeginDisabled(weight_map_index < 0 or
                           weight_map_index >= world::terrain::texture_count);

      if (ImGui::Button("Import Texture Weight Map",
                        {ImGui::GetContentRegionAvail().x, 0.0f})) {
         static constexpr GUID load_texture_weight_map_picker_guid =
            {0xe4be48b6, 0x7e63, 0x48d1, {0x98, 0xa6, 0x70, 0xe4, 0xf9, 0x10, 0x48, 0x7c}};

         auto path = utility::show_file_open_picker(
            {.title = L"Import Texture Weight Map",
             .ok_button_label = L"Import",
             .filters = {utility::file_picker_filter{.name = L"Texture",
                                                     .filter = L"*.tga;*.png"}},
             .picker_guid = load_texture_weight_map_picker_guid,
             .window = _window,
             .must_exist = true});

         if (path) {
            _terrain_import_texture_weight_map_context.error_message.clear();

            try {
               container::dynamic_array_2d<uint8> loaded_weight_map =
                  world::load_texture_weight_map(*path);

               const std::ptrdiff_t imported_length = loaded_weight_map.s_width();

               if (imported_length != _world.terrain.length) {
                  throw world::terrain_map_load_error{fmt::format(
                     "Imported texture weight map length ({0} x {0}) "
                     "does not match terrain length ({1} x {1}).",
                     imported_length, _world.terrain.length)};
               }

               if (weight_map_index < 0 or
                   weight_map_index >= world::terrain::texture_count) {
                  std::terminate();
               }

               _edit_stack_world.apply(edits::make_set_terrain_area(0, 0, weight_map_index,
                                                                    std::move(loaded_weight_map)),
                                       _edit_context, {.closed = true});

               open = false;
            }
            catch (world::terrain_map_load_error& e) {
               _terrain_import_texture_weight_map_context.error_message = e.what();
            }
         }
      }

      ImGui::EndDisabled();

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