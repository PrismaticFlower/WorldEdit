
#include "world_edit.hpp"

#include "container/enum_array.hpp"
#include "imgui_ext.hpp"
#include "math/vector_funcs.hpp"
#include "utility/file_pickers.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace std::literals;

namespace we {

namespace {

enum class resolution { x128, x256, x512, x1024, x2048, x4096, count };

constexpr container::enum_array<uint32, resolution> resolution_values =
   container::make_enum_array<uint32, resolution>({{resolution::x128, 128},
                                                   {resolution::x256, 256},
                                                   {resolution::x512, 512},
                                                   {resolution::x1024, 1024},
                                                   {resolution::x2048, 2048},
                                                   {resolution::x4096, 4096}});

constexpr container::enum_array<const char*, resolution> resolution_names =
   container::make_enum_array<const char*, resolution>(
      {{resolution::x128, "128 x 128"},
       {resolution::x256, "256 x 256"},
       {resolution::x512, "512 x 512"},
       {resolution::x1024, "1024 x 1024"},
       {resolution::x2048, "2048 x 2048"},
       {resolution::x4096, "4096 x 4096"}});

auto to_resolution_enum_index(const uint32 value) -> int
{
   for (int i = 0; i < resolution_values.size(); ++i) {
      if (resolution_values[i] == value) return i;
   }

   return std::to_underlying(resolution::x512);
}

}

void world_edit::ui_show_render_env_map() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 660.0f * _display_scale},
                           ImGuiCond_FirstUseEver, {0.0f, 0.0f});

   if (ImGui::Begin("Render Environment Map", &_render_env_map_open,
                    ImGuiWindowFlags_AlwaysAutoResize)) {
      int resolution = to_resolution_enum_index(_env_map_render_params.length);

      if (ImGui::SliderInt("Resolution", &resolution, 0,
                           static_cast<int>(resolution_values.size()) - 1,
                           resolution_names[resolution], ImGuiSliderFlags_AlwaysClamp)) {
         _env_map_render_params.length = resolution_values[resolution];
      }

      ImGui::DragFloat3("Render Offset", &_env_map_render_offset);

      float3 selection_centre;

      _env_map_render_params.positionWS = selection_centre + _env_map_render_offset;

      if (ImGui::Button("Render & Save", {ImGui::CalcItemWidth(), 0.0f})) {
         static constexpr GUID save_env_map_picker_guid = {0xc0bd5c3d,
                                                           0x90e9,
                                                           0x443d,
                                                           {0xaa, 0x64, 0x8d, 0x8f,
                                                            0xc0, 0x7f, 0xb5, 0x46}};

         auto path = utility::show_file_save_picker(
            {.title = L"Save Environment Map"s,
             .ok_button_label = L"Save"s,
             .default_folder = _project_dir,
             .filters = {utility::file_picker_filter{.name = L"Truevision TGA"s,
                                                     .filter = L"*.tga"s}},
             .picker_guid = save_env_map_picker_guid,
             .window = _window});

         if (path) {
            _env_map_save_path = std::move(*path);
            _env_map_render_requested = true;
         }
      }

      if (not _env_map_save_error.empty()) {
         ImGui::SeparatorText("Error");
         ImGui::Text("An error occured while saving the environment map!");
         ImGui::Separator();
         ImGui::TextUnformatted(_env_map_save_error.c_str(),
                                _env_map_save_error.c_str() +
                                   _env_map_save_error.size());
      }
   }

   ImGui::End();
}

}