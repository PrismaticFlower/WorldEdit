
#include "world_edit.hpp"

#include "actions/imgui_ext.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_stdlib.h"
#include "utility/look_for.hpp"
#include "utility/overload.hpp"

#include <range/v3/view/enumerate.hpp>

using namespace std::literals;
using ranges::views::enumerate;

namespace we {

void world_edit::update_ui() noexcept
{
   ImGui_ImplDX12_NewFrame();
   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();
   ImGui::ShowDemoWindow(nullptr);

   if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
         if (ImGui::MenuItem("Open Project")) open_project_with_picker();

         ImGui::Separator();

         const bool loaded_project = not _project_dir.empty();

         if (ImGui::BeginMenu("Load World", loaded_project)) {
            auto worlds_path = _project_dir / L"Worlds"sv;

            for (auto& known_world : _project_world_paths) {
               auto relative_path =
                  std::filesystem::relative(known_world, worlds_path);

               if (ImGui::MenuItem(relative_path.string().c_str())) {
                  load_world(known_world);
               }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Browse...")) load_world_with_picker();

            ImGui::EndMenu();
         }

         const bool loaded_world = not _world_path.empty();

         ImGui::MenuItem("Save World", "Ctrl + S", nullptr, false);

         if (ImGui::MenuItem("Save World As...", nullptr, nullptr, loaded_world)) {
            save_world_with_picker();
         }

         ImGui::Separator();

         if (ImGui::MenuItem("Close World", nullptr, nullptr, loaded_world)) {
            close_world();
         }

         ImGui::EndMenu();
      }

      // TODO: Enable once world editing is actually a thing.

      if (ImGui::BeginMenu("Edit", false)) {
         ImGui::MenuItem("Undo", "Ctrl + Z");
         ImGui::MenuItem("Redo", "Ctrl + Y");

         ImGui::Separator();

         ImGui::MenuItem("Cut", "Ctrl + X");
         ImGui::MenuItem("Copy", "Ctrl + C");
         ImGui::MenuItem("Paste", "Ctrl + V");

         ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
   }

   ImGui::SetNextWindowPos({0.0f, 32.0f * _display_scale});
   ImGui::SetNextWindowSize({224.0f * _display_scale, 512.0f * _display_scale});

   ImGui::Begin("World Active Context", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove);

   ImGui::TextUnformatted("Active Layers");
   ImGui::Separator();
   ImGui::BeginChild("Active Layers", ImVec2{0.0f, 208.0f * _display_scale});

   for (auto [i, layer] : enumerate(_world.layer_descriptions)) {
      if (ImGui::Selectable(layer.name.c_str(), _world_layers_draw_mask[i])) {
         _world_layers_draw_mask[i] = not _world_layers_draw_mask[i];
      }
   }

   ImGui::EndChild();

   ImGui::TextUnformatted("Active Entities");
   ImGui::Separator();
   ImGui::BeginChild("Active Entities", ImVec2{0.0f, 236.0f * _display_scale});

   if (ImGui::Selectable("Objects", _world_draw_mask.objects)) {
      _world_draw_mask.objects = not _world_draw_mask.objects;
   }

   if (ImGui::Selectable("Lights", _world_draw_mask.lights)) {
      _world_draw_mask.lights = not _world_draw_mask.lights;
   }

   if (ImGui::Selectable("Paths", _world_draw_mask.paths)) {
      _world_draw_mask.paths = not _world_draw_mask.paths;
   }

   if (ImGui::Selectable("Regions", _world_draw_mask.regions)) {
      _world_draw_mask.regions = not _world_draw_mask.regions;
   }

   if (ImGui::Selectable("Sectors", _world_draw_mask.sectors)) {
      _world_draw_mask.sectors = not _world_draw_mask.sectors;
   }

   if (ImGui::Selectable("Portals", _world_draw_mask.portals)) {
      _world_draw_mask.portals = not _world_draw_mask.portals;
   }

   if (ImGui::Selectable("Hintnodes", _world_draw_mask.hintnodes)) {
      _world_draw_mask.hintnodes = not _world_draw_mask.hintnodes;
   }

   if (ImGui::Selectable("Barriers", _world_draw_mask.barriers)) {
      _world_draw_mask.barriers = not _world_draw_mask.barriers;
   }

   if (ImGui::Selectable("Planning Hubs", _world_draw_mask.planning_hubs)) {
      _world_draw_mask.planning_hubs = not _world_draw_mask.planning_hubs;
   }

   if (ImGui::Selectable("Planning Connections", _world_draw_mask.planning_connections)) {
      _world_draw_mask.planning_connections = not _world_draw_mask.planning_connections;
   }

   if (ImGui::Selectable("Boundaries", _world_draw_mask.boundaries)) {
      _world_draw_mask.boundaries = not _world_draw_mask.boundaries;
   }

   ImGui::EndChild();

   ImGui::End();

   if (not _interaction_targets.selection.empty()) {
      ImGui::Begin("Selection");

      boost::variant2::visit(
         overload{
            [&](world::object_id id) {
               world::object* object =
                  look_for(_world.objects, [id](const world::object& object) {
                     return id == object.id;
                  });

               if (not object) return;

               ImGui::InputText("Name", object, &world::object::name,
                                &_undo_stack, &_world);
               ImGui::InputTextAutoComplete(
                  "Class Name", object, &world::object::class_name,
                  &_undo_stack, &_world, [&] {
                     std::array<we::lowercase_string, 6> entries;

                     _asset_libraries.odfs.enumerate_known([&](auto range) {
                        std::size_t matching_count = 0;

                        for (const lowercase_string& asset : range) {
                           if (not asset.contains(object->class_name)) {
                              continue;
                           }

                           entries[matching_count] = asset;

                           ++matching_count;

                           if (matching_count == entries.size()) break;
                        }
                     });

                     return entries;
                  });
               ImGui::LayerPick("Layer", object, &_undo_stack, &_world);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", object, &world::object::rotation,
                               &_undo_stack, &_world);
               ImGui::DragFloat3("Position", object, &world::object::position,
                                 &_undo_stack, &_world);

               ImGui::Separator();

               ImGui::SliderInt("Team", object, &world::object::team, &_undo_stack,
                                &_world, 0, 15, "%d", ImGuiSliderFlags_AlwaysClamp);

               ImGui::Separator();

               for (auto [i, prop] : enumerate(object->instance_properties)) {
                  if (prop.key.contains("Path")) {
                     ImGui::InputKeyValueAutoComplete(
                        object, &world::object::instance_properties, i,
                        &_undo_stack, &_world, [&] {
                           std::array<std::string, 6> entries;

                           std::size_t matching_count = 0;

                           for (auto& path : _world.paths) {
                              if (not path.name.contains(prop.value)) {
                                 continue;
                              }

                              entries[matching_count] = path.name;

                              ++matching_count;

                              if (matching_count == entries.size()) break;
                           }

                           return entries;
                        });
                  }
                  else if (prop.key.contains("Region")) {
                     ImGui::InputKeyValueAutoComplete(
                        object, &world::object::instance_properties, i,
                        &_undo_stack, &_world, [&] {
                           std::array<std::string, 6> entries;

                           std::size_t matching_count = 0;

                           for (auto& region : _world.regions) {
                              if (not region.description.contains(prop.value)) {
                                 continue;
                              }

                              entries[matching_count] = region.description;

                              ++matching_count;

                              if (matching_count == entries.size()) break;
                           }

                           return entries;
                        });
                  }
                  else {
                     ImGui::InputKeyValue(object, &world::object::instance_properties,
                                          i, &_undo_stack, &_world);
                  }
               }
            },
            [&](world::light_id id) {
               world::light* light =
                  look_for(_world.lights, [id](const world::light& light) {
                     return id == light.id;
                  });

               if (not light) return;

               ImGui::InputText("Name", light, &world::light::name,
                                &_undo_stack, &_world);
               ImGui::LayerPick("Layer", light, &_undo_stack, &_world);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", light, &world::light::rotation,
                               &_undo_stack, &_world);
               ImGui::DragFloat3("Position", light, &world::light::position,
                                 &_undo_stack, &_world);

               ImGui::Separator();

               ImGui::ColorEdit3("Color", light, &world::light::color,
                                 &_undo_stack, &_world,
                                 ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

               ImGui::Checkbox("Static", light, &world::light::static_,
                               &_undo_stack, &_world);
               ImGui::SameLine();
               ImGui::Checkbox("Shadow Caster", light, &world::light::shadow_caster,
                               &_undo_stack, &_world);
               ImGui::SameLine();
               ImGui::Checkbox("Specular Caster", light,
                               &world::light::specular_caster, &_undo_stack, &_world);

               ImGui::EnumSelect("Light Type", light, &world::light::light_type,
                                 &_undo_stack, &_world,
                                 {enum_select_option{"Directional",
                                                     world::light_type::directional},
                                  enum_select_option{"Point", world::light_type::point},
                                  enum_select_option{"Spot", world::light_type::spot}});

               ImGui::Separator();

               ImGui::DragFloat("Range", light, &world::light::range,
                                &_undo_stack, &_world);
               ImGui::DragFloat("Inner Cone Angle", light,
                                &world::light::inner_cone_angle, &_undo_stack,
                                &_world, 0.01f, 0.0f, light->outer_cone_angle,
                                "%.3f", ImGuiSliderFlags_AlwaysClamp);
               ImGui::DragFloat("Outer Cone Angle", light,
                                &world::light::outer_cone_angle, &_undo_stack,
                                &_world, 0.01f, light->inner_cone_angle, 1.570f,
                                "%.3f", ImGuiSliderFlags_AlwaysClamp);

               ImGui::Separator();

               ImGui::DragFloat2("Directional Texture Tiling", light,
                                 &world::light::directional_texture_tiling,
                                 &_undo_stack, &_world, 0.01f);
               ImGui::DragFloat2("Directional Texture Offset", light,
                                 &world::light::directional_texture_offset,
                                 &_undo_stack, &_world, 0.01f);

               ImGui::InputTextAutoComplete(
                  "Texture", light, &world::light::texture, &_undo_stack, &_world, [&] {
                     std::array<std::string, 6> entries;

                     _asset_libraries.textures.enumerate_known([&](auto range) {
                        std::size_t matching_count = 0;

                        for (const std::string& asset : range) {
                           if (not asset.contains(light->texture)) {
                              continue;
                           }

                           entries[matching_count] = asset;

                           ++matching_count;

                           if (matching_count == entries.size()) break;
                        }
                     });

                     return entries;
                  });
               ImGui::InputTextAutoComplete("Directional Region", light,
                                            &world::light::directional_region,
                                            &_undo_stack, &_world, [&] {
                                               std::array<std::string, 6> entries;

                                               std::size_t matching_count = 0;

                                               for (auto& region : _world.regions) {
                                                  if (not region.description.contains(
                                                         light->directional_region)) {
                                                     continue;
                                                  }

                                                  entries[matching_count] =
                                                     region.description;

                                                  ++matching_count;

                                                  if (matching_count == entries.size())
                                                     break;
                                               }

                                               return entries;
                                            });
            },
            [&](world::path_id id) {
               world::path* path =
                  look_for(_world.paths, [id](const world::path& path) {
                     return id == path.id;
                  });

               if (not path) return;

               ImGui::InputText("Name", path, &world::path::name, &_undo_stack, &_world);
               ImGui::LayerPick("Layer", path, &_undo_stack, &_world);

               ImGui::Separator();

               ImGui::EnumSelect(
                  "Spline Type", path, &world::path::spline_type, &_undo_stack, &_world,
                  {enum_select_option{"None", world::path_spline_type::none},
                   enum_select_option{"Linear", world::path_spline_type::linear},
                   enum_select_option{"Hermite", world::path_spline_type::hermite},
                   enum_select_option{"Catmull-Rom",
                                      world::path_spline_type::catmull_rom}});

               for (auto [i, prop] : enumerate(path->properties)) {
                  ImGui::InputKeyValue(path, &world::path::properties, i,
                                       &_undo_stack, &_world);
               }

               ImGui::Text("Nodes");
               ImGui::BeginChild("Nodes", {}, true);

               for (auto [i, node] : enumerate(path->nodes)) {
                  ImGui::PushID(static_cast<int>(i));

                  ImGui::Text("Node %i", static_cast<int>(i));
                  ImGui::Separator();

                  ImGui::DragQuat("Rotation", path, i, &world::path::node::rotation,
                                  &_undo_stack, &_world);
                  ImGui::DragFloat3("Position", path, i, &world::path::node::position,
                                    &_undo_stack, &_world);

                  for (auto [prop_index, prop] : enumerate(node.properties)) {
                     ImGui::InputKeyValue(path, i, prop_index, &_undo_stack, &_world);
                  }

                  ImGui::PopID();
               }

               ImGui::EndChild();
            },
            [&](world::path_id_node_pair id_node) {
               auto [id, node_index] = id_node;

               world::path* path =
                  look_for(_world.paths, [id](const world::path& path) {
                     return id == path.id;
                  });

               if (not path) return;

               if (node_index >= path->nodes.size()) return;
            },
            [&](world::region_id id) {
               world::region* region =
                  look_for(_world.regions, [id](const world::region& region) {
                     return id == region.id;
                  });

               if (not region) return;

               ImGui::InputText("Name", region, &world::region::name,
                                &_undo_stack, &_world);
               ImGui::LayerPick("Layer", region, &_undo_stack, &_world);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", region, &world::region::rotation,
                               &_undo_stack, &_world);
               ImGui::DragFloat3("Position", region, &world::region::position,
                                 &_undo_stack, &_world);
               ImGui::DragFloat3("Size", region, &world::region::size,
                                 &_undo_stack, &_world);

               ImGui::EnumSelect(
                  "Shape", region, &world::region::shape, &_undo_stack, &_world,
                  {enum_select_option{"Box", world::region_shape::box},
                   enum_select_option{"Sphere", world::region_shape::sphere},
                   enum_select_option{"Cylinder", world::region_shape::cylinder}});

               ImGui::Separator();

               ImGui::InputText("Description", region, &world::region::description,
                                &_undo_stack, &_world);
            },
            [&](world::sector_id id) {
               world::sector* sector =
                  look_for(_world.sectors, [id](const world::sector& sector) {
                     return id == sector.id;
                  });

               if (not sector) return;
            },
            [&](world::portal_id id) {
               world::portal* portal =
                  look_for(_world.portals, [id](const world::portal& portal) {
                     return id == portal.id;
                  });

               if (not portal) return;
            },
            [&](world::hintnode_id id) {
               world::hintnode* hintnode =
                  look_for(_world.hintnodes, [id](const world::hintnode& hintnode) {
                     return id == hintnode.id;
                  });

               if (not hintnode) return;

               ImGui::InputText("Name", hintnode, &world::hintnode::name,
                                &_undo_stack, &_world);
               ImGui::LayerPick("Layer", hintnode, &_undo_stack, &_world);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", hintnode, &world::hintnode::rotation,
                               &_undo_stack, &_world);
               ImGui::DragFloat3("Position", hintnode, &world::hintnode::position,
                                 &_undo_stack, &_world);
            },
            [&](world::barrier_id id) {
               world::barrier* barrier =
                  look_for(_world.barriers, [id](const world::barrier& barrier) {
                     return id == barrier.id;
                  });

               if (not barrier) return;
            },
            [&](world::planning_hub_id id) { (void)id; },
            [&](world::planning_connection_id id) { (void)id; },
            [&](world::boundary_id id) {
               world::boundary* boundary =
                  look_for(_world.boundaries, [id](const world::boundary& boundary) {
                     return id == boundary.id;
                  });

               if (not boundary) return;

               world::path* path =
                  look_for(_world.paths, [&](const world::path& path) {
                     return path.name == boundary->name;
                  });

               if (not path) return;
            },
         },
         _interaction_targets.selection.front());

      ImGui::End();
   }

   ImGui::Value("Undo Stack Size", (unsigned)_undo_stack.applied_size());
   ImGui::Value("Redo Stack Size", (unsigned)_undo_stack.reverted_size());

   if (ImGui::Button("Undo")) _undo_stack.revert(_world);
   if (ImGui::Button("Redo")) _undo_stack.reapply(_world);
}

}