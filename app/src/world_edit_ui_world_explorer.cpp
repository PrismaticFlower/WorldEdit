#include "world_edit.hpp"

#include "edits/creation_entity_set.hpp"
#include "edits/set_value.hpp"
#include "utility/string_icompare.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

namespace {

void layer_filter(world::active_layers& mask, const world::world& world)
{
   if (ImGui::BeginCombo("Layer Filter", "Layer Filter")) {
      for (int i = 0; i < world.layer_descriptions.size(); ++i) {
         if (ImGui::Selectable(world.layer_descriptions[i].name.c_str(), mask[i],
                               ImGuiSelectableFlags_NoAutoClosePopups)) {
            mask[i] = not mask[i];
         }
      }

      ImGui::EndCombo();
   }
}

void fill_sort_map(std::vector<uint32>& world_explorer_sort_map,
                   ImGuiSortDirection direction,
                   const std::invocable<uint32, uint32> auto& cmp) noexcept
{
   if (direction == ImGuiSortDirection_Ascending) {
      if (std::is_sorted(world_explorer_sort_map.begin(),
                         world_explorer_sort_map.end(),
                         [&](const uint32 left, const uint32 right) {
                            return cmp(left, right);
                         })) {
         return;
      }

      std::sort(world_explorer_sort_map.begin(), world_explorer_sort_map.end(),
                [&](const uint32 left, const uint32 right) {
                   return cmp(left, right);
                });
   }
   else {
      if (std::is_sorted(world_explorer_sort_map.begin(),
                         world_explorer_sort_map.end(),
                         [&](const uint32 left, const uint32 right) {
                            return cmp(right, left);
                         })) {
         return;
      }

      std::sort(world_explorer_sort_map.begin(), world_explorer_sort_map.end(),
                [&](const uint32 left, const uint32 right) {
                   return cmp(right, left);
                });
   }
}

}

