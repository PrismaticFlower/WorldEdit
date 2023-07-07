
#include "world_edit.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/odf/default_object_class_definition.hpp"
#include "edits/add_sector_object.hpp"
#include "edits/bundle.hpp"
#include "edits/creation_entity_set.hpp"
#include "edits/delete_entity.hpp"
#include "edits/insert_entity.hpp"
#include "edits/insert_node.hpp"
#include "edits/insert_point.hpp"
#include "edits/set_value.hpp"
#include "math/vector_funcs.hpp"
#include "resource.h"
#include "utility/file_pickers.hpp"
#include "utility/os_execute.hpp"
#include "utility/overload.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"
#include "world/utility/make_command_post_linked_entities.hpp"
#include "world/utility/object_properties.hpp"
#include "world/utility/raycast.hpp"
#include "world/utility/sector_fill.hpp"
#include "world/utility/world_utilities.hpp"
#include "world/world_io_load.hpp"
#include "world/world_io_save.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

#include <backends/imgui_impl_win32.h>
#include <imgui.h>

using namespace std::literals;

namespace we {

world_edit::world_edit(const HWND window, utility::command_line command_line)
   : _imgui_context{ImGui::CreateContext(), &ImGui::DestroyContext},
     _window{window},
     _renderer_use_debug_layer{command_line.get_flag("-gpu_debug_layer")}
{
   async::task<settings::settings> settings_load = _thread_pool->exec([]() {
      try {
         return settings::load(".settings");
      }
      catch (std::exception&) {
         // No saved settings for us 🙁
      }

      return settings::settings{};
   });

   try {
      _renderer =
         graphics::make_renderer({.window = window,
                                  .thread_pool = _thread_pool,
                                  .asset_libraries = _asset_libraries,
                                  .error_output = _stream,
                                  .use_debug_layer = _renderer_use_debug_layer});
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }

   initialize_commands();
   initialize_hotkeys();

   ImGui_ImplWin32_Init(window);

   // Call this to initialize the ImGui font and display scaling values.
   dpi_changed(GetDpiForWindow(_window));

   if (auto start_project = command_line.get_or("-project"sv, ""sv);
       not start_project.empty()) {
      open_project(start_project);
   }

   if (auto start_world = command_line.get_or("-world"sv, ""sv);
       not start_world.empty()) {
      load_world(start_world);
   }

   RECT rect{};
   GetWindowRect(window, &rect);

   _camera.aspect_ratio(static_cast<float>(rect.right - rect.left) /
                        static_cast<float>(rect.bottom - rect.top));

   _settings = settings_load.get();
}

void world_edit::update()
{
   const float delta_time =
      std::chrono::duration<float>(
         std::chrono::steady_clock::now() -
         std::exchange(_last_update, std::chrono::steady_clock::now()))
         .count();

   if (not _focused) return;

   wait_for_swap_chain_ready();

   if (_applied_user_display_scale != _settings.ui.extra_scaling) {
      dpi_changed(_current_dpi);
   }

   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();

   _gizmo.update(make_camera_ray(_camera,
                                 {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                                 {ImGui::GetMainViewport()->Size.x,
                                  ImGui::GetMainViewport()->Size.y}),
                 ImGui::IsKeyDown(ImGuiKey_MouseLeft) and
                    not ImGui::GetIO().WantCaptureMouse);

   // Input!
   update_input();
   update_hovered_entity();

   // UI!
   update_window_text();
   update_ui();

   // Logic!
   update_object_classes();

   _asset_libraries.update_loaded();

   // Render!
   update_camera(delta_time);

   _gizmo.draw(_tool_visualizers);

   try {
      _renderer->draw_frame(_camera, _world, _interaction_targets, _world_draw_mask,
                            _world_layers_draw_mask, _tool_visualizers,
                            _object_classes, _settings.graphics);
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }
}

void world_edit::wait_for_swap_chain_ready() noexcept
{
   try {
      _renderer->wait_for_swap_chain_ready();
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }
}

void world_edit::update_window_text() noexcept
{
   if (_window_unsaved_star != _edit_stack_world.modified_flag()) {
      SetWindowTextA(_window, fmt::format("WorldEdit - {}{}",
                                          _edit_stack_world.modified_flag() ? "*" : "",
                                          _world_path.filename().string())
                                 .c_str());
      _window_unsaved_star = _edit_stack_world.modified_flag();
   }
}

void world_edit::update_input() noexcept
{
   _hotkeys.update(ImGui::GetIO().WantCaptureMouse or _gizmo.want_capture_mouse(),
                   ImGui::GetIO().WantCaptureKeyboard);

   _mouse_movement_x = std::exchange(_queued_mouse_movement_x, 0);
   _mouse_movement_y = std::exchange(_queued_mouse_movement_y, 0);
}

void world_edit::update_hovered_entity() noexcept
{
   graphics::camera_ray ray =
      make_camera_ray(_camera, {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                      {ImGui::GetMainViewport()->Size.x,
                       ImGui::GetMainViewport()->Size.y});

   _interaction_targets.hovered_entity = {};
   _cursor_surface_normalWS = std::nullopt;
   float hovered_entity_distance = std::numeric_limits<float>::max();

   if (ImGui::GetIO().WantCaptureMouse or _gizmo.want_capture_mouse()) return;

   world::active_entity_types raycast_mask = _world_hit_mask;

   if (_interaction_targets.creation_entity) {
      if (_entity_creation_context.using_from_object_bbox) {
         raycast_mask = world::active_entity_types{.objects = true, .terrain = false};
      }
      else if (std::holds_alternative<world::planning_connection>(
                  *_interaction_targets.creation_entity)) {
         raycast_mask = world::active_entity_types{.objects = false,
                                                   .planning_hubs = true,
                                                   .terrain = false};
      }
      else if (_entity_creation_context.using_pick_sector) {
         raycast_mask = world::active_entity_types{.objects = false,
                                                   .sectors = true,
                                                   .terrain = false};
      }
   }

   if (raycast_mask.objects) {
      if (std::optional<world::raycast_result<world::object>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                            _world.objects, _object_classes);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            _cursor_surface_normalWS = hit->normalWS;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.lights) {
      if (std::optional<world::raycast_result<world::light>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                            _world.lights);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.paths) {
      if (std::optional<world::raycast_result<world::path>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                            _world.paths);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity =
               world::path_id_node_pair{hit->id, hit->node_index};
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.regions) {
      if (std::optional<world::raycast_result<world::region>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                            _world.regions);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.sectors) {
      if (std::optional<world::raycast_result<world::sector>> hit =
             world::raycast(ray.origin, ray.direction, _world.sectors);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            _cursor_surface_normalWS = hit->normalWS;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.portals) {
      if (std::optional<world::raycast_result<world::portal>> hit =
             world::raycast(ray.origin, ray.direction, _world.portals);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.hintnodes) {
      if (std::optional<world::raycast_result<world::hintnode>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                            _world.hintnodes);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.barriers) {
      if (std::optional<world::raycast_result<world::barrier>> hit =
             world::raycast(ray.origin, ray.direction, _world.barriers,
                            _settings.graphics.barrier_height);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.planning_hubs) {
      if (std::optional<world::raycast_result<world::planning_hub>> hit =
             world::raycast(ray.origin, ray.direction, _world.planning_hubs,
                            _settings.graphics.planning_hub_height);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.planning_connections) {
      if (std::optional<world::raycast_result<world::planning_connection>> hit =
             world::raycast(ray.origin, ray.direction, _world.planning_connections,
                            _world.planning_hubs, _world.planning_hub_index,
                            _settings.graphics.planning_connection_height);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.boundaries) {
      if (std::optional<world::raycast_result<world::boundary>> hit =
             world::raycast(ray.origin, ray.direction, _world.boundaries,
                            _settings.graphics.boundary_height);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (raycast_mask.terrain) {
      if (auto hit = _terrain_collision.raycast(ray.origin, ray.direction); hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = std::nullopt;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (hovered_entity_distance == std::numeric_limits<float>::max()) {
      if (float hit = -(dot(ray.origin, float3{0.0f, 1.0f, 0.0f}) - _editor_floor_height) /
                      dot(ray.direction, float3{0.0f, 1.0f, 0.0f});
          hit > 0.0f) {
         _interaction_targets.hovered_entity = std::nullopt;
         hovered_entity_distance = hit;
      }
   }

   if (hovered_entity_distance != std::numeric_limits<float>::max()) {
      _cursor_positionWS = ray.origin + ray.direction * hovered_entity_distance;
   }

   if (_interaction_targets.creation_entity and _interaction_targets.hovered_entity) {
      const bool from_bbox_wants_hover =
         (_entity_creation_context.using_from_object_bbox and
          std::holds_alternative<world::object_id>(*_interaction_targets.hovered_entity));

      const bool connection_placement_wants_hover =
         (std::holds_alternative<world::planning_connection>(
             *_interaction_targets.creation_entity) and
          std::holds_alternative<world::planning_hub_id>(
             *_interaction_targets.hovered_entity));

      const bool pick_sector_wants_hover = _entity_creation_context.using_pick_sector;

      const bool pick_sector_object_wants_hover =
         (_selection_edit_context.using_add_object_to_sector or
          _selection_edit_context.add_hovered_object) and
         std::holds_alternative<world::object_id>(*_interaction_targets.hovered_entity);

      const bool tool_wants_hover =
         from_bbox_wants_hover or connection_placement_wants_hover or
         pick_sector_wants_hover or pick_sector_object_wants_hover;

      if (not tool_wants_hover) {
         _interaction_targets.hovered_entity = std::nullopt;
      }
   }
}

void world_edit::update_object_classes() noexcept
{
   std::array<std::span<const world::object>, 2> object_spans{_world.objects};

   if (_interaction_targets.creation_entity and
       std::holds_alternative<world::object>(*_interaction_targets.creation_entity)) {
      object_spans[1] =
         std::span{&std::get<world::object>(*_interaction_targets.creation_entity), 1};
   }

   _object_classes.update(object_spans);
}

void world_edit::update_camera(const float delta_time)
{
   float3 camera_position = _camera.position();

   float sprint_factor = 1.0f;

   if (_move_sprint) {
      const float sprint_time =
         std::chrono::duration<float>(std::chrono::steady_clock::now() - _sprint_start)
            .count();
      sprint_factor = std::pow(sprint_time + 1.0f, _settings.camera.sprint_power);
   }
   else {
      _sprint_start = std::chrono::steady_clock::now();
   }

   const float camera_movement_scale =
      delta_time * _settings.camera.move_speed * sprint_factor;

   const bool is_perspective =
      _camera.projection() == graphics::camera_projection::perspective;

   if (_move_camera_forward) {
      const float3 forward = is_perspective ? _camera.forward() : _camera.up();

      camera_position += (forward * camera_movement_scale);
   }
   if (_move_camera_back) {
      const float3 back = is_perspective ? _camera.back() : _camera.down();

      camera_position += (back * camera_movement_scale);
   }
   if (_move_camera_left) {
      camera_position += (_camera.left() * camera_movement_scale);
   }
   if (_move_camera_right) {
      camera_position += (_camera.right() * camera_movement_scale);
   }
   if (_move_camera_up) {
      camera_position += (_camera.up() * camera_movement_scale);
   }
   if (_move_camera_down) {
      camera_position += (_camera.down() * camera_movement_scale);
   }

   if (_pan_camera) {
      const float camera_pan_scale = _settings.camera.pan_sensitivity;

      camera_position += _camera.left() * (_mouse_movement_x * camera_pan_scale);
      camera_position += _camera.up() * (_mouse_movement_y * camera_pan_scale);
   }

   _camera.position(camera_position);

   if (_rotate_camera) {
      const float camera_look_scale = _settings.camera.look_sensitivity;

      _camera.yaw(_camera.yaw() + (-_mouse_movement_x * camera_look_scale));
      _camera.pitch(_camera.pitch() + (-_mouse_movement_y * camera_look_scale));
   }

   if (_rotate_camera or _pan_camera) {
      SetCursor(nullptr);
      SetCursorPos(_rotate_camera_cursor_position.x,
                   _rotate_camera_cursor_position.y);
   }

   if (_camera.fov() != _settings.camera.fov) {
      _camera.fov(_settings.camera.fov);
   }

   if (_camera.view_width() != _settings.camera.view_width) {
      _camera.view_width(_settings.camera.view_width);
   }
}

void world_edit::select_hovered_entity(const select_method method) noexcept
{
   if (method == select_method::single) {
      if (not _interaction_targets.hovered_entity) {
         _interaction_targets.selection.clear();

         return;
      }

      _interaction_targets.selection.clear();
   }
   else if (method == select_method::multi) {
      if (not _interaction_targets.hovered_entity) {
         return;
      }

      for (auto& selected : _interaction_targets.selection) {
         if (selected == _interaction_targets.hovered_entity) return;
      }
   }

   _interaction_targets.selection.push_back(*_interaction_targets.hovered_entity);

   _selection_edit_tool = selection_edit_tool::none;
}

void world_edit::deselect_hovered_entity() noexcept
{
   if (not _interaction_targets.hovered_entity) return;

   std::erase_if(_interaction_targets.selection, [&](const auto& selected) {
      return selected == _interaction_targets.hovered_entity;
   });
}

void world_edit::place_creation_entity() noexcept
{
   if (not _interaction_targets.creation_entity) return;

   std::visit(
      overload{
         [&](const world::object& object) {
            world::object new_object = object;

            new_object.name =
               world::create_unique_name(_world.objects, new_object.name);
            new_object.instance_properties = world::make_object_instance_properties(
               *_object_classes[object.class_name].definition,
               new_object.instance_properties);
            new_object.id = _world.next_id.objects.aquire();

            _last_created_entities.last_object = new_object.id;
            _last_created_entities.last_used_object_classes.insert(new_object.class_name);

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_object)),
                                    _edit_context);

            if (not object.name.empty()) {
               _edit_stack_world.apply(edits::make_set_creation_value(
                                          &world::object::name,
                                          world::create_unique_name(_world.objects,
                                                                    object.name),
                                          object.name),
                                       _edit_context, {.transparent = true});
            }

            if (_entity_creation_config.command_post_auto_place_meta_entities and
                string::iequals(_object_classes[object.class_name]
                                   .definition->header.class_label,
                                "commandpost")) {
               command_post_auto_place_meta_entities(_world.objects.back());
            }

            if (_entity_creation_config.auto_add_object_to_sectors) {
               add_object_to_sectors(_world.objects.back());
            }

            _last_created_entities.last_layer = object.layer;
         },
         [&](const world::light& light) {
            world::light new_light = light;

            new_light.name = world::create_unique_name(_world.lights, new_light.name);
            new_light.id = _world.next_id.lights.aquire();

            if (world::is_region_light(light)) {
               new_light.region_name =
                  world::create_unique_light_region_name(_world.lights, _world.regions,
                                                         light.region_name.empty()
                                                            ? light.name
                                                            : light.region_name);
            }

            _last_created_entities.last_light = new_light.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_light)),
                                    _edit_context);

            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::light::name,
                                       world::create_unique_name(_world.lights,
                                                                 light.name),
                                       light.name),
                                    _edit_context, {.transparent = true});

            if (world::is_region_light(light)) {
               _edit_stack_world.apply(edits::make_set_creation_value(
                                          &world::light::region_name,
                                          world::create_unique_light_region_name(
                                             _world.lights, _world.regions,
                                             light.region_name.empty() ? light.name
                                                                       : light.region_name),
                                          light.region_name),
                                       _edit_context, {.transparent = true});
            }

            _last_created_entities.last_layer = light.layer;
         },
         [&](const world::path& path) {
            if (const world::path* existing_path =
                   world::find_entity(_world.paths, path.name);
                existing_path) {
               if (path.nodes.empty()) std::terminate();

               if (_entity_creation_config.placement_node_insert ==
                   placement_node_insert::nearest) {

                  const world::clostest_node_result closest =
                     find_closest_node(path.nodes[0].position, *existing_path);

                  _edit_stack_world.apply(edits::make_insert_node(existing_path->id,
                                                                  closest.next_is_forward
                                                                     ? closest.index + 1
                                                                     : closest.index,
                                                                  path.nodes[0]),
                                          _edit_context);
               }
               else {
                  _edit_stack_world
                     .apply(edits::make_insert_node(existing_path->id,
                                                    existing_path->nodes.size(),
                                                    path.nodes[0]),
                            _edit_context);
               }
            }
            else {
               world::path new_path = path;

               new_path.name = path.name;
               new_path.id = _world.next_id.paths.aquire();

               _last_created_entities.last_path;

               _edit_stack_world.apply(edits::make_insert_entity(std::move(new_path)),
                                       _edit_context);
            }

            _last_created_entities.last_layer = path.layer;
         },
         [&](const world::region& region) {
            world::region new_region = region;

            new_region.name =
               world::create_unique_name(_world.regions, new_region.name);
            new_region.id = _world.next_id.regions.aquire();

            _last_created_entities.last_region = new_region.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_region)),
                                    _edit_context);

            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::region::name,
                                       world::create_unique_name(_world.regions,
                                                                 region.name),
                                       region.name),
                                    _edit_context, {.transparent = true});

            _last_created_entities.last_layer = region.layer;
         },
         [&](const world::sector& sector) {
            if (sector.points.empty()) std::terminate();

            if (const world::sector* existing_sector =
                   world::find_entity(_world.sectors, sector.name);
                existing_sector and sector.points.size() == 1) {
               _edit_stack_world
                  .apply(edits::make_insert_point(existing_sector->id,
                                                  existing_sector->points.size(),
                                                  sector.points[0]),
                         _edit_context);

               if (_entity_creation_config.auto_fill_sector) {
                  _edit_stack_world.apply(
                     edits::make_set_value(existing_sector->id, &world::sector::objects,
                                           world::sector_fill(*existing_sector,
                                                              _world.objects,
                                                              _object_classes),
                                           existing_sector->objects),
                     _edit_context);
               }
            }
            else {
               world::sector new_sector = sector;

               new_sector.name =
                  world::create_unique_name(_world.sectors, sector.name);
               new_sector.id = _world.next_id.sectors.aquire();

               if (_entity_creation_config.auto_fill_sector) {
                  new_sector.objects =
                     world::sector_fill(new_sector, _world.objects, _object_classes);
               }

               _edit_stack_world.apply(edits::make_insert_entity(std::move(new_sector)),
                                       _edit_context);

               if (sector.points.size() > 1) {
                  _edit_stack_world
                     .apply(edits::make_set_creation_value(&world::sector::points,
                                                           {sector.points[0]},
                                                           sector.points),
                            _edit_context, {.closed = true, .transparent = true});
               }
            }
         },
         [&](const world::portal& portal) {
            world::portal new_portal = portal;

            new_portal.name =
               world::create_unique_name(_world.portals, new_portal.name);
            new_portal.id = _world.next_id.portals.aquire();

            _last_created_entities.last_portal = new_portal.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_portal)),
                                    _edit_context);

            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::portal::name,
                                       world::create_unique_name(_world.portals,
                                                                 portal.name),
                                       portal.name),
                                    _edit_context, {.transparent = true});
         },
         [&](const world::hintnode& hintnode) {
            world::hintnode new_hintnode = hintnode;

            new_hintnode.name =
               world::create_unique_name(_world.hintnodes, new_hintnode.name);
            new_hintnode.id = _world.next_id.hintnodes.aquire();

            _last_created_entities.last_hintnode = new_hintnode.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_hintnode)),
                                    _edit_context);

            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::hintnode::name,
                                       world::create_unique_name(_world.hintnodes,
                                                                 hintnode.name),
                                       hintnode.name),
                                    _edit_context, {.transparent = true});

            _last_created_entities.last_layer = hintnode.layer;
         },
         [&](const world::barrier& barrier) {
            world::barrier new_barrier = barrier;

            new_barrier.name =
               world::create_unique_name(_world.barriers, new_barrier.name);
            new_barrier.id = _world.next_id.barriers.aquire();

            _last_created_entities.last_barrier = new_barrier.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_barrier)),
                                    _edit_context);

            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::barrier::name,
                                       world::create_unique_name(_world.barriers,
                                                                 barrier.name),
                                       barrier.name),
                                    _edit_context, {.transparent = true});
         },
         [&](const world::planning_hub& hub) {
            if (_entity_creation_context.hub_sizing_started) {
               world::planning_hub new_hub = hub;

               new_hub.name =
                  world::create_unique_name(_world.planning_hubs, new_hub.name);
               new_hub.id = _world.next_id.planning_hubs.aquire();

               _last_created_entities.last_planning_hub = new_hub.id;

               _edit_stack_world
                  .apply(edits::make_insert_entity(std::move(new_hub),
                                                   _world.planning_hubs.size()),
                         _edit_context);

               _edit_stack_world.apply(edits::make_set_creation_value(
                                          &world::planning_hub::name,
                                          world::create_unique_name(_world.planning_hubs,
                                                                    hub.name),
                                          hub.name),
                                       _edit_context, {.transparent = true});

               _entity_creation_context.hub_sizing_started = false;
            }
            else {
               _entity_creation_context.hub_sizing_started = true;
            }
         },
         [&](const world::planning_connection& connection) {
            if (not(_interaction_targets.hovered_entity and
                    std::holds_alternative<world::planning_hub_id>(
                       *_interaction_targets.hovered_entity))) {
               return;
            }

            if (_entity_creation_context.connection_link_started) {
               world::planning_connection new_connection = connection;

               new_connection.name =
                  world::create_unique_name(_world.planning_connections,
                                            new_connection.name);
               new_connection.id = _world.next_id.planning_connections.aquire();

               _last_created_entities.last_planning_connection = new_connection.id;

               _edit_stack_world.apply(edits::make_insert_entity(std::move(new_connection)),
                                       _edit_context);

               _edit_stack_world.apply(edits::make_set_creation_value(
                                          &world::planning_connection::name,
                                          world::create_unique_name(_world.planning_connections,
                                                                    connection.name),
                                          connection.name),
                                       _edit_context, {.transparent = true});

               _entity_creation_context.connection_link_started = false;
            }
            else {
               _edit_stack_world.apply(edits::make_set_creation_value(
                                          &world::planning_connection::start,
                                          std::get<world::planning_hub_id>(
                                             *_interaction_targets.hovered_entity),
                                          connection.start),
                                       _edit_context);

               _entity_creation_context.connection_link_started = true;
            }
         },
         [&](const world::boundary& boundary) {
            world::boundary new_boundary = boundary;

            new_boundary.name =
               world::create_unique_name(_world.boundaries, new_boundary.name);
            new_boundary.id = _world.next_id.boundaries.aquire();

            _last_created_entities.last_boundary = new_boundary.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_boundary)),
                                    _edit_context);

            _edit_stack_world.apply(edits::make_set_creation_value(
                                       &world::boundary::name,
                                       world::create_unique_name(_world.boundaries,
                                                                 boundary.name),
                                       boundary.name),
                                    _edit_context, {.transparent = true});
         },
      },
      *_interaction_targets.creation_entity);

   _entity_creation_config.placement_mode = placement_mode::cursor;
}

