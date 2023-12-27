#include "world_edit.hpp"

#include "edits/set_terrain_area.hpp"
#include "edits/set_value.hpp"
#include "math/vector_funcs.hpp"
#include "utility/file_pickers.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"
#include "world/utility/load_terrain_brush.hpp"
#include "world/utility/raycast_terrain.hpp"

#include "imgui.h"
#include "imgui_ext.hpp"

#include <numbers>

namespace we {

namespace {

template<typename Value>
auto make_set_terrain_value(Value world::terrain::*value_member_ptr,
                            Value new_value, Value original_value)
   -> std::unique_ptr<edits::edit<world::edit_context>>
{
   return std::make_unique<edits::set_global_value<world::terrain, Value>>(
      &world::world::terrain, value_member_ptr, std::move(new_value),
      std::move(original_value));
}

template<typename Container, typename Value>
auto make_set_terrain_value_indexed(Container world::terrain::*value_member_ptr,
                                    uint32 index, Value new_value, Value original_value)
   -> std::unique_ptr<edits::edit<world::edit_context>>
{
   return std::make_unique<edits::set_global_value_indexed<world::terrain, Container, Value>>(
      &world::world::terrain, value_member_ptr, index, std::move(new_value),
      std::move(original_value));
}

auto get_position(int32 x, int32 y, const world::terrain& terrain) noexcept -> float3
{
   const int32 terrain_half_length = terrain.length / 2;

   return float3{(x - terrain_half_length) * terrain.grid_scale,
                 terrain.height_map[{std::clamp(x, 0, terrain.length - 1),
                                     std::clamp(y, 0, terrain.length - 1)}] *
                    terrain.height_scale,
                 (y - terrain_half_length + 1) * terrain.grid_scale};
}

auto get_rotation_name(const terrain_brush_rotation rotation) -> const char*
{
   if (rotation == terrain_brush_rotation::r0) return "0d";
   if (rotation == terrain_brush_rotation::r90) return "90d";
   if (rotation == terrain_brush_rotation::r180) return "180d";
   if (rotation == terrain_brush_rotation::r270) return "270d";

   return "";
}

struct brush {
   brush(float2 centre, float radius, terrain_brush_falloff falloff,
         terrain_brush_rotation rotation, int32 brush_left, int32 brush_top,
         const container::dynamic_array_2d<uint8>* const custom_brush_falloff_map)
      : centre{centre},
        radius{radius},
        falloff{falloff},
        rotation{rotation},
        brush_left{brush_left},
        brush_top{brush_top}
   {
      if (custom_brush_falloff_map) {
         this->custom_brush_falloff_map = {custom_brush_falloff_map->data(),
                                           custom_brush_falloff_map->size()};

         custom_brush_width =
            static_cast<int32>(custom_brush_falloff_map->sshape()[0]);
         custom_brush_width_max = custom_brush_width - 1;
         custom_brush_height_max =
            static_cast<int32>(custom_brush_falloff_map->sshape()[1]) - 1;
      }
   }

   auto weight(int32 x, int32 y) const noexcept -> float
   {
      const float distance =
         we::distance(float2{static_cast<float>(x), static_cast<float>(y)}, centre);
      const float normalized_distance = std::clamp(distance / radius, 0.0f, 1.0f);

      switch (falloff) {
      case terrain_brush_falloff::none:
         return 1.0f;
      case terrain_brush_falloff::cone:
         return 1.0f - normalized_distance;
      case terrain_brush_falloff::smooth:
         return 1.0f - (normalized_distance * normalized_distance);
      case terrain_brush_falloff::bell:
         return 1.0f - (normalized_distance * normalized_distance *
                        (3.0f - 2.0f * normalized_distance)); // smoothstep, in case you're wondering
      case terrain_brush_falloff::ramp: {
         float ramp_distance = 0.0f;

         switch (rotation) {
         case terrain_brush_rotation::r0:
            ramp_distance = fabs(static_cast<float>(x) - radius - centre.x);
            break;
         case terrain_brush_rotation::r90:
            ramp_distance = fabs(static_cast<float>(y) - radius - centre.y);
            break;
         case terrain_brush_rotation::r180:
            ramp_distance = fabs(static_cast<float>(x) + radius - centre.x);
            break;
         case terrain_brush_rotation::r270:
            ramp_distance = fabs(static_cast<float>(y) + radius - centre.y);
            break;
         }

         return std::clamp(ramp_distance / (radius * 2.0f), 0.0f, 1.0f);
      }
      case terrain_brush_falloff::custom: {
         if (custom_brush_falloff_map.empty()) return 1.0f;

         int32 x_offset = brush_left;
         int32 y_offset = brush_top;

         if (rotation == terrain_brush_rotation::r90 or
             rotation == terrain_brush_rotation::r270) {
            std::swap(x, y);
            std::swap(x_offset, y_offset);
         }

         const int32 clamped_x = std::clamp(x - x_offset, 0, custom_brush_width_max);
         const int32 clamped_y = std::clamp(y - y_offset, 0, custom_brush_height_max);
         const int32 flipped_x = custom_brush_width_max - clamped_x;
         const int32 flipped_y = custom_brush_height_max - clamped_y;

         switch (rotation) {
         case terrain_brush_rotation::r0:
            return custom_brush_falloff_map[clamped_y * custom_brush_width + clamped_x] /
                   255.0f;
         case terrain_brush_rotation::r90:
            return custom_brush_falloff_map[flipped_y * custom_brush_width + clamped_x] /
                   255.0f;
         case terrain_brush_rotation::r180:
            return custom_brush_falloff_map[flipped_y * custom_brush_width + flipped_x] /
                   255.0f;
         case terrain_brush_rotation::r270:
            return custom_brush_falloff_map[clamped_y * custom_brush_width + flipped_x] /
                   255.0f;
         }

         std::unreachable();
      }
      }

      std::unreachable();
   }

