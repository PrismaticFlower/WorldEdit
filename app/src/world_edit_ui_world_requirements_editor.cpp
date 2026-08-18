#include "world_edit.hpp"

#include "imgui_ext.hpp"

#include "assets/req/builder.hpp"

#include "edits/add_world_req_entry.hpp"
#include "edits/add_world_req_list.hpp"
#include "edits/delete_world_req_entry.hpp"
#include "edits/delete_world_req_list.hpp"
#include "edits/set_value.hpp"
#include "edits/set_world_req_entry.hpp"

#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include "world/utility/region_properties.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

enum class classification {
   generic,
   world_ref,
   layer_ref,
   gamemode_ref,
};

auto classify(const world::requirement_list& list) noexcept -> classification
{
   if (string::iequals(list.file_type, "path")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "congraph")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "envfx")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "world")) {
      return classification::layer_ref;
   }
   if (string::iequals(list.file_type, "prop")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "povs")) {
      return classification::world_ref;
   }
   if (string::iequals(list.file_type, "lvl")) {
      return classification::gamemode_ref;
   }

   return classification::generic;
}

bool is_editable(const classification list_classification,
                 std::string_view entry, const world::world& world) noexcept
{
   if (list_classification == classification::world_ref) {
      return not string::iequals(entry, world.name);
   }
   else if (list_classification == classification::layer_ref) {
      auto [left, right] = string::split_first_of_exclusive(entry, "_");

      if (string::iequals(entry, world.name)) return false;
      if (not string::iequals(left, world.name)) return true;

      for (const world::layer_description& layer : world.layer_descriptions) {
         if (string::iequals(right, layer.name)) return false;
      }
   }
   else if (list_classification == classification::gamemode_ref) {
      auto [left, right] = string::split_first_of_exclusive(entry, "_");

      if (string::iequals(entry, world.name)) return false;
      if (not string::iequals(left, world.name)) return true;

      for (const world::game_mode_description& game_mode : world.game_modes) {
         if (string::iequals(right, game_mode.name)) return false;
      }
   }

   return true;
}

auto classification_string(const classification list_classification) -> const char*
{
   switch (list_classification) {
   case classification::world_ref:
      return "[World Dependency]";
   case classification::layer_ref:
      return "[Layer Dependency]";
   case classification::gamemode_ref:
      return "[Game Mode Dependency]";
   }

   return "";
}

}