void world_edit::command_post_auto_place_meta_entities(const world::object& object) noexcept
{
   world::command_post_linked_entities command_post_linked_entities =
      world::make_command_post_linked_entities(
         {.name = object.name,
          .layer = object.layer,
          .position = object.position,
          .capture_radius = _entity_creation_config.command_post_capture_radius,
          .control_radius = _entity_creation_config.command_post_control_radius,
          .control_height = _entity_creation_config.command_post_control_height,
          .spawn_radius = _entity_creation_config.command_post_spawn_radius},
         _world.objects, _world.paths, _world.regions, _object_classes,
         _terrain_collision);

   for (std::size_t i = 0; i < object.instance_properties.size(); ++i) {
      const auto& [key, value] = object.instance_properties[i];
      if (string::iequals(key, "CaptureRegion")) {
         _edit_stack_world.apply(edits::make_set_instance_property_value(
                                    object.id, i,
                                    command_post_linked_entities.capture_region.description,
                                    value),
                                 _edit_context,
                                 {.closed = true, .transparent = true});
      }
      else if (string::iequals(key, "ControlRegion")) {
         _edit_stack_world.apply(edits::make_set_instance_property_value(
                                    object.id, i,
                                    command_post_linked_entities.control_region.description,
                                    value),
                                 _edit_context,
                                 {.closed = true, .transparent = true});
      }
      else if (string::iequals(key, "SpawnPath")) {
         _edit_stack_world.apply(edits::make_set_instance_property_value(
                                    object.id, i,
                                    command_post_linked_entities.spawn_path.name, value),
                                 _edit_context,
                                 {.closed = true, .transparent = true});
      }
   }

   command_post_linked_entities.capture_region.id = _world.next_id.regions.aquire();
   command_post_linked_entities.control_region.id = _world.next_id.regions.aquire();
   command_post_linked_entities.spawn_path.id = _world.next_id.paths.aquire();

   _edit_stack_world.apply(edits::make_insert_entity(std::move(
                              command_post_linked_entities.capture_region)),
                           _edit_context, {.transparent = true});
   _edit_stack_world.apply(edits::make_insert_entity(std::move(
                              command_post_linked_entities.control_region)),
                           _edit_context, {.transparent = true});
   _edit_stack_world.apply(edits::make_insert_entity(
                              std::move(command_post_linked_entities.spawn_path)),
                           _edit_context, {.transparent = true});
}