   auto visualizer_color(int32 x, int32 y) const noexcept -> uint32
   {
      const uint32 brush_weight = static_cast<uint32>(weight(x, y) * 255.0f + 0.5f);

      return (brush_weight << 24u) | 0x00ffffff;
   }

private:
   float2 centre;
   float radius = 0.0f;
   terrain_brush_falloff falloff = terrain_brush_falloff::none;
   terrain_brush_rotation rotation = terrain_brush_rotation::r0;
   int32 brush_left = 0;
   int32 brush_top = 0;
   std::span<const uint8> custom_brush_falloff_map = {};
   int32 custom_brush_width_max = 0;
   int32 custom_brush_height_max = 0;
   int32 custom_brush_width = 0;
};

struct texture_axis_name {
   world::texture_axis axis = {};
   const char* name = nullptr;
};

constexpr std::array<texture_axis_name, 12> texture_axis_names = {
   texture_axis_name{world::texture_axis::xz, "XZ"},
   texture_axis_name{world::texture_axis::xy, "XY"},
   texture_axis_name{world::texture_axis::yz, "YZ"},
   texture_axis_name{world::texture_axis::zx, "ZX"},
   texture_axis_name{world::texture_axis::yx, "YX"},
   texture_axis_name{world::texture_axis::zy, "ZY"},
   texture_axis_name{world::texture_axis::negative_xz, "Negative XZ"},
   texture_axis_name{world::texture_axis::negative_xy, "Negative XY"},
   texture_axis_name{world::texture_axis::negative_yz, "Negative YZ"},
   texture_axis_name{world::texture_axis::negative_zx, "Negative ZX"},
   texture_axis_name{world::texture_axis::negative_yx, "Negative YX"},
   texture_axis_name{world::texture_axis::negative_zy, "Negative ZY"},
};

}

void world_edit::ui_show_terrain_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSizeConstraints({520.0f * _display_scale, 620.0f * _display_scale},
                                       {std::numeric_limits<float>::max(),
                                        620.0f * _display_scale});

