#include "world_edit.hpp"

#include "edits/creation_entity_set.hpp"
#include "edits/set_value.hpp"
#include "utility/string_icompare.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace we {

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
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Objects", 5,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Class Name");
            ImGui::TableSetupColumn("Class Label");
            ImGui::TableSetupColumn("Layer");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::object& object : _world.objects) {
               if (not _world_explorer_filter.empty() and
                   not string::icontains(object.name, _world_explorer_filter)) {
                  continue;
               }
               if (not _world_explorer_class_filter.empty() and
                   not string::icontains(object.class_name,
                                         _world_explorer_class_filter)) {
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
                  _object_classes[object.class_name].definition->header.class_label;

               ImGui::TextUnformatted(class_label.data(),
                                      class_label.data() + class_label.size());
               ImGui::TableNextColumn();
               ImGui::Text(_world.layer_descriptions[object.layer].name.c_str());
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(object.id));

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
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Lights", 5,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Color");
            ImGui::TableSetupColumn("Layer");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::light& light : _world.lights) {
               if (not _world_explorer_filter.empty() and
                   not string::icontains(light.name, _world_explorer_filter)) {
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
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(light.id));

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
         ImGui::PopItemWidth();

         const bool show_all_nodes = _world_explorer_path_show_all_nodes;

         if (ImGui::BeginTable("Paths", show_all_nodes ? 5 : 4,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            if (show_all_nodes) ImGui::TableSetupColumn("Node");
            ImGui::TableSetupColumn("Nodes");
            ImGui::TableSetupColumn("Layer");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::path& path : _world.paths) {
               if (not _world_explorer_filter.empty() and
                   not string::icontains(path.name, _world_explorer_filter)) {
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
                  if (show_all_nodes) {
                     ImGui::TableNextColumn();
                     ImGui::Text("%i", i);
                  }
                  ImGui::TableNextColumn();
                  ImGui::Text("%i", static_cast<int>(path.nodes.size()));
                  ImGui::TableNextColumn();
                  ImGui::Text(_world.layer_descriptions[path.layer].name.c_str());
                  ImGui::TableNextColumn();
                  ImGui::Text("%i", static_cast<int>(path.id));

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
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Regions", 5,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Description");
            ImGui::TableSetupColumn("Shape");
            ImGui::TableSetupColumn("Layer");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::region& region : _world.regions) {
               if (not _world_explorer_filter.empty() and
                   not string::icontains(region.name, _world_explorer_filter)) {
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
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(region.id));

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

         if (ImGui::BeginTable("Sectors", 4,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Points");
            ImGui::TableSetupColumn("Objects");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::sector& sector : _world.sectors) {
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
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(sector.id));

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

         if (ImGui::BeginTable("Portals", 4,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Sector 1");
            ImGui::TableSetupColumn("Sector 2");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::portal& portal : _world.portals) {
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
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(portal.id));

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
         ImGui::PopItemWidth();

         if (ImGui::BeginTable("Hintnodes", 4,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Layer");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::hintnode& hintnode : _world.hintnodes) {
               if (not _world_explorer_filter.empty() and
                   not string::icontains(hintnode.name, _world_explorer_filter)) {
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
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(hintnode.id));

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

         if (ImGui::BeginTable("Barriers", 8,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Soldier");
            ImGui::TableSetupColumn("Hover");
            ImGui::TableSetupColumn("Small");
            ImGui::TableSetupColumn("Medium");
            ImGui::TableSetupColumn("Huge");
            ImGui::TableSetupColumn("Flyer");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::barrier& barrier : _world.barriers) {
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
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(barrier.id));

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

         if (ImGui::BeginTable("AI Planning Hubs", 2,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::planning_hub& hub : _world.planning_hubs) {
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
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(hub.id));

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

         if (ImGui::BeginTable("AI Planning Connections", 10,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Start");
            ImGui::TableSetupColumn("End");
            ImGui::TableSetupColumn("Soldier");
            ImGui::TableSetupColumn("Hover");
            ImGui::TableSetupColumn("Small");
            ImGui::TableSetupColumn("Medium");
            ImGui::TableSetupColumn("Huge");
            ImGui::TableSetupColumn("Flyer");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::planning_connection& connection :
                 _world.planning_connections) {
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
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(connection.id));

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

         if (ImGui::BeginTable("Boundaries", 2,
                               ImGuiTableFlags_Reorderable | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("ID");
            ImGui::TableHeadersRow();

            for (const world::boundary& boundary : _world.boundaries) {
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
               ImGui::TableNextColumn();
               ImGui::Text("%i", static_cast<int>(boundary.id));

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