void world_edit::add_object_to_sectors(const world::object& object) noexcept
{
   if (object.name.empty()) return;

   for (const auto& sector : _world.sectors) {
      if (not world::inside_sector(sector, object, _object_classes)) continue;

      bool already_exists = false;

      for (const auto& sector_object : sector.objects) {
         if (string::iequals(sector_object, object.name)) {
            already_exists = true;
            break;
         }
      }

      if (already_exists) continue;

      _edit_stack_world.apply(edits::make_add_sector_object(sector.id, object.name),
                              _edit_context, {.transparent = true});
   }
}

void world_edit::cycle_creation_entity_object_class() noexcept
{
   if (not _interaction_targets.creation_entity or
       not std::holds_alternative<world::object>(*_interaction_targets.creation_entity)) {
      return;
   }

   if (_last_created_entities.last_used_object_classes.empty()) return;

   const lowercase_string& class_name =
      _last_created_entities.last_used_object_classes
         [(_entity_creation_config.cycle_object_class_index++) %
          _last_created_entities.last_used_object_classes.size()];

   _edit_stack_world
      .apply(edits::make_set_creation_value(
                &world::object::class_name, class_name,
                std::get<world::object>(*_interaction_targets.creation_entity).class_name),
             _edit_context, {.closed = true});
}

