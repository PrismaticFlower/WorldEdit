
#include "world_edit.hpp"

#include "edits/add_property.hpp"
#include "edits/creation_entity_set.hpp"
#include "edits/imgui_ext.hpp"
#include "edits/set_value.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_ext.hpp"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_stdlib.h"
#include "utility/look_for.hpp"
#include "utility/overload.hpp"
#include "world/utility/hintnode_traits.hpp"
#include "world/utility/object_properties.hpp"
#include "world/utility/path_properties.hpp"
#include "world/utility/region_properties.hpp"
#include "world/utility/snapping.hpp"
#include "world/utility/world_utilities.hpp"

#include <numbers>

using namespace std::literals;

namespace we {

namespace {

struct placement_traits {
   bool has_new_path = false;
   bool has_placement_rotation = true;
   bool has_point_at = true;
   bool has_placement_mode = true;
   bool has_lock_axis = true;
   bool has_placement_alignment = true;
   bool has_placement_ground = true;
   bool has_node_placement_insert = false;
   bool has_resize_to = false;
   bool has_from_bbox = false;
   bool has_from_line = false;
};

auto surface_rotation_degrees(const float3 surface_normal,
                              const float fallback_angle) -> float
{
   if (surface_normal.x == 0.0f and surface_normal.z == 0.0f) {
      return fallback_angle;
   }

   const float2 direction = normalize(float2{surface_normal.x, surface_normal.z});

   const float angle =
      std::atan2(-direction.x, -direction.y) + std::numbers::pi_v<float>;

   return std::fmod(angle * 180.0f / std::numbers::pi_v<float>, 360.0f);
}

auto align_position_to_grid(const float2 position, const float alignment) -> float2
{
   return float2{std::round(position.x / alignment) * alignment,
                 std::round(position.y / alignment) * alignment};
}

auto align_position_to_grid(const float3 position, const float alignment) -> float3
{
   return float3{std::round(position.x / alignment) * alignment, position.y,
                 std::round(position.z / alignment) * alignment};
}

}

void world_edit::update_ui() noexcept
{
   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();
   ImGui::ShowDemoWindow(nullptr);

   _tool_visualizers.clear();

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

         if (ImGui::MenuItem("Save World",
                             get_display_string(
                                _hotkeys.query_binding("", "save")))) {
            save_world(_world_path);
         }

         if (ImGui::MenuItem("Save World As...", nullptr, nullptr, loaded_world)) {
            save_world_with_picker();
         }

         ImGui::Separator();

         if (ImGui::MenuItem("Close World", nullptr, nullptr, loaded_world)) {
            close_world();
         }

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Edit")) {
         if (ImGui::MenuItem("Undo",
                             get_display_string(
                                _hotkeys.query_binding("", "edit.undo")))) {
            _edit_stack_world.revert(_edit_context);
         }
         if (ImGui::MenuItem("Redo",
                             get_display_string(
                                _hotkeys.query_binding("", "edit.redo")))) {
            _edit_stack_world.reapply(_edit_context);
         }

         ImGui::Separator();

         ImGui::MenuItem("Cut", nullptr, nullptr, false);
         ImGui::MenuItem("Copy", nullptr, nullptr, false);
         ImGui::MenuItem("Paste", nullptr, nullptr, false);

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Create")) {
         if (ImGui::MenuItem("Object")) {
            const world::object* base_object =
               world::find_entity(_world.objects, _entity_creation_context.last_object);

            world::object new_object;

            if (base_object) {
               new_object = *base_object;

               new_object.name =
                  world::create_unique_name(_world.objects, base_object->name);
               new_object.id = world::max_id;
            }
            else {
               new_object =
                  world::object{.name = "",
                                .class_name = lowercase_string{"com_bldg_controlzone"sv},
                                .id = world::max_id};
            }

            _edit_stack_world
               .apply(edits::make_creation_entity_set(std::move(new_object),
                                                      _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("Light")) {
            const world::light* base_light =
               world::find_entity(_world.lights, _entity_creation_context.last_light);

            world::light new_light;

            if (base_light) {
               new_light = *base_light;

               new_light.name =
                  world::create_unique_name(_world.lights, base_light->name);
               new_light.id = world::max_id;
            }
            else {
               new_light = world::light{.name = "", .id = world::max_id};
            }

            _edit_stack_world
               .apply(edits::make_creation_entity_set(std::move(new_light),
                                                      _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("Path")) {
            const world::path* base_path =
               world::find_entity(_world.paths, _entity_creation_context.last_path);

            _edit_stack_world
               .apply(edits::make_creation_entity_set(
                         world::path{.name = world::create_unique_name(
                                        _world.paths, base_path ? base_path->name : "Path 0"),
                                     .layer = base_path ? base_path->layer : 0,
                                     .nodes = {world::path::node{}},
                                     .id = world::max_id},
                         _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("Region")) {
            const world::region* base_region =
               world::find_entity(_world.regions, _entity_creation_context.last_region);

            world::region new_region;

            if (base_region) {
               new_region = *base_region;

               new_region.name =
                  world::create_unique_name(_world.regions, base_region->name);
               new_region.id = world::max_id;
            }
            else {
               new_region =
                  world::region{.name = world::create_unique_name(_world.lights, "Region0"),
                                .id = world::max_id};
            }

            _edit_stack_world
               .apply(edits::make_creation_entity_set(std::move(new_region),
                                                      _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("Sector")) {
            const world::sector* base_sector =
               world::find_entity(_world.sectors, _entity_creation_context.last_sector);

            _edit_stack_world
               .apply(edits::make_creation_entity_set(
                         world::sector{.name = world::create_unique_name(
                                          _world.sectors,
                                          base_sector ? base_sector->name : "Sector0"),
                                       .base = 0.0f,
                                       .height = 10.0f,
                                       .points = {{0.0f, 0.0f}},
                                       .id = world::max_id},
                         _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("Portal")) {
            const world::portal* base_portal =
               world::find_entity(_world.portals, _entity_creation_context.last_portal);

            world::portal new_portal;

            if (base_portal) {
               new_portal = *base_portal;

               new_portal.name =
                  world::create_unique_name(_world.portals, base_portal->name);
               new_portal.id = world::max_id;
            }
            else {
               new_portal = world::portal{.name = "Portal0", .id = world::max_id};
            }

            _edit_stack_world
               .apply(edits::make_creation_entity_set(std::move(new_portal),
                                                      _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("Hintnode")) {
            const world::hintnode* base_hintnode =
               world::find_entity(_world.hintnodes,
                                  _entity_creation_context.last_hintnode);

            world::hintnode new_hintnode;

            if (base_hintnode) {
               new_hintnode = *base_hintnode;

               new_hintnode.name =
                  world::create_unique_name(_world.hintnodes, base_hintnode->name);
               new_hintnode.id = world::max_id;
            }
            else {
               new_hintnode = world::hintnode{.name = "Hint0", .id = world::max_id};
            }

            _edit_stack_world
               .apply(edits::make_creation_entity_set(std::move(new_hintnode),
                                                      _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("Barrier")) {
            const world::barrier* base_barrier =
               world::find_entity(_world.barriers, _entity_creation_context.last_barrier);

            world::barrier new_barrier;

            if (base_barrier) {
               new_barrier = *base_barrier;

               new_barrier.name =
                  world::create_unique_name(_world.barriers, base_barrier->name);
               new_barrier.id = world::max_id;
            }
            else {
               new_barrier = world::barrier{.name = "Barrier0", .id = world::max_id};
            }

            _edit_stack_world
               .apply(edits::make_creation_entity_set(std::move(new_barrier),
                                                      _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("AI Planning Hub")) {
            _entity_creation_context.hub_sizing_started = false;

            const world::planning_hub* base_hub =
               world::find_entity(_world.planning_hubs,
                                  _entity_creation_context.last_planning_hub);

            world::planning_hub new_hub;

            if (base_hub) {
               new_hub = *base_hub;

               new_hub.name =
                  world::create_unique_name(_world.planning_hubs, base_hub->name);
               new_hub.id = world::max_id;
            }
            else {
               new_hub = world::planning_hub{.name = "Hub0", .id = world::max_id};
            }

            _edit_stack_world
               .apply(edits::make_creation_entity_set(std::move(new_hub),
                                                      _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("AI Planning Connection") and
             not _world.planning_hubs.empty()) {
            _entity_creation_context.connection_link_started = false;
            _world_draw_mask.planning_hubs = true;
            _world_draw_mask.planning_connections = true;

            const world::planning_connection* base_connection =
               world::find_entity(_world.planning_connections,
                                  _entity_creation_context.last_planning_connection);

            world::planning_connection new_connection;

            if (base_connection) {
               new_connection = *base_connection;

               new_connection.name =
                  world::create_unique_name(_world.planning_connections,
                                            base_connection->name);
               new_connection.id = world::max_id;
            }
            else {
               new_connection =
                  world::planning_connection{.name = "Connection0",
                                             .start = _world.planning_hubs[0].id,
                                             .end = _world.planning_hubs[0].id,
                                             .id = world::max_id};
            }

            _edit_stack_world
               .apply(edits::make_creation_entity_set(std::move(new_connection),
                                                      _interaction_targets.creation_entity),
                      _edit_context);
         }

         if (ImGui::MenuItem("Boundary")) {
            const world::boundary* base_boundary =
               world::find_entity(_world.boundaries,
                                  _entity_creation_context.last_boundary);

            world::boundary new_boundary;

            if (base_boundary) {
               new_boundary = *base_boundary;

               new_boundary.name =
                  world::create_unique_name(_world.boundaries, base_boundary->name);
               new_boundary.id = world::max_id;
            }
            else {
               new_boundary = world::boundary{.name = "Boundary", .id = world::max_id};
            }

            _edit_stack_world
               .apply(edits::make_creation_entity_set(std::move(new_boundary),
                                                      _interaction_targets.creation_entity),
                      _edit_context);
         }

         ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Developer")) {
         if (ImGui::MenuItem("Reload Shaders")) {
            try {
               _renderer->reload_shaders();
            }
            catch (graphics::gpu::exception& e) {
               handle_gpu_error(e);
            }
         }

         ImGui::Selectable("Show GPU Profiler", &_settings.graphics.show_profiler);

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

   for (std::size_t i = 0; i < _world.layer_descriptions.size(); ++i) {
      if (ImGui::Selectable(_world.layer_descriptions[i].name.c_str(),
                            _world_layers_draw_mask[i])) {
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

   if (ImGui::Selectable("AI Planning", _world_draw_mask.planning_hubs or
                                           _world_draw_mask.planning_connections)) {
      _world_draw_mask.planning_hubs = not _world_draw_mask.planning_hubs;
      _world_draw_mask.planning_connections = not _world_draw_mask.planning_connections;
   }

   if (ImGui::Selectable("Boundaries", _world_draw_mask.boundaries)) {
      _world_draw_mask.boundaries = not _world_draw_mask.boundaries;
   }

   ImGui::EndChild();

   ImGui::End();

   if (_hotkeys_show) {
      ImGui::SetNextWindowPos({ImGui::GetIO().DisplaySize.x, 32.0f * _display_scale},
                              ImGuiCond_Always, {1.0f, 0.0f});
      ImGui::SetNextWindowSizeConstraints({224.0f * _display_scale, -1.0f},
                                          {224.0f * _display_scale, -1.0f});

      ImGui::Begin("Hotkeys", &_hotkeys_show,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                      ImGuiWindowFlags_NoBringToFrontOnFocus |
                      ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs |
                      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration);

      ImGui::End();
   }

   if (not _interaction_targets.selection.empty()) {
      ImGui::SetNextWindowPos({232.0f * _display_scale, 32.0f * _display_scale},
                              ImGuiCond_Once, {0.0f, 0.0f});

      bool selection_open = true;

      ImGui::Begin("Selection", &selection_open, ImGuiWindowFlags_NoCollapse);

      std::visit(
         overload{
            [&](world::object_id id) {
               world::object* object =
                  look_for(_world.objects, [id](const world::object& object) {
                     return id == object.id;
                  });

               if (not object) return;

               ImGui::InputText("Name", object, &world::object::name,
                                &_edit_stack_world, &_edit_context);
               ImGui::InputTextAutoComplete(
                  "Class Name", object, &world::object::class_name,
                  &_edit_stack_world, &_edit_context, [&] {
                     std::array<std::string, 6> entries;
                     std::size_t matching_count = 0;

                     _asset_libraries.odfs.enumerate_known(
                        [&](const lowercase_string& asset) {
                           if (matching_count == entries.size()) return;
                           if (not asset.contains(object->class_name)) return;

                           entries[matching_count] = asset;

                           ++matching_count;
                        });

                     return entries;
                  });

               if (ImGui::IsItemDeactivatedAfterEdit()) {
                  std::vector<world::instance_property> new_instance_properties =
                     world::make_object_instance_properties(
                        *_object_classes[object->class_name].definition,
                        object->instance_properties);

                  if (new_instance_properties != object->instance_properties) {
                     _edit_stack_world.apply(
                        edits::make_set_value(object->id, &world::object::instance_properties,
                                              std::move(new_instance_properties),
                                              object->instance_properties),
                        _edit_context, {.transparent = true});
                  }
               }

               ImGui::LayerPick("Layer", object, &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", object, &world::object::rotation,
                               &_edit_stack_world, &_edit_context);
               ImGui::DragFloat3("Position", object, &world::object::position,
                                 &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               ImGui::SliderInt("Team", object, &world::object::team,
                                &_edit_stack_world, &_edit_context, 0, 15, "%d",
                                ImGuiSliderFlags_AlwaysClamp);

               ImGui::Separator();

               for (std::size_t i = 0; i < object->instance_properties.size(); ++i) {
                  world::instance_property& prop = object->instance_properties[i];

                  if (prop.key.contains("Path")) {
                     ImGui::InputKeyValueAutoComplete(
                        object, &world::object::instance_properties, i,
                        &_edit_stack_world, &_edit_context, [&] {
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
                        &_edit_stack_world, &_edit_context, [&] {
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
                                          i, &_edit_stack_world, &_edit_context);
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
                                &_edit_stack_world, &_edit_context);
               ImGui::LayerPick("Layer", light, &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", light, &world::light::rotation,
                               &_edit_stack_world, &_edit_context);
               ImGui::DragFloat3("Position", light, &world::light::position,
                                 &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               ImGui::ColorEdit3("Color", light, &world::light::color,
                                 &_edit_stack_world, &_edit_context,
                                 ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

               ImGui::Checkbox("Static", light, &world::light::static_,
                               &_edit_stack_world, &_edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("Shadow Caster", light, &world::light::shadow_caster,
                               &_edit_stack_world, &_edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("Specular Caster", light, &world::light::specular_caster,
                               &_edit_stack_world, &_edit_context);

               ImGui::EnumSelect(
                  "Light Type", light, &world::light::light_type,
                  &_edit_stack_world, &_edit_context,
                  {enum_select_option{"Directional", world::light_type::directional},
                   enum_select_option{"Point", world::light_type::point},
                   enum_select_option{"Spot", world::light_type::spot},
                   enum_select_option{"Directional Region Box",
                                      world::light_type::directional_region_box},
                   enum_select_option{"Directional Region Sphere",
                                      world::light_type::directional_region_sphere},
                   enum_select_option{"Directional Region Cylinder",
                                      world::light_type::directional_region_cylinder}});

               ImGui::Separator();

               ImGui::DragFloat("Range", light, &world::light::range,
                                &_edit_stack_world, &_edit_context);
               ImGui::DragFloat("Inner Cone Angle", light,
                                &world::light::inner_cone_angle, &_edit_stack_world,
                                &_edit_context, 0.01f, 0.0f, light->outer_cone_angle,
                                "%.3f", ImGuiSliderFlags_AlwaysClamp);
               ImGui::DragFloat("Outer Cone Angle", light,
                                &world::light::outer_cone_angle, &_edit_stack_world,
                                &_edit_context, 0.01f, light->inner_cone_angle,
                                1.570f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

               ImGui::Separator();

               ImGui::DragFloat2("Directional Texture Tiling", light,
                                 &world::light::directional_texture_tiling,
                                 &_edit_stack_world, &_edit_context, 0.01f);
               ImGui::DragFloat2("Directional Texture Offset", light,
                                 &world::light::directional_texture_offset,
                                 &_edit_stack_world, &_edit_context, 0.01f);

               ImGui::InputTextAutoComplete(
                  "Texture", light, &world::light::texture, &_edit_stack_world,
                  &_edit_context, [&] {
                     std::array<std::string, 6> entries;
                     std::size_t matching_count = 0;

                     _asset_libraries.textures.enumerate_known(
                        [&](const lowercase_string& asset) {
                           if (matching_count == entries.size()) return;
                           if (not asset.contains(light->texture)) return;

                           entries[matching_count] = asset;

                           ++matching_count;
                        });

                     return entries;
                  });

               ImGui::Separator();

               ImGui::InputText("Region Name", light, &world::light::region_name,
                                &_edit_stack_world, &_edit_context);
               ImGui::DragQuat("Region Rotation", light, &world::light::region_rotation,
                               &_edit_stack_world, &_edit_context);
               ImGui::DragFloat3("Region Size", light, &world::light::region_size,
                                 &_edit_stack_world, &_edit_context);
            },
            [&](world::path_id id) {
               world::path* path =
                  look_for(_world.paths, [id](const world::path& path) {
                     return id == path.id;
                  });

               if (not path) return;

               ImGui::InputText("Name", path, &world::path::name,
                                &_edit_stack_world, &_edit_context);
               ImGui::LayerPick("Layer", path, &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               ImGui::EnumSelect(
                  "Path Type", path, &world::path::type, &_edit_stack_world,
                  &_edit_context,
                  {enum_select_option{"None", world::path_type::none},
                   enum_select_option{"Entity Follow", world::path_type::entity_follow},
                   enum_select_option{"Formation", world::path_type::formation},
                   enum_select_option{"Patrol", world::path_type::patrol}});

               ImGui::EnumSelect(
                  "Spline Type", path, &world::path::spline_type,
                  &_edit_stack_world, &_edit_context,
                  {enum_select_option{"None", world::path_spline_type::none},
                   enum_select_option{"Linear", world::path_spline_type::linear},
                   enum_select_option{"Hermite", world::path_spline_type::hermite},
                   enum_select_option{"Catmull-Rom",
                                      world::path_spline_type::catmull_rom}});

               for (std::size_t i = 0; i < path->properties.size(); ++i) {
                  ImGui::InputKeyValue(path, &world::path::properties, i,
                                       &_edit_stack_world, &_edit_context);
               }

               if (ImGui::BeginCombo("Add Property", "<select property>")) {
                  for (const char* prop : world::get_path_properties(path->type)) {
                     if (ImGui::Selectable(prop)) {
                        _edit_stack_world.apply(edits::make_add_property(path->id, prop),
                                                _edit_context);
                     }
                  }

                  ImGui::EndCombo();
               }

               ImGui::Text("Nodes");
               ImGui::BeginChild("Nodes", {}, true);

               for (std::size_t i = 0; i < path->nodes.size(); ++i) {
                  ImGui::PushID(static_cast<int>(i));

                  ImGui::Text("Node %i", static_cast<int>(i));
                  ImGui::Separator();

                  ImGui::DragQuat("Rotation", path, i, &world::path::node::rotation,
                                  &_edit_stack_world, &_edit_context);
                  ImGui::DragFloat3("Position", path, i, &world::path::node::position,
                                    &_edit_stack_world, &_edit_context);

                  for (std::size_t prop_index = 0;
                       prop_index < path->nodes[i].properties.size(); ++prop_index) {
                     ImGui::InputKeyValue(path, i, prop_index,
                                          &_edit_stack_world, &_edit_context);
                  }

                  if (ImGui::BeginCombo("Add Property", "<select property>")) {
                     for (const char* prop :
                          world::get_path_node_properties(path->type)) {
                        if (ImGui::Selectable(prop)) {
                           _edit_stack_world.apply(edits::make_add_property(path->id,
                                                                            i, prop),
                                                   _edit_context);
                        }
                     }

                     ImGui::EndCombo();
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
                                &_edit_stack_world, &_edit_context);
               ImGui::LayerPick("Layer", region, &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", region, &world::region::rotation,
                               &_edit_stack_world, &_edit_context);
               ImGui::DragFloat3("Position", region, &world::region::position,
                                 &_edit_stack_world, &_edit_context);
               ImGui::DragFloat3("Size", region, &world::region::size, &_edit_stack_world,
                                 &_edit_context, 1.0f, 0.0f, 1e10f);

               ImGui::EnumSelect("Shape", region, &world::region::shape,
                                 &_edit_stack_world, &_edit_context,
                                 {enum_select_option{"Box", world::region_shape::box},
                                  enum_select_option{"Sphere", world::region_shape::sphere},
                                  enum_select_option{"Cylinder",
                                                     world::region_shape::cylinder}});

               ImGui::Separator();

               ImGui::InputText("Description", region, &world::region::description,
                                &_edit_stack_world, &_edit_context);
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
                                &_edit_stack_world, &_edit_context);
               ImGui::LayerPick("Layer", hintnode, &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               ImGui::DragQuat("Rotation", hintnode, &world::hintnode::rotation,
                               &_edit_stack_world, &_edit_context);
               ImGui::DragFloat3("Position", hintnode, &world::hintnode::position,
                                 &_edit_stack_world, &_edit_context);
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

      if (not selection_open) _interaction_targets.selection.clear();
   }

   if (_interaction_targets.creation_entity) {
      if (std::exchange(_entity_creation_context.activate_point_at, false)) {
         _entity_creation_context.placement_rotation =
            placement_rotation::manual_quaternion;
         _entity_creation_context.placement_mode = placement_mode::manual;

         _edit_stack_world.close_last(); // Make sure we don't coalesce with a previous point at.
         _entity_creation_context.using_point_at = true;
      }

      if (std::exchange(_entity_creation_context.activate_extend_to, false)) {
         _edit_stack_world.close_last();

         _entity_creation_context.using_extend_to = true;
         _entity_creation_context.using_shrink_to = false;
      }

      if (std::exchange(_entity_creation_context.activate_shrink_to, false)) {
         _edit_stack_world.close_last();

         _entity_creation_context.using_extend_to = false;
         _entity_creation_context.using_shrink_to = true;
      }

      if (std::exchange(_entity_creation_context.activate_from_object_bbox, false)) {
         _edit_stack_world.close_last();

         _entity_creation_context.using_from_object_bbox = true;
      }

      if (std::exchange(_entity_creation_context.activate_from_line, false)) {
         _edit_stack_world.close_last();

         _entity_creation_context.using_from_line = true;
      }

      bool continue_creation = true;

      ImGui::SetNextWindowPos({232.0f * _display_scale, 32.0f * _display_scale},
                              ImGuiCond_Once, {0.0f, 0.0f});

      ImGui::Begin("Create", &continue_creation,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                      ImGuiWindowFlags_AlwaysAutoResize);

      world::creation_entity& creation_entity = *_interaction_targets.creation_entity;

      const placement_traits traits = std::visit(
         overload{
            [&](const world::object& object) {
               ImGui::Text("Object");
               ImGui::Separator();

               ImGui::InputText("Name", &creation_entity, &world::object::name,
                                &_edit_stack_world, &_edit_context,
                                [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.objects,
                                                                *edited_value);
                                });

               ImGui::InputTextAutoComplete(
                  "Class Name", &creation_entity, &world::object::class_name,
                  &_edit_stack_world, &_edit_context, [&] {
                     std::array<std::string, 6> entries;
                     std::size_t matching_count = 0;

                     _asset_libraries.odfs.enumerate_known(
                        [&](const lowercase_string& asset) {
                           if (matching_count == entries.size()) return;
                           if (not asset.contains(object.class_name)) return;

                           entries[matching_count] = asset;

                           ++matching_count;
                        });

                     return entries;
                  });

               if (ImGui::IsItemDeactivatedAfterEdit()) {
                  std::vector<world::instance_property> new_instance_properties =
                     world::make_object_instance_properties(
                        *_object_classes[object.class_name].definition,
                        object.instance_properties);

                  if (new_instance_properties != object.instance_properties) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::object::instance_properties,
                                                std::move(new_instance_properties),
                                                object.instance_properties),
                                             _edit_context, {.transparent = true});
                  }
               }

               ImGui::LayerPick<world::object>("Layer", &creation_entity,
                                               &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               if (_entity_creation_context.placement_rotation !=
                   placement_rotation::manual_quaternion) {
                  ImGui::DragRotationEuler("Rotation", &creation_entity,
                                           &world::object::rotation,
                                           &world::edit_context::euler_rotation,
                                           &_edit_stack_world, &_edit_context);
               }
               else {
                  ImGui::DragQuat("Rotation", &creation_entity, &world::object::rotation,
                                  &_edit_stack_world, &_edit_context);
               }

               if (ImGui::DragFloat3("Position", &creation_entity, &world::object::position,
                                     &_edit_stack_world, &_edit_context)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;
               }

               if ((_entity_creation_context.placement_rotation ==
                       placement_rotation::surface or
                    _entity_creation_context.placement_mode == placement_mode::cursor) and
                   not _entity_creation_context.using_point_at) {
                  quaternion new_rotation = object.rotation;
                  float3 new_position = object.position;
                  float3 new_euler_rotation = _edit_context.euler_rotation;

                  if (_entity_creation_context.placement_rotation ==
                         placement_rotation::surface and
                      _cursor_surface_normalWS) {
                     const float new_y_angle =
                        surface_rotation_degrees(*_cursor_surface_normalWS,
                                                 _edit_context.euler_rotation.y);
                     new_euler_rotation = {_edit_context.euler_rotation.x, new_y_angle,
                                           _edit_context.euler_rotation.z};
                     new_rotation = make_quat_from_euler(
                        new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
                  }

                  if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                     new_position = _cursor_positionWS;

                     if (_entity_creation_context.placement_ground ==
                         placement_ground::bbox) {

                        const math::bounding_box bbox =
                           object.rotation *
                           _object_classes[object.class_name].model->bounding_box;

                        new_position.y -= bbox.min.y;
                     }

                     if (_entity_creation_context.placement_alignment ==
                         placement_alignment::grid) {
                        new_position =
                           align_position_to_grid(new_position,
                                                  _entity_creation_context.alignment);
                     }
                     else if (_entity_creation_context.placement_alignment ==
                              placement_alignment::snapping) {
                        const std::optional<float3> snapped_position =
                           world::get_snapped_position(object, new_position,
                                                       _world.objects,
                                                       _entity_creation_context.snap_distance,
                                                       _object_classes);

                        if (snapped_position) new_position = *snapped_position;
                     }

                     if (_entity_creation_context.lock_x_axis) {
                        new_position.x = object.position.x;
                     }
                     if (_entity_creation_context.lock_y_axis) {
                        new_position.y = object.position.y;
                     }
                     if (_entity_creation_context.lock_z_axis) {
                        new_position.z = object.position.z;
                     }
                  }

                  if (new_rotation != object.rotation or new_position != object.position) {
                     _edit_stack_world.apply(edits::make_set_creation_location<world::object>(
                                                new_rotation, object.rotation, new_position,
                                                object.position, new_euler_rotation,
                                                _edit_context.euler_rotation),
                                             _edit_context);
                  }
               }

               if (_entity_creation_context.using_point_at) {
                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       object.position, 0xffffffffu);

                  const quaternion new_rotation =
                     look_at_quat(_cursor_positionWS, object.position);

                  if (new_rotation != object.rotation) {
                     _edit_stack_world
                        .apply(edits::make_set_creation_value(&world::object::rotation,
                                                              new_rotation,
                                                              object.rotation),
                               _edit_context);
                  }
               }

               ImGui::Separator();

               ImGui::SliderInt("Team", &creation_entity, &world::object::team,
                                &_edit_stack_world, &_edit_context, 0, 15, "%d",
                                ImGuiSliderFlags_AlwaysClamp);

               return placement_traits{};
            },
            [&](const world::light& light) {
               ImGui::InputText("Name", &creation_entity, &world::light::name,
                                &_edit_stack_world, &_edit_context,
                                [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.lights,
                                                                *edited_value);
                                });

               ImGui::LayerPick<world::light>("Layer", &creation_entity,
                                              &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               if (_entity_creation_context.placement_rotation !=
                   placement_rotation::manual_quaternion) {
                  ImGui::DragRotationEuler("Rotation", &creation_entity,
                                           &world::light::rotation,
                                           &world::edit_context::euler_rotation,
                                           &_edit_stack_world, &_edit_context);
               }
               else {
                  ImGui::DragQuat("Rotation", &creation_entity, &world::light::rotation,
                                  &_edit_stack_world, &_edit_context);
               }

               if (ImGui::DragFloat3("Position", &creation_entity, &world::light::position,
                                     &_edit_stack_world, &_edit_context)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;
               }

               if ((_entity_creation_context.placement_rotation ==
                       placement_rotation::surface or
                    _entity_creation_context.placement_mode == placement_mode::cursor) and
                   not _entity_creation_context.using_point_at) {
                  quaternion new_rotation = light.rotation;
                  float3 new_position = light.position;
                  float3 new_euler_rotation = _edit_context.euler_rotation;

                  if (_entity_creation_context.placement_rotation ==
                         placement_rotation::surface and
                      _cursor_surface_normalWS) {
                     const float new_y_angle =
                        surface_rotation_degrees(*_cursor_surface_normalWS,
                                                 _edit_context.euler_rotation.y);
                     new_euler_rotation = {_edit_context.euler_rotation.x, new_y_angle,
                                           _edit_context.euler_rotation.z};
                     new_rotation = make_quat_from_euler(
                        new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
                  }

                  if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                     new_position = _cursor_positionWS;
                     if (_entity_creation_context.placement_alignment ==
                         placement_alignment::grid) {
                        new_position =
                           align_position_to_grid(new_position,
                                                  _entity_creation_context.alignment);
                     }
                     else if (_entity_creation_context.placement_alignment ==
                              placement_alignment::snapping) {
                        const std::optional<float3> snapped_position =
                           world::get_snapped_position(new_position, _world.objects,
                                                       _entity_creation_context.snap_distance,
                                                       _object_classes);

                        if (snapped_position) new_position = *snapped_position;
                     }

                     if (_entity_creation_context.lock_x_axis) {
                        new_position.x = light.position.x;
                     }
                     if (_entity_creation_context.lock_y_axis) {
                        new_position.y = light.position.y;
                     }
                     if (_entity_creation_context.lock_z_axis) {
                        new_position.z = light.position.z;
                     }
                  }

                  if (new_rotation != light.rotation or new_position != light.position) {
                     _edit_stack_world.apply(edits::make_set_creation_location<world::light>(
                                                new_rotation, light.rotation, new_position,
                                                light.position, new_euler_rotation,
                                                _edit_context.euler_rotation),
                                             _edit_context);
                  }
               }

               if (_entity_creation_context.using_point_at) {
                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       light.position, 0xffffffffu);

                  const quaternion new_rotation =
                     look_at_quat(_cursor_positionWS, light.position);

                  if (new_rotation != light.rotation) {
                     _edit_stack_world
                        .apply(edits::make_set_creation_value(&world::light::rotation,
                                                              new_rotation,
                                                              light.rotation),
                               _edit_context);
                  }
               }

               ImGui::Separator();

               ImGui::ColorEdit3("Color", &creation_entity, &world::light::color,
                                 &_edit_stack_world, &_edit_context,
                                 ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

               ImGui::Checkbox("Static", &creation_entity, &world::light::static_,
                               &_edit_stack_world, &_edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("Shadow Caster", &creation_entity,
                               &world::light::shadow_caster, &_edit_stack_world,
                               &_edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("Specular Caster", &creation_entity,
                               &world::light::specular_caster,
                               &_edit_stack_world, &_edit_context);

               ImGui::EnumSelect(
                  "Light Type", &creation_entity, &world::light::light_type,
                  &_edit_stack_world, &_edit_context,
                  {enum_select_option{"Directional", world::light_type::directional},
                   enum_select_option{"Point", world::light_type::point},
                   enum_select_option{"Spot", world::light_type::spot},
                   enum_select_option{"Directional Region Box",
                                      world::light_type::directional_region_box},
                   enum_select_option{"Directional Region Sphere",
                                      world::light_type::directional_region_sphere},
                   enum_select_option{"Directional Region Cylinder",
                                      world::light_type::directional_region_cylinder}});

               ImGui::Separator();

               if (light.light_type == world::light_type::point or
                   light.light_type == world::light_type::spot) {
                  ImGui::DragFloat("Range", &creation_entity, &world::light::range,
                                   &_edit_stack_world, &_edit_context);

                  if (light.light_type == world::light_type::spot) {
                     ImGui::DragFloat("Inner Cone Angle", &creation_entity,
                                      &world::light::inner_cone_angle,
                                      &_edit_stack_world, &_edit_context, 0.01f,
                                      0.0f, light.outer_cone_angle, "%.3f",
                                      ImGuiSliderFlags_AlwaysClamp);
                     ImGui::DragFloat("Outer Cone Angle", &creation_entity,
                                      &world::light::outer_cone_angle,
                                      &_edit_stack_world, &_edit_context, 0.01f,
                                      light.inner_cone_angle, 1.570f, "%.3f",
                                      ImGuiSliderFlags_AlwaysClamp);
                  }

                  ImGui::Separator();
               }

               ImGui::InputTextAutoComplete(
                  "Texture", &creation_entity, &world::light::texture,
                  &_edit_stack_world, &_edit_context, [&] {
                     std::array<std::string, 6> entries;
                     std::size_t matching_count = 0;

                     _asset_libraries.textures.enumerate_known(
                        [&](const lowercase_string& asset) {
                           if (matching_count == entries.size()) return;
                           if (not asset.contains(light.texture)) return;

                           entries[matching_count] = asset;

                           ++matching_count;
                        });

                     return entries;
                  });

               if (world::is_directional_light(light) and not light.texture.empty()) {
                  ImGui::DragFloat2("Directional Texture Tiling", &creation_entity,
                                    &world::light::directional_texture_tiling,
                                    &_edit_stack_world, &_edit_context, 0.01f);
                  ImGui::DragFloat2("Directional Texture Offset", &creation_entity,
                                    &world::light::directional_texture_offset,
                                    &_edit_stack_world, &_edit_context, 0.01f);
               }

               if (is_region_light(light)) {
                  ImGui::Separator();

                  ImGui::InputText("Region Name", &creation_entity,
                                   &world::light::region_name, &_edit_stack_world,
                                   &_edit_context, [&](std::string* edited_value) {
                                      *edited_value =
                                         world::create_unique_light_region_name(
                                            _world.lights, _world.regions,
                                            light.region_name.empty()
                                               ? light.name
                                               : light.region_name);
                                   });

                  if (_entity_creation_context.placement_rotation !=
                      placement_rotation::manual_quaternion) {
                     ImGui::DragRotationEuler("Rotation", &creation_entity,
                                              &world::light::region_rotation,
                                              &world::edit_context::light_region_euler_rotation,
                                              &_edit_stack_world, &_edit_context);
                  }
                  else {
                     ImGui::DragQuat("Region Rotation", &creation_entity,
                                     &world::light::region_rotation,
                                     &_edit_stack_world, &_edit_context);
                  }

                  ImGui::DragFloat3("Region Size", &creation_entity,
                                    &world::light::region_size,
                                    &_edit_stack_world, &_edit_context);
               }

               return placement_traits{.has_placement_ground = false};
            },
            [&](const world::path& path) {
               const world::path* existing_path =
                  world::find_entity(_world.paths, path.name);

               if (existing_path) {
                  ImGui::LabelText("Name", existing_path->name.c_str());
                  ImGui::LayerPick("Layer", existing_path, &_edit_stack_world,
                                   &_edit_context);

                  ImGui::EnumSelect(
                     "Path Type", existing_path, &world::path::type,
                     &_edit_stack_world, &_edit_context,
                     {enum_select_option{"None", world::path_type::none},
                      enum_select_option{"Entity Follow", world::path_type::entity_follow},
                      enum_select_option{"Formation", world::path_type::formation},
                      enum_select_option{"Patrol", world::path_type::patrol}});

                  ImGui::EnumSelect(
                     "Spline Type", existing_path, &world::path::spline_type,
                     &_edit_stack_world, &_edit_context,
                     {enum_select_option{"None", world::path_spline_type::none},
                      enum_select_option{"Linear", world::path_spline_type::linear},
                      enum_select_option{"Hermite", world::path_spline_type::hermite},
                      enum_select_option{"Catmull-Rom",
                                         world::path_spline_type::catmull_rom}});

                  ImGui::Separator();

                  for (std::size_t i = 0; i < existing_path->properties.size(); ++i) {
                     ImGui::InputKeyValue(existing_path, &world::path::properties,
                                          i, &_edit_stack_world, &_edit_context);
                  }

                  if (ImGui::BeginCombo("Add Property", "<select property>")) {
                     for (const char* prop :
                          world::get_path_properties(existing_path->type)) {
                        if (ImGui::Selectable(prop)) {
                           _edit_stack_world
                              .apply(edits::make_add_property(existing_path->id, prop),
                                     _edit_context);
                        }
                     }

                     ImGui::EndCombo();
                  }
               }
               else {
                  ImGui::InputText("Name", &creation_entity, &world::path::name,
                                   &_edit_stack_world, &_edit_context,
                                   [&](std::string* edited_value) {
                                      *edited_value =
                                         world::create_unique_name(_world.paths,
                                                                   path.name.empty()
                                                                      ? "Path 0"sv
                                                                      : path.name);
                                   });

                  ImGui::LayerPick<world::path>("Layer", &creation_entity,
                                                &_edit_stack_world, &_edit_context);

                  ImGui::EnumSelect(
                     "Path Type", &creation_entity, &world::path::type,
                     &_edit_stack_world, &_edit_context,
                     {enum_select_option{"None", world::path_type::none},
                      enum_select_option{"Entity Follow", world::path_type::entity_follow},
                      enum_select_option{"Formation", world::path_type::formation},
                      enum_select_option{"Patrol", world::path_type::patrol}});

                  ImGui::EnumSelect(
                     "Spline Type", &creation_entity, &world::path::spline_type,
                     &_edit_stack_world, &_edit_context,
                     {enum_select_option{"None", world::path_spline_type::none},
                      enum_select_option{"Linear", world::path_spline_type::linear},
                      enum_select_option{"Hermite", world::path_spline_type::hermite},
                      enum_select_option{"Catmull-Rom",
                                         world::path_spline_type::catmull_rom}});
               }

               ImGui::Separator();

               if (path.nodes.size() != 1) std::terminate();

               ImGui::Text("Next Node");

               if (_entity_creation_context.placement_rotation !=
                   placement_rotation::manual_quaternion) {
                  ImGui::DragRotationEulerPathNode("Rotation", &creation_entity,
                                                   &world::edit_context::euler_rotation,
                                                   &_edit_stack_world, &_edit_context);
               }
               else {
                  ImGui::DragQuatPathNode("Rotation", &creation_entity,
                                          &_edit_stack_world, &_edit_context);
               }

               if (ImGui::DragFloat3PathNode("Position", &creation_entity,
                                             &_edit_stack_world, &_edit_context)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;
               }
               if ((_entity_creation_context.placement_rotation ==
                       placement_rotation::surface or
                    _entity_creation_context.placement_mode == placement_mode::cursor) and
                   not _entity_creation_context.using_point_at) {
                  quaternion new_rotation = path.nodes[0].rotation;
                  float3 new_position = path.nodes[0].position;
                  float3 new_euler_rotation = _edit_context.euler_rotation;

                  if (_entity_creation_context.placement_rotation ==
                         placement_rotation::surface and
                      _cursor_surface_normalWS) {
                     const float new_y_angle =
                        surface_rotation_degrees(*_cursor_surface_normalWS,
                                                 _edit_context.euler_rotation.y);
                     new_euler_rotation = {_edit_context.euler_rotation.x, new_y_angle,
                                           _edit_context.euler_rotation.z};
                     new_rotation = make_quat_from_euler(
                        new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
                  }

                  if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                     new_position = _cursor_positionWS;

                     if (_entity_creation_context.placement_alignment ==
                         placement_alignment::grid) {
                        new_position =
                           align_position_to_grid(new_position,
                                                  _entity_creation_context.alignment);
                     }
                     else if (_entity_creation_context.placement_alignment ==
                              placement_alignment::snapping) {
                        const std::optional<float3> snapped_position =
                           world::get_snapped_position(new_position, _world.objects,
                                                       _entity_creation_context.snap_distance,
                                                       _object_classes);

                        if (snapped_position) new_position = *snapped_position;
                     }

                     if (_entity_creation_context.lock_x_axis) {
                        new_position.x = path.nodes[0].position.x;
                     }
                     if (_entity_creation_context.lock_y_axis) {
                        new_position.y = path.nodes[0].position.y;
                     }
                     if (_entity_creation_context.lock_z_axis) {
                        new_position.z = path.nodes[0].position.z;
                     }
                  }

                  if (new_rotation != path.nodes[0].rotation or
                      new_position != path.nodes[0].position) {
                     _edit_stack_world.apply(edits::make_set_creation_path_node_location(
                                                new_rotation, path.nodes[0].rotation,
                                                new_position, path.nodes[0].position,
                                                new_euler_rotation,
                                                _edit_context.euler_rotation),
                                             _edit_context);
                  }
               }

               if (_entity_creation_context.using_point_at) {
                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       path.nodes[0].position,
                                                       0xffffffffu);

                  const quaternion new_rotation =
                     look_at_quat(_cursor_positionWS, path.nodes[0].position);

                  if (new_rotation != path.nodes[0].rotation) {
                     _edit_stack_world.apply(edits::make_set_creation_path_node_value(
                                                &world::path::node::rotation,
                                                new_rotation, path.nodes[0].rotation),
                                             _edit_context);
                  }
               }

               ImGui::Separator();

               if (ImGui::Button("New Path", {ImGui::CalcItemWidth(), 0.0f}) or
                   std::exchange(_entity_creation_context.finish_current_path, false)) {

                  _edit_stack_world.apply(edits::make_set_creation_value(
                                             &world::path::name,
                                             world::create_unique_name(_world.paths,
                                                                       path.name),
                                             path.name),
                                          _edit_context);
               }

               if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Create another new path and stop adding "
                                    "nodes to the current one.");
               }

               return placement_traits{.has_new_path = true,
                                       .has_node_placement_insert = true};
            },
            [&](const world::region& region) {
               ImGui::InputText("Name", &creation_entity, &world::region::name,
                                &_edit_stack_world, &_edit_context,
                                [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.regions,
                                                                region.name);
                                });

               ImGui::LayerPick<world::region>("Layer", &creation_entity,
                                               &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               const world::region_type start_region_type =
                  world::get_region_type(region.description);
               world::region_type region_type = start_region_type;

               if (ImGui::EnumSelect(
                      "Type", &region_type,
                      {
                         enum_select_option{"Typeless", world::region_type::typeless},
                         enum_select_option{"Death Region", world::region_type::deathregion},
                         enum_select_option{"Sound Stream", world::region_type::soundstream},
                         enum_select_option{"Sound Static", world::region_type::soundstatic},
                         enum_select_option{"Sound Space", world::region_type::soundspace},
                         enum_select_option{"Sound Trigger", world::region_type::soundtrigger},
                         enum_select_option{"Foley FX", world::region_type::foleyfx},
                         enum_select_option{"Shadow", world::region_type::shadow},
                         enum_select_option{"Map Bounds", world::region_type::mapbounds},
                         enum_select_option{"Rumble", world::region_type::rumble},
                         enum_select_option{"Reflection", world::region_type::reflection},
                         enum_select_option{"Rain Shadow", world::region_type::rainshadow},
                         enum_select_option{"Danger", world::region_type::danger},
                         enum_select_option{"Damage Region", world::region_type::damage_region},
                         enum_select_option{"AI Vis", world::region_type::ai_vis},
                         enum_select_option{"Color Grading (Shader Patch)",
                                            world::region_type::colorgrading},
                      })) {
                  if (region_type != start_region_type) {
                     _edit_stack_world
                        .apply(edits::make_set_creation_value(&world::region::description,
                                                              to_string(region_type),
                                                              region.description),
                               _edit_context);

                     const world::region_allowed_shapes allowed_shapes =
                        world::get_region_allowed_shapes(region_type);

                     if (allowed_shapes == world::region_allowed_shapes::sphere and
                         region.shape != world::region_shape::sphere) {
                        _edit_stack_world.apply(
                           edits::make_set_creation_value(&world::region::shape,
                                                          world::region_shape::sphere,
                                                          region.shape),
                           _edit_context, {.closed = true, .transparent = true});
                     }
                     else if (allowed_shapes == world::region_allowed_shapes::box_cylinder and
                              region.shape == world::region_shape::sphere) {
                        _edit_stack_world.apply(
                           edits::make_set_creation_value(&world::region::shape,
                                                          world::region_shape::box,
                                                          region.shape),
                           _edit_context, {.closed = true, .transparent = true});
                     }
                  }
               }

               switch (region_type) {
               case world::region_type::soundstream: {
                  world::sound_stream_properties properties =
                     world::unpack_region_sound_stream(region.description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  value_changed |=
                     ImGui::InputText("Stream Name", &properties.sound_name);

                  value_changed |= ImGui::DragFloat("Min Distance Divisor",
                                                    &properties.min_distance_divisor,
                                                    1.0f, 1.0f, 1000000.0f, "%.3f",
                                                    ImGuiSliderFlags_AlwaysClamp);

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_sound_stream(properties),
                                                region.description),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::soundstatic: {
                  world::sound_stream_properties properties =
                     world::unpack_region_sound_static(region.description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  value_changed |=
                     ImGui::InputText("Sound Name", &properties.sound_name);

                  value_changed |= ImGui::DragFloat("Min Distance Divisor",
                                                    &properties.min_distance_divisor,
                                                    1.0f, 1.0f, 1000000.0f, "%.3f",
                                                    ImGuiSliderFlags_AlwaysClamp);

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_sound_static(properties),
                                                region.description),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::soundspace: {
                  world::sound_space_properties properties =
                     world::unpack_region_sound_space(region.description);

                  if (ImGui::InputText("Sound Space Name", &properties.sound_space_name)) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_sound_space(properties),
                                                region.description),
                                             _edit_context);
                  }

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::soundtrigger: {
                  world::sound_trigger_properties properties =
                     world::unpack_region_sound_trigger(region.description);

                  if (ImGui::InputText("Region Name", &properties.region_name)) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_sound_trigger(properties),
                                                region.description),
                                             _edit_context);
                  }

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::foleyfx: {
                  world::foley_fx_region_properties properties =
                     world::unpack_region_foley_fx(region.description);

                  if (ImGui::InputText("Group ID", &properties.group_id)) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_foley_fx(properties),
                                                region.description),
                                             _edit_context);
                  }

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::shadow: {
                  world::shadow_region_properties properties =
                     world::unpack_region_shadow(region.description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  if (float directional0 = properties.directional0.value_or(1.0f);
                      ImGui::DragFloat("Directional Light 0 Strength",
                                       &directional0, 0.01f, 0.0f, 1.0f, "%.3f",
                                       ImGuiSliderFlags_AlwaysClamp)) {
                     properties.directional0 = directional0;
                     value_changed = true;
                  }

                  if (float directional1 = properties.directional1.value_or(1.0f);
                      ImGui::DragFloat("Directional Light 1 Strength",
                                       &directional1, 0.01f, 0.0f, 1.0f, "%.3f",
                                       ImGuiSliderFlags_AlwaysClamp)) {
                     properties.directional1 = directional1;
                     value_changed = true;
                  }

                  if (float3 color_top =
                         properties.color_top.value_or(float3{0.0f, 0.0f, 0.0f});
                      ImGui::ColorEdit3("Ambient Light Top", &color_top.x)) {
                     properties.color_top = color_top;
                     value_changed = true;
                  }

                  if (float3 color_bottom =
                         properties.color_bottom.value_or(float3{0.0f, 0.0f, 0.0f});
                      ImGui::ColorEdit3("Ambient Light Bottom", &color_bottom.x)) {
                     properties.color_bottom = color_bottom;
                     value_changed = true;
                  }

                  value_changed |=
                     ImGui::InputText("Environment Map", &properties.env_map);

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_shadow(properties),
                                                region.description),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::rumble: {
                  world::rumble_region_properties properties =
                     world::unpack_region_rumble(region.description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  value_changed |=
                     ImGui::InputText("Rumble Class", &properties.rumble_class);
                  value_changed |= ImGui::InputText("Particle Effect",
                                                    &properties.particle_effect);

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_rumble(properties),
                                                region.description),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::damage_region: {
                  world::damage_region_properties properties =
                     world::unpack_region_damage(region.description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  if (float damage_rate = properties.damage_rate.value_or(0.0f);
                      ImGui::DragFloat("Damage Rate", &damage_rate)) {
                     properties.damage_rate = damage_rate;
                     value_changed = true;
                  }

                  if (float person_scale = properties.person_scale.value_or(1.0f);
                      ImGui::DragFloat("Person Scale", &person_scale, 0.01f)) {
                     properties.person_scale = person_scale;
                     value_changed = true;
                  }

                  if (float animal_scale = properties.animal_scale.value_or(1.0f);
                      ImGui::DragFloat("Animal Scale", &animal_scale, 0.01f)) {
                     properties.animal_scale = animal_scale;
                     value_changed = true;
                  }

                  if (float droid_scale = properties.droid_scale.value_or(1.0f);
                      ImGui::DragFloat("Droid Scale", &droid_scale, 0.01f)) {
                     properties.droid_scale = droid_scale;
                     value_changed = true;
                  }

                  if (float vehicle_scale = properties.vehicle_scale.value_or(1.0f);
                      ImGui::DragFloat("Vehicle Scale", &vehicle_scale, 0.01f)) {
                     properties.vehicle_scale = vehicle_scale;
                     value_changed = true;
                  }

                  if (float building_scale = properties.building_scale.value_or(1.0f);
                      ImGui::DragFloat("Building Scale", &building_scale, 0.01f)) {
                     properties.building_scale = building_scale;
                     value_changed = true;
                  }

                  if (float building_dead_scale =
                         properties.building_scale.value_or(1.0f);
                      ImGui::DragFloat("Building Dead Scale",
                                       &building_dead_scale, 0.01f)) {
                     properties.building_dead_scale = building_dead_scale;
                     value_changed = true;
                  }

                  if (float building_unbuilt_scale =
                         properties.building_scale.value_or(1.0f);
                      ImGui::DragFloat("Building Unbuilt Scale",
                                       &building_unbuilt_scale, 0.01f)) {
                     properties.building_unbuilt_scale = building_unbuilt_scale;
                     value_changed = true;
                  }

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_damage(properties),
                                                region.description),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::ai_vis: {
                  world::ai_vis_region_properties properties =
                     world::unpack_region_ai_vis(region.description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  if (float crouch = properties.crouch.value_or(1.0f);
                      ImGui::DragFloat("Crouch", &crouch, 0.01f, 0.0f, 1e10f)) {
                     properties.crouch = crouch;
                     value_changed = true;
                  }

                  if (float stand = properties.stand.value_or(1.0f);
                      ImGui::DragFloat("Stand", &stand, 0.01f, 0.0f, 1e10f)) {
                     properties.stand = stand;
                     value_changed = true;
                  }

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_ai_vis(properties),
                                                region.description),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               case world::region_type::colorgrading: {
                  world::colorgrading_region_properties properties =
                     world::unpack_region_colorgrading(region.description);

                  ImGui::BeginGroup();

                  bool value_changed = false;

                  value_changed |= ImGui::InputText("Config", &properties.config);
                  value_changed |=
                     ImGui::DragFloat("Fade Length", &properties.fade_length);

                  if (value_changed) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::region::description,
                                                world::pack_region_colorgrading(properties),
                                                region.description),
                                             _edit_context);
                  }

                  ImGui::EndGroup();

                  if (ImGui::IsItemDeactivatedAfterEdit()) {
                     _edit_stack_world.close_last();
                  }
               } break;
               }

               ImGui::InputText("Description", &creation_entity,
                                &world::region::description, &_edit_stack_world,
                                &_edit_context,
                                [&]([[maybe_unused]] std::string* edited_value) {});

               ImGui::Separator();

               if (_entity_creation_context.placement_rotation !=
                   placement_rotation::manual_quaternion) {
                  ImGui::DragRotationEuler("Rotation", &creation_entity,
                                           &world::region::rotation,
                                           &world::edit_context::euler_rotation,
                                           &_edit_stack_world, &_edit_context);
               }
               else {
                  ImGui::DragQuat("Rotation", &creation_entity, &world::region::rotation,
                                  &_edit_stack_world, &_edit_context);
               }

               if (ImGui::DragFloat3("Position", &creation_entity, &world::region::position,
                                     &_edit_stack_world, &_edit_context)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;
               }

               if ((_entity_creation_context.placement_rotation ==
                       placement_rotation::surface or
                    _entity_creation_context.placement_mode == placement_mode::cursor) and
                   not _entity_creation_context.using_point_at) {
                  quaternion new_rotation = region.rotation;
                  float3 new_position = region.position;
                  float3 new_euler_rotation = _edit_context.euler_rotation;

                  if (_entity_creation_context.placement_rotation ==
                         placement_rotation::surface and
                      _cursor_surface_normalWS) {
                     const float new_y_angle =
                        surface_rotation_degrees(*_cursor_surface_normalWS,
                                                 _edit_context.euler_rotation.y);
                     new_euler_rotation = {_edit_context.euler_rotation.x, new_y_angle,
                                           _edit_context.euler_rotation.z};
                     new_rotation = make_quat_from_euler(
                        new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
                  }

                  if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                     new_position = _cursor_positionWS;

                     if (_entity_creation_context.placement_ground ==
                         placement_ground::bbox) {
                        switch (region.shape) {
                        case world::region_shape::box: {
                           const std::array<float3, 8> bbox_corners = math::to_corners(
                              {.min = -region.size, .max = region.size});

                           float min_y = std::numeric_limits<float>::max();
                           float max_y = std::numeric_limits<float>::lowest();

                           for (const float3& v : bbox_corners) {
                              const float3 rotated_corner = region.rotation * v;

                              min_y = std::min(rotated_corner.y, min_y);
                              max_y = std::max(rotated_corner.y, max_y);
                           }

                           new_position.y += (std::abs(max_y - min_y) / 2.0f);
                        } break;
                        case world::region_shape::sphere: {
                           new_position.y += length(region.size);
                        } break;
                        case world::region_shape::cylinder: {
                           const float cylinder_radius =
                              length(float2{region.size.x, region.size.z});
                           const std::array<float3, 8> bbox_corners = math::to_corners(
                              {.min = {-cylinder_radius, -region.size.y, -cylinder_radius},
                               .max = {cylinder_radius, region.size.y, cylinder_radius}});

                           float min_y = std::numeric_limits<float>::max();
                           float max_y = std::numeric_limits<float>::lowest();

                           for (const float3& v : bbox_corners) {
                              const float3 rotated_corner = region.rotation * v;

                              min_y = std::min(rotated_corner.y, min_y);
                              max_y = std::max(rotated_corner.y, max_y);
                           }

                           new_position.y += (std::abs(max_y - min_y) / 2.0f);
                        } break;
                        }
                     }

                     if (_entity_creation_context.placement_alignment ==
                         placement_alignment::grid) {
                        new_position =
                           align_position_to_grid(new_position,
                                                  _entity_creation_context.alignment);
                     }
                     else if (_entity_creation_context.placement_alignment ==
                              placement_alignment::snapping) {
                        const std::optional<float3> snapped_position =
                           world::get_snapped_position(new_position, _world.objects,
                                                       _entity_creation_context.snap_distance,
                                                       _object_classes);

                        if (snapped_position) new_position = *snapped_position;
                     }

                     if (_entity_creation_context.lock_x_axis) {
                        new_position.x = region.position.x;
                     }
                     if (_entity_creation_context.lock_y_axis) {
                        new_position.y = region.position.y;
                     }
                     if (_entity_creation_context.lock_z_axis) {
                        new_position.z = region.position.z;
                     }
                  }

                  if (new_rotation != region.rotation or new_position != region.position) {
                     _edit_stack_world.apply(edits::make_set_creation_location<world::region>(
                                                new_rotation, region.rotation, new_position,
                                                region.position, new_euler_rotation,
                                                _edit_context.euler_rotation),
                                             _edit_context);
                  }
               }

               if (_entity_creation_context.using_point_at) {
                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       region.position, 0xffffffffu);

                  const quaternion new_rotation =
                     look_at_quat(_cursor_positionWS, region.position);

                  if (new_rotation != region.rotation) {
                     _edit_stack_world
                        .apply(edits::make_set_creation_value(&world::region::rotation,
                                                              new_rotation,
                                                              region.rotation),
                               _edit_context);
                  }
               }

               ImGui::Separator();

               switch (world::get_region_allowed_shapes(region_type)) {
               case world::region_allowed_shapes::all: {
                  ImGui::EnumSelect(
                     "Shape", &creation_entity, &world::region::shape,
                     &_edit_stack_world, &_edit_context,
                     {enum_select_option{"Box", world::region_shape::box},
                      enum_select_option{"Sphere", world::region_shape::sphere},
                      enum_select_option{"Cylinder", world::region_shape::cylinder}});
               } break;
               case world::region_allowed_shapes::sphere: {
                  ImGui::LabelText("Shape", "Sphere");
               } break;
               case world::region_allowed_shapes::box_cylinder: {
                  ImGui::EnumSelect("Shape", &creation_entity, &world::region::shape,
                                    &_edit_stack_world, &_edit_context,
                                    {enum_select_option{"Box", world::region_shape::box},
                                     enum_select_option{"Cylinder",
                                                        world::region_shape::cylinder}});
               } break;
               }

               float3 region_size = region.size;

               switch (region.shape) {
               case world::region_shape::box: {
                  ImGui::DragFloat3("Size", &region_size, 0.0f, 1e10f);
               } break;
               case world::region_shape::sphere: {
                  float radius = length(region_size);

                  if (ImGui::DragFloat("Radius", &radius, 0.1f)) {
                     const float radius_sq = radius * radius;
                     const float size = std::sqrt(radius_sq / 3.0f);

                     region_size = {size, size, size};
                  }
               } break;
               case world::region_shape::cylinder: {
                  float height = region_size.y * 2.0f;

                  if (ImGui::DragFloat("Height", &height, 0.1f, 0.0f, 1e10f)) {
                     region_size.y = height / 2.0f;
                  }

                  float radius = length(float2{region_size.x, region_size.z});

                  if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 1e10f)) {
                     const float radius_sq = radius * radius;
                     const float size = std::sqrt(radius_sq / 2.0f);

                     region_size.x = size;
                     region_size.z = size;
                  }
               } break;
               }

               if (ImGui::Button("Extend To", {ImGui::CalcItemWidth(), 0.0f})) {
                  _entity_creation_context.activate_extend_to = true;
               }
               if (ImGui::Button("Shrink To", {ImGui::CalcItemWidth(), 0.0f})) {
                  _entity_creation_context.activate_shrink_to = true;
               }

               if (_entity_creation_context.using_extend_to or
                   _entity_creation_context.using_shrink_to) {
                  _entity_creation_context.placement_mode = placement_mode::manual;

                  if (not _entity_creation_context.resize_start_size) {
                     _entity_creation_context.resize_start_size = region.size;
                  }

                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       region.position, 0xffffffffu);

                  const float3 region_start_size =
                     *_entity_creation_context.resize_start_size;

                  switch (region.shape) {
                  case world::region_shape::box: {
                     const quaternion inverse_region_rotation =
                        conjugate(region.rotation);

                     const float3 cursorRS = inverse_region_rotation *
                                             (_cursor_positionWS - region.position);

                     if (_entity_creation_context.using_extend_to) {
                        region_size = max(abs(cursorRS), region_start_size);
                     }
                     else {
                        region_size = min(abs(cursorRS), region_start_size);
                     }
                  } break;
                  case world::region_shape::sphere: {
                     const float start_radius = length(region_start_size);
                     const float new_radius =
                        distance(region.position, _cursor_positionWS);

                     const float radius = _entity_creation_context.using_extend_to
                                             ? std::max(start_radius, new_radius)
                                             : std::min(start_radius, new_radius);
                     const float radius_sq = radius * radius;
                     const float size = std::sqrt(radius_sq / 3.0f);

                     region_size = {size, size, size};
                  } break;
                  case world::region_shape::cylinder: {
                     const float start_radius =
                        length(float2{region_start_size.x, region_start_size.z});
                     const float start_height = region_start_size.y;

                     const quaternion inverse_region_rotation =
                        conjugate(region.rotation);

                     const float3 cursorRS = inverse_region_rotation *
                                             (_cursor_positionWS - region.position);

                     const float new_radius = length(float2{cursorRS.x, cursorRS.z});
                     const float new_height = std::abs(cursorRS.y);

                     const float radius = std::max(start_radius, new_radius);
                     const float radius_sq = radius * radius;
                     const float size = std::sqrt(radius_sq / 2.0f);

                     region_size = {size, std::max(start_height, new_height), size};
                  }
                  }
               }
               else {
                  _entity_creation_context.resize_start_size = std::nullopt;
               }

               if (region_size != region.size) {
                  _edit_stack_world.apply(edits::make_set_creation_value(&world::region::size,
                                                                         region_size,
                                                                         region.size),
                                          _edit_context);
               }

               ImGui::Separator();
               if (ImGui::Button("From Object Bounds", {ImGui::CalcItemWidth(), 0.0f})) {
                  _entity_creation_context.activate_from_object_bbox = true;
               }

               if (_entity_creation_context.using_from_object_bbox and
                   _interaction_targets.hovered_entity and
                   std::holds_alternative<world::object_id>(
                      *_interaction_targets.hovered_entity)) {
                  _entity_creation_context.placement_rotation =
                     placement_rotation::manual_quaternion;
                  _entity_creation_context.placement_mode = placement_mode::manual;

                  const world::object* object =
                     world::find_entity(_world.objects,
                                        std::get<world::object_id>(
                                           *_interaction_targets.hovered_entity));

                  if (object) {
                     math::bounding_box bbox =
                        _object_classes[object->class_name].model->bounding_box;

                     const float3 size = abs(bbox.max - bbox.min) / 2.0f;
                     const float3 position =
                        object->rotation *
                        ((conjugate(object->rotation) * object->position) +
                         ((bbox.min + bbox.max) / 2.0f));

                     _edit_stack_world.apply(edits::make_set_creation_region_metrics(
                                                object->rotation, region.rotation,
                                                position, region.position, size,
                                                region.size),
                                             _edit_context);
                  }
               }

               return placement_traits{.has_resize_to = true, .has_from_bbox = true};
            },
            [&](const world::sector& sector) {
               ImGui::InputText("Name", &creation_entity, &world::sector::name,
                                &_edit_stack_world, &_edit_context,
                                [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.sectors,
                                                                sector.name);
                                });

               ImGui::DragFloat("Base", &creation_entity, &world::sector::base,
                                &_edit_stack_world, &_edit_context, 1.0f, 0.0f,
                                0.0f, "Y:%.3f");
               ImGui::DragFloat("Height", &creation_entity, &world::sector::height,
                                &_edit_stack_world, &_edit_context);

               if (sector.points.empty()) std::terminate();

               ImGui::DragSectorPoint("Position", &creation_entity,
                                      &_edit_stack_world, &_edit_context);

               if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                  float2 new_position = sector.points[0];

                  new_position = {_cursor_positionWS.x, _cursor_positionWS.z};

                  if (_entity_creation_context.placement_alignment ==
                      placement_alignment::grid) {
                     new_position =
                        align_position_to_grid(new_position,
                                               _entity_creation_context.alignment);
                  }
                  else if (_entity_creation_context.placement_alignment ==
                           placement_alignment::snapping) {
                     // What should snapping for sectors do?
                     ImGui::Text("Snapping is currently unimplemented for "
                                 "sectors. Sorry!");
                  }

                  if (_entity_creation_context.lock_x_axis) {
                     new_position.x = sector.points[0].x;
                  }
                  if (_entity_creation_context.lock_z_axis) {
                     new_position.y = sector.points[0].y;
                  }

                  if (new_position != sector.points[0]) {
                     _edit_stack_world.apply(edits::make_set_creation_sector_point(
                                                new_position, sector.points[0]),
                                             _edit_context);
                  }
               }

               if (ImGui::Button("New Sector", {ImGui::CalcItemWidth(), 0.0f}) or
                   std::exchange(_entity_creation_context.finish_current_sector, false)) {

                  _edit_stack_world.apply(edits::make_set_creation_value(
                                             &world::sector::name,
                                             world::create_unique_name(_world.sectors,
                                                                       sector.name),
                                             sector.name),
                                          _edit_context);
               }

               if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Create another new sector and stop adding "
                                    "points to the current one.");
               }

               ImGui::Checkbox("Auto-Fill Object List",
                               &_entity_creation_context.auto_fill_sector);

               if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip(
                     "Auto-Fill the sector's object list with objects "
                     "inside "
                     "the sector from active layers as points are added. "
                     "This "
                     "will add a separate entry to the Undo stack.");
               }

               if (const world::sector* existing_sector =
                      world::find_entity(_world.sectors, sector.name);
                   existing_sector and not existing_sector->points.empty()) {
                  const float2 start_point = existing_sector->points.back();
                  const float2 mid_point = sector.points[0];
                  const float2 end_point = existing_sector->points[0];

                  const float3 line_bottom_start = {start_point.x,
                                                    existing_sector->base,
                                                    start_point.y};
                  const float3 line_bottom_mid = {mid_point.x, sector.base,
                                                  mid_point.y};
                  const float3 line_bottom_end = {end_point.x, existing_sector->base,
                                                  end_point.y};

                  const float3 line_top_start = {start_point.x,
                                                 existing_sector->base +
                                                    existing_sector->height,
                                                 start_point.y};
                  const float3 line_top_mid = {mid_point.x, sector.base + sector.height,
                                               mid_point.y};
                  const float3 line_top_end = {end_point.x,
                                               existing_sector->base +
                                                  existing_sector->height,
                                               end_point.y};

                  _tool_visualizers.lines.emplace_back(line_bottom_start,
                                                       line_bottom_mid, 0xffffffffu);
                  _tool_visualizers.lines.emplace_back(line_top_start,
                                                       line_top_mid, 0xffffffffu);
                  _tool_visualizers.lines.emplace_back(line_bottom_mid,
                                                       line_bottom_end, 0xffffffffu);
                  _tool_visualizers.lines.emplace_back(line_top_mid,
                                                       line_top_end, 0xffffffffu);
               }

               return placement_traits{.has_placement_rotation = false,
                                       .has_point_at = false,
                                       .has_placement_ground = false};
            },
            [&](const world::portal& portal) {
               ImGui::InputText("Name", &creation_entity, &world::portal::name,
                                &_edit_stack_world, &_edit_context,
                                [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.portals,
                                                                *edited_value);
                                });

               ImGui::DragFloat("Width", &creation_entity,
                                &world::portal::width, &_edit_stack_world,
                                &_edit_context, 1.0f, 0.25f, 1e10f);
               ImGui::DragFloat("Height", &creation_entity,
                                &world::portal::height, &_edit_stack_world,
                                &_edit_context, 1.0f, 0.25f, 1e10f);

               ImGui::InputTextAutoComplete(
                  "Linked Sector 1", &creation_entity, &world::portal::sector1,
                  &_edit_stack_world, &_edit_context, [&] {
                     std::array<std::string, 6> entries;
                     std::size_t matching_count = 0;

                     for (const auto& sector : _world.sectors) {
                        if (sector.name.contains(portal.sector1)) {
                           if (matching_count == entries.size()) break;

                           entries[matching_count] = sector.name;

                           ++matching_count;
                        }
                     }

                     return entries;
                  });

               ImGui::InputTextAutoComplete(
                  "Linked Sector 2", &creation_entity, &world::portal::sector2,
                  &_edit_stack_world, &_edit_context, [&] {
                     std::array<std::string, 6> entries;
                     std::size_t matching_count = 0;

                     for (const auto& sector : _world.sectors) {
                        if (sector.name.contains(portal.sector2)) {
                           if (matching_count == entries.size()) break;

                           entries[matching_count] = sector.name;

                           ++matching_count;
                        }
                     }

                     return entries;
                  });

               if (_entity_creation_context.placement_rotation !=
                   placement_rotation::manual_quaternion) {
                  ImGui::DragRotationEuler("Rotation", &creation_entity,
                                           &world::portal::rotation,
                                           &world::edit_context::euler_rotation,
                                           &_edit_stack_world, &_edit_context);
               }
               else {
                  ImGui::DragQuat("Rotation", &creation_entity, &world::portal::rotation,
                                  &_edit_stack_world, &_edit_context);
               }

               if (ImGui::DragFloat3("Position", &creation_entity, &world::portal::position,
                                     &_edit_stack_world, &_edit_context)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;
               }
               if ((_entity_creation_context.placement_rotation ==
                       placement_rotation::surface or
                    _entity_creation_context.placement_mode == placement_mode::cursor) and
                   not _entity_creation_context.using_point_at) {
                  quaternion new_rotation = portal.rotation;
                  float3 new_position = portal.position;
                  float3 new_euler_rotation = _edit_context.euler_rotation;

                  if (_entity_creation_context.placement_rotation ==
                         placement_rotation::surface and
                      _cursor_surface_normalWS) {
                     const float new_y_angle =
                        surface_rotation_degrees(*_cursor_surface_normalWS,
                                                 _edit_context.euler_rotation.y);
                     new_euler_rotation = {_edit_context.euler_rotation.x, new_y_angle,
                                           _edit_context.euler_rotation.z};
                     new_rotation = make_quat_from_euler(
                        new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
                  }

                  if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                     new_position = _cursor_positionWS;

                     if (_entity_creation_context.placement_ground ==
                         placement_ground::bbox) {
                        new_position.y += (portal.height / 2.0f);
                     }

                     if (_entity_creation_context.lock_x_axis) {
                        new_position.x = portal.position.x;
                     }
                     if (_entity_creation_context.lock_y_axis) {
                        new_position.y = portal.position.y;
                     }
                     if (_entity_creation_context.lock_z_axis) {
                        new_position.z = portal.position.z;
                     }
                  }

                  if (new_rotation != portal.rotation or new_position != portal.position) {
                     _edit_stack_world.apply(edits::make_set_creation_location<world::portal>(
                                                new_rotation, portal.rotation, new_position,
                                                portal.position, new_euler_rotation,
                                                _edit_context.euler_rotation),
                                             _edit_context);
                  }
               }

               if (_entity_creation_context.using_point_at) {
                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       portal.position, 0xffffffffu);

                  const quaternion new_rotation =
                     look_at_quat(_cursor_positionWS, portal.position);

                  if (new_rotation != portal.rotation) {
                     _edit_stack_world
                        .apply(edits::make_set_creation_value(&world::portal::rotation,
                                                              new_rotation,
                                                              portal.rotation),
                               _edit_context);
                  }
               }

               if (ImGui::Button("Extend To", {ImGui::CalcItemWidth(), 0.0f})) {
                  _entity_creation_context.activate_extend_to = true;
               }
               if (ImGui::Button("Shrink To", {ImGui::CalcItemWidth(), 0.0f})) {
                  _entity_creation_context.activate_shrink_to = true;
               }

               if (_entity_creation_context.using_extend_to or
                   _entity_creation_context.using_shrink_to) {
                  _entity_creation_context.placement_mode = placement_mode::manual;

                  if (not _entity_creation_context.resize_portal_start_width) {
                     _entity_creation_context.resize_portal_start_width = portal.width;
                  }

                  if (not _entity_creation_context.resize_portal_start_height) {
                     _entity_creation_context.resize_portal_start_height =
                        portal.height;
                  }

                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       portal.position, 0xffffffffu);

                  const quaternion inverse_rotation = conjugate(portal.rotation);

                  const float3 positionPS = inverse_rotation * portal.position;
                  const float3 cursor_positionPS = inverse_rotation * _cursor_positionWS;
                  const float3 size = abs(positionPS - cursor_positionPS);

                  const float width =
                     _entity_creation_context.using_extend_to
                        ? std::max(*_entity_creation_context.resize_portal_start_width,
                                   size.x * 2.0f)
                        : std::min(*_entity_creation_context.resize_portal_start_width,
                                   size.x * 2.0f);
                  const float height =
                     _entity_creation_context.using_extend_to
                        ? std::max(*_entity_creation_context.resize_portal_start_height,
                                   size.y * 2.0f)
                        : std::min(*_entity_creation_context.resize_portal_start_height,
                                   size.y * 2.0f);

                  if (width != portal.width or height != portal.height) {
                     _edit_stack_world.apply(
                        edits::make_set_creation_portal_size(width, portal.width,
                                                             height, portal.height),
                        _edit_context);
                  }
               }
               else {
                  _entity_creation_context.resize_portal_start_width = std::nullopt;
                  _entity_creation_context.resize_portal_start_height = std::nullopt;
               }

               ImGui::Separator();

               return placement_traits{.has_placement_alignment = false,
                                       .has_resize_to = true};
            },
            [&](const world::hintnode& hintnode) {
               ImGui::InputText("Name", &creation_entity,
                                &world::hintnode::name, &_edit_stack_world,
                                &_edit_context, [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.objects,
                                                                *edited_value);
                                });

               ImGui::LayerPick<world::hintnode>("Layer", &creation_entity,
                                                 &_edit_stack_world, &_edit_context);

               ImGui::Separator();

               if (_entity_creation_context.placement_rotation !=
                   placement_rotation::manual_quaternion) {
                  ImGui::DragRotationEuler("Rotation", &creation_entity,
                                           &world::hintnode::rotation,
                                           &world::edit_context::euler_rotation,
                                           &_edit_stack_world, &_edit_context);
               }
               else {
                  ImGui::DragQuat("Rotation", &creation_entity,
                                  &world::hintnode::rotation,
                                  &_edit_stack_world, &_edit_context);
               }

               if (ImGui::DragFloat3("Position", &creation_entity,
                                     &world::hintnode::position,
                                     &_edit_stack_world, &_edit_context)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;
               }

               if ((_entity_creation_context.placement_rotation ==
                       placement_rotation::surface or
                    _entity_creation_context.placement_mode == placement_mode::cursor) and
                   not _entity_creation_context.using_point_at) {
                  quaternion new_rotation = hintnode.rotation;
                  float3 new_position = hintnode.position;
                  float3 new_euler_rotation = _edit_context.euler_rotation;

                  if (_entity_creation_context.placement_rotation ==
                         placement_rotation::surface and
                      _cursor_surface_normalWS) {
                     const float new_y_angle =
                        surface_rotation_degrees(*_cursor_surface_normalWS,
                                                 _edit_context.euler_rotation.y);
                     new_euler_rotation = {_edit_context.euler_rotation.x, new_y_angle,
                                           _edit_context.euler_rotation.z};
                     new_rotation = make_quat_from_euler(
                        new_euler_rotation * std::numbers::pi_v<float> / 180.0f);
                  }

                  if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                     new_position = _cursor_positionWS;

                     if (_entity_creation_context.placement_alignment ==
                         placement_alignment::grid) {
                        new_position =
                           align_position_to_grid(new_position,
                                                  _entity_creation_context.alignment);
                     }
                     else if (_entity_creation_context.placement_alignment ==
                              placement_alignment::snapping) {
                        const std::optional<float3> snapped_position =
                           world::get_snapped_position(new_position, _world.objects,
                                                       _entity_creation_context.snap_distance,
                                                       _object_classes);

                        if (snapped_position) new_position = *snapped_position;
                     }

                     if (_entity_creation_context.lock_x_axis) {
                        new_position.x = hintnode.position.x;
                     }
                     if (_entity_creation_context.lock_y_axis) {
                        new_position.y = hintnode.position.y;
                     }
                     if (_entity_creation_context.lock_z_axis) {
                        new_position.z = hintnode.position.z;
                     }
                  }

                  if (new_rotation != hintnode.rotation or
                      new_position != hintnode.position) {
                     _edit_stack_world.apply(edits::make_set_creation_location<world::hintnode>(
                                                new_rotation, hintnode.rotation,
                                                new_position, hintnode.position,
                                                new_euler_rotation,
                                                _edit_context.euler_rotation),
                                             _edit_context);
                  }
               }

               if (_entity_creation_context.using_point_at) {
                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       hintnode.position, 0xffffffffu);

                  const quaternion new_rotation =
                     look_at_quat(_cursor_positionWS, hintnode.position);

                  if (new_rotation != hintnode.rotation) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::hintnode::rotation,
                                                new_rotation, hintnode.rotation),
                                             _edit_context);
                  }
               }

               ImGui::Separator();

               ImGui::EnumSelect(
                  "Type", &creation_entity, &world::hintnode::type,
                  &_edit_stack_world, &_edit_context,
                  {enum_select_option{"Snipe", world::hintnode_type::snipe},
                   enum_select_option{"Patrol", world::hintnode_type::patrol},
                   enum_select_option{"Cover", world::hintnode_type::cover},
                   enum_select_option{"Access", world::hintnode_type::access},
                   enum_select_option{"Jet Jump", world::hintnode_type::jet_jump},
                   enum_select_option{"Mine", world::hintnode_type::mine},
                   enum_select_option{"Land", world::hintnode_type::land},
                   enum_select_option{"Fortification", world::hintnode_type::fortification},
                   enum_select_option{"Vehicle Cover",
                                      world::hintnode_type::vehicle_cover}});

               const world::hintnode_traits hintnode_traits =
                  world::get_hintnode_traits(hintnode.type);

               if (hintnode_traits.has_command_post) {
                  if (ImGui::BeginCombo("Command Post", hintnode.command_post.c_str())) {
                     for (const auto& object : _world.objects) {
                        const assets::odf::properties& properties =
                           _object_classes[object.class_name].definition->header_properties;

                        if (not properties.contains("ClassLabel")) continue;

                        if (properties["ClassLabel"] == "commandpost") {
                           if (ImGui::Selectable(object.name.c_str())) {
                              _edit_stack_world.apply(edits::make_set_creation_value(
                                                         &world::hintnode::command_post,
                                                         object.name,
                                                         hintnode.command_post),
                                                      _edit_context);
                           }
                        }
                     }
                     ImGui::EndCombo();
                  }
               }

               if (hintnode_traits.has_primary_stance) {
                  ImGui::Separator();
                  ImGui::EditFlags("Primary Stance", &creation_entity,
                                   &world::hintnode::primary_stance,
                                   &_edit_stack_world, &_edit_context,
                                   {{"Stand", world::stance_flags::stand},
                                    {"Crouch", world::stance_flags::crouch},
                                    {"Prone", world::stance_flags::prone}});
               }

               if (hintnode_traits.has_secondary_stance) {
                  ImGui::Separator();
                  ImGui::EditFlags("Secondary Stance", &creation_entity,
                                   &world::hintnode::secondary_stance,
                                   &_edit_stack_world, &_edit_context,
                                   {{"Stand", world::stance_flags::stand},
                                    {"Crouch", world::stance_flags::crouch},
                                    {"Prone", world::stance_flags::prone},
                                    {"Left", world::stance_flags::left},
                                    {"Right", world::stance_flags::right}});
               }

               if (hintnode_traits.has_mode) {
                  ImGui::EnumSelect(
                     "Mode", &creation_entity, &world::hintnode::mode,
                     &_edit_stack_world, &_edit_context,
                     {enum_select_option{"None", world::hintnode_mode::none},
                      enum_select_option{"Attack", world::hintnode_mode::attack},
                      enum_select_option{"Defend", world::hintnode_mode::defend},
                      enum_select_option{"Both", world::hintnode_mode::both}});
               }

               if (hintnode_traits.has_radius) {
                  ImGui::DragFloat("Radius", &creation_entity,
                                   &world::hintnode::radius, &_edit_stack_world,
                                   &_edit_context, 0.25f, 0.0f, 1e10f, "%.3f",
                                   ImGuiSliderFlags_AlwaysClamp);
               }

               return placement_traits{.has_placement_ground = false};
            },
            [&](const world::barrier& barrier) {
               ImGui::InputText("Name", &creation_entity, &world::barrier::name,
                                &_edit_stack_world, &_edit_context,
                                [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.barriers,
                                                                barrier.name);
                                });

               ImGui::DragBarrierRotation("Rotation", &creation_entity,
                                          &_edit_stack_world, &_edit_context);

               if (ImGui::DragFloat2XZ("Position", &creation_entity,
                                       &world::barrier::position,
                                       &_edit_stack_world, &_edit_context, 0.25f)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;
               }

               if (_entity_creation_context.placement_mode == placement_mode::cursor and
                   not _entity_creation_context.using_point_at) {
                  float2 new_position = barrier.position;

                  if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                     new_position =
                        float2{_cursor_positionWS.x, _cursor_positionWS.z};

                     if (_entity_creation_context.placement_alignment ==
                         placement_alignment::grid) {
                        new_position =
                           align_position_to_grid(new_position,
                                                  _entity_creation_context.alignment);
                     }
                     else if (_entity_creation_context.placement_alignment ==
                              placement_alignment::snapping) {
                        const std::optional<float3> snapped_position =
                           world::get_snapped_position({new_position.x,
                                                        _cursor_positionWS.y,
                                                        new_position.y},
                                                       _world.objects,
                                                       _entity_creation_context.snap_distance,
                                                       _object_classes);

                        if (snapped_position) {
                           new_position = {snapped_position->x, snapped_position->z};
                        }
                     }

                     if (_entity_creation_context.lock_x_axis) {
                        new_position.x = barrier.position.x;
                     }
                     if (_entity_creation_context.lock_z_axis) {
                        new_position.y =
                           barrier.position.y; // NB: Usage of Y under lock Z.
                     }
                  }

                  if (new_position != barrier.position) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::barrier::position,
                                                new_position, barrier.position),
                                             _edit_context);
                  }
               }

               if (_entity_creation_context.using_point_at) {
                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       float3{barrier.position.x,
                                                              _cursor_positionWS.y,
                                                              barrier.position.y},
                                                       0xffffffffu);

                  const float new_angle =
                     surface_rotation_degrees(normalize(
                                                 float3{barrier.position.x,
                                                        _cursor_positionWS.y,
                                                        barrier.position.y} -
                                                 _cursor_positionWS),
                                              barrier.rotation_angle) /
                     180.0f * std::numbers::pi_v<float>;

                  if (new_angle != barrier.rotation_angle) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::barrier::rotation_angle,
                                                new_angle, barrier.rotation_angle),
                                             _edit_context);
                  }
               }

               ImGui::DragFloat2XZ("Size", &creation_entity,
                                   &world::barrier::size, &_edit_stack_world,
                                   &_edit_context, 1.0f, 0.0f, 1e10f);

               if (ImGui::Button("Extend To", {ImGui::CalcItemWidth(), 0.0f})) {
                  _entity_creation_context.activate_extend_to = true;
               }
               if (ImGui::Button("Shrink To", {ImGui::CalcItemWidth(), 0.0f})) {
                  _entity_creation_context.activate_shrink_to = true;
               }

               if (_entity_creation_context.using_extend_to or
                   _entity_creation_context.using_shrink_to) {
                  _entity_creation_context.placement_mode = placement_mode::manual;

                  if (not _entity_creation_context.resize_barrier_start_position) {
                     _entity_creation_context.resize_barrier_start_position =
                        barrier.position;
                     _entity_creation_context.resize_barrier_start_size =
                        barrier.size;
                  }

                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       float3{barrier.position.x,
                                                              _cursor_positionWS.y,
                                                              barrier.position.y},
                                                       0xffffffffu);

                  const float2 barrier_start_position =
                     *_entity_creation_context.resize_barrier_start_position;
                  const float2 barrier_start_size =
                     *_entity_creation_context.resize_barrier_start_size;

                  const float4x4 barrier_rotation = make_rotation_matrix_from_euler(
                     {0.0f, barrier.rotation_angle, 0.0f});
                  const float4x4 inverse_barrier_rotation =
                     transpose(barrier_rotation);

                  const float3 cursor_positionOS =
                     inverse_barrier_rotation *
                     float3{_cursor_positionWS.x, 0.0f, _cursor_positionWS.z};
                  const float3 barrier_positionOS =
                     inverse_barrier_rotation * float3{barrier_start_position.x, 0.0f,
                                                       barrier_start_position.y};

                  std::array<float3, 4> cornersOS{
                     float3{-barrier_start_size.x, 0.0f, -barrier_start_size.y} +
                        barrier_positionOS,
                     float3{-barrier_start_size.x, 0.0f, barrier_start_size.y} +
                        barrier_positionOS,
                     float3{barrier_start_size.x, 0.0f, barrier_start_size.y} +
                        barrier_positionOS,
                     float3{barrier_start_size.x, 0.0f, -barrier_start_size.y} +
                        barrier_positionOS,
                  };

                  const float3 cursor_vectorOS = cursor_positionOS - barrier_positionOS;

                  if (cursor_vectorOS.x < 0.0f) {
                     cornersOS[0].x = cursor_positionOS.x;
                     cornersOS[1].x = cursor_positionOS.x;
                  }
                  else {
                     cornersOS[2].x = cursor_positionOS.x;
                     cornersOS[3].x = cursor_positionOS.x;
                  }

                  if (cursor_vectorOS.z < 0.0f) {
                     cornersOS[0].z = cursor_positionOS.z;
                     cornersOS[3].z = cursor_positionOS.z;
                  }
                  else {
                     cornersOS[1].z = cursor_positionOS.z;
                     cornersOS[2].z = cursor_positionOS.z;
                  }

                  float3 new_positionOS{};

                  for (const auto& corner : cornersOS) {
                     new_positionOS += corner;
                  }

                  new_positionOS /= 4.0f;

                  float2 barrier_new_size =
                     abs(float2{cornersOS[2].x, cornersOS[2].z} -
                         float2{cornersOS[0].x, cornersOS[0].z}) /
                     2.0f;

                  if (_entity_creation_context.using_extend_to) {
                     barrier_new_size = max(barrier_new_size, barrier_start_size);
                  }
                  else {
                     barrier_new_size = min(barrier_new_size, barrier_start_size);
                  }

                  float3 new_positionWS = barrier_rotation * new_positionOS;

                  if (barrier_new_size.x == barrier_start_size.x) {
                     new_positionWS.x = barrier_start_position.x;
                  }
                  if (barrier_new_size.y == barrier_start_size.y) {
                     new_positionWS.z = barrier_start_position.y;
                  }

                  const float2 barrier_new_position = {new_positionWS.x,
                                                       new_positionWS.z};

                  if (barrier_new_position != barrier_start_position or
                      barrier_new_size != barrier_start_size) {
                     _edit_stack_world.apply(edits::make_set_creation_barrier_metrics(
                                                barrier.rotation_angle, barrier.rotation_angle,
                                                barrier_new_position, barrier_start_position,
                                                barrier_new_size, barrier_start_size),
                                             _edit_context);
                  }
               }
               else {
                  _entity_creation_context.resize_barrier_start_position = std::nullopt;
                  _entity_creation_context.resize_barrier_start_size = std::nullopt;
               }

               ImGui::Separator();
               if (ImGui::Button("From Object Bounds", {ImGui::CalcItemWidth(), 0.0f})) {
                  _entity_creation_context.activate_from_object_bbox = true;
               }

               if (_entity_creation_context.using_from_object_bbox and
                   _interaction_targets.hovered_entity and
                   std::holds_alternative<world::object_id>(
                      *_interaction_targets.hovered_entity)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;

                  const world::object* object =
                     world::find_entity(_world.objects,
                                        std::get<world::object_id>(
                                           *_interaction_targets.hovered_entity));

                  if (object) {
                     math::bounding_box bbox =
                        _object_classes[object->class_name].model->bounding_box;

                     const float3 size = abs(bbox.max - bbox.min) / 2.0f;
                     const float3 position =
                        object->rotation *
                        ((conjugate(object->rotation) * object->position) +
                         ((bbox.min + bbox.max) / 2.0f));

                     std::array<float3, 8> corners = math::to_corners(bbox);

                     for (auto& corner : corners) {
                        corner = object->rotation * corner + object->position;
                     }

                     const float rotation_angle =
                        std::atan2(corners[1].z - corners[0].z,
                                   corners[1].x - corners[0].x);

                     _edit_stack_world.apply(edits::make_set_creation_barrier_metrics(
                                                rotation_angle, barrier.rotation_angle,
                                                float2{position.x, position.z},
                                                barrier.position,
                                                float2{size.x, size.z}, barrier.size),
                                             _edit_context);
                  }
               }

               if (ImGui::Button("From Line", {ImGui::CalcItemWidth(), 0.0f})) {
                  _entity_creation_context.activate_from_line = true;
               }

               if (ImGui::IsItemHovered()) {
                  ImGui::SetTooltip("Place a line and then place resize and "
                                    "reposition the barrier around that line.");
               }

               if (_entity_creation_context.using_from_line) {
                  _entity_creation_context.placement_mode = placement_mode::manual;

                  if (_entity_creation_context.from_line_start) {
                     _tool_visualizers.lines
                        .emplace_back(*_entity_creation_context.from_line_start,
                                      _cursor_positionWS, 0xffffffffu);

                     const float3 start = *_entity_creation_context.from_line_start;
                     const float3 end = _cursor_positionWS;

                     const float3 position = (start + end) / 2.0f;
                     const float height = distance(start, end) / 2.0f;
                     const float rotation_angle =
                        std::atan2(start.x - end.x, start.z - end.z);

                     _edit_stack_world.apply(edits::make_set_creation_barrier_metrics(
                                                rotation_angle, barrier.rotation_angle,
                                                float2{position.x, position.z},
                                                barrier.position,
                                                float2{barrier.size.x, height},
                                                barrier.size),
                                             _edit_context);

                     if (std::exchange(_entity_creation_context.from_line_click, false)) {
                        _entity_creation_context.using_from_line = false;
                     }
                  }
                  else if (std::exchange(_entity_creation_context.from_line_click,
                                         false)) {
                     _entity_creation_context.from_line_start = _cursor_positionWS;
                  }
               }
               else {
                  _entity_creation_context.from_line_start = std::nullopt;
               }

               ImGui::Separator();

               ImGui::EditFlags("Flags", &creation_entity, &world::barrier::flags,
                                &_edit_stack_world, &_edit_context,
                                {{"Soldier", world::ai_path_flags::soldier},
                                 {"Hover", world::ai_path_flags::hover},
                                 {"Small", world::ai_path_flags::small},
                                 {"Medium", world::ai_path_flags::medium},
                                 {"Huge", world::ai_path_flags::huge},
                                 {"Flyer", world::ai_path_flags::flyer}});

               return placement_traits{.has_placement_rotation = false,
                                       .has_placement_ground = false,
                                       .has_resize_to = true,
                                       .has_from_bbox = true,
                                       .has_from_line = true};
            },
            [&](const world::planning_hub& hub) {
               ImGui::InputText("Name", &creation_entity,
                                &world::planning_hub::name, &_edit_stack_world,
                                &_edit_context, [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.planning_hubs,
                                                                hub.name);
                                });

               if (ImGui::DragFloat2XZ("Position", &creation_entity,
                                       &world::planning_hub::position,
                                       &_edit_stack_world, &_edit_context, 0.25f)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;
               }

               if (_entity_creation_context.placement_mode == placement_mode::cursor and
                   not _entity_creation_context.hub_sizing_started) {
                  float2 new_position = hub.position;

                  if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                     new_position =
                        float2{_cursor_positionWS.x, _cursor_positionWS.z};

                     if (_entity_creation_context.placement_alignment ==
                         placement_alignment::grid) {
                        new_position =
                           align_position_to_grid(new_position,
                                                  _entity_creation_context.alignment);
                     }
                     else if (_entity_creation_context.placement_alignment ==
                              placement_alignment::snapping) {
                        const std::optional<float3> snapped_position =
                           world::get_snapped_position({new_position.x,
                                                        _cursor_positionWS.y,
                                                        new_position.y},
                                                       _world.objects,
                                                       _entity_creation_context.snap_distance,
                                                       _object_classes);

                        if (snapped_position) {
                           new_position = {snapped_position->x, snapped_position->z};
                        }
                     }

                     if (_entity_creation_context.lock_x_axis) {
                        new_position.x = hub.position.x;
                     }
                     if (_entity_creation_context.lock_z_axis) {
                        new_position.y = hub.position.y; // NB: Usage of Y under lock Z.
                     }
                  }

                  if (new_position != hub.position) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::planning_hub::position,
                                                new_position, hub.position),
                                             _edit_context);
                  }
               }

               ImGui::DragFloat("Radius", &creation_entity,
                                &world::planning_hub::radius, &_edit_stack_world,
                                &_edit_context, 1.0f, 0.0f, 1e10f);

               if (_entity_creation_context.hub_sizing_started) {
                  _tool_visualizers.lines.emplace_back(_cursor_positionWS,
                                                       float3{hub.position.x,
                                                              _cursor_positionWS.y,
                                                              hub.position.y},
                                                       0xffffffffu);

                  const float new_radius =
                     distance(float2{_cursor_positionWS.x, _cursor_positionWS.z},
                              hub.position);

                  if (new_radius != hub.radius) {
                     _edit_stack_world
                        .apply(edits::make_set_creation_value(&world::planning_hub::radius,
                                                              new_radius, hub.radius),
                               _edit_context);
                  }
               }

               return placement_traits{
                  .has_placement_rotation = false,
                  .has_point_at = false,
                  .has_placement_ground = false,
               };
            },
            [&](const world::planning_connection& connection) {
               ImGui::InputText("Name", &creation_entity,
                                &world::planning_connection::name, &_edit_stack_world,
                                &_edit_context, [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.planning_connections,
                                                                connection.name);
                                });

               ImGui::Text(
                  "Start: %s",
                  _world
                     .planning_hubs[_world.planning_hub_index.at(connection.start)]
                     .name.c_str());
               ImGui::Text(
                  "End: %s",
                  _world
                     .planning_hubs[_world.planning_hub_index.at(connection.end)]
                     .name.c_str());

               if (_entity_creation_context.connection_link_started and
                   _interaction_targets.hovered_entity and
                   std::holds_alternative<world::planning_hub_id>(
                      *_interaction_targets.hovered_entity)) {
                  const world::planning_hub_id end_id =
                     std::get<world::planning_hub_id>(*_interaction_targets.hovered_entity);

                  _edit_stack_world
                     .apply(edits::make_set_creation_value(&world::planning_connection::end,
                                                           end_id, connection.end),
                            _edit_context);
               }

               ImGui::Separator();

               ImGui::EditFlags("Flags", &creation_entity,
                                &world::planning_connection::flags,
                                &_edit_stack_world, &_edit_context,
                                {{"Soldier", world::ai_path_flags::soldier},
                                 {"Hover", world::ai_path_flags::hover},
                                 {"Small", world::ai_path_flags::small},
                                 {"Medium", world::ai_path_flags::medium},
                                 {"Huge", world::ai_path_flags::huge},
                                 {"Flyer", world::ai_path_flags::flyer}});

               ImGui::Separator();

               ImGui::Checkbox("Jump", &creation_entity,
                               &world::planning_connection::jump,
                               &_edit_stack_world, &_edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("Jet Jump", &creation_entity,
                               &world::planning_connection::jet_jump,
                               &_edit_stack_world, &_edit_context);
               ImGui::SameLine();
               ImGui::Checkbox("One Way", &creation_entity,
                               &world::planning_connection::one_way,
                               &_edit_stack_world, &_edit_context);

               bool is_dynamic = connection.dynamic_group != 0;

               if (ImGui::Checkbox("Dynamic", &is_dynamic)) {
                  _edit_stack_world.apply(edits::make_set_creation_value(
                                             &world::planning_connection::dynamic_group,
                                             is_dynamic ? int8{1} : int8{0},
                                             connection.dynamic_group),
                                          _edit_context);
               }

               if (is_dynamic) {
                  ImGui::SliderInt("Dynamic Group", &creation_entity,
                                   &world::planning_connection::dynamic_group,
                                   &_edit_stack_world, &_edit_context, 1, 8,
                                   "%d", ImGuiSliderFlags_AlwaysClamp);
               }

               if (ImGui::CollapsingHeader("Forward Branch Weights")) {
                  world::planning_branch_weights weights = connection.forward_weights;

                  bool changed = false;

                  ImGui::PushID("Forward Branch Weights");

                  // clang-format off
                  changed |= ImGui::DragFloat("Soldier", &weights.soldier, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Hover", &weights.hover, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Small", &weights.small, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Medium", &weights.medium, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Huge", &weights.huge, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Flyer", &weights.flyer, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  // clang-format on

                  ImGui::PopID();

                  if (changed) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::planning_connection::forward_weights,
                                                weights, connection.forward_weights),
                                             _edit_context);
                  }
               }

               if (ImGui::CollapsingHeader("Backward Branch Weights")) {
                  world::planning_branch_weights weights = connection.backward_weights;

                  ImGui::PushID("Backward Branch Weights");

                  bool changed = false;

                  // clang-format off
                  changed |= ImGui::DragFloat("Soldier", &weights.soldier, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Hover", &weights.hover, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Small", &weights.small, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Medium", &weights.medium, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Huge", &weights.huge, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  changed |= ImGui::DragFloat("Flyer", &weights.flyer, 0.5f, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                  // clang-format on

                  ImGui::PopID();

                  if (changed) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::planning_connection::backward_weights,
                                                weights, connection.backward_weights),
                                             _edit_context);
                  }
               }

               return placement_traits{.has_placement_rotation = false,
                                       .has_point_at = false,
                                       .has_placement_mode = false,
                                       .has_lock_axis = false,
                                       .has_placement_alignment = false,
                                       .has_placement_ground = false};
            },
            [&](const world::boundary& boundary) {
               ImGui::InputText("Name", &creation_entity,
                                &world::boundary::name, &_edit_stack_world,
                                &_edit_context, [&](std::string* edited_value) {
                                   *edited_value =
                                      world::create_unique_name(_world.boundaries,
                                                                boundary.name);
                                });

               if (ImGui::DragFloat2XZ("Position", &creation_entity,
                                       &world::boundary::position,
                                       &_edit_stack_world, &_edit_context, 0.25f)) {
                  _entity_creation_context.placement_mode = placement_mode::manual;
               }

               if (_entity_creation_context.placement_mode == placement_mode::cursor and
                   not _entity_creation_context.using_point_at) {
                  float2 new_position = boundary.position;

                  if (_entity_creation_context.placement_mode == placement_mode::cursor) {
                     new_position =
                        float2{_cursor_positionWS.x, _cursor_positionWS.z};

                     if (_entity_creation_context.placement_alignment ==
                         placement_alignment::grid) {
                        new_position =
                           align_position_to_grid(new_position,
                                                  _entity_creation_context.alignment);
                     }
                     else if (_entity_creation_context.placement_alignment ==
                              placement_alignment::snapping) {
                        const std::optional<float3> snapped_position =
                           world::get_snapped_position({new_position.x,
                                                        _cursor_positionWS.y,
                                                        new_position.y},
                                                       _world.objects,
                                                       _entity_creation_context.snap_distance,
                                                       _object_classes);

                        if (snapped_position) {
                           new_position = {snapped_position->x, snapped_position->z};
                        }
                     }

                     if (_entity_creation_context.lock_x_axis) {
                        new_position.x = boundary.position.x;
                     }
                     if (_entity_creation_context.lock_z_axis) {
                        new_position.y =
                           boundary.position.y; // NB: Usage of Y under lock Z.
                     }
                  }

                  if (new_position != boundary.position) {
                     _edit_stack_world.apply(edits::make_set_creation_value(
                                                &world::boundary::position,
                                                new_position, boundary.position),
                                             _edit_context);
                  }
               }

               ImGui::DragFloat2XZ("Size", &creation_entity,
                                   &world::boundary::size, &_edit_stack_world,
                                   &_edit_context, 1.0f, 0.0f, 1e10f);

               return placement_traits{.has_placement_rotation = false,
                                       .has_point_at = false,
                                       .has_placement_ground = false};
            },
         },
         creation_entity);

      if (traits.has_placement_rotation) {
         ImGui::Separator();

         ImGui::Text("Rotation");

         ImGui::BeginTable("Rotation", 3,
                           ImGuiTableFlags_NoSavedSettings |
                              ImGuiTableFlags_SizingStretchSame);

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Manual", _entity_creation_context.placement_rotation ==
                                            placement_rotation::manual_euler)) {
            _entity_creation_context.placement_rotation =
               placement_rotation::manual_euler;
         }

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Manual (Quat)",
                               _entity_creation_context.placement_rotation ==
                                  placement_rotation::manual_quaternion)) {
            _entity_creation_context.placement_rotation =
               placement_rotation::manual_quaternion;
         }

         ImGui::TableNextColumn();

         if (ImGui::Selectable("Around Cursor", _entity_creation_context.placement_rotation ==
                                                   placement_rotation::surface)) {
            _entity_creation_context.placement_rotation = placement_rotation::surface;
         }
         ImGui::EndTable();
      }

      if (traits.has_point_at) {
         if (not traits.has_placement_rotation) ImGui::Separator();

         if (ImGui::Button("Point At", {ImGui::CalcItemWidth(), 0.0f})) {
            _entity_creation_context.activate_point_at = true;
         }
      }

      if (traits.has_placement_mode) {
         ImGui::Separator();

         ImGui::Text("Placement");

         ImGui::BeginTable("Placement", 2,
                           ImGuiTableFlags_NoSavedSettings |
                              ImGuiTableFlags_SizingStretchSame);

         ImGui::TableNextColumn();
         if (ImGui::Selectable("Manual", _entity_creation_context.placement_mode ==
                                            placement_mode::manual)) {
            _entity_creation_context.placement_mode = placement_mode::manual;
         }

         ImGui::TableNextColumn();

         if (ImGui::Selectable("At Cursor", _entity_creation_context.placement_mode ==
                                               placement_mode::cursor)) {
            _entity_creation_context.placement_mode = placement_mode::cursor;
         }
         ImGui::EndTable();
      }

      if (_entity_creation_context.placement_mode == placement_mode::cursor) {
         if (traits.has_lock_axis) {
            ImGui::Separator();

            ImGui::Text("Locked Position");

            ImGui::BeginTable("Locked Position", 3,
                              ImGuiTableFlags_NoSavedSettings |
                                 ImGuiTableFlags_SizingStretchSame);

            ImGui::TableNextColumn();
            ImGui::Selectable("X", &_entity_creation_context.lock_x_axis);
            ImGui::TableNextColumn();
            ImGui::Selectable("Y", &_entity_creation_context.lock_y_axis);
            ImGui::TableNextColumn();
            ImGui::Selectable("Z", &_entity_creation_context.lock_z_axis);

            ImGui::EndTable();
         }

         if (traits.has_placement_alignment) {
            ImGui::Separator();

            ImGui::Text("Align To");

            ImGui::BeginTable("Align To", 3,
                              ImGuiTableFlags_NoSavedSettings |
                                 ImGuiTableFlags_SizingStretchSame);

            ImGui::TableNextColumn();
            if (ImGui::Selectable("None", _entity_creation_context.placement_alignment ==
                                             placement_alignment::none)) {
               _entity_creation_context.placement_alignment =
                  placement_alignment::none;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Grid", _entity_creation_context.placement_alignment ==
                                             placement_alignment::grid)) {
               _entity_creation_context.placement_alignment =
                  placement_alignment::grid;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Snapping", _entity_creation_context.placement_alignment ==
                                                 placement_alignment::snapping)) {
               _entity_creation_context.placement_alignment =
                  placement_alignment::snapping;
            }
            ImGui::EndTable();
         }

         if (traits.has_placement_ground) {
            ImGui::Separator();

            ImGui::Text("Ground With");

            ImGui::BeginTable("Ground With", 2,
                              ImGuiTableFlags_NoSavedSettings |
                                 ImGuiTableFlags_SizingStretchSame);

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Origin", _entity_creation_context.placement_ground ==
                                               placement_ground::origin)) {
               _entity_creation_context.placement_ground = placement_ground::origin;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Bounding Box", _entity_creation_context.placement_ground ==
                                                     placement_ground::bbox)) {
               _entity_creation_context.placement_ground = placement_ground::bbox;
            }

            ImGui::EndTable();
         }

         if (traits.has_node_placement_insert) {
            ImGui::Separator();

            ImGui::Text("Node Insertion");

            ImGui::BeginTable("Node Insertion", 2,
                              ImGuiTableFlags_NoSavedSettings |
                                 ImGuiTableFlags_SizingStretchSame);

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Nearest", _entity_creation_context.placement_node_insert ==
                                                placement_node_insert::nearest)) {
               _entity_creation_context.placement_node_insert =
                  placement_node_insert::nearest;
            }

            ImGui::TableNextColumn();
            if (ImGui::Selectable("Append", _entity_creation_context.placement_node_insert ==
                                               placement_node_insert::append)) {
               _entity_creation_context.placement_node_insert =
                  placement_node_insert::append;
            }

            ImGui::EndTable();
         }

         if (_entity_creation_context.placement_alignment == placement_alignment::grid) {
            ImGui::Separator();
            ImGui::DragFloat("Alignment Grid Size",
                             &_entity_creation_context.alignment, 1.0f, 1.0f,
                             1e10f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
         }
         else if (_entity_creation_context.placement_alignment ==
                  placement_alignment::snapping) {
            ImGui::Separator();
            ImGui::DragFloat("Snap Distance",
                             &_entity_creation_context.snap_distance, 0.1f,
                             0.0f, 1e10f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
         }
      }

      ImGui::End();

      if (_hotkeys_show) {
         ImGui::Begin("Hotkeys");

         if (traits.has_new_path) {
            ImGui::Text("New Path");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.finish_path")));
         }

         if (traits.has_placement_rotation) {
            ImGui::Text("Change Rotation Mode");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.cycle_rotation_mode")));
         }

         if (traits.has_point_at) {
            ImGui::Text("Point At");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.activate_point_at")));
         }

         if (traits.has_placement_mode) {
            ImGui::Text("Change Placement Mode");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.cycle_placement_mode")));
         }

         if (traits.has_lock_axis) {
            ImGui::Text("Lock X Position");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.lock_x_axis")));

            ImGui::Text("Lock Y Position");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.lock_y_axis")));

            ImGui::Text("Lock Z Position");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.lock_z_axis")));
         }

         if (traits.has_placement_alignment) {
            ImGui::Text("Change Alignment Mode");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.cycle_alignment_mode")));
         }

         if (traits.has_placement_ground) {
            ImGui::Text("Change Grounding Mode");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.cycle_ground_mode")));
         }

         if (traits.has_resize_to) {
            ImGui::Text("Extend To");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.activate_extend_to")));

            ImGui::Text("Shrink To");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.activate_shrink_to")));
         }

         if (traits.has_from_bbox) {
            ImGui::Text("From Object Bounds");
            ImGui::BulletText(get_display_string(_hotkeys.query_binding(
               "Entity Creation",
               "entity_creation.activate_from_object_bbox")));
         }

         if (traits.has_from_line) {
            ImGui::Text("From Line");
            ImGui::BulletText(get_display_string(
               _hotkeys.query_binding("Entity Creation",
                                      "entity_creation.activate_from_line")));
         }

         ImGui::End();
      }

      if (not continue_creation) {
         _entity_creation_context.using_point_at = false;
         _entity_creation_context.using_extend_to = false;
         _entity_creation_context.using_shrink_to = false;
         _entity_creation_context.using_from_object_bbox = false;

         _edit_stack_world.apply(edits::make_creation_entity_set(std::nullopt,
                                                                 creation_entity),
                                 _edit_context);
      }
   }
}
}