void world_edit::ui_show_world_explorer() noexcept
{
   ImGui::SetNextWindowPos({0.0f, ImGui::GetIO().DisplaySize.y}, ImGuiCond_Once,
                           {0.0f, 1.0f});
   ImGui::SetNextWindowSize({1216.0f * _display_scale, 272.0f * _display_scale},
                            ImGuiCond_Once);

   const bool is_shift_down = ImGui::GetIO().KeyShift;
   const bool is_ctrl_down = ImGui::GetIO().KeyCtrl;

   ImGui::Begin("World Explorer", &_world_explorer_open);

   if (ImGui::BeginTabBar("Explorer", ImGuiTabBarFlags_Reorderable)) {
      if (ImGui::BeginTabItem("Objects")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. cp", &_world_explorer_filter);
         ImGui::SameLine();
         ImGui::InputTextWithHint("Class Name Filter", "e.g. com_bldg_controlzone",
                                  &_world_explorer_class_filter);
         ImGui::SameLine();
         layer_filter(_world_explorer_layers_mask, _world);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Objects", 4,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Class Name");
            ImGui::TableSetupColumn("Class Label");
            ImGui::TableSetupColumn("Layer");
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.objects.size()) {
                  _world_explorer_sort_map.resize(_world.objects.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               const int name_column = 0;
               const int class_name_column = 1;
               const int class_label_column = 2;
               const int layer_column = 3;

               if (sort_specs.ColumnIndex == name_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(_world.objects[left].name,
                                                  _world.objects[right].name);
                     });
               }
               else if (sort_specs.ColumnIndex == class_name_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return string::iless_than(
                                      _world.objects[left].class_name,
                                      _world.objects[right].class_name);
                                });
               }
               else if (sort_specs.ColumnIndex == class_label_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return string::iless_than(
                                      _object_classes[_world.objects[left].class_handle]
                                         .definition->header.class_label,
                                      _object_classes[_world.objects[right].class_handle]
                                         .definition->header.class_label);
                                });
               }
               else if (sort_specs.ColumnIndex == layer_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(
                           _world.layer_descriptions[_world.objects[left].layer].name,
                           _world
                              .layer_descriptions[_world.objects[right].layer]
                              .name);
                     });
               }
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.objects.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::object& object = _world.objects[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(object.name, _world_explorer_filter)) {
                  continue;
               }
               if (not _world_explorer_class_filter.empty() and
                   not string::icontains(object.class_name,
                                         _world_explorer_class_filter)) {
                  continue;
               }
               if (not _world_explorer_layers_mask[object.layer]) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(object.id));

               const bool is_selected =
                  world::is_selected(object.id, _interaction_targets.selection);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(object.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover = ImGui::IsItemHovered();
               ImGui::TableNextColumn();
               ImGui::Text(object.class_name.c_str());
               ImGui::TableNextColumn();
               const std::string_view class_label =
                  _object_classes[object.class_handle].definition->header.class_label;

               ImGui::TextUnformatted(class_label.data(),
                                      class_label.data() + class_label.size());
               ImGui::TableNextColumn();
               ImGui::Text(_world.layer_descriptions[object.layer].name.c_str());

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(object.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(object.id);
                  }
               }

               if (hover) {
                  _interaction_targets.hovered_entity = object.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Lights")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. sun", &_world_explorer_filter);
         ImGui::SameLine();
         layer_filter(_world_explorer_layers_mask, _world);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Lights", 4,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Color");
            ImGui::TableSetupColumn("Layer");
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.lights.size()) {
                  _world_explorer_sort_map.resize(_world.lights.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               const int name_column = 0;
               const int type_column = 1;
               const int color_column = 2;
               const int layer_column = 3;

               if (sort_specs.ColumnIndex == name_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(_world.lights[left].name,
                                                  _world.lights[right].name);
                     });
               }
               else if (sort_specs.ColumnIndex == type_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return _world.lights[left].light_type <
                                          _world.lights[right].light_type;
                                });
               }
               else if (sort_specs.ColumnIndex == color_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return std::bit_cast<std::array<float, 3>>(
                                             _world.lights[left].color) <
                                          std::bit_cast<std::array<float, 3>>(
                                             _world.lights[right].color);
                                });
               }
               else if (sort_specs.ColumnIndex == layer_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(
                           _world.layer_descriptions[_world.lights[left].layer].name,
                           _world.layer_descriptions[_world.lights[right].layer].name);
                     });
               }
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.lights.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::light& light = _world.lights[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(light.name, _world_explorer_filter)) {
                  continue;
               }
               if (not _world_explorer_layers_mask[light.layer]) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(light.id));

               const bool is_selected =
                  world::is_selected(light.id, _interaction_targets.selection);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(light.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover = ImGui::IsItemHovered();
               ImGui::TableNextColumn();
               ImGui::Text([&] {
                  // clang-format off
                     switch (light.light_type) {
                     case world::light_type::directional: return "Directional";
                     case world::light_type::point: return "Point";
                     case world::light_type::spot: return "Spot";
                     case world::light_type::directional_region_box: return "Directional Region Box";
                     case world::light_type::directional_region_sphere: return "Directional Region Sphere";
                     case world::light_type::directional_region_cylinder: return "Directional Region Cylinder";
                     default: return "";
                     }
                  // clang-format on
               }());
               ImGui::TableNextColumn();
               ImGui::ColorButton("##Color",
                                  {light.color.x, light.color.y, light.color.z, 1.0f},
                                  ImGuiColorEditFlags_NoTooltip |
                                     ImGuiColorEditFlags_NoSidePreview,
                                  {0.0f, 16.0f * _display_scale});
               ImGui::SameLine();
               ImGui::Text("R:%.3f G:%.3f B:%.3f", light.color.x, light.color.y,
                           light.color.z);
               ImGui::TableNextColumn();
               ImGui::Text(_world.layer_descriptions[light.layer].name.c_str());

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(light.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(light.id);
                  }
               }

               if (hover) {
                  _interaction_targets.hovered_entity = light.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Paths")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. spawn", &_world_explorer_filter);
         ImGui::SameLine();
         ImGui::Checkbox("Show All Nodes", &_world_explorer_path_show_all_nodes);
         ImGui::SameLine();
         layer_filter(_world_explorer_layers_mask, _world);
         ImGui::PopItemWidth();

         const bool show_all_nodes = _world_explorer_path_show_all_nodes;

         if (ImGui::BeginTable("Paths", show_all_nodes ? 5 : 4,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            // clang-format off
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type"); 
            if (show_all_nodes) ImGui::TableSetupColumn("Node", ImGuiTableColumnFlags_NoSort); 
            ImGui::TableSetupColumn("Nodes");
            ImGui::TableSetupColumn("Layer");
            // clang-format on
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.paths.size()) {
                  _world_explorer_sort_map.resize(_world.paths.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               const int name_column = 0;
               const int type_column = 1;
               const int nodes_column = 2;
               const int layer_column = 3;

               if (sort_specs.ColumnIndex == name_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return string::iless_than(_world.paths[left].name,
                                                             _world.paths[right].name);
                                });
               }
               else if (sort_specs.ColumnIndex == type_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return _world.paths[left].type <
                                          _world.paths[right].type;
                                });
               }
               else if (sort_specs.ColumnIndex == nodes_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return _world.paths[left].nodes.size() <
                                          _world.paths[right].nodes.size();
                                });
               }
               else if (sort_specs.ColumnIndex == layer_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(
                           _world.layer_descriptions[_world.paths[left].layer].name,
                           _world.layer_descriptions[_world.paths[right].layer].name);
                     });
               }
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.paths.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::path& path = _world.paths[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(path.name, _world_explorer_filter)) {
                  continue;
               }
               if (not _world_explorer_layers_mask[path.layer]) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(path.id));

               for (int i = 0; i < (show_all_nodes ? path.nodes.size() : 1); ++i) {
                  const bool is_selected =
                     show_all_nodes
                        ? world::is_selected(path.id, static_cast<uint32>(i),
                                             _interaction_targets.selection)
                        : world::is_selected(path.id, _interaction_targets.selection);

                  ImGui::PushID(i);

                  ImGui::TableNextRow();

                  ImGui::TableNextColumn();
                  const bool select =
                     ImGui::Selectable(path.name.c_str(), is_selected,
                                       ImGuiSelectableFlags_SpanAllColumns);
                  const bool hover = ImGui::IsItemHovered();
                  ImGui::TableNextColumn();
                  ImGui::Text([&] {
                     // clang-format off
                     switch (path.type) {
                     case world::path_type::none: return "None";
                     case world::path_type::entity_follow: return "Entity Follow";
                     case world::path_type::formation: return "Formation";
                     case world::path_type::patrol: return "Patrol";
                     default: return "";
                     }
                     // clang-format on
                  }());
                  if (show_all_nodes) {
                     ImGui::TableNextColumn();
                     ImGui::Text("%i", i);
                  }
                  ImGui::TableNextColumn();
                  ImGui::Text("%i", static_cast<int>(path.nodes.size()));
                  ImGui::TableNextColumn();
                  ImGui::Text(_world.layer_descriptions[path.layer].name.c_str());

                  if (show_all_nodes) {
                     if (select and path.nodes.size() != 0) {
                        if (is_ctrl_down) {
                           _interaction_targets.selection.remove(
                              world::make_path_id_node_mask(path.id,
                                                            static_cast<uint32>(i)));
                        }
                        else {
                           if (not is_shift_down) {
                              _interaction_targets.selection.clear();
                           }

                           _interaction_targets.selection.add(
                              world::make_path_id_node_mask(path.id,
                                                            static_cast<uint32>(i)));
                        }
                     }
                  }
                  else {
                     if (select) {
                        if (not is_shift_down) {
                           _interaction_targets.selection.clear();
                        }

                        for (uint32 node = 0; node < path.nodes.size(); ++node) {
                           if (is_ctrl_down) {
                              _interaction_targets.selection.remove(
                                 world::make_path_id_node_mask(path.id,
                                                               static_cast<uint32>(i)));
                           }
                           else {
                              _interaction_targets.selection.add(
                                 world::make_path_id_node_mask(path.id,
                                                               static_cast<uint32>(i)));
                           }
                        }
                     }
                  }

                  if (hover and path.nodes.size() != 0) {
                     _interaction_targets.hovered_entity =
                        world::make_path_id_node_mask(path.id, static_cast<uint32>(i));
                  }

                  ImGui::PopID();
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Regions")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. shadow", &_world_explorer_filter);
         ImGui::SameLine();
         layer_filter(_world_explorer_layers_mask, _world);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Regions", 4,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Description");
            ImGui::TableSetupColumn("Shape");
            ImGui::TableSetupColumn("Layer");
            ImGui::TableHeadersRow();
            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.regions.size()) {
                  _world_explorer_sort_map.resize(_world.regions.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               const int name_column = 0;
               const int description_column = 1;
               const int shape_column = 2;
               const int layer_column = 3;

               if (sort_specs.ColumnIndex == name_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(_world.regions[left].name,
                                                  _world.regions[right].name);
                     });
               }
               else if (sort_specs.ColumnIndex == description_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return string::iless_than(
                                      _world.regions[left].description,
                                      _world.regions[right].description);
                                });
               }
               else if (sort_specs.ColumnIndex == shape_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return _world.regions[left].shape <
                                          _world.regions[right].shape;
                                });
               }
               else if (sort_specs.ColumnIndex == layer_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(
                           _world.layer_descriptions[_world.regions[left].layer].name,
                           _world
                              .layer_descriptions[_world.regions[right].layer]
                              .name);
                     });
               }
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.regions.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::region& region = _world.regions[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(region.name, _world_explorer_filter)) {
                  continue;
               }
               if (not _world_explorer_layers_mask[region.layer]) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(region.id));

               const bool is_selected =
                  world::is_selected(region.id, _interaction_targets.selection);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(region.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover = ImGui::IsItemHovered();
               ImGui::TableNextColumn();
               ImGui::Text(region.description.c_str());
               ImGui::TableNextColumn();
               ImGui::Text([&] {
                  // clang-format off
                     switch (region.shape) {
                     case world::region_shape::box: return "Box";
                     case world::region_shape::sphere: return "Sphere";
                     case world::region_shape::cylinder: return "Cylinder";
                     default: return "";
                     }
                  // clang-format on
               }());
               ImGui::TableNextColumn();
               ImGui::Text(_world.layer_descriptions[region.layer].name.c_str());

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(region.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(region.id);
                  }
               }

               if (hover) {
                  _interaction_targets.hovered_entity = region.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Sectors")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. sector", &_world_explorer_filter);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Sectors", 3,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Points");
            ImGui::TableSetupColumn("Objects");
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.sectors.size()) {
                  _world_explorer_sort_map.resize(_world.sectors.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               const int name_column = 0;
               const int points_column = 1;
               const int objects_column = 2;

               if (sort_specs.ColumnIndex == name_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(_world.sectors[left].name,
                                                  _world.sectors[right].name);
                     });
               }
               else if (sort_specs.ColumnIndex == points_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return _world.sectors[left].points.size() <
                                          _world.sectors[right].points.size();
                                });
               }
               else if (sort_specs.ColumnIndex == objects_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return _world.sectors[left].objects.size() <
                                          _world.sectors[right].objects.size();
                                });
               }
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.sectors.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::sector& sector = _world.sectors[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(sector.name, _world_explorer_filter)) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(sector.id));

               const bool is_selected =
                  world::is_selected(sector.id, _interaction_targets.selection);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(sector.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover = ImGui::IsItemHovered();
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(sector.points.size()));
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(sector.objects.size()));

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(sector.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(sector.id);
                  }
               }

               if (hover) {
                  _interaction_targets.hovered_entity = sector.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Portals")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. door", &_world_explorer_filter);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Portals", 3,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Sector 1");
            ImGui::TableSetupColumn("Sector 2");
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.portals.size()) {
                  _world_explorer_sort_map.resize(_world.portals.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               const int name_column = 0;
               const int sector1_column = 1;
               const int sector2_column = 2;

               if (sort_specs.ColumnIndex == name_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(_world.portals[left].name,
                                                  _world.portals[right].name);
                     });
               }
               else if (sort_specs.ColumnIndex == sector1_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(_world.portals[left].sector1,
                                                  _world.portals[right].sector1);
                     });
               }
               else if (sort_specs.ColumnIndex == sector2_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(_world.portals[left].sector2,
                                                  _world.portals[right].sector2);
                     });
               }
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.portals.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::portal& portal = _world.portals[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(portal.name, _world_explorer_filter)) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(portal.id));

               const bool is_selected =
                  world::is_selected(portal.id, _interaction_targets.selection);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(portal.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover = ImGui::IsItemHovered();
               ImGui::TableNextColumn();
               ImGui::Text(portal.sector1.c_str());
               ImGui::TableNextColumn();
               ImGui::Text(portal.sector2.c_str());

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(portal.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(portal.id);
                  }
               }

               if (hover) {
                  _interaction_targets.hovered_entity = portal.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Hintnodes")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. snipe", &_world_explorer_filter);
         ImGui::SameLine();
         layer_filter(_world_explorer_layers_mask, _world);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Hintnodes", 3,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Layer");
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.hintnodes.size()) {
                  _world_explorer_sort_map.resize(_world.hintnodes.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               const int name_column = 0;
               const int type_column = 1;
               const int layer_column = 2;

               if (sort_specs.ColumnIndex == name_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(_world.hintnodes[left].name,
                                                  _world.hintnodes[right].name);
                     });
               }
               else if (sort_specs.ColumnIndex == type_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return _world.hintnodes[left].type <
                                          _world.hintnodes[right].type;
                                });
               }
               else if (sort_specs.ColumnIndex == layer_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection, [&](const uint32 left, const uint32 right) {
                     return string::iless_than(
                        _world.layer_descriptions[_world.hintnodes[left].layer].name,
                        _world.layer_descriptions[_world.hintnodes[right].layer].name);
                  });
               }
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.hintnodes.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::hintnode& hintnode = _world.hintnodes[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(hintnode.name, _world_explorer_filter)) {
                  continue;
               }
               if (not _world_explorer_layers_mask[hintnode.layer]) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(hintnode.id));

               const bool is_selected =
                  world::is_selected(hintnode.id, _interaction_targets.selection);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(hintnode.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover = ImGui::IsItemHovered();
               ImGui::TableNextColumn();
               ImGui::Text([&] {
                  // clang-format off
                     switch (hintnode.type) {
                        case world::hintnode_type::snipe: return "Snipe";
                        case world::hintnode_type::patrol: return "Patrol";
                        case world::hintnode_type::cover: return "Cover";
                        case world::hintnode_type::access: return "Access";
                        case world::hintnode_type::jet_jump: return "Jet Jump";
                        case world::hintnode_type::mine: return "Mine";
                        case world::hintnode_type::land: return "Land";
                        case world::hintnode_type::fortification: return "Fortification";
                        case world::hintnode_type::vehicle_cover: return "Vehicle Cover";
                        default: return "";
                     }
                  // clang-format on
               }());
               ImGui::TableNextColumn();
               ImGui::Text(_world.layer_descriptions[hintnode.layer].name.c_str());

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(hintnode.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(hintnode.id);
                  }
               }

               if (hover) {
                  _interaction_targets.hovered_entity = hintnode.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Barriers")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. Door", &_world_explorer_filter);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Barriers", 7,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Soldier");
            ImGui::TableSetupColumn("Hover");
            ImGui::TableSetupColumn("Small");
            ImGui::TableSetupColumn("Medium");
            ImGui::TableSetupColumn("Huge");
            ImGui::TableSetupColumn("Flyer");
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.barriers.size()) {
                  _world_explorer_sort_map.resize(_world.barriers.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               const int name_column = 0;
               const int soldier_column = 1;
               const int hover_column = 2;
               const int small_column = 3;
               const int medium_column = 4;
               const int huge_column = 5;
               const int flyer_column = 6;

               if (sort_specs.ColumnIndex == name_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(_world.barriers[left].name,
                                                  _world.barriers[right].name);
                     });
               }
               else if (sort_specs.ColumnIndex == soldier_column or
                        sort_specs.ColumnIndex == hover_column or
                        sort_specs.ColumnIndex == small_column or
                        sort_specs.ColumnIndex == medium_column or
                        sort_specs.ColumnIndex == huge_column or
                        sort_specs.ColumnIndex == flyer_column) {
                  using world::ai_path_flags;

                  ai_path_flags flag = {};

                  // clang-format off

                  if (sort_specs.ColumnIndex == soldier_column) flag = ai_path_flags::soldier;
                  else if (sort_specs.ColumnIndex == hover_column) flag = ai_path_flags::hover;
                  else if (sort_specs.ColumnIndex == small_column) flag = ai_path_flags::small;
                  else if (sort_specs.ColumnIndex == medium_column) flag = ai_path_flags::medium;
                  else if (sort_specs.ColumnIndex == huge_column) flag = ai_path_flags::huge;
                  else if (sort_specs.ColumnIndex == flyer_column) flag = ai_path_flags::flyer;

                  // clang-format on

                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&, flag](const uint32 left, const uint32 right) {
                                   return (_world.barriers[left].flags & flag) <
                                          (_world.barriers[right].flags & flag);
                                });
               }
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.barriers.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::barrier& barrier = _world.barriers[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(barrier.name, _world_explorer_filter)) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(barrier.id));

               const bool is_selected =
                  world::is_selected(barrier.id, _interaction_targets.selection);

               using world::ai_path_flags;

               bool soldier = are_flags_set(barrier.flags, ai_path_flags::soldier);
               bool hover = are_flags_set(barrier.flags, ai_path_flags::hover);
               bool small = are_flags_set(barrier.flags, ai_path_flags::small);
               bool medium = are_flags_set(barrier.flags, ai_path_flags::medium);
               bool huge = are_flags_set(barrier.flags, ai_path_flags::huge);
               bool flyer = are_flags_set(barrier.flags, ai_path_flags::flyer);

               ImGui::TableNextRow(ImGuiTableRowFlags_None);

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(barrier.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover_entity = ImGui::IsItemHovered();
               ImGui::TableNextColumn();
               ImGui::Text(soldier ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(hover ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(small ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(medium ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(huge ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(flyer ? "X" : "-");

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(barrier.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(barrier.id);
                  }
               }

               if (hover_entity) {
                  _interaction_targets.hovered_entity = barrier.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("AI Planning Hubs")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. Hub", &_world_explorer_filter);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("AI Planning Hubs", 1,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.planning_hubs.size()) {
                  _world_explorer_sort_map.resize(_world.planning_hubs.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                             [&](const uint32 left, const uint32 right) {
                                return string::iless_than(
                                   _world.planning_hubs[left].name,
                                   _world.planning_hubs[right].name);
                             });
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.planning_hubs.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::planning_hub& hub = _world.planning_hubs[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(hub.name, _world_explorer_filter)) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(hub.id));

               const bool is_selected =
                  world::is_selected(hub.id, _interaction_targets.selection);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(hub.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover = ImGui::IsItemHovered();

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(hub.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(hub.id);
                  }
               }

               if (hover) {
                  _interaction_targets.hovered_entity = hub.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("AI Planning Connections")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. Connection",
                                  &_world_explorer_filter);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("AI Planning Connections", 9,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Start");
            ImGui::TableSetupColumn("End");
            ImGui::TableSetupColumn("Soldier");
            ImGui::TableSetupColumn("Hover");
            ImGui::TableSetupColumn("Small");
            ImGui::TableSetupColumn("Medium");
            ImGui::TableSetupColumn("Huge");
            ImGui::TableSetupColumn("Flyer");
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() !=
                   _world.planning_connections.size()) {
                  _world_explorer_sort_map.resize(_world.planning_connections.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               const int name_column = 0;
               const int start_column = 1;
               const int end_column = 2;
               const int soldier_column = 3;
               const int hover_column = 4;
               const int small_column = 5;
               const int medium_column = 6;
               const int huge_column = 7;
               const int flyer_column = 8;

               if (sort_specs.ColumnIndex == name_column) {
                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&](const uint32 left, const uint32 right) {
                                   return string::iless_than(
                                      _world.planning_connections[left].name,
                                      _world.planning_connections[right].name);
                                });
               }
               else if (sort_specs.ColumnIndex == start_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(
                           _world
                              .planning_hubs[_world.planning_connections[left].start_hub_index]
                              .name,
                           _world
                              .planning_hubs[_world.planning_connections[right].start_hub_index]
                              .name);
                     });
               }
               else if (sort_specs.ColumnIndex == end_column) {
                  fill_sort_map(
                     _world_explorer_sort_map, sort_specs.SortDirection,
                     [&](const uint32 left, const uint32 right) {
                        return string::iless_than(
                           _world
                              .planning_hubs[_world.planning_connections[left].end_hub_index]
                              .name,
                           _world
                              .planning_hubs[_world.planning_connections[right].end_hub_index]
                              .name);
                     });
               }
               else if (sort_specs.ColumnIndex == soldier_column or
                        sort_specs.ColumnIndex == hover_column or
                        sort_specs.ColumnIndex == small_column or
                        sort_specs.ColumnIndex == medium_column or
                        sort_specs.ColumnIndex == huge_column or
                        sort_specs.ColumnIndex == flyer_column) {
                  using world::ai_path_flags;

                  ai_path_flags flag = {};

                  // clang-format off

                  if (sort_specs.ColumnIndex == soldier_column) flag = ai_path_flags::soldier;
                  else if (sort_specs.ColumnIndex == hover_column) flag = ai_path_flags::hover;
                  else if (sort_specs.ColumnIndex == small_column) flag = ai_path_flags::small;
                  else if (sort_specs.ColumnIndex == medium_column) flag = ai_path_flags::medium;
                  else if (sort_specs.ColumnIndex == huge_column) flag = ai_path_flags::huge;
                  else if (sort_specs.ColumnIndex == flyer_column) flag = ai_path_flags::flyer;

                  // clang-format on

                  fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                                [&, flag](const uint32 left, const uint32 right) {
                                   return (_world.planning_connections[left].flags & flag) <
                                          (_world.planning_connections[right].flags &
                                           flag);
                                });
               }
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.planning_connections.size();
                 ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::planning_connection& connection =
                  _world.planning_connections[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(connection.name, _world_explorer_filter)) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(connection.id));

               const bool is_selected =
                  world::is_selected(connection.id, _interaction_targets.selection);

               using world::ai_path_flags;

               const ai_path_flags flags = connection.flags;

               bool soldier = are_flags_set(flags, ai_path_flags::soldier);
               bool hover = are_flags_set(flags, ai_path_flags::hover);
               bool small = are_flags_set(flags, ai_path_flags::small);
               bool medium = are_flags_set(flags, ai_path_flags::medium);
               bool huge = are_flags_set(flags, ai_path_flags::huge);
               bool flyer = are_flags_set(flags, ai_path_flags::flyer);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(connection.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover_entity = ImGui::IsItemHovered();
               ImGui::TableNextColumn();
               ImGui::Text(
                  _world.planning_hubs[connection.start_hub_index].name.c_str());
               ImGui::TableNextColumn();
               ImGui::Text(_world.planning_hubs[connection.end_hub_index].name.c_str());
               ImGui::TableNextColumn();
               ImGui::Text(soldier ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(hover ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(small ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(medium ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(huge ? "X" : "-");
               ImGui::TableNextColumn();
               ImGui::Text(flyer ? "X" : "-");

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(connection.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(connection.id);
                  }
               }

               if (hover_entity) {
                  _interaction_targets.hovered_entity = connection.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Boundaries")) {
         ImGui::PushItemWidth(std::floor((ImGui::GetContentRegionAvail().x) / 4.0f));
         ImGui::InputTextWithHint("Name Filter", "e.g. Boundary",
                                  &_world_explorer_filter);
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Boundaries", 1,
                               ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable |
                                  ImGuiTableFlags_SortTristate)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs* table_sort_specs = ImGui::TableGetSortSpecs();
            const bool sorting = table_sort_specs->SpecsCount >= 1;

            if (sorting) {
               if (_world_explorer_sort_map.size() != _world.boundaries.size()) {
                  _world_explorer_sort_map.resize(_world.boundaries.size());

                  for (uint32 i = 0; i < _world_explorer_sort_map.size(); ++i) {
                     _world_explorer_sort_map[i] = i;
                  }
               }

               const ImGuiTableColumnSortSpecs sort_specs =
                  table_sort_specs->Specs[0];

               fill_sort_map(_world_explorer_sort_map, sort_specs.SortDirection,
                             [&](const uint32 left, const uint32 right) {
                                return string::iless_than(
                                   _world.boundaries[left].name,
                                   _world.boundaries[right].name);
                             });
            }

            for (uint32 unsorted_index = 0;
                 unsorted_index < _world.boundaries.size(); ++unsorted_index) {
               const uint32 index =
                  sorting ? _world_explorer_sort_map[unsorted_index] : unsorted_index;

               const world::boundary& boundary = _world.boundaries[index];

               if (not _world_explorer_filter.empty() and
                   not string::icontains(boundary.name, _world_explorer_filter)) {
                  continue;
               }

               ImGui::PushID(std::to_underlying(boundary.id));

               const bool is_selected =
                  world::is_selected(boundary.id, _interaction_targets.selection);

               ImGui::TableNextRow();

               ImGui::TableNextColumn();
               const bool select =
                  ImGui::Selectable(boundary.name.c_str(), is_selected,
                                    ImGuiSelectableFlags_SpanAllColumns);
               const bool hover = ImGui::IsItemHovered();

               if (select) {
                  if (is_ctrl_down) {
                     _interaction_targets.selection.remove(boundary.id);
                  }
                  else {
                     if (not is_shift_down) {
                        _interaction_targets.selection.clear();
                     }

                     _interaction_targets.selection.add(boundary.id);
                  }
               }

               if (hover) {
                  _interaction_targets.hovered_entity = boundary.id;
               }

               ImGui::PopID();
            }

            ImGui::EndTable();
         }

         ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
   }

   ImGui::End();
}

}