void world_edit::undo() noexcept
{
   _edit_stack_world.revert(_edit_context);

   if (_interaction_targets.creation_entity) {
      _entity_creation_context = {};
      _entity_creation_config.placement_mode = placement_mode::manual;
      _entity_creation_config.placement_rotation =
         _entity_creation_config.placement_rotation == placement_rotation::surface
            ? placement_rotation::manual_euler
            : _entity_creation_config.placement_rotation;
   }

   _selection_edit_tool = selection_edit_tool::none;
}

void world_edit::redo() noexcept
{
   _edit_stack_world.reapply(_edit_context);
}

void world_edit::delete_selected() noexcept
{
   if (_interaction_targets.creation_entity) {
      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::nullopt,
                                                _interaction_targets.creation_entity),
                _edit_context);

      return;
   }

   for (const auto& generic_selected : _interaction_targets.selection) {
      std::visit(
         [&](const auto selected) {
            _edit_stack_world.apply(edits::make_delete_entity(selected, _world),
                                    _edit_context);
         },
         generic_selected);
   }

   _interaction_targets.selection.clear();
}

void world_edit::align_selection() noexcept
{
   if (_interaction_targets.selection.empty()) return;

   edits::bundle_vector bundle;

   const float alignment = _world.terrain.grid_scale;

   const auto align_position = [=](const float3 position) {
      return float3{std::round(position.x / alignment) * alignment, position.y,
                    std::round(position.z / alignment) * alignment};
   };

   for (const auto& selected : _interaction_targets.selection) {
      if (std::holds_alternative<world::object_id>(selected)) {
         const world::object* object =
            world::find_entity(_world.objects, std::get<world::object_id>(selected));

         if (object) {
            bundle.push_back(edits::make_set_value(object->id, &world::object::position,
                                                   align_position(object->position),
                                                   object->position));
         }
      }
      else if (std::holds_alternative<world::path_id_node_pair>(selected)) {
         const auto [id, node_index] = std::get<world::path_id_node_pair>(selected);

         const world::path* path = world::find_entity(_world.paths, id);

         if (path) {
            const world::path::node& node = path->nodes[node_index];

            bundle.push_back(
               edits::make_set_path_node_value(path->id, node_index,
                                               &world::path::node::position,
                                               align_position(node.position),
                                               node.position));
         }
      }
      else if (std::holds_alternative<world::light_id>(selected)) {
         const world::light* light =
            world::find_entity(_world.lights, std::get<world::light_id>(selected));

         if (light) {
            bundle.push_back(edits::make_set_value(light->id, &world::light::position,
                                                   align_position(light->position),
                                                   light->position));
         }
      }
      else if (std::holds_alternative<world::region_id>(selected)) {
         const world::region* region =
            world::find_entity(_world.regions, std::get<world::region_id>(selected));

         if (region) {
            bundle.push_back(edits::make_set_value(region->id, &world::region::position,
                                                   align_position(region->position),
                                                   region->position));
         }
      }
      else if (std::holds_alternative<world::sector_id>(selected)) {
         const world::sector* sector =
            world::find_entity(_world.sectors, std::get<world::sector_id>(selected));

         if (sector) {
            std::vector<float2> new_points = sector->points;

            for (auto& point : new_points) {
               point = round(point / alignment) * alignment;
            }

            bundle.push_back(
               edits::make_set_value(sector->id, &world::sector::points,
                                     std::move(new_points), sector->points));
         }
      }
      else if (std::holds_alternative<world::portal_id>(selected)) {
         const world::portal* portal =
            world::find_entity(_world.portals, std::get<world::portal_id>(selected));

         if (portal) {
            bundle.push_back(edits::make_set_value(portal->id, &world::portal::position,
                                                   align_position(portal->position),
                                                   portal->position));
         }
      }
      else if (std::holds_alternative<world::hintnode_id>(selected)) {
         const world::hintnode* hintnode =
            world::find_entity(_world.hintnodes,
                               std::get<world::hintnode_id>(selected));

         if (hintnode) {
            bundle.push_back(
               edits::make_set_value(hintnode->id, &world::hintnode::position,
                                     align_position(hintnode->position),
                                     hintnode->position));
         }
      }
      else if (std::holds_alternative<world::barrier_id>(selected)) {
         const world::barrier* barrier =
            world::find_entity(_world.barriers, std::get<world::barrier_id>(selected));

         if (barrier) {
            bundle.push_back(edits::make_set_value(barrier->id, &world::barrier::position,
                                                   align_position(barrier->position),
                                                   barrier->position));
         }
      }
      else if (std::holds_alternative<world::planning_hub_id>(selected)) {
         const world::planning_hub* planning_hub =
            world::find_entity(_world.planning_hubs,
                               std::get<world::planning_hub_id>(selected));

         if (planning_hub) {
            bundle.push_back(
               edits::make_set_value(planning_hub->id, &world::planning_hub::position,
                                     align_position(planning_hub->position),
                                     planning_hub->position));
         }
      }
      else if (std::holds_alternative<world::boundary_id>(selected)) {
         const world::boundary* boundary =
            world::find_entity(_world.boundaries,
                               std::get<world::boundary_id>(selected));

         if (boundary) {
            bundle.push_back(
               edits::make_set_value(boundary->id, &world::boundary::position,
                                     round(boundary->position / alignment) * alignment,
                                     boundary->position));
         }
      }
   }

   if (bundle.size() == 1) {
      _edit_stack_world.apply(std::move(bundle.back()), _edit_context,
                              {.closed = true});
   }
   else if (not bundle.empty()) {
      _edit_stack_world.apply(edits::make_bundle(std::move(bundle)),
                              _edit_context, {.closed = true});
   }
}