   if (ImGui::Begin("Terrain Editor", &_terrain_editor_open,
                    ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
      ImGui::SeparatorText("Edit Target");

      if (ImGui::BeginTable("Edit Target", 3,
                            ImGuiTableFlags_NoSavedSettings |
                               ImGuiTableFlags_SizingStretchSame)) {

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Height", _terrain_editor_config.edit_target ==
                                            terrain_edit_target::height)) {
            _terrain_editor_config.edit_target = terrain_edit_target::height;
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Texture", _terrain_editor_config.edit_target ==
                                             terrain_edit_target::texture)) {
            _terrain_editor_config.edit_target = terrain_edit_target::texture;
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Colour", _terrain_editor_config.edit_target ==
                                            terrain_edit_target::color)) {
            _terrain_editor_config.edit_target = terrain_edit_target::color;
         }

         ImGui::EndTable();
      }

      if (ImGui::CollapsingHeader("Brush", ImGuiTreeNodeFlags_DefaultOpen)) {
         if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
            terrain_editor_config::height_config& config =
               _terrain_editor_config.height;

            ImGui::SeparatorText("Brush Mode");

            if (ImGui::Selectable("Raise", config.brush_mode ==
                                              terrain_brush_mode::raise)) {
               config.brush_mode = terrain_brush_mode::raise;
            }

            if (ImGui::Selectable("Lower", config.brush_mode ==
                                              terrain_brush_mode::lower)) {
               config.brush_mode = terrain_brush_mode::lower;
            }

            if (ImGui::Selectable("Overwrite", config.brush_mode ==
                                                  terrain_brush_mode::overwrite)) {
               config.brush_mode = terrain_brush_mode::overwrite;
            }

            if (ImGui::Selectable("Pull Towards", config.brush_mode ==
                                                     terrain_brush_mode::pull_towards)) {
               config.brush_mode = terrain_brush_mode::pull_towards;
            }

            if (ImGui::Selectable("Blend", config.brush_mode ==
                                              terrain_brush_mode::blend)) {
               config.brush_mode = terrain_brush_mode::blend;
            }
         }
         else if (_terrain_editor_config.edit_target == terrain_edit_target::texture) {
            terrain_editor_config::texture_config& config =
               _terrain_editor_config.texture;

            ImGui::SeparatorText("Brush Mode");

            if (ImGui::Selectable("Paint", config.brush_mode ==
                                              terrain_texture_brush_mode::paint)) {
               config.brush_mode = terrain_texture_brush_mode::paint;
            }

            if (ImGui::Selectable("Spray", config.brush_mode ==
                                              terrain_texture_brush_mode::spray)) {
               config.brush_mode = terrain_texture_brush_mode::spray;
            }

            if (ImGui::Selectable("Erase", config.brush_mode ==
                                              terrain_texture_brush_mode::erase)) {
               config.brush_mode = terrain_texture_brush_mode::erase;
            }

            if (ImGui::Selectable("Soften", config.brush_mode ==
                                               terrain_texture_brush_mode::soften)) {
               config.brush_mode = terrain_texture_brush_mode::soften;
            }
         }
         else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
            terrain_editor_config::color_config& config = _terrain_editor_config.color;

            ImGui::SeparatorText("Brush Mode");

            if (ImGui::Selectable("Paint", config.brush_mode ==
                                              terrain_color_brush_mode::paint)) {
               config.brush_mode = terrain_color_brush_mode::paint;
            }

            if (ImGui::Selectable("Spray", config.brush_mode ==
                                              terrain_color_brush_mode::spray)) {
               config.brush_mode = terrain_color_brush_mode::spray;
            }

            if (ImGui::Selectable("Blur", config.brush_mode ==
                                             terrain_color_brush_mode::blur)) {
               config.brush_mode = terrain_color_brush_mode::blur;
            }
         }

         terrain_brush_falloff& brush_falloff = [&]() -> terrain_brush_falloff& {
            if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
               return _terrain_editor_config.height.brush_falloff;
            }
            else if (_terrain_editor_config.edit_target == terrain_edit_target::texture) {
               return _terrain_editor_config.texture.brush_falloff;
            }
            else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
               return _terrain_editor_config.color.brush_falloff;
            }

            return _terrain_editor_config.height.brush_falloff;
         }();

         ImGui::SeparatorText("Brush Falloff");

         if (ImGui::Selectable("None", brush_falloff == terrain_brush_falloff::none)) {
            brush_falloff = terrain_brush_falloff::none;
         }

         if (ImGui::Selectable("Cone", brush_falloff == terrain_brush_falloff::cone)) {
            brush_falloff = terrain_brush_falloff::cone;
         }

         if (ImGui::Selectable("Smooth", brush_falloff == terrain_brush_falloff::smooth)) {
            brush_falloff = terrain_brush_falloff::smooth;
         }

         if (ImGui::Selectable("Bell", brush_falloff == terrain_brush_falloff::bell)) {
            brush_falloff = terrain_brush_falloff::bell;
         }

         if (ImGui::Selectable("Ramp", brush_falloff == terrain_brush_falloff::ramp)) {
            brush_falloff = terrain_brush_falloff::ramp;
         }

         if (ImGui::Selectable("Custom", brush_falloff == terrain_brush_falloff::custom)) {
            brush_falloff = terrain_brush_falloff::custom;
         }

         ImGui::SeparatorText("Brush Settings");

         if (brush_falloff != terrain_brush_falloff::custom) {
            if (int32 size = std::max(_terrain_editor_config.brush_size_x,
                                      _terrain_editor_config.brush_size_y);
                ImGui::SliderInt("Size", &size, 0, _world.terrain.length / 2,
                                 "%d", ImGuiSliderFlags_AlwaysClamp)) {
               _terrain_editor_config.brush_size_x = size;
               _terrain_editor_config.brush_size_y = size;
            }

            ImGui::SliderInt("Size (X)", &_terrain_editor_config.brush_size_x,
                             0, _world.terrain.length / 2, "%d",
                             ImGuiSliderFlags_AlwaysClamp);

            ImGui::SliderInt("Size (Y)", &_terrain_editor_config.brush_size_y,
                             0, _world.terrain.length / 2, "%d",
                             ImGuiSliderFlags_AlwaysClamp);
         }

         if (brush_falloff == terrain_brush_falloff::custom) {
            if (ImGui::BeginCombo("Custom Brush", [&] {
                   if (_terrain_editor_config.custom_brush_index >=
                       std::ssize(_terrain_editor_config.custom_brushes)) {
                      return "<none>";
                   }

                   return _terrain_editor_config
                      .custom_brushes[_terrain_editor_config.custom_brush_index]
                      .name.c_str();
                }())) {

               for (int32 i = 0;
                    i < _terrain_editor_config.custom_brushes.size(); ++i) {
                  if (ImGui::Selectable(
                         _terrain_editor_config.custom_brushes[i].name.c_str(),
                         _terrain_editor_config.custom_brush_index == i)) {
                     _terrain_editor_config.custom_brush_index = i;
                  }
               }

               if (ImGui::Selectable("Load...")) {
                  static constexpr GUID load_brush_picker_guid = {0xc5b909c5,
                                                                  0xe07f,
                                                                  0x4380,
                                                                  {0x8a, 0xaf, 0x99,
                                                                   0x41, 0x21, 0xe5,
                                                                   0x2d, 0xea}};

                  auto path = utility::show_file_open_picker(
                     {.title = L"Load brush...",
                      .ok_button_label = L"Import",
                      .filters = {utility::file_picker_filter{
                         .name = L"Texture", .filter = L"*.tga"}},
                      .picker_guid = load_brush_picker_guid,
                      .window = _window,
                      .must_exist = true});

                  if (path) {
                     try {
                        container::dynamic_array_2d<uint8> falloff_map =
                           world::load_brush(*path);
                        std::string name = path->stem().string();

                        assert(falloff_map.shape()[0] % 2 == 1 and
                               falloff_map.shape()[1] % 2 == 1);

                        bool found = false;

                        for (int32 i = 0;
                             i < _terrain_editor_config.custom_brushes.size(); ++i) {
                           auto& brush = _terrain_editor_config.custom_brushes[i];

                           if (string::iequals(brush.name, name)) {
                              brush.falloff_map = std::move(falloff_map);

                              _terrain_editor_config.custom_brush_index = i;

                              found = true;
                              break;
                           }
                        }

                        if (not found) {
                           _terrain_editor_config.custom_brush_index =
                              static_cast<int32>(
                                 _terrain_editor_config.custom_brushes.size());
                           _terrain_editor_config.custom_brushes
                              .emplace_back(std::move(name), std::move(falloff_map));
                        }

                        _terrain_editor_config.custom_brush_error.clear();
                     }
                     catch (world::brush_load_error& e) {
                        _terrain_editor_config.custom_brush_error = e.what();
                     }
                  }
               }

               ImGui::EndCombo();
            }

            if (not _terrain_editor_config.custom_brush_error.empty()) {
               ImGui::SeparatorText("Custom Brush Error");
               ImGui::TextWrapped(_terrain_editor_config.custom_brush_error.c_str());
            }
         }

         if (brush_falloff == terrain_brush_falloff::ramp or
             brush_falloff == terrain_brush_falloff::custom) {
            terrain_brush_rotation& brush_rotation = [&]() -> terrain_brush_rotation& {
               if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
                  return _terrain_editor_config.height.brush_rotation;
               }
               else if (_terrain_editor_config.edit_target ==
                        terrain_edit_target::texture) {
                  return _terrain_editor_config.texture.brush_rotation;
               }
               else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
                  return _terrain_editor_config.color.brush_rotation;
               }

               return _terrain_editor_config.height.brush_rotation;
            }();

            if (int i = std::to_underlying(brush_rotation);
                ImGui::SliderInt("Brush Rotation", &i, 0, 3,
                                 get_rotation_name(brush_rotation),
                                 ImGuiSliderFlags_AlwaysClamp)) {
               if (i == 0) brush_rotation = terrain_brush_rotation::r0;
               if (i == 1) brush_rotation = terrain_brush_rotation::r90;
               if (i == 2) brush_rotation = terrain_brush_rotation::r180;
               if (i == 3) brush_rotation = terrain_brush_rotation::r270;
            }
         }

         if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
            terrain_editor_config::height_config& config =
               _terrain_editor_config.height;

            if (config.brush_mode == terrain_brush_mode::pull_towards or
                config.brush_mode == terrain_brush_mode::overwrite) {
               if (float height = config.brush_height * _world.terrain.height_scale;
                   ImGui::DragFloat("Height", &height, _world.terrain.height_scale,
                                    -32768.0f * _world.terrain.height_scale,
                                    32767.0f * _world.terrain.height_scale, "%.3f",
                                    ImGuiSliderFlags_AlwaysClamp |
                                       ImGuiSliderFlags_NoRoundToFormat)) {
                  config.brush_height =
                     std::trunc(height / _world.terrain.height_scale);
               }
            }

            if (config.brush_mode == terrain_brush_mode::pull_towards or
                config.brush_mode == terrain_brush_mode::blend) {
               ImGui::SliderFloat("Speed", &config.brush_speed, 0.125f, 1.0f,
                                  "%.2f", ImGuiSliderFlags_NoRoundToFormat);
            }

            if (config.brush_mode == terrain_brush_mode::raise or
                config.brush_mode == terrain_brush_mode::lower) {
               ImGui::DragFloat("Rate", &config.brush_rate, 0.02f, 0.1f, 10.0f);
            }
         }
         else if (_terrain_editor_config.edit_target == terrain_edit_target::texture) {
            terrain_editor_config::texture_config& config =
               _terrain_editor_config.texture;

            if (config.brush_mode == terrain_texture_brush_mode::paint) {
               ImGui::SliderFloat("Texture Weight", &config.brush_texture_weight, 0.0f,
                                  255.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
            }

            if (config.brush_mode == terrain_texture_brush_mode::spray or
                config.brush_mode == terrain_texture_brush_mode::erase) {
               if (float rate = config.brush_rate / 4.0f;
                   ImGui::SliderFloat("Rate", &rate, 1.0f, 100.0f, "%.0f")) {
                  config.brush_rate = rate * 4.0f;
               }
            }

            if (config.brush_mode == terrain_texture_brush_mode::soften) {
               ImGui::SliderFloat("Speed", &config.brush_speed, 0.125f, 1.0f,
                                  "%.2f", ImGuiSliderFlags_NoRoundToFormat);
            }
         }
         else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
            terrain_editor_config::color_config& config = _terrain_editor_config.color;

            if (config.brush_mode == terrain_color_brush_mode::paint or
                config.brush_mode == terrain_color_brush_mode::spray) {
               ImGui::ColorEdit3("Colour", &config.brush_color.x);
            }

            if (config.brush_mode == terrain_color_brush_mode::spray) {
               ImGui::DragFloat("Rate", &config.brush_rate, 0.05f, 0.1f, 10.0f);
            }

            if (config.brush_mode == terrain_color_brush_mode::blur) {
               ImGui::SliderFloat("Speed", &config.brush_speed, 0.125f, 1.0f,
                                  "%.2f", ImGuiSliderFlags_NoRoundToFormat);
            }
         }
      }

      if (_terrain_editor_config.edit_target == terrain_edit_target::texture and
          ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
         terrain_editor_config::texture_config& config = _terrain_editor_config.texture;

         for (uint32 i = 0; i < world::terrain::texture_count; ++i) {
            const float size = 64.0f * _display_scale;

            ImGui::PushID(i);

            const ImVec2 cursor_position = ImGui::GetCursorPos();

            if (ImGui::Selectable("##select", config.edit_texture == i,
                                  ImGuiSelectableFlags_None, {size, size})) {
               config.edit_texture = i;
            }

            void* const texture_id =
               _renderer->request_imgui_texture_id(_world.terrain.texture_names[i],
                                                   graphics::fallback_imgui_texture::missing_diffuse);

            ImGui::SetCursorPos(cursor_position);
            ImGui::Image(texture_id, {size, size});

            if (ImGui::IsItemHovered()) {
               ImGui::SetTooltip("%u - %s", i,
                                 _world.terrain.texture_names[i].c_str());
            }

            ImGui::PopID();

            if ((i + 1) % 4 != 0) {
               ImGui::SameLine();
            }
         }

         ImGui::Separator();

         const uint32 texture = _terrain_editor_config.texture.edit_texture;

         auto texture_name_auto_complete = [&] {
            std::array<std::string_view, 6> entries;
            std::size_t matching_count = 0;

            _asset_libraries.textures.view_existing(
               [&](const std::span<const assets::stable_string> assets) noexcept {
                  for (const std::string_view asset : assets) {
                     if (matching_count == entries.size()) break;
                     if (not string::icontains(asset,
                                               _world.terrain.texture_names[texture])) {
                        continue;
                     }

                     entries[matching_count] = asset;

                     ++matching_count;
                  }
               });

            return entries;
         };

         if (absl::InlinedVector<char, 256> texture_name =
                {_world.terrain.texture_names[texture].begin(),
                 _world.terrain.texture_names[texture].end()};
             ImGui::InputTextAutoComplete(
                "Name", &texture_name,
                [](void* callback) {
                   return (*static_cast<decltype(texture_name_auto_complete)*>(
                      callback))();
                },
                &texture_name_auto_complete)) {
            _edit_stack_world.apply(make_set_terrain_value_indexed(
                                       &world::terrain::texture_names, texture,
                                       std::string{texture_name.begin(),
                                                   texture_name.end()},
                                       _world.terrain.texture_names[texture]),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         if (ImGui::BeginCombo("Axis Mapping", [&] {
                for (auto [axis, name] : texture_axis_names) {
                   if (axis == _world.terrain.texture_axes[texture])
                      return name;
                }

                return "";
             }())) {

            for (auto [axis, name] : texture_axis_names) {
               if (ImGui::Selectable(name, axis == _world.terrain.texture_axes[texture])) {
                  _edit_stack_world.apply(make_set_terrain_value_indexed(
                                             &world::terrain::texture_axes, texture, axis,
                                             _world.terrain.texture_axes[texture]),
                                          _edit_context, {.closed = true});
               }
            }

            ImGui::EndCombo();
         }

         if (float scale = 1.0f / _world.terrain.texture_scales[texture];
             ImGui::DragFloat("Scale", &scale, 1.0f)) {
            scale = std::max(scale, 1.0f);

            _edit_stack_world.apply(make_set_terrain_value_indexed(
                                       &world::terrain::texture_scales, texture,
                                       1.0f / scale,
                                       _world.terrain.texture_scales[texture]),
                                    _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         ImGui::SetItemTooltip("Also called 'Tilerange' in Zero Editor.");
      }

      if (ImGui::CollapsingHeader("Terrain Settings")) {
         if (bool active = _world.terrain.active_flags.terrain;
             ImGui::Checkbox("Terrain Enabled", &active)) {
            world::active_flags flags = _world.terrain.active_flags;

            flags.terrain = active;

            _edit_stack_world.apply(make_set_terrain_value(&world::terrain::active_flags,
                                                           flags, _world.terrain.active_flags),
                                    _edit_context, {.closed = true});
         }

         auto detail_texture_auto_complete = [&] {
            std::array<std::string_view, 6> entries;
            std::size_t matching_count = 0;

            _asset_libraries.textures.view_existing(
               [&](const std::span<const assets::stable_string> assets) noexcept {
                  for (const std::string_view asset : assets) {
                     if (matching_count == entries.size()) break;
                     if (not string::icontains(asset, _world.terrain.detail_texture_name)) {
                        continue;
                     }

                     entries[matching_count] = asset;

                     ++matching_count;
                  }
               });

            return entries;
         };

         if (absl::InlinedVector<char, 256> detail_texture =
                {_world.terrain.detail_texture_name.begin(),
                 _world.terrain.detail_texture_name.end()};
             ImGui::InputTextAutoComplete(
                "Detail Texture", &detail_texture,
                [](void* callback) {
                   return (*static_cast<decltype(detail_texture_auto_complete)*>(
                      callback))();
                },
                &detail_texture_auto_complete)) {
            _edit_stack_world
               .apply(make_set_terrain_value(&world::terrain::detail_texture_name,
                                             std::string{detail_texture.begin(),
                                                         detail_texture.end()},
                                             _world.terrain.detail_texture_name),
                      _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit()) {
            _edit_stack_world.close_last();
         }

         ImGui::SeparatorText("Advanced");

         ImGui::Text("Edit these if you're the adventurous sort or "
                     "just know what "
                     "you're doing.");

         if (float height_scale = _world.terrain.height_scale;
             ImGui::DragFloat("Height Scale", &height_scale, 0.0025f)) {
            _edit_stack_world.apply(
               make_set_terrain_value(&world::terrain::height_scale,
                                      std::max(static_cast<float>(height_scale), 0.0f),
                                      _world.terrain.height_scale),
               _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit())
            _edit_stack_world.close_last();

         if (float grid_scale = _world.terrain.grid_scale;
             ImGui::DragFloat("Grid Scale", &grid_scale, 1.0f, 1.0f, 16777216.0f)) {
            _edit_stack_world
               .apply(make_set_terrain_value(&world::terrain::grid_scale,
                                             std::max(static_cast<float>(grid_scale), 1.0f),
                                             _world.terrain.grid_scale),
                      _edit_context);
         }

         if (ImGui::IsItemDeactivatedAfterEdit())
            _edit_stack_world.close_last();
      }
   }

   ImGui::End();

   if (not _terrain_editor_open) return;

   if (_hotkeys_view_show) {
      ImGui::Begin("Hotkeys");

      ImGui::SeparatorText("Terrain Editing");

      ImGui::Text("Cycle Brush Mode");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Cycle Brush Mode")));

      ImGui::Text("Cycle Brush Falloff");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Cycle Brush Falloff")));

      ImGui::Text("Cycle Brush Rotation");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Cycle Brush Rotation")));

      ImGui::Text("Increase Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Increase Brush Size")));

      ImGui::Text("Decrease Brush Size");
      ImGui::BulletText(get_display_string(
         _hotkeys.query_binding("Terrain Editing", "Decrease Brush Size")));

      ImGui::End();
   }

   graphics::camera_ray ray =
      make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                      {ImGui::GetMainViewport()->Size.x,
                       ImGui::GetMainViewport()->Size.y});

   float hit_distance = -1.0f;

   if (_terrain_editor_context.brush_active and
       _terrain_editor_config.edit_target == terrain_edit_target::height and
       _terrain_editor_config.height.brush_mode == terrain_brush_mode::overwrite) {
      if (float hit = -(dot(ray.origin, float3{0.0f, 1.0f, 0.0f}) -
                        _terrain_editor_context.brush_plane_height) /
                      dot(ray.direction, float3{0.0f, 1.0f, 0.0f});
          hit > 0.0f) {
         hit_distance = hit;
      }
   }
   else {
      if (auto hit = world::raycast(ray.origin, ray.direction, _world.terrain); hit) {
         hit_distance = *hit;
      }
   }

   if (hit_distance < 0.0f) return;

   float3 cursor_positionWS = ray.origin + ray.direction * hit_distance;

   float2 terrain_point{cursor_positionWS.x, cursor_positionWS.z};

   terrain_point = round(terrain_point / _world.terrain.grid_scale) +
                   (_world.terrain.length / 2.0f) - float2{0.0f, 1.0f};

   const int32 terrain_x = static_cast<int32>(terrain_point.x);
   const int32 terrain_y = static_cast<int32>(terrain_point.y);

   if (_terrain_editor_context.brush_held != _terrain_editor_context.brush_active) {
      _terrain_editor_context.brush_active = _terrain_editor_context.brush_held;

      if (not _terrain_editor_context.brush_held) {
         _terrain_editor_context = {};
         _edit_stack_world.close_last();
      }
      else {
         _terrain_editor_context.brush_plane_height = cursor_positionWS.y;
         _terrain_editor_context.last_brush_update = std::chrono::steady_clock::now();

         for (uint64& v : _terrain_editor_maps.active_mask) v = 0;
      }
   }

   const terrain_brush_falloff brush_falloff = [&] {
      if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
         return _terrain_editor_config.height.brush_falloff;
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::texture) {
         return _terrain_editor_config.texture.brush_falloff;
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
         return _terrain_editor_config.color.brush_falloff;
      }

      return terrain_brush_falloff::none;
   }();

   const terrain_brush_rotation brush_rotation = [&] {
      if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
         return _terrain_editor_config.height.brush_rotation;
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::texture) {
         return _terrain_editor_config.texture.brush_rotation;
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
         return _terrain_editor_config.color.brush_rotation;
      }

      return terrain_brush_rotation::r0;
   }();

   const container::dynamic_array_2d<uint8>* const custom_brush_falloff_map =
      [&]() -> const container::dynamic_array_2d<uint8>* {
      if (brush_falloff != terrain_brush_falloff::custom) {
         return nullptr;
      }

      if (_terrain_editor_config.custom_brush_index >=
          _terrain_editor_config.custom_brushes.size()) {
         return nullptr;
      }

      return &_terrain_editor_config
                 .custom_brushes[_terrain_editor_config.custom_brush_index]
                 .falloff_map;
   }();

   int32 brush_size_x =
      not custom_brush_falloff_map
         ? _terrain_editor_config.brush_size_x
         : static_cast<int32>(custom_brush_falloff_map->sshape()[0] - 1) / 2;
   int32 brush_size_y =
      not custom_brush_falloff_map
         ? _terrain_editor_config.brush_size_y
         : static_cast<int32>(custom_brush_falloff_map->sshape()[1] - 1) / 2;

   if (brush_falloff == terrain_brush_falloff::custom and
       (brush_rotation == terrain_brush_rotation::r90 or
        brush_rotation == terrain_brush_rotation::r270)) {
      std::swap(brush_size_x, brush_size_y);
   }

   const float brush_radius =
      static_cast<float>(std::max(brush_size_x, brush_size_y));

   const brush brush{terrain_point,
                     brush_radius,
                     brush_falloff,
                     brush_rotation,
                     terrain_x - brush_size_x,
                     terrain_y - brush_size_y,
                     custom_brush_falloff_map};

   if (_terrain_editor_context.brush_active) {
      if (_terrain_editor_maps.length < _world.terrain.length) {
         _terrain_editor_maps = {.length = _world.terrain.length};
      }

      const float delta_time =
         std::chrono::duration<float>(
            std::chrono::steady_clock::now() -
            std::exchange(_terrain_editor_context.last_brush_update,
                          std::chrono::steady_clock::now()))
            .count();

      int32 left = std::clamp(terrain_x - brush_size_x, 0, _world.terrain.length - 1);
      int32 top = std::clamp(terrain_y - brush_size_y, 0, _world.terrain.length - 1);
      int32 right = std::clamp(terrain_x + brush_size_x + 1, 0, _world.terrain.length);
      int32 bottom =
         std::clamp(terrain_y + brush_size_y + 1, 0, _world.terrain.length);

      if (left >= right or top >= bottom) return;

      const int32 active_mask_factor = terrain_editor_maps::active_mask_factor;
      const int32 active_mask_left = left / active_mask_factor;
      const int32 active_mask_right =
         (right + (active_mask_factor - 1)) / active_mask_factor;
      const int32 active_mask_top = top / active_mask_factor;
      const int32 active_mask_bottom =
         (bottom + (active_mask_factor - 1)) / active_mask_factor;

      if (_terrain_editor_config.edit_target == terrain_edit_target::height) {
         const terrain_editor_config::height_config& config =
            _terrain_editor_config.height;

         for (int32 mask_y = active_mask_top; mask_y < active_mask_bottom; ++mask_y) {
            for (int32 mask_x = active_mask_left; mask_x < active_mask_right; ++mask_x) {
               uint64& mask =
                  _terrain_editor_maps
                     .active_mask[{mask_x / _terrain_editor_maps.mask_word_bits, mask_y}];

               const uint64 mask_bit =
                  1ull << (mask_x % _terrain_editor_maps.mask_word_bits);

               if (mask & mask_bit) continue;

               const int32 patch_left = mask_x * active_mask_factor;
               const int32 patch_right = patch_left + active_mask_factor;
               const int32 patch_top = mask_y * active_mask_factor;
               const int32 patch_bottom = patch_top + active_mask_factor;

               for (int32 y = patch_top; y < patch_bottom; ++y) {
                  for (int32 x = patch_left; x < patch_right; ++x) {
                     _terrain_editor_maps.height[{x, y}] =
                        _world.terrain.height_map[{x, y}];
                  }
               }

               mask |= mask_bit;
            }
         }

         container::dynamic_array_2d<int16> area{right - left, bottom - top};

         if (config.brush_mode == terrain_brush_mode::raise) {
            const float height_increase =
               (config.brush_rate / _world.terrain.height_scale) * delta_time;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);
                  float& v = _terrain_editor_maps.height[{x, y}];

                  v += (height_increase * weight);

                  area[{x - left, y - top}] =
                     static_cast<int16>(std::min(v, 32767.0f));
               }
            }
         }
         else if (config.brush_mode == terrain_brush_mode::lower) {
            const float height_decrease =
               (config.brush_rate / _world.terrain.height_scale) * delta_time;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);
                  float& v = _terrain_editor_maps.height[{x, y}];

                  v -= (height_decrease * weight);

                  area[{x - left, y - top}] =
                     static_cast<int16>(std::max(v, -32768.0f));
               }
            }
         }
         else if (config.brush_mode == terrain_brush_mode::overwrite) {
            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);

                  if (weight <= 0.0f) continue;

                  area[{x - left, y - top}] =
                     static_cast<int16>(config.brush_height * weight);
               }
            }
         }
         else if (config.brush_mode == terrain_brush_mode::pull_towards) {
            const float time_weight =
               std::clamp(delta_time * config.brush_speed, 0.0f, 1.0f);
            const float target_height = config.brush_height;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);
                  float& v = _terrain_editor_maps.height[{x, y}];

                  v = std::lerp(v, std::lerp(v, target_height, weight), time_weight);

                  area[{x - left, y - top}] = static_cast<int16>(v);
               }
            }
         }
         else if (config.brush_mode == terrain_brush_mode::blend) {
            double total_height = 0.0;
            double total_weight = 0.0;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);

                  total_height += _terrain_editor_maps.height[{x, y}] * weight;
                  total_weight += weight;
               }
            }

            const float time_weight =
               std::clamp(delta_time * config.brush_speed, 0.0f, 1.0f);
            const float target_height =
               static_cast<float>(total_height / total_weight);

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);
                  float& v = _terrain_editor_maps.height[{x, y}];

                  v = std::lerp(v, std::lerp(v, target_height, weight), time_weight);

                  area[{x - left, y - top}] = static_cast<int16>(v);
               }
            }
         }

         _edit_stack_world.apply(edits::make_set_terrain_area(left, top,
                                                              std::move(area)),
                                 _edit_context);
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::texture) {
         const terrain_editor_config::texture_config& config =
            _terrain_editor_config.texture;
         const uint32 texture = config.edit_texture;

         for (int32 mask_y = active_mask_top; mask_y < active_mask_bottom; ++mask_y) {
            for (int32 mask_x = active_mask_left; mask_x < active_mask_right; ++mask_x) {
               uint64& mask =
                  _terrain_editor_maps
                     .active_mask[{mask_x / _terrain_editor_maps.mask_word_bits, mask_y}];

               const uint64 mask_bit =
                  1ull << (mask_x % _terrain_editor_maps.mask_word_bits);

               if (mask & mask_bit) continue;

               const int32 patch_left = mask_x * active_mask_factor;
               const int32 patch_right = patch_left + active_mask_factor;
               const int32 patch_top = mask_y * active_mask_factor;
               const int32 patch_bottom = patch_top + active_mask_factor;

               for (int32 y = patch_top; y < patch_bottom; ++y) {
                  for (int32 x = patch_left; x < patch_right; ++x) {
                     _terrain_editor_maps.height[{x, y}] =
                        _world.terrain.texture_weight_maps[texture][{x, y}];
                  }
               }

               mask |= mask_bit;
            }
         }

         container::dynamic_array_2d<uint8> area{right - left, bottom - top};

         if (config.brush_mode == terrain_texture_brush_mode::paint) {
            const uint8 max_value = static_cast<uint8>(config.brush_texture_weight);

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);

                  area[{x - left, y - top}] = std::clamp(
                     std::max(static_cast<uint8>(config.brush_texture_weight * weight + 0.5f),
                              _world.terrain.texture_weight_maps[texture][{x, y}]),
                     uint8{0}, max_value);
               }
            }
         }
         else if (config.brush_mode == terrain_texture_brush_mode::spray) {
            const float weight_increase = config.brush_rate * delta_time;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);
                  float& v = _terrain_editor_maps.height[{x, y}];

                  v = std::min(v + (weight_increase * weight), 255.0f);

                  area[{x - left, y - top}] = static_cast<uint8>(v);
               }
            }
         }
         else if (config.brush_mode == terrain_texture_brush_mode::erase) {
            const float height_decrease = config.brush_rate * delta_time;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {

                  const float weight = brush.weight(x, y);
                  float& v = _terrain_editor_maps.height[{x, y}];

                  v = std::max(v - (height_decrease * weight), 0.0f);

                  area[{x - left, y - top}] = static_cast<uint8>(v);
               }
            }
         }
         else if (config.brush_mode == terrain_texture_brush_mode::soften) {
            double total_texture_weight = 0.0;
            double total_weight = 0.0;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);

                  total_texture_weight += _terrain_editor_maps.height[{x, y}] * weight;
                  total_weight += weight;
               }
            }

            const float time_weight =
               std::clamp(delta_time * config.brush_speed, 0.0f, 1.0f);
            const float target_texture_weight =
               static_cast<float>(total_texture_weight / total_weight);

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);
                  float& v = _terrain_editor_maps.height[{x, y}];

                  v = std::lerp(v, std::lerp(v, target_texture_weight, weight),
                                time_weight);

                  area[{x - left, y - top}] = static_cast<uint8>(v);
               }
            }
         }

         _edit_stack_world.apply(edits::make_set_terrain_area(left, top, texture,
                                                              std::move(area)),
                                 _edit_context);
      }
      else if (_terrain_editor_config.edit_target == terrain_edit_target::color) {
         const terrain_editor_config::color_config& config =
            _terrain_editor_config.color;

         for (int32 mask_y = active_mask_top; mask_y < active_mask_bottom; ++mask_y) {
            for (int32 mask_x = active_mask_left; mask_x < active_mask_right; ++mask_x) {
               uint64& mask =
                  _terrain_editor_maps
                     .active_mask[{mask_x / _terrain_editor_maps.mask_word_bits, mask_y}];

               const uint64 mask_bit =
                  1ull << (mask_x % _terrain_editor_maps.mask_word_bits);

               if (mask & mask_bit) continue;

               const int32 patch_left = mask_x * active_mask_factor;
               const int32 patch_right = patch_left + active_mask_factor;
               const int32 patch_top = mask_y * active_mask_factor;
               const int32 patch_bottom = patch_top + active_mask_factor;

               for (int32 y = patch_top; y < patch_bottom; ++y) {
                  for (int32 x = patch_left; x < patch_right; ++x) {
                     const float4 color =
                        utility::unpack_srgb_bgra(_world.terrain.color_map[{x, y}]);

                     _terrain_editor_maps.color[{x, y}] =
                        float3{color.x, color.y, color.z};
                  }
               }

               mask |= mask_bit;
            }
         }

         container::dynamic_array_2d<uint32> area{right - left, bottom - top};

         if (config.brush_mode == terrain_color_brush_mode::paint) {
            const float3 brush_color = config.brush_color;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);
                  float3& color = _terrain_editor_maps.color[{x, y}];

                  color = weight * config.brush_color + (1.0f - weight) * color;

                  area[{x - left, y - top}] =
                     utility::pack_srgb_bgra({color.x, color.y, color.z, 1.0f});
               }
            }
         }
         else if (config.brush_mode == terrain_color_brush_mode::spray) {
            const float3 brush_color = config.brush_color;
            const float time_weight = config.brush_rate * delta_time;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y) * time_weight;
                  float3& color = _terrain_editor_maps.color[{x, y}];

                  color = weight * brush_color + (1.0f - weight) * color;

                  area[{x - left, y - top}] =
                     utility::pack_srgb_bgra({color.x, color.y, color.z, 1.0f});
               }
            }
         }
         else if (config.brush_mode == terrain_color_brush_mode::blur) {
            float3 total_color;
            float total_weight = 0.0f;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);

                  total_color += _terrain_editor_maps.color[{x, y}] * weight;
                  total_weight += weight;
               }
            }

            const float time_weight =
               std::clamp(delta_time * config.brush_speed, 0.0f, 1.0f);
            const float3 average_color = total_color / total_weight;

            for (int32 y = top; y < bottom; ++y) {
               for (int32 x = left; x < right; ++x) {
                  const float weight = brush.weight(x, y);
                  float3& color = _terrain_editor_maps.color[{x, y}];

                  const float3 target_color =
                     time_weight * average_color + (1.0f - time_weight) * color;

                  color = weight * target_color + (1.0f - weight) * color;

                  area[{x - left, y - top}] =
                     utility::pack_srgb_bgra({color.x, color.y, color.z, 1.0f});
               }
            }
         }

         _edit_stack_world.apply(edits::make_set_terrain_area_color_map(left, top,
                                                                        std::move(area)),
                                 _edit_context);
      }
   }

   if (const int32 max_size = std::max(brush_size_x, brush_size_y); max_size == 0) {
      const float3 point = get_position(terrain_x, terrain_y, _world.terrain);
      const uint32 color = brush.visualizer_color(terrain_x, terrain_y);

      _tool_visualizers.add_line_overlay(point, color,
                                         point + float3{0.0f, _world.terrain.grid_scale,
                                                        0.0f},
                                         color);
   }
   else if (max_size <= 8) {
      for (int32 y = -brush_size_y; y < brush_size_y; ++y) {
         for (int32 x = -brush_size_x; x < brush_size_x; ++x) {
            const std::array vertices{
               get_position(terrain_x + x + 0, terrain_y + y + 0, _world.terrain),
               get_position(terrain_x + x + 1, terrain_y + y + 0, _world.terrain),
               get_position(terrain_x + x + 1, terrain_y + y + 1, _world.terrain),
               get_position(terrain_x + x + 0, terrain_y + y + 1, _world.terrain)};

            const std::array colors{
               brush.visualizer_color(terrain_x + x + 0, terrain_y + y + 0),
               brush.visualizer_color(terrain_x + x + 1, terrain_y + y + 0),
               brush.visualizer_color(terrain_x + x + 1, terrain_y + y + 1),
               brush.visualizer_color(terrain_x + x + 0, terrain_y + y + 1)};

            _tool_visualizers.add_line_overlay(vertices[0], colors[0],
                                               vertices[1], colors[1]);
            _tool_visualizers.add_line_overlay(vertices[3], colors[3],
                                               vertices[0], colors[0]);
         }
      }

      for (int32 y = -brush_size_y; y < brush_size_y; ++y) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_x + brush_size_x, terrain_y + y, _world.terrain),
            brush.visualizer_color(terrain_x + brush_size_x, terrain_y + y),
            get_position(terrain_x + brush_size_x, terrain_y + y + 1, _world.terrain),
            brush.visualizer_color(terrain_x + brush_size_x, terrain_y + y + 1));
      }

      for (int32 x = -brush_size_x; x < brush_size_x; ++x) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_x + x, terrain_y + brush_size_y, _world.terrain),
            brush.visualizer_color(terrain_x + x, terrain_y + brush_size_y),
            get_position(terrain_x + x + 1, terrain_y + brush_size_y, _world.terrain),
            brush.visualizer_color(terrain_x + x + 1, terrain_y + brush_size_y));
      }
   }
   else {
      for (int32 y = -brush_size_y; y < brush_size_y; ++y) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_x, terrain_y + y, _world.terrain),
            brush.visualizer_color(terrain_x, terrain_y + y),
            get_position(terrain_x, terrain_y + y + 1, _world.terrain),
            brush.visualizer_color(terrain_x, terrain_y + y + 1));
      }

      for (int32 x = -brush_size_x; x < brush_size_x; ++x) {
         _tool_visualizers.add_line_overlay(
            get_position(terrain_x + x, terrain_y, _world.terrain),
            brush.visualizer_color(terrain_x + x, terrain_y),
            get_position(terrain_x + x + 1, terrain_y, _world.terrain),
            brush.visualizer_color(terrain_x + x + 1, terrain_y));
      }
   }
}
}