void world_edit::ui_show_world_requirements_editor() noexcept
{
   ImGui::SetNextWindowPos({tool_window_start_x * _display_scale, 32.0f * _display_scale},
                           ImGuiCond_Once, {0.0f, 0.0f});
   ImGui::SetNextWindowSize({400.0f * _display_scale, 256.0f * _display_scale},
                            ImGuiCond_FirstUseEver);
   ImGui::SetNextWindowSizeConstraints({400.0f * _display_scale, 0.0f},
                                       {std::numeric_limits<float>::max(),
                                        std::numeric_limits<float>::max()});

   if (ImGui::Begin("World Requirements (.req) Editor",
                    &_world_requirements_editor_open)) {
      ImGui::SeparatorText(".req Sections");

      for (int list_index = 0; list_index < _world.requirements.size(); ++list_index) {
         ImGui::PushID(list_index);

         const world::requirement_list& list = _world.requirements[list_index];

         if (ImGui::TreeNode(list.file_type.c_str())) {

            if (list.platform != world::platform::all) {
               switch (list.platform) {
               case world::platform::pc: {
                  ImGui::LabelText("Platform", "PC");
               } break;
               case world::platform::xbox: {
                  ImGui::LabelText("Platform", "Xbox");
               } break;
               case world::platform::ps2: {
                  ImGui::LabelText("Platform", "PS2");
               } break;
               }
            }

            if (list.alignment != 0) {
               ImGui::Value("Alignment", list.alignment);
            }

            ImGui::SeparatorText("Required Files");

            const classification list_classification = classify(list);

            if (ImGui::BeginTable("Required Files", 2)) {

               for (int entry_index = 0; entry_index < list.entries.size();
                    ++entry_index) {
                  ImGui::PushID(entry_index);

                  ImGui::TableNextRow();

                  if (is_editable(list_classification,
                                  list.entries[entry_index], _world)) {
                     ImGui::TableNextColumn();

                     if (absl::InlinedVector<char, 256>
                            entry{list.entries[entry_index].begin(),
                                  list.entries[entry_index].end()};
                         ImGui::InputText("##file", &entry)) {
                        if (not entry.empty()) {
                           _edit_stack_world.apply(edits::make_set_world_req_entry(
                                                      list_index, entry_index,
                                                      {entry.data(), entry.size()}),
                                                   _edit_context);
                        }
                     }

                     if (ImGui::IsItemDeactivatedAfterEdit()) {
                        _edit_stack_world.close_last();
                     }

                     ImGui::TableNextColumn();

                     if (ImGui::Button("Delete")) {
                        _edit_stack_world.apply(edits::make_delete_world_req_entry(
                                                   list_index, entry_index, _world),
                                                _edit_context);
                     }
                  }
                  else {
                     ImGui::TableNextColumn();

                     ImGui::TextUnformatted(list.entries[entry_index].c_str());

                     ImGui::TableNextColumn();

                     ImGui::TextUnformatted(classification_string(list_classification));
                  }

                  ImGui::PopID();
               }

               ImGui::EndTable();
            }

            ImGui::SeparatorText("Add File");

            ImGui::InputTextWithHint("##add", "i.e test_map", &_req_editor_add_entry);
            ImGui::SameLine();

            const bool can_add_file = not _req_editor_add_entry.empty();

            if (not can_add_file) ImGui::BeginDisabled();

            if (ImGui::Button("Add")) {
               _edit_stack_world.apply(edits::make_add_world_req_entry(list_index,
                                                                       std::move(_req_editor_add_entry)),
                                       _edit_context);

               _req_editor_add_entry = "";
            }

            if (not can_add_file) ImGui::EndDisabled();

            ImGui::SeparatorText("Delete Section");

            if (ImGui::Button("Delete")) {
               _edit_stack_world.apply(edits::make_delete_world_req_list(list_index, _world),
                                       _edit_context);
            }

            ImGui::TreePop();
         }

         ImGui::PopID();
      }

      ImGui::SeparatorText("Add New File Type");

      ImGui::InputTextWithHint("##create", "i.e model", &_req_editor_new_name);
      ImGui::SameLine();

      const bool can_add_type = not _req_editor_new_name.empty();

      if (not can_add_type) ImGui::BeginDisabled();

      if (ImGui::Button("Add")) {
         _edit_stack_world.apply(edits::make_add_world_req_list(
                                    std::move(_req_editor_new_name)),
                                 _edit_context);

         _req_editor_new_name = "";
      }

      if (not can_add_type) ImGui::EndDisabled();

      ImGui::Separator();

      if (ImGui::Button("Repair World Requirements", {ImGui::CalcItemWidth(), 0.0f})) {
         bool has_water = false;
         bool has_shadow = false;
         bool has_rumble = false;

         if (_world.terrain.active_flags.water) {
            for (bool water : _world.terrain.water_map) has_water |= water;
         }

         for (const world::region& region : _world.regions) {
            switch (world::get_region_type(region.description)) {
            case world::region_type::shadow: {
               has_shadow = true;
            } break;
            case world::region_type::rumble: {
               has_rumble = true;
            } break;
            }
         }

         std::vector<world::requirement_list> requirements;
         requirements.reserve(14);

         if (has_rumble) {
            requirements.push_back({
               .file_type = "class",
            });

            for (const world::region& region : _world.regions) {
               switch (world::get_region_type(region.description)) {
               case world::region_type::rumble: {
                  world::rumble_region_properties properties =
                     world::unpack_region_rumble(region.description);

                  if (not properties.rumble_class.empty()) {
                     assets::req::add_to(requirements.back().entries,
                                         properties.rumble_class);
                  }
               } break;
               }
            }
         }

         requirements.push_back({
            .file_type = "texture",
            .entries = {fmt::format("{}_map", _world.name)},
         });

         if (has_shadow) {
            for (const world::region& region : _world.regions) {
               switch (world::get_region_type(region.description)) {
               case world::region_type::shadow: {
                  world::shadow_region_properties properties =
                     world::unpack_region_shadow(region.description);

                  if (not properties.env_map.empty()) {
                     assets::req::add_to(requirements.back().entries, properties.env_map);
                  }
               } break;
               }
            }
         }

         if (has_water) {
            requirements.push_back({
               .file_type = "texture",
               .platform = world::platform::ps2,
            });

            for (int32 i = 0;
                 i < _world.effects.water.speckle_textures_ps2.count; ++i) {
               requirements.back().entries.push_back(
                  fmt::format("{}{}",
                              _world.effects.water.speckle_textures_ps2.prefix, i));
            }

            requirements.push_back({
               .file_type = "texture",
               .platform = world::platform::xbox,
            });

            const world::water::animated_textures& xbox_normal_maps =
               _world.effects.water.normal_map_textures.per_platform
                  ? _world.effects.water.normal_map_textures.xbox
                  : _world.effects.water.normal_map_textures.pc;

            for (int32 i = 0; i < xbox_normal_maps.count; ++i) {
               requirements.back().entries.push_back(
                  fmt::format("{}{}", xbox_normal_maps.prefix, i));
            }

            requirements.push_back({
               .file_type = "texture",
               .platform = world::platform::pc,
            });

            for (int32 i = 0;
                 i < _world.effects.water.bump_map_textures_pc.count; ++i) {
               requirements.back().entries.push_back(
                  fmt::format("{}{}",
                              _world.effects.water.bump_map_textures_pc.prefix, i));
            }

            for (int32 i = 0;
                 i < _world.effects.water.normal_map_textures.pc.count; ++i) {
               requirements.back().entries.push_back(
                  fmt::format("{}{}",
                              _world.effects.water.normal_map_textures.pc.prefix, i));
            }

            for (int32 i = 0;
                 i < _world.effects.water.specular_mask_textures_pc.count; ++i) {
               requirements.back().entries.push_back(
                  fmt::format("{}{}",
                              _world.effects.water.specular_mask_textures_pc.prefix, i));
            }
         }

         if (_world.effects.heat_shimmer.enable.per_platform
                ? _world.effects.heat_shimmer.enable.xbox
                : _world.effects.heat_shimmer.enable.pc) {
            requirements.push_back({
               .file_type = "texture",
               .platform = world::platform::xbox,
               .entries = {_world.effects.heat_shimmer.bump_map.per_platform
                              ? _world.effects.heat_shimmer.bump_map.xbox.name
                              : _world.effects.heat_shimmer.bump_map.pc.name},
            });
         }

         requirements.push_back({
            .file_type = "path",
            .entries = {_world.name},
         });

         requirements.push_back({
            .file_type = "congraph",
            .entries = {_world.name},
         });

         requirements.push_back({
            .file_type = "envfx",
            .entries = {_world.name},
         });

         requirements.push_back({
            .file_type = "world",
            .entries = {_world.name},
         });

         for (const int layer_index : _world.common_layers) {
            requirements.back().entries.push_back(
               fmt::format("{}_{}", _world.name,
                           _world.layer_descriptions[layer_index].name));
         }

         requirements.push_back({
            .file_type = "prop",
            .entries = {_world.name},
         });

         requirements.push_back({
            .file_type = "boundary",
            .entries = {_world.name},
         });

         requirements.push_back({
            .file_type = "class",
            .entries = {"bluelight", "redlight", "greenlight", "whitelight"},
         });

         requirements.push_back({
            .file_type = "config",
            .entries = {"flyerspray", "bigwalkerstomp", "walkerstomp",
                        "hailfire_wake", "dustwake"},
         });

         requirements.push_back({
            .file_type = "lvl",
         });

         for (const world::game_mode_description& game_mode : _world.game_modes) {
            requirements.back().entries.push_back(
               fmt::format("{}_{}", _world.name, game_mode.name));
         }

         requirements.push_back({
            .file_type = "povs",
            .entries = {_world.name},
         });

         _edit_stack_world.apply(edits::make_set_value(&_world.requirements,
                                                       std::move(requirements)),
                                 _edit_context, {.closed = true});
      }
   }

   ImGui::End();
}

}