void world_edit::new_entity_from_selection() noexcept
{
   if (_interaction_targets.selection.empty()) return;

   auto& selected = _interaction_targets.selection.front();

   if (std::holds_alternative<world::object_id>(selected)) {
      world::object* object =
         world::find_entity(_world.objects, std::get<world::object_id>(selected));

      if (not object) return;

      world::object new_object = *object;

      new_object.name = world::create_unique_name(_world.objects, new_object.name);
      new_object.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_object),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.objects = true;
   }
   else if (std::holds_alternative<world::light_id>(selected)) {
      world::light* light =
         world::find_entity(_world.lights, std::get<world::light_id>(selected));

      if (not light) return;

      world::light new_light = *light;

      new_light.name = world::create_unique_name(_world.lights, new_light.name);
      new_light.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_light),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.lights = true;
   }
   else if (std::holds_alternative<world::path_id_node_pair>(selected)) {
      auto [id, node_index] = std::get<world::path_id_node_pair>(selected);

      world::path* path = world::find_entity(_world.paths, id);

      if (not path) return;

      world::path new_path = *path;

      new_path.name = world::create_unique_name(_world.paths, new_path.name);
      new_path.nodes = {world::path::node{}};
      new_path.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_path),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.paths = true;
   }
   else if (std::holds_alternative<world::region_id>(selected)) {
      world::region* region =
         world::find_entity(_world.regions, std::get<world::region_id>(selected));

      if (not region) return;

      world::region new_region = *region;

      new_region.name = world::create_unique_name(_world.regions, new_region.name);
      new_region.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_region),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.regions = true;
   }
   else if (std::holds_alternative<world::sector_id>(selected)) {
      world::sector* sector =
         world::find_entity(_world.sectors, std::get<world::sector_id>(selected));

      if (not sector) return;

      world::sector new_sector = *sector;

      new_sector.name = world::create_unique_name(_world.sectors, new_sector.name);
      new_sector.points = {{0.0f, 0.0f}};
      new_sector.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_sector),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.sectors = true;
   }
   else if (std::holds_alternative<world::portal_id>(selected)) {
      world::portal* portal =
         world::find_entity(_world.portals, std::get<world::portal_id>(selected));

      if (not portal) return;

      world::portal new_portal = *portal;

      new_portal.name = world::create_unique_name(_world.portals, new_portal.name);
      new_portal.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_portal),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.portals = true;
   }
   else if (std::holds_alternative<world::hintnode_id>(selected)) {
      world::hintnode* hintnode =
         world::find_entity(_world.hintnodes, std::get<world::hintnode_id>(selected));

      if (not hintnode) return;

      world::hintnode new_hintnode = *hintnode;

      new_hintnode.name =
         world::create_unique_name(_world.hintnodes, new_hintnode.name);
      new_hintnode.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_hintnode),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.hintnodes = true;
   }
   else if (std::holds_alternative<world::barrier_id>(selected)) {
      world::barrier* barrier =
         world::find_entity(_world.barriers, std::get<world::barrier_id>(selected));

      if (not barrier) return;

      world::barrier new_barrier = *barrier;

      new_barrier.name = world::create_unique_name(_world.barriers, new_barrier.name);
      new_barrier.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_barrier),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.barriers = true;
   }
   else if (std::holds_alternative<world::planning_hub_id>(selected)) {
      world::planning_hub* hub =
         world::find_entity(_world.planning_hubs,
                            std::get<world::planning_hub_id>(selected));

      if (not hub) return;

      world::planning_hub new_hub = *hub;

      new_hub.name = world::create_unique_name(_world.planning_hubs, new_hub.name);
      new_hub.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_hub),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.planning_hubs = true;
      _world_draw_mask.planning_connections = true;
   }
   else if (std::holds_alternative<world::planning_connection_id>(selected)) {
      world::planning_connection* connection =
         world::find_entity(_world.planning_connections,
                            std::get<world::planning_connection_id>(selected));

      if (not connection) return;

      world::planning_connection new_connection = *connection;

      new_connection.name =
         world::create_unique_name(_world.planning_connections, new_connection.name);
      new_connection.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_connection),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.planning_hubs = true;
      _world_draw_mask.planning_connections = true;
   }
   else if (std::holds_alternative<world::boundary_id>(selected)) {
      world::boundary* boundary =
         world::find_entity(_world.boundaries, std::get<world::boundary_id>(selected));

      if (not boundary) return;

      world::boundary new_boundary = *boundary;

      new_boundary.name =
         world::create_unique_name(_world.boundaries, new_boundary.name);
      new_boundary.id = world::max_id;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::move(new_boundary),
                                                _interaction_targets.creation_entity),
                _edit_context);
      _world_draw_mask.boundaries = true;
   }

   _entity_creation_context = {};
}

void world_edit::ask_to_save_world() noexcept
{
   if (_world_path.empty()) return;
   if (not _edit_stack_world.modified_flag()) return;

   int result =
      MessageBoxW(_window, L"You have unsaved changes in your current world. They'll be lost once the world is closed.\n\nSave them?",
                  L"Unsaved Changes", MB_YESNO);

   if (result == IDYES) save_world(_world_path);
}

void world_edit::open_project(std::filesystem::path path) noexcept
{
   if (not std::filesystem::exists(path / L"Worlds")) {
      if (MessageBoxW(_window, L"The selected folder does not appear to be a mod data folder. Are you sure you wish to open it?",
                      L"Not a Data Folder", MB_YESNO) != IDYES) {
         return;
      }
   }

   _project_dir = path;
   _asset_libraries.source_directory(_project_dir);
   _project_world_paths.clear();

   close_world();
   enumerate_project_worlds();
}

void world_edit::open_project_with_picker() noexcept
{
   static constexpr GUID open_project_picker_guid = {0xe66983ff,
                                                     0x54e0,
                                                     0x4520,
                                                     {0x9c, 0xb5, 0xa0, 0x71,
                                                      0x65, 0x1d, 0x42, 0xa4}};

   auto path = utility::show_folder_picker({.title = L"Open Data Folder",
                                            .ok_button_label = L"Open",
                                            .picker_guid = open_project_picker_guid,
                                            .window = _window});

   if (not path or not std::filesystem::exists(*path)) return;

   open_project(*path);
}

void world_edit::load_world(std::filesystem::path path) noexcept
{
   if (not std::filesystem::exists(path)) return;

   close_world();

   try {
      _world = world::load_world(path, _stream);
      _world_path = path;
      _terrain_collision = world::terrain_collision{_world.terrain};

      _camera.position({0.0f, 0.0f, 0.0f});
      _camera.pitch(0.0f);
      _camera.yaw(0.0f);

      SetWindowTextA(_window,
                     fmt::format("WorldEdit - {}", _world_path.filename().string())
                        .c_str());
      _window_unsaved_star = false;
   }
   catch (std::exception& e) {
      _stream.write(fmt::format("Failed to load world '{}'! Reason: {}",
                                path.filename().string(), e.what()));
   }
}

void world_edit::load_world_with_picker() noexcept
{
   static constexpr GUID load_world_picker_guid = {0xa552d07d,
                                                   0xb0c9,
                                                   0x4aca,
                                                   {0x8e, 0x49, 0xdc, 0xe9,
                                                    0x96, 0x51, 0xa1, 0x3e}};

   auto path = utility::show_file_open_picker(
      {.title = L"Load World"s,
       .ok_button_label = L"Load"s,
       .forced_start_folder = _project_dir,
       .filters = {utility::file_picker_filter{.name = L"World"s, .filter = L"*.wld"s}},
       .picker_guid = load_world_picker_guid,
       .window = _window,
       .must_exist = true});

   if (not path) return;

   load_world(*path);
}

void world_edit::save_world(std::filesystem::path path) noexcept
{
   try {
      if (not std::filesystem::exists(path.parent_path())) {
         std::filesystem::create_directories(path.parent_path());
      }

      world::save_world(path, _world);

      _edit_stack_world.clear_modified_flag();
   }
   catch (std::exception& e) {
      auto message =
         fmt::format("Failed to save world!\n   Reason: \n{}\n"
                     "   Incomplete save data maybe present on disk.\n",
                     string::indent(2, e.what()));

      _stream.write(message);

      MessageBoxA(_window, message.data(), "Failed to save world!", MB_OK);
   }
}

void world_edit::save_world_with_picker() noexcept
{
   static constexpr GUID save_world_picker_guid = {0xe458b1ee,
                                                   0xf22a,
                                                   0x4a19,
                                                   {0x8a, 0xed, 0xa0, 0x77,
                                                    0x98, 0xa3, 0xb0, 0x53}};

   auto path = utility::show_file_save_picker(
      {.title = L"Save World"s,
       .ok_button_label = L"Save"s,
       .forced_start_folder = _world_path,
       .filters = {utility::file_picker_filter{.name = L"World"s, .filter = L"*.wld"s}},
       .picker_guid = save_world_picker_guid,
       .window = _window,
       .must_exist = true});

   if (not path) return;

   save_world(*path);
}

void world_edit::close_world() noexcept
{
   ask_to_save_world();

   _object_classes.clear();
   _world = {};
   _interaction_targets = {};
   _entity_creation_context = {};
   _world_draw_mask = {};
   _world_hit_mask = {};
   _world_layers_draw_mask = {true};
   _world_layers_hit_mask = {true};
   _world_path.clear();

   _terrain_collision = {};

   _edit_stack_world.clear();
   _edit_stack_world.clear_modified_flag();

   _renderer->mark_dirty_terrain();

   SetWindowTextA(_window, "WorldEdit");
   _window_unsaved_star = false;
}

void world_edit::enumerate_project_worlds() noexcept
{
   try {
      for (auto& file : std::filesystem::recursive_directory_iterator{
              _project_dir / L"Worlds"}) {
         if (not file.is_regular_file()) continue;

         auto extension = file.path().extension();
         constexpr auto wld_extension = L".wld"sv;

         if (CompareStringEx(LOCALE_NAME_INVARIANT, NORM_IGNORECASE,
                             extension.native().c_str(), (int)extension.native().size(),
                             wld_extension.data(), (int)wld_extension.size(),
                             nullptr, nullptr, 0) == CSTR_EQUAL) {
            _project_world_paths.push_back(file.path());
         }
      }
   }
   catch (std::filesystem::filesystem_error&) {
      MessageBoxW(_window, L"Unable to enumerate data folder worlds. Loading worlds will require manual navigation.",
                  L"Error", MB_OK);
   }
}

void world_edit::open_odfs_for_selected() noexcept
{
   for (auto& selected : _interaction_targets.selection) {
      if (not std::holds_alternative<world::object_id>(selected)) continue;

      const world::object* object =
         world::find_entity(_world.objects, std::get<world::object_id>(selected));

      if (not object) continue;

      open_odf_in_text_editor(object->class_name);
   }
}

void world_edit::open_odf_in_text_editor(const lowercase_string& asset_name) noexcept
{
   const auto asset_path = _asset_libraries.odfs.query_path(asset_name);

   if (asset_path.empty()) return;

   const std::string command_line =
      fmt::format("{} {}", _settings.preferences.text_editor, asset_path.string());

   if (not utility::os_execute_async(command_line)) {
      MessageBoxA(_window,
                  fmt::format("Unable to execute command line '{}'.", command_line)
                     .c_str(),
                  "Failed to open .odf editor!", MB_OK);
   }
}

void world_edit::resized(uint16 width, uint16 height)
{
   if (width == 0 or height == 0) return;

   _camera.aspect_ratio(float(width) / float(height));

   try {
      _renderer->window_resized(width, height);
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }

   update();
}

void world_edit::focused()
{
   _focused = true;
}

void world_edit::unfocused()
{
   _focused = false;
   _hotkeys.clear_state();
}

bool world_edit::idling() const noexcept
{
   return not _focused;
}

bool world_edit::can_close() const noexcept
{
   if (not _edit_stack_world.modified_flag()) return true;

   int result =
      MessageBoxW(_window, L"You have unsaved changes in your world. If you quit now you will lose them.\n\nAre you sure you want to quit?",
                  L"Unsaved Changes", MB_YESNO | MB_DEFBUTTON2);

   return result == IDYES;
}

void world_edit::key_down(const key key) noexcept
{
   _hotkeys.notify_key_down(key);
}

void world_edit::key_up(const key key) noexcept
{
   _hotkeys.notify_key_up(key);
}

void world_edit::mouse_movement(const int32 x_movement, const int32 y_movement) noexcept
{
   _queued_mouse_movement_x += x_movement;
   _queued_mouse_movement_y += y_movement;
}

void world_edit::dpi_changed(const int new_dpi) noexcept
{
   _current_dpi = new_dpi;
   _display_scale = (_current_dpi / 96.0f) * _settings.ui.extra_scaling;
   _applied_user_display_scale = _settings.ui.extra_scaling;

   const static std::span<std::byte> roboto_regular = [] {
      const HRSRC resource =
         FindResourceW(nullptr, MAKEINTRESOURCEW(RES_ID_ROBOTO_REGULAR),
                       MAKEINTRESOURCEW(RES_TYPE_BINARY));

      return std::span{static_cast<std::byte*>(
                          LockResource(LoadResource(nullptr, resource))),
                       SizeofResource(nullptr, resource)};
   }();

   ImFontConfig font_config{};
   font_config.FontDataOwnedByAtlas = false;

   ImGui::GetIO().Fonts->Clear();
   ImGui::GetIO().Fonts->AddFontFromMemoryTTF(roboto_regular.data(),
                                              static_cast<int>(roboto_regular.size()),
                                              std::floor(16.0f * _display_scale),
                                              &font_config);
   ImGui::GetStyle() = ImGuiStyle{};
   ImGui::GetStyle().ScaleAllSizes(_display_scale);

   try {
      _renderer->recreate_imgui_font_atlas();
      _renderer->display_scale_changed(_display_scale);
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }
}

void world_edit::handle_gpu_error(graphics::gpu::exception& e) noexcept
{
   using graphics::gpu::error;

   switch (e.error()) {
   case error::out_of_memory: {
      switch (MessageBoxA(_window,
                          fmt::format("Run out of GPU Memory: {}\n\nThe editor "
                                      "will now exit.\n\nSave world?",
                                      e.what())
                             .c_str(),
                          "GPU Memory Exhausted", MB_YESNO | MB_ICONERROR)) {
      case IDYES:
         save_world_with_picker();
         [[fallthrough]];
      case IDNO:
      default:
         std::terminate();
      }
   } break;
   case error::device_removed: {
      _renderer = nullptr;

      try {
         _renderer =
            graphics::make_renderer({.window = _window,
                                     .thread_pool = _thread_pool,
                                     .asset_libraries = _asset_libraries,
                                     .error_output = _stream,
                                     .use_debug_layer = _renderer_use_debug_layer});

         _renderer->recreate_imgui_font_atlas();
         _renderer->display_scale_changed(_display_scale);
      }
      catch (graphics::gpu::exception& e) {
         if (e.error() != error::device_removed) {
            return handle_gpu_error(e);
         }
      }

   } break;
   case error::no_suitable_device: {
      switch (MessageBoxA(
         _window,
         fmt::format("{}\n\nIt is possible the GPU "
                     "being used was removed and the remaining ones do not met "
                     "requirements.\n\nThe editor "
                     "will now exit.\n\nSave world?",
                     e.what())
            .c_str(),
         "No Suitable GPU", MB_YESNO | MB_ICONERROR)) {
      case IDYES:
         save_world_with_picker();
         [[fallthrough]];
      case IDNO:
      default:
         std::terminate();
      }
   } break;
   }
}
}
