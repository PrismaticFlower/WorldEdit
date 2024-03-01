
#include "world_edit.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/odf/default_object_class_definition.hpp"
#include "assets/texture/save_env_map.hpp"
#include "edits/add_sector_object.hpp"
#include "edits/bundle.hpp"
#include "edits/creation_entity_set.hpp"
#include "edits/delete_entity.hpp"
#include "edits/insert_entity.hpp"
#include "edits/insert_node.hpp"
#include "edits/insert_point.hpp"
#include "edits/set_value.hpp"
#include "graphics/frustum.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "resource.h"
#include "utility/file_pickers.hpp"
#include "utility/os_execute.hpp"
#include "utility/overload.hpp"
#include "utility/show_in_explorer.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"
#include "world/io/load.hpp"
#include "world/io/save.hpp"
#include "world/utility/grounding.hpp"
#include "world/utility/make_command_post_linked_entities.hpp"
#include "world/utility/mouse_pick_measurement.hpp"
#include "world/utility/object_properties.hpp"
#include "world/utility/raycast.hpp"
#include "world/utility/raycast_terrain.hpp"
#include "world/utility/sector_fill.hpp"
#include "world/utility/selection_bbox.hpp"
#include "world/utility/terrain_cut.hpp"
#include "world/utility/world_utilities.hpp"

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
     _renderer_use_debug_layer{command_line.get_flag("-gpu_debug_layer")},
     _renderer_use_legacy_barriers{
        command_line.get_flag("-gpu_legacy_barriers")},
     _renderer_never_use_shader_model_6_6{
        command_line.get_flag("-gpu_no_shader_model_6_6")}
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

   _current_dpi = GetDpiForWindow(_window);
   _display_scale.value = (_current_dpi / 96.0f) * _settings.ui.extra_scaling;
   _applied_user_display_scale = _settings.ui.extra_scaling;

   try {
      _renderer = graphics::make_renderer(
         {.window = window,
          .thread_pool = _thread_pool,
          .asset_libraries = _asset_libraries,
          .error_output = _stream,
          .display_scale = _display_scale.value,
          .use_debug_layer = _renderer_use_debug_layer,
          .use_legacy_barriers = _renderer_use_legacy_barriers,
          .never_use_shader_model_6_6 = _renderer_never_use_shader_model_6_6});
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }

   initialize_commands();
   initialize_hotkeys();

   ImGui_ImplWin32_Init(window);

   initialize_imgui_font();
   initialize_imgui_style();

   if (auto start_project = command_line.get_or("-project"sv, ""sv);
       not start_project.empty()) {
      open_project(start_project);
   }

   if (auto start_world = command_line.get_or("-world"sv, ""sv);
       not start_world.empty()) {
      load_world(start_world);
   }

   RECT rect{};
   GetClientRect(window, &rect);

   _camera.aspect_ratio(static_cast<float>(rect.right - rect.left) /
                        static_cast<float>(rect.bottom - rect.top));

   _settings = settings_load.get();
}

world_edit::~world_edit()
{
   if (not _project_dir.empty()) close_project();
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

   _tool_visualizers.clear();

   if (not _rotate_camera and not _pan_camera) {
      _gizmo.update(make_camera_ray(_camera,
                                    {ImGui::GetMousePos().x,
                                     ImGui::GetMousePos().y},
                                    {ImGui::GetMainViewport()->Size.x,
                                     ImGui::GetMainViewport()->Size.y}),
                    ImGui::IsKeyDown(ImGuiKey_MouseLeft) and
                       not ImGui::GetIO().WantCaptureMouse,
                    _camera, _settings.ui.gizmo_scale);
   }

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

   ui_draw_select_box();

   try {
      _renderer->draw_frame(_camera, _world, _interaction_targets,
                            _world_draw_mask, _world_layers_draw_mask,
                            _tool_visualizers, _object_classes,
                            {
                               .draw_terrain_grid = _draw_terrain_grid,
                               .draw_overlay_grid = _draw_overlay_grid,
                               .overlay_grid_height = _editor_floor_height,
                               .overlay_grid_size = _editor_grid_size,
                            },
                            _settings.graphics);

      if (_env_map_render_requested) {
         _env_map_render_requested = false;

         graphics::env_map_result env_map =
            _renderer->draw_env_map(_env_map_render_params, _world,
                                    _world_layers_draw_mask, _object_classes);

         try {
            assets::texture::save_env_map(_env_map_save_path,
                                          {.length = env_map.length,
                                           .row_pitch = env_map.row_pitch,
                                           .item_pitch = env_map.item_pitch,
                                           .data = {env_map.data.get(),
                                                    env_map.item_pitch * 6}});

            _env_map_save_error.clear();
         }
         catch (std::runtime_error& e) {
            _env_map_save_error = e.what();
         }
      }

      _world.terrain.untracked_clear_dirty_rects();
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);

      ImGui::EndFrame();
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
   if (_rotate_camera or _pan_camera) return;
   if (_terrain_editor_open) return;

   world::active_entity_types raycast_mask = _world_hit_mask;

   if (_interaction_targets.creation_entity.holds_entity()) {
      if (_entity_creation_context.using_from_object_bbox) {
         raycast_mask = world::active_entity_types{.objects = true, .terrain = false};
      }
      else if (_interaction_targets.creation_entity.is<world::planning_connection>()) {
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
                            _world.paths, _settings.graphics.path_node_size);
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
             world::raycast(ray.origin, ray.direction,
                            _world.planning_connections, _world.planning_hubs,
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

   if (raycast_mask.measurements) {
      if (std::optional<world::measurement_id> hit =
             world::mouse_pick(std::bit_cast<float2>(ImGui::GetMousePos()),
                               std::bit_cast<float2>(ImGui::GetMainViewport()->Size),
                               _camera.view_projection_matrix(), _world.measurements);
          hit) {
         _interaction_targets.hovered_entity = *hit;
         hovered_entity_distance = 0.0f;
      }
   }

   if (raycast_mask.terrain and _world.terrain.active_flags.terrain) {
      if (auto hit = world::raycast(ray.origin, ray.direction, _world.terrain); hit) {
         if (*hit < hovered_entity_distance) {
            if (not raycast_mask.objects or
                not world::point_inside_terrain_cut(ray.origin + ray.direction * *hit,
                                                    ray.direction, _world_layers_hit_mask,
                                                    _world.objects, _object_classes)) {
               _interaction_targets.hovered_entity = std::nullopt;
               hovered_entity_distance = *hit;
            }
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

   if (_interaction_targets.creation_entity.holds_entity() and
       _interaction_targets.hovered_entity) {
      const bool from_bbox_wants_hover =
         (_entity_creation_context.using_from_object_bbox and
          std::holds_alternative<world::object_id>(*_interaction_targets.hovered_entity));

      const bool connection_placement_wants_hover =
         _interaction_targets.creation_entity.is<world::planning_connection>() and
         std::holds_alternative<world::planning_hub_id>(
            *_interaction_targets.hovered_entity);

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

   if (_interaction_targets.creation_entity.is<world::object>()) {
      object_spans[1] =
         std::span{&_interaction_targets.creation_entity.get<world::object>(), 1};
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

   if (_orbit_camera_active) {
      if (_move_camera_forward) _camera_orbit_distance -= camera_movement_scale;
      if (_move_camera_back) _camera_orbit_distance += camera_movement_scale;
      if (_camera_orbit_distance < 0.0f) _camera_orbit_distance = 0.0f;

      _camera.position(_camera_orbit_centre + _camera_orbit_distance * _camera.back());
   }

   if (_camera.fov() != _settings.camera.fov) {
      _camera.fov(_settings.camera.fov);
   }

   if (_camera.view_width() != _settings.camera.view_width) {
      _camera.view_width(_settings.camera.view_width);
   }
}

void world_edit::setup_orbit_camera() noexcept
{
   if (not _interaction_targets.selection.empty()) {
      const math::bounding_box selection_bbox =
         world::selection_bbox_for_camera(_world, _interaction_targets.selection,
                                          _object_classes,
                                          _settings.graphics.path_node_size);

      const float3 bounding_sphere_centre =
         (selection_bbox.max + selection_bbox.min) / 2.0f;
      const float bounding_sphere_radius =
         distance(selection_bbox.max, selection_bbox.min) / 2.0f;

      _camera_orbit_centre = bounding_sphere_centre;
      _camera_orbit_distance = bounding_sphere_radius;
   }
   else if (not _world.objects.empty()) {
      math::bounding_box objects_bbox{.min = float3{FLT_MAX, FLT_MAX, FLT_MAX},
                                      .max = float3{-FLT_MAX, -FLT_MAX, -FLT_MAX}};

      for (const auto& object : _world.objects) {
         if (object.hidden) continue;
         if (not _world_layers_draw_mask[object.layer]) continue;

         math::bounding_box bbox =
            _object_classes[object.class_name].model->bounding_box;

         bbox = object.rotation * bbox + object.position;

         objects_bbox = math::combine(bbox, objects_bbox);
      }

      const bool active_object = objects_bbox.min.x != FLT_MAX;

      if (active_object) {
         const float3 bounding_sphere_centre =
            (objects_bbox.max + objects_bbox.min) / 2.0f;
         const float bounding_sphere_radius =
            distance(objects_bbox.max, objects_bbox.min) / 2.0f;

         _camera_orbit_centre = bounding_sphere_centre;
         _camera_orbit_distance = bounding_sphere_radius;
      }
      else {
         _camera_orbit_centre = float3{0.0f, 0.0f, 0.0f};
         _camera_orbit_distance =
            (_world.terrain.length * _world.terrain.grid_scale) / 2.0f;
      }
   }
   else {
      _camera_orbit_centre = float3{0.0f, 0.0f, 0.0f};
      _camera_orbit_distance =
         (_world.terrain.length * _world.terrain.grid_scale) / 2.0f;
   }
}

void world_edit::ui_draw_select_box() noexcept
{
   if (not _selecting_entity) return;

   const float2 current_cursor_position = {ImGui::GetIO().MousePos.x,
                                           ImGui::GetIO().MousePos.y};
   const float2 rect_min = min(current_cursor_position, _select_start_position);
   const float2 rect_max = max(current_cursor_position, _select_start_position);

   if (distance(rect_max, rect_min) >= (8.0f * _display_scale)) {
      const float3 color =
         utility::compress_srgb(_settings.graphics.hover_color) * 255.0f + 0.5f;

      ImGui::GetBackgroundDrawList()->AddRectFilled(std::bit_cast<ImVec2>(rect_min),
                                                    std::bit_cast<ImVec2>(rect_max),
                                                    IM_COL32(color.x, color.y,
                                                             color.z, 0x10));
      ImGui::GetBackgroundDrawList()->AddRect(std::bit_cast<ImVec2>(rect_min),
                                              std::bit_cast<ImVec2>(rect_max),
                                              IM_COL32(color.x, color.y, color.z, 0xff),
                                              0.0f, ImDrawFlags_None,
                                              1.0f * _display_scale);
   }
}

void world_edit::start_entity_select() noexcept
{
   _select_start_position = std::bit_cast<float2>(ImGui::GetMousePos());
   _selecting_entity = true;
}

void world_edit::finish_entity_select(const select_method method) noexcept
{
   _selecting_entity = false;
   _selection_edit_tool = selection_edit_tool::none;

   const float2 current_cursor_position =
      std::bit_cast<float2>(ImGui::GetMousePos());
   const float2 rect_min = min(current_cursor_position, _select_start_position);
   const float2 rect_max = max(current_cursor_position, _select_start_position);
   const bool drag_select = distance(rect_max, rect_min) >= (8.0f * _display_scale);

   if (drag_select) {
      const float2 window_size =
         std::bit_cast<float2>(ImGui::GetMainViewport()->Size);

      const float2 start_ndc_pos =
         ((rect_min + 0.5f) / window_size * 2.0f - 1.0f) * float2{1.0f, -1.0f};
      const float2 end_ndc_pos =
         ((rect_max + 0.5f) / window_size * 2.0f - 1.0f) * float2{1.0f, -1.0f};

      const float2 min_ndc_pos = min(start_ndc_pos, end_ndc_pos);
      const float2 max_ndc_pos = max(start_ndc_pos, end_ndc_pos);

      using namespace graphics;

      if (method == select_method::replace) {
         _interaction_targets.selection.clear();
      }

      frustum frustum{_camera.inv_view_projection_matrix(),
                      {min_ndc_pos.x, min_ndc_pos.y, 0.0f},
                      {max_ndc_pos.x, max_ndc_pos.y, 1.0f}};

      if (_world_hit_mask.objects) {
         for (auto& object : _world.objects) {
            if (not _world_layers_hit_mask[object.layer] or object.hidden) {
               continue;
            }

            math::bounding_box bbox =
               _object_classes[object.class_name].model->bounding_box;
            bbox = object.rotation * bbox + object.position;

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.add(object.id);
            }
         }
      }

      if (_world_hit_mask.lights) {
         for (auto& light : _world.lights) {
            if (not _world_layers_hit_mask[light.layer] or light.hidden) {
               continue;
            }

            const bool inside = [&] {
               switch (light.light_type) {
               case world::light_type::directional:
                  return intersects(frustum, light.position, 2.8284f);
               case world::light_type::point:
                  return intersects(frustum, light.position, light.range);
               case world::light_type::spot: {
                  const float half_range = light.range / 2.0f;
                  const float outer_cone_radius =
                     half_range * std::tan(light.outer_cone_angle);
                  const float inner_cone_radius =
                     half_range * std::tan(light.inner_cone_angle);
                  const float radius = std::min(outer_cone_radius, inner_cone_radius);

                  math::bounding_box bbox{.min = {-radius, -radius, 0.0f},
                                          .max = {radius, radius, light.range}};

                  bbox = light.rotation * bbox + light.position;

                  return intersects(frustum, bbox);
               }
               case world::light_type::directional_region_box: {
                  math::bounding_box bbox{.min = {-light.region_size},
                                          .max = {light.region_size}};

                  bbox = light.region_rotation * bbox + light.position;

                  return intersects(frustum, bbox);
               }
               case world::light_type::directional_region_sphere:
                  return intersects(frustum, light.position, length(light.region_size));
               case world::light_type::directional_region_cylinder: {
                  const float cylinder_length =
                     length(float2{light.region_size.x, light.region_size.z});

                  math::bounding_box bbox{.min = {-cylinder_length,
                                                  -light.region_size.y, -cylinder_length},
                                          .max = {cylinder_length,
                                                  light.region_size.y, cylinder_length}};

                  bbox = light.region_rotation * bbox + light.position;

                  return intersects(frustum, bbox);
               }
               default:
                  return false;
               }
            }();

            if (inside) {
               _interaction_targets.selection.add(light.id);
            }
         }
      }

      if (_world_hit_mask.paths) {
         for (auto& path : _world.paths) {
            if (not _world_layers_hit_mask[path.layer] or path.hidden) {
               continue;
            }

            for (uint32 i = 0; i < path.nodes.size(); ++i) {
               if (intersects(frustum, path.nodes[i].position,
                              0.707f * (_settings.graphics.path_node_size / 0.5f))) {
                  _interaction_targets.selection.add(
                     world::path_id_node_pair{path.id, i});
               }
            }
         }
      }

      if (_world_hit_mask.regions) {
         for (auto& region : _world.regions) {
            if (not _world_layers_hit_mask[region.layer] or region.hidden) {
               continue;
            }

            const bool inside = [&] {
               switch (region.shape) {
               case world::region_shape::box: {
                  math::bounding_box bbox{.min = {-region.size}, .max = {region.size}};

                  bbox = region.rotation * bbox + region.position;

                  return intersects(frustum, bbox);
               }
               case world::region_shape::sphere:
                  return intersects(frustum, region.position, length(region.size));
               case world::region_shape::cylinder: {
                  const float cylinder_length =
                     length(float2{region.size.x, region.size.z});

                  math::bounding_box bbox{.min = {-cylinder_length, -region.size.y,
                                                  -cylinder_length},
                                          .max = {cylinder_length, region.size.y,
                                                  cylinder_length}};

                  bbox = region.rotation * bbox + region.position;

                  return intersects(frustum, bbox);
               }
               default:
                  return false;
               }
            }();

            if (inside) {
               _interaction_targets.selection.add(region.id);
            }
         }
      }

      if (_world_hit_mask.sectors) {
         for (auto& sector : _world.sectors) {
            if (sector.hidden) continue;

            float2 point_min{FLT_MAX, FLT_MAX};
            float2 point_max{-FLT_MAX, -FLT_MAX};

            for (auto& point : sector.points) {
               point_min = min(point, point_min);
               point_max = max(point, point_max);
            }

            math::bounding_box bbox{{point_min.x, sector.base, point_min.y},
                                    {point_max.x, sector.base + sector.height,
                                     point_max.y}};

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.add(sector.id);
            }
         }
      }

      if (_world_hit_mask.portals) {
         for (auto& portal : _world.portals) {
            if (portal.hidden) continue;

            if (intersects(frustum, portal.position,
                           std::max(portal.height, portal.width))) {
               _interaction_targets.selection.add(portal.id);
            }
         }
      }

      if (_world_hit_mask.hintnodes) {
         for (auto& hintnode : _world.hintnodes) {
            if (not _world_layers_hit_mask[hintnode.layer] or hintnode.hidden) {
               continue;
            }

            if (intersects(frustum, hintnode.position, 2.0f)) {
               _interaction_targets.selection.add(hintnode.id);
            }
         }
      }

      if (_world_hit_mask.barriers) {
         for (auto& barrier : _world.barriers) {
            if (barrier.hidden) continue;

            math::bounding_box bbox{{-barrier.size.x, -_settings.graphics.barrier_height,
                                     -barrier.size.y},
                                    {barrier.size.x, _settings.graphics.barrier_height,
                                     barrier.size.y}};
            bbox = make_quat_from_euler({0.0f, barrier.rotation_angle, 0.0f}) * bbox +
                   barrier.position;

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.add(barrier.id);
            }
         }
      }

      if (_world_hit_mask.planning_hubs) {
         for (auto& hub : _world.planning_hubs) {
            if (hub.hidden) continue;

            math::bounding_box bbox{{-hub.radius, -_settings.graphics.planning_hub_height,
                                     -hub.radius},
                                    {hub.radius, _settings.graphics.planning_hub_height,
                                     hub.radius}};
            bbox = bbox + hub.position;

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.add(hub.id);
            }
         }
      }

      // Skip connections

      if (_world_hit_mask.boundaries) {
         for (auto& boundary : _world.boundaries) {
            if (boundary.hidden) continue;

            math::bounding_box bbox{{-boundary.size.x + boundary.position.x,
                                     -_settings.graphics.boundary_height,
                                     -boundary.size.y + boundary.position.y},
                                    {boundary.size.x + boundary.position.x,
                                     _settings.graphics.boundary_height,
                                     boundary.size.y + boundary.position.y}};

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.add(boundary.id);
            }
         }
      }

      if (_world_hit_mask.measurements) {
         for (auto& measurement : _world.measurements) {
            if (measurement.hidden) continue;

            math::bounding_box bbox{min(measurement.start, measurement.end),
                                    max(measurement.start, measurement.end)};

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.add(measurement.id);
            }
         }
      }
   }
   else {
      if (method == select_method::replace) {
         _interaction_targets.selection.clear();
      }

      if (not _interaction_targets.hovered_entity) return;

      _interaction_targets.selection.add(*_interaction_targets.hovered_entity);
   }
}

void world_edit::start_entity_deselect() noexcept
{
   start_entity_select();
}

void world_edit::finish_entity_deselect() noexcept
{
   _selecting_entity = false;

   const float2 current_cursor_position =
      std::bit_cast<float2>(ImGui::GetMousePos());
   const float2 rect_min = min(current_cursor_position, _select_start_position);
   const float2 rect_max = max(current_cursor_position, _select_start_position);
   const bool drag_select = distance(rect_max, rect_min) >= (8.0f * _display_scale);

   if (drag_select) {
      const float2 window_size =
         std::bit_cast<float2>(ImGui::GetMainViewport()->Size);

      const float2 start_ndc_pos =
         ((rect_min + 0.5f) / window_size * 2.0f - 1.0f) * float2{1.0f, -1.0f};
      const float2 end_ndc_pos =
         ((rect_max + 0.5f) / window_size * 2.0f - 1.0f) * float2{1.0f, -1.0f};

      const float2 min_ndc_pos = min(start_ndc_pos, end_ndc_pos);
      const float2 max_ndc_pos = max(start_ndc_pos, end_ndc_pos);

      using namespace graphics;

      frustum frustum{_camera.inv_view_projection_matrix(),
                      {min_ndc_pos.x, min_ndc_pos.y, 0.0f},
                      {max_ndc_pos.x, max_ndc_pos.y, 1.0f}};

      if (_world_hit_mask.objects) {
         for (auto& object : _world.objects) {
            if (not _world_layers_hit_mask[object.layer] or object.hidden) {
               continue;
            }

            math::bounding_box bbox =
               _object_classes[object.class_name].model->bounding_box;
            bbox = object.rotation * bbox + object.position;

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.remove(object.id);
            }
         }
      }

      if (_world_hit_mask.lights) {
         for (auto& light : _world.lights) {
            if (not _world_layers_hit_mask[light.layer] or light.hidden) {
               continue;
            }

            const bool inside = [&] {
               switch (light.light_type) {
               case world::light_type::directional:
                  return intersects(frustum, light.position, 2.8284f);
               case world::light_type::point:
                  return intersects(frustum, light.position, light.range);
               case world::light_type::spot: {
                  const float half_range = light.range / 2.0f;
                  const float outer_cone_radius =
                     half_range * std::tan(light.outer_cone_angle);
                  const float inner_cone_radius =
                     half_range * std::tan(light.inner_cone_angle);
                  const float radius = std::min(outer_cone_radius, inner_cone_radius);

                  math::bounding_box bbox{.min = {-radius, -radius, 0.0f},
                                          .max = {radius, radius, light.range}};

                  bbox = light.rotation * bbox + light.position;

                  return intersects(frustum, bbox);
               }
               case world::light_type::directional_region_box: {
                  math::bounding_box bbox{.min = {-light.region_size},
                                          .max = {light.region_size}};

                  bbox = light.region_rotation * bbox + light.position;

                  return intersects(frustum, bbox);
               }
               case world::light_type::directional_region_sphere:
                  return intersects(frustum, light.position, length(light.region_size));
               case world::light_type::directional_region_cylinder: {
                  const float cylinder_length =
                     length(float2{light.region_size.x, light.region_size.z});

                  math::bounding_box bbox{.min = {-cylinder_length,
                                                  -light.region_size.y, -cylinder_length},
                                          .max = {cylinder_length,
                                                  light.region_size.y, cylinder_length}};

                  bbox = light.region_rotation * bbox + light.position;

                  return intersects(frustum, bbox);
               }
               default:
                  return false;
               }
            }();

            if (inside) {
               _interaction_targets.selection.remove(light.id);
            }
         }
      }

      if (_world_hit_mask.paths) {
         for (auto& path : _world.paths) {
            if (not _world_layers_hit_mask[path.layer] or path.hidden) {
               continue;
            }

            for (uint32 i = 0; i < path.nodes.size(); ++i) {
               if (intersects(frustum, path.nodes[i].position,
                              0.707f * (_settings.graphics.path_node_size / 0.5f))) {
                  _interaction_targets.selection.remove(
                     world::path_id_node_pair{path.id, i});
               }
            }
         }
      }

      if (_world_hit_mask.regions) {
         for (auto& region : _world.regions) {
            if (not _world_layers_hit_mask[region.layer] or region.hidden) {
               continue;
            }

            const bool inside = [&] {
               switch (region.shape) {
               case world::region_shape::box: {
                  math::bounding_box bbox{.min = {-region.size}, .max = {region.size}};

                  bbox = region.rotation * bbox + region.position;

                  return intersects(frustum, bbox);
               }
               case world::region_shape::sphere:
                  return intersects(frustum, region.position, length(region.size));
               case world::region_shape::cylinder: {
                  const float cylinder_length =
                     length(float2{region.size.x, region.size.z});

                  math::bounding_box bbox{.min = {-cylinder_length, -region.size.y,
                                                  -cylinder_length},
                                          .max = {cylinder_length, region.size.y,
                                                  cylinder_length}};

                  bbox = region.rotation * bbox + region.position;

                  return intersects(frustum, bbox);
               }
               default:
                  return false;
               }
            }();

            if (inside) {
               _interaction_targets.selection.remove(region.id);
            }
         }
      }

      if (_world_hit_mask.sectors) {
         for (auto& sector : _world.sectors) {
            if (sector.hidden) continue;

            float2 point_min{FLT_MAX, FLT_MAX};
            float2 point_max{-FLT_MAX, -FLT_MAX};

            for (auto& point : sector.points) {
               point_min = min(point, point_min);
               point_max = max(point, point_max);
            }

            math::bounding_box bbox{{point_min.x, sector.base, point_min.y},
                                    {point_max.x, sector.base + sector.height,
                                     point_max.y}};

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.remove(sector.id);
            }
         }
      }

      if (_world_hit_mask.portals) {
         for (auto& portal : _world.portals) {
            if (portal.hidden) continue;

            if (intersects(frustum, portal.position,
                           std::max(portal.height, portal.width))) {
               _interaction_targets.selection.remove(portal.id);
            }
         }
      }

      if (_world_hit_mask.hintnodes) {
         for (auto& hintnode : _world.hintnodes) {
            if (not _world_layers_hit_mask[hintnode.layer] or hintnode.hidden) {
               continue;
            }

            if (intersects(frustum, hintnode.position, 2.0f)) {
               _interaction_targets.selection.remove(hintnode.id);
            }
         }
      }

      if (_world_hit_mask.barriers) {
         for (auto& barrier : _world.barriers) {
            if (barrier.hidden) continue;

            math::bounding_box bbox{{-barrier.size.x, -_settings.graphics.barrier_height,
                                     -barrier.size.y},
                                    {barrier.size.x, _settings.graphics.barrier_height,
                                     barrier.size.y}};
            bbox = make_quat_from_euler({0.0f, barrier.rotation_angle, 0.0f}) * bbox +
                   barrier.position;

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.remove(barrier.id);
            }
         }
      }

      if (_world_hit_mask.planning_hubs) {
         for (auto& hub : _world.planning_hubs) {
            if (hub.hidden) continue;

            math::bounding_box bbox{{-hub.radius, -_settings.graphics.planning_hub_height,
                                     -hub.radius},
                                    {hub.radius, _settings.graphics.planning_hub_height,
                                     hub.radius}};
            bbox = bbox + hub.position;

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.remove(hub.id);
            }
         }
      }

      if (_world_hit_mask.planning_connections) {
         for (auto& connection : _world.planning_connections) {
            if (connection.hidden) continue;

            const world::planning_hub& start =
               _world.planning_hubs[connection.start_hub_index];
            const world::planning_hub& end =
               _world.planning_hubs[connection.end_hub_index];

            const float height = _settings.graphics.planning_connection_height;

            const math::bounding_box start_bbox{
               .min = float3{-start.radius, -height, -start.radius} + start.position,
               .max = float3{start.radius, height, start.radius} + start.position};
            const math::bounding_box end_bbox{
               .min = float3{-end.radius, -height, -end.radius} + end.position,
               .max = float3{end.radius, height, end.radius} + end.position};

            const math::bounding_box bbox = math::combine(start_bbox, end_bbox);

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.remove(connection.id);
            }
         }
      }

      if (_world_hit_mask.boundaries) {
         for (auto& boundary : _world.boundaries) {
            if (boundary.hidden) continue;

            math::bounding_box bbox{{-boundary.size.x + boundary.position.x,
                                     -_settings.graphics.boundary_height,
                                     -boundary.size.y + boundary.position.y},
                                    {boundary.size.x + boundary.position.x,
                                     _settings.graphics.boundary_height,
                                     boundary.size.y + boundary.position.y}};

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.remove(boundary.id);
            }
         }
      }

      if (_world_hit_mask.measurements) {
         for (auto& measurement : _world.measurements) {
            if (measurement.hidden) continue;

            math::bounding_box bbox{min(measurement.start, measurement.end),
                                    max(measurement.start, measurement.end)};

            if (intersects(frustum, bbox)) {
               _interaction_targets.selection.remove(measurement.id);
            }
         }
      }
   }
   else {
      if (not _interaction_targets.hovered_entity) return;

      _interaction_targets.selection.remove(*_interaction_targets.hovered_entity);
   }
}

void world_edit::place_creation_entity() noexcept
{
   if (not _interaction_targets.creation_entity.holds_entity()) return;

   const auto report_limit_reached =
      [this]<typename... Args>(fmt::format_string<Args...> string, const Args&... args) {
         std::string display = fmt::vformat(string, fmt::make_format_args(args...));

         MessageBoxA(_window, display.c_str(), "Limit Reached", MB_OK);
      };

   if (_interaction_targets.creation_entity.is<world::object>()) {
      world::object& object =
         _interaction_targets.creation_entity.get<world::object>();
      world::object new_object = object;

      new_object.name = world::create_unique_name(_world.objects, new_object.name);
      new_object.instance_properties = world::make_object_instance_properties(
         *_object_classes[object.class_name].definition, new_object.instance_properties);
      new_object.id = _world.next_id.objects.aquire();

      _last_created_entities.last_object = new_object.id;
      _last_created_entities.last_used_object_classes.insert(new_object.class_name);

      if (_world.objects.size() == _world.objects.max_size()) {
         report_limit_reached("Max Objects ({}) Reached", _world.objects.max_size());

         return;
      }

      _edit_stack_world.apply(edits::make_insert_entity(std::move(new_object)),
                              _edit_context);

      if (not object.name.empty()) {
         _edit_stack_world.apply(
            edits::make_set_creation_value(&world::object::name,
                                           world::create_unique_name(_world.objects,
                                                                     object.name),
                                           object.name),
            _edit_context, {.transparent = true});
      }

      if (_entity_creation_config.command_post_auto_place_meta_entities and
          string::iequals(_object_classes[object.class_name].definition->header.class_label,
                          "commandpost")) {
         command_post_auto_place_meta_entities(_world.objects.back());
      }

      if (_entity_creation_config.auto_add_object_to_sectors) {
         add_object_to_sectors(_world.objects.back());
      }

      _last_created_entities.last_layer = object.layer;
   }
   else if (_interaction_targets.creation_entity.is<world::light>()) {
      world::light& light = _interaction_targets.creation_entity.get<world::light>();
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

      if (_world.lights.size() == _world.lights.max_size()) {
         report_limit_reached("Max Lights ({}) Reached", _world.lights.max_size());

         return;
      }

      _edit_stack_world.apply(edits::make_insert_entity(std::move(new_light)),
                              _edit_context);

      _edit_stack_world.apply(edits::make_set_creation_value(
                                 &world::light::name,
                                 world::create_unique_name(_world.lights, light.name),
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
   }
   else if (_interaction_targets.creation_entity.is<world::path>()) {
      world::path& path = _interaction_targets.creation_entity.get<world::path>();

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
            _edit_stack_world.apply(edits::make_insert_node(existing_path->id,
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

         if (_world.paths.size() == _world.paths.max_size()) {
            report_limit_reached("Max Paths ({}) Reached", _world.paths.max_size());

            return;
         }

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_path)),
                                 _edit_context);
      }

      _last_created_entities.last_layer = path.layer;
   }
   else if (_interaction_targets.creation_entity.is<world::region>()) {
      world::region& region =
         _interaction_targets.creation_entity.get<world::region>();
      world::region new_region = region;

      new_region.name = world::create_unique_name(_world.regions, new_region.name);
      new_region.id = _world.next_id.regions.aquire();

      _last_created_entities.last_region = new_region.id;

      if (_world.regions.size() == _world.regions.max_size()) {
         report_limit_reached("Max Regions ({}) Reached", _world.regions.max_size());

         return;
      }

      _edit_stack_world.apply(edits::make_insert_entity(std::move(new_region)),
                              _edit_context);

      _edit_stack_world
         .apply(edits::make_set_creation_value(&world::region::name,
                                               world::create_unique_name(_world.regions,
                                                                         region.name),
                                               region.name),
                _edit_context, {.transparent = true});

      _last_created_entities.last_layer = region.layer;
   }
   else if (_interaction_targets.creation_entity.is<world::sector>()) {
      world::sector& sector =
         _interaction_targets.creation_entity.get<world::sector>();

      if (sector.points.empty()) std::terminate();

      if (world::sector* existing_sector =
             world::find_entity(_world.sectors, sector.name);
          existing_sector and sector.points.size() == 1) {
         _edit_stack_world
            .apply(edits::make_insert_point(existing_sector->id,
                                            existing_sector->points.size(),
                                            sector.points[0]),
                   _edit_context);

         if (_entity_creation_config.auto_fill_sector) {
            _edit_stack_world
               .apply(edits::make_set_value(&existing_sector->objects,
                                            world::sector_fill(*existing_sector,
                                                               _world.objects,
                                                               _object_classes)),
                      _edit_context);
         }
      }
      else {
         world::sector new_sector = sector;

         new_sector.name = world::create_unique_name(_world.sectors, sector.name);
         new_sector.id = _world.next_id.sectors.aquire();

         if (_entity_creation_config.auto_fill_sector) {
            new_sector.objects =
               world::sector_fill(new_sector, _world.objects, _object_classes);
         }

         if (_world.sectors.size() == _world.sectors.max_size()) {
            report_limit_reached("Max sectors ({}) Reached",
                                 _world.sectors.max_size());

            return;
         }

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_sector)),
                                 _edit_context);

         if (sector.points.size() > 1) {
            _edit_stack_world.apply(edits::make_set_creation_value(&world::sector::points,
                                                                   {sector.points[0]},
                                                                   sector.points),
                                    _edit_context,
                                    {.closed = true, .transparent = true});
         }
      }
   }
   else if (_interaction_targets.creation_entity.is<world::portal>()) {
      world::portal& portal =
         _interaction_targets.creation_entity.get<world::portal>();
      world::portal new_portal = portal;

      new_portal.name = world::create_unique_name(_world.portals, new_portal.name);
      new_portal.id = _world.next_id.portals.aquire();

      _last_created_entities.last_portal = new_portal.id;

      if (_world.portals.size() == _world.portals.max_size()) {
         report_limit_reached("Max portals ({}) Reached", _world.portals.max_size());

         return;
      }

      _edit_stack_world.apply(edits::make_insert_entity(std::move(new_portal)),
                              _edit_context);

      _edit_stack_world
         .apply(edits::make_set_creation_value(&world::portal::name,
                                               world::create_unique_name(_world.portals,
                                                                         portal.name),
                                               portal.name),
                _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::hintnode>()) {
      world::hintnode& hintnode =
         _interaction_targets.creation_entity.get<world::hintnode>();
      world::hintnode new_hintnode = hintnode;

      new_hintnode.name =
         world::create_unique_name(_world.hintnodes, new_hintnode.name);
      new_hintnode.id = _world.next_id.hintnodes.aquire();

      _last_created_entities.last_hintnode = new_hintnode.id;

      if (_world.hintnodes.size() == _world.hintnodes.max_size()) {
         report_limit_reached("Max hintnodes ({}) Reached",
                              _world.hintnodes.max_size());

         return;
      }

      _edit_stack_world.apply(edits::make_insert_entity(std::move(new_hintnode)),
                              _edit_context);

      _edit_stack_world.apply(
         edits::make_set_creation_value(&world::hintnode::name,
                                        world::create_unique_name(_world.hintnodes,
                                                                  hintnode.name),
                                        hintnode.name),
         _edit_context, {.transparent = true});

      _last_created_entities.last_layer = hintnode.layer;
   }
   else if (_interaction_targets.creation_entity.is<world::barrier>()) {
      world::barrier& barrier =
         _interaction_targets.creation_entity.get<world::barrier>();
      world::barrier new_barrier = barrier;

      new_barrier.name = world::create_unique_name(_world.barriers, new_barrier.name);
      new_barrier.id = _world.next_id.barriers.aquire();

      _last_created_entities.last_barrier = new_barrier.id;

      if (_world.barriers.size() == _world.barriers.max_size()) {
         report_limit_reached("Max barriers ({}) Reached", _world.barriers.max_size());

         return;
      }

      _edit_stack_world.apply(edits::make_insert_entity(std::move(new_barrier)),
                              _edit_context);

      _edit_stack_world
         .apply(edits::make_set_creation_value(&world::barrier::name,
                                               world::create_unique_name(_world.barriers,
                                                                         barrier.name),
                                               barrier.name),
                _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::planning_hub>()) {
      if (_entity_creation_context.hub_sizing_started) {
         world::planning_hub& hub =
            _interaction_targets.creation_entity.get<world::planning_hub>();
         world::planning_hub new_hub = hub;

         new_hub.name = world::create_unique_name(_world.planning_hubs, new_hub.name);
         new_hub.id = _world.next_id.planning_hubs.aquire();

         _last_created_entities.last_planning_hub = new_hub.id;

         if (_world.planning_hubs.size() == _world.planning_hubs.max_size()) {
            report_limit_reached("Max AI planning hubs ({}) Reached",
                                 _world.planning_hubs.max_size());

            return;
         }

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_hub)),
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
   }
   else if (_interaction_targets.creation_entity.is<world::planning_connection>()) {
      world::planning_connection& connection =
         _interaction_targets.creation_entity.get<world::planning_connection>();

      if (not(_interaction_targets.hovered_entity and
              std::holds_alternative<world::planning_hub_id>(
                 *_interaction_targets.hovered_entity))) {
         return;
      }

      if (_entity_creation_context.connection_link_started) {
         if (connection.start_hub_index == connection.end_hub_index) {
            return;
         }

         for (auto& other_connection : _world.planning_connections) {
            if ((connection.start_hub_index == other_connection.start_hub_index and
                 connection.end_hub_index == other_connection.end_hub_index) or
                (connection.start_hub_index == other_connection.end_hub_index and
                 connection.end_hub_index == other_connection.start_hub_index)) {
               if (not _world.planning_hubs.empty()) {
                  _edit_stack_world.apply(edits::make_set_creation_value(
                                             &world::planning_connection::start_hub_index,
                                             uint32{0}, connection.start_hub_index),
                                          _edit_context);

                  _edit_stack_world.apply(edits::make_set_creation_value(
                                             &world::planning_connection::end_hub_index,
                                             uint32{0}, connection.end_hub_index),
                                          _edit_context, {.transparent = true});
               }
               else {
                  _edit_stack_world.apply(edits::make_creation_entity_set(
                                             world::creation_entity_none),
                                          _edit_context);
               }

               _entity_creation_context.connection_link_started = false;

               return;
            }
         }

         world::planning_connection new_connection = connection;

         new_connection.name = world::create_unique_name(_world.planning_connections,
                                                         new_connection.name);
         new_connection.id = _world.next_id.planning_connections.aquire();

         _last_created_entities.last_planning_connection = new_connection.id;

         if (_world.planning_connections.size() ==
             _world.planning_connections.max_size()) {
            report_limit_reached("Max AI planning connections ({}) Reached",
                                 _world.planning_connections.max_size());

            return;
         }

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
                                    &world::planning_connection::start_hub_index,
                                    get_hub_index(_world.planning_hubs,
                                                  std::get<world::planning_hub_id>(
                                                     *_interaction_targets.hovered_entity)),
                                    connection.start_hub_index),
                                 _edit_context);

         _entity_creation_context.connection_link_started = true;
      }
   }
   else if (_interaction_targets.creation_entity.is<world::boundary>()) {
      world::boundary& boundary =
         _interaction_targets.creation_entity.get<world::boundary>();
      world::boundary new_boundary = boundary;

      new_boundary.name =
         world::create_unique_name(_world.boundaries, new_boundary.name);
      new_boundary.id = _world.next_id.boundaries.aquire();

      _last_created_entities.last_boundary = new_boundary.id;

      if (_world.boundaries.size() == _world.boundaries.max_size()) {
         report_limit_reached("Max boundaries ({}) Reached",
                              _world.boundaries.max_size());

         return;
      }

      _edit_stack_world.apply(edits::make_insert_entity(std::move(new_boundary)),
                              _edit_context);

      _edit_stack_world.apply(
         edits::make_set_creation_value(&world::boundary::name,
                                        world::create_unique_name(_world.boundaries,
                                                                  boundary.name),
                                        boundary.name),
         _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::measurement>()) {
      if (_entity_creation_context.measurement_started) {
         world::measurement& measurement =
            _interaction_targets.creation_entity.get<world::measurement>();
         world::measurement new_measurement = measurement;

         new_measurement.id = _world.next_id.measurements.aquire();

         if (_world.measurements.size() == _world.measurements.max_size()) {
            report_limit_reached("Max measurements ({}) Reached",
                                 _world.measurements.max_size());

            return;
         }

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_measurement)),
                                 _edit_context);

         _entity_creation_context.measurement_started = false;
      }
      else {
         _entity_creation_context.measurement_started = true;
      }
   }

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
         _world.terrain);

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
   if (not _interaction_targets.creation_entity.is<world::object>()) {
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
                _interaction_targets.creation_entity.get<world::object>().class_name),
             _edit_context, {.closed = true});
}

void world_edit::toggle_planning_entity_type() noexcept
{
   if (_interaction_targets.creation_entity.is<world::planning_hub>() and
       not _world.planning_hubs.empty()) {
      const world::planning_connection* base_connection =
         world::find_entity(_world.planning_connections,
                            _last_created_entities.last_planning_connection);

      world::planning_connection new_connection;

      if (base_connection) {
         new_connection = *base_connection;

         new_connection.name = world::create_unique_name(_world.planning_connections,
                                                         base_connection->name);
         new_connection.id = world::max_id;
      }
      else {
         new_connection = world::planning_connection{.name = "Connection0",
                                                     .start_hub_index = 0,
                                                     .end_hub_index = 0,
                                                     .id = world::max_id};
      }

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_connection)),
                              _edit_context);
      _entity_creation_context = {};
   }
   else if (_interaction_targets.creation_entity.is<world::planning_connection>() and
            not _world.planning_hubs.empty()) {
      const world::planning_hub* base_hub =
         world::find_entity(_world.planning_hubs,
                            _last_created_entities.last_planning_hub);

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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_hub)),
                              _edit_context);
      _entity_creation_context = {};
   }
}

void world_edit::undo() noexcept
{
   _edit_stack_world.revert(_edit_context);

   if (_interaction_targets.creation_entity.holds_entity()) {
      _entity_creation_context = {};

      _cursor_placement_undo_lock =
         _entity_creation_config.placement_mode == placement_mode::cursor or
         _entity_creation_config.placement_rotation == placement_rotation::surface;
      _cursor_placement_lock_position = {ImGui::GetMousePos().x,
                                         ImGui::GetMousePos().y};
   }

   _selection_edit_tool = selection_edit_tool::none;
}

void world_edit::redo() noexcept
{
   _edit_stack_world.reapply(_edit_context);

   if (_interaction_targets.creation_entity.holds_entity()) {
      _entity_creation_context = {};

      _cursor_placement_undo_lock =
         _entity_creation_config.placement_mode == placement_mode::cursor or
         _entity_creation_config.placement_rotation == placement_rotation::surface;
      _cursor_placement_lock_position = {ImGui::GetMousePos().x,
                                         ImGui::GetMousePos().y};
   }
}

void world_edit::delete_selected() noexcept
{
   if (_interaction_targets.creation_entity.holds_entity()) {
      _edit_stack_world.apply(edits::make_creation_entity_set(world::creation_entity_none),
                              _edit_context);

      return;
   }

   bool first_delete = true;

   while (not _interaction_targets.selection.empty()) {
      const auto& generic_selected = _interaction_targets.selection.view().back();

      if (not is_valid(generic_selected, _world)) continue;

      if (std::holds_alternative<world::path_id_node_pair>(generic_selected)) {
         // This fixes up path node indices in the rest of the selection.

         const world::path_id_node_pair dying_id_node =
            std::get<world::path_id_node_pair>(generic_selected);

         for (auto& other_generic_selected :
              _interaction_targets.selection.view_updatable()) {
            if (not std::holds_alternative<world::path_id_node_pair>(other_generic_selected) or
                other_generic_selected == generic_selected) {
               continue;
            }

            world::path_id_node_pair& path_id_node =
               std::get<world::path_id_node_pair>(other_generic_selected);

            if (dying_id_node.id == path_id_node.id and
                path_id_node.node_index > dying_id_node.node_index) {
               path_id_node.node_index -= 1;
            }
         }
      }

      std::visit(
         [&](const auto& selected) {
            _edit_stack_world.apply(edits::make_delete_entity(selected, _world),
                                    _edit_context,
                                    {.transparent =
                                        not std::exchange(first_delete, false)});
         },
         generic_selected);

      _interaction_targets.selection.remove(generic_selected);
   }
}

void world_edit::align_selection() noexcept
{
   if (_interaction_targets.selection.empty()) return;

   edits::bundle_vector bundle;
   bundle.reserve(_interaction_targets.selection.size());

   const float alignment = _world.terrain.grid_scale;

   const auto align_position = [=](const float3 position) {
      return float3{std::round(position.x / alignment) * alignment, position.y,
                    std::round(position.z / alignment) * alignment};
   };

   for (const auto& selected : _interaction_targets.selection) {
      if (std::holds_alternative<world::object_id>(selected)) {
         world::object* object =
            world::find_entity(_world.objects, std::get<world::object_id>(selected));

         if (object) {
            bundle.push_back(edits::make_set_value(&object->position,
                                                   align_position(object->position)));
         }
      }
      else if (std::holds_alternative<world::path_id_node_pair>(selected)) {
         const auto [id, node_index] = std::get<world::path_id_node_pair>(selected);

         world::path* path = world::find_entity(_world.paths, id);

         if (path) {
            const float3 position = path->nodes[node_index].position;

            bundle.push_back(edits::make_set_vector_value(&path->nodes, node_index,
                                                          &world::path::node::position,
                                                          align_position(position)));
         }
      }
      else if (std::holds_alternative<world::light_id>(selected)) {
         world::light* light =
            world::find_entity(_world.lights, std::get<world::light_id>(selected));

         if (light) {
            bundle.push_back(edits::make_set_value(&light->position,
                                                   align_position(light->position)));
         }
      }
      else if (std::holds_alternative<world::region_id>(selected)) {
         world::region* region =
            world::find_entity(_world.regions, std::get<world::region_id>(selected));

         if (region) {
            bundle.push_back(edits::make_set_value(&region->position,
                                                   align_position(region->position)));
         }
      }
      else if (std::holds_alternative<world::sector_id>(selected)) {
         world::sector* sector =
            world::find_entity(_world.sectors, std::get<world::sector_id>(selected));

         if (sector) {
            std::vector<float2> new_points = sector->points;

            for (auto& point : new_points) {
               point = round(point / alignment) * alignment;
            }

            bundle.push_back(
               edits::make_set_value(&sector->points, std::move(new_points)));
         }
      }
      else if (std::holds_alternative<world::portal_id>(selected)) {
         world::portal* portal =
            world::find_entity(_world.portals, std::get<world::portal_id>(selected));

         if (portal) {
            bundle.push_back(edits::make_set_value(&portal->position,
                                                   align_position(portal->position)));
         }
      }
      else if (std::holds_alternative<world::hintnode_id>(selected)) {
         world::hintnode* hintnode =
            world::find_entity(_world.hintnodes,
                               std::get<world::hintnode_id>(selected));

         if (hintnode) {
            bundle.push_back(edits::make_set_value(&hintnode->position,
                                                   align_position(hintnode->position)));
         }
      }
      else if (std::holds_alternative<world::barrier_id>(selected)) {
         world::barrier* barrier =
            world::find_entity(_world.barriers, std::get<world::barrier_id>(selected));

         if (barrier) {
            bundle.push_back(edits::make_set_value(&barrier->position,
                                                   align_position(barrier->position)));
         }
      }
      else if (std::holds_alternative<world::planning_hub_id>(selected)) {
         world::planning_hub* planning_hub =
            world::find_entity(_world.planning_hubs,
                               std::get<world::planning_hub_id>(selected));

         if (planning_hub) {
            bundle.push_back(
               edits::make_set_value(&planning_hub->position,
                                     align_position(planning_hub->position)));
         }
      }
      else if (std::holds_alternative<world::boundary_id>(selected)) {
         world::boundary* boundary =
            world::find_entity(_world.boundaries,
                               std::get<world::boundary_id>(selected));

         if (boundary) {
            bundle.push_back(
               edits::make_set_value(&boundary->position,
                                     round(boundary->position / alignment) * alignment));
         }
      }
      else if (std::holds_alternative<world::measurement_id>(selected)) {
         world::measurement* measurement =
            world::find_entity(_world.measurements,
                               std::get<world::measurement_id>(selected));

         if (measurement) {
            bundle.push_back(
               edits::make_set_value(&measurement->start,
                                     round(measurement->start / alignment) * alignment));
            bundle.push_back(
               edits::make_set_value(&measurement->end,
                                     round(measurement->end / alignment) * alignment));
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

void world_edit::hide_selection() noexcept
{
   edits::bundle_vector bundle;
   bundle.reserve(_interaction_targets.selection.size());

   for (const auto& selected : _interaction_targets.selection) {
      if (std::holds_alternative<world::object_id>(selected)) {
         world::object* object =
            world::find_entity(_world.objects, std::get<world::object_id>(selected));

         if (object and not object->hidden) {
            bundle.push_back(edits::make_set_value(&object->hidden, true));
         }
      }
      else if (std::holds_alternative<world::light_id>(selected)) {
         world::light* light =
            world::find_entity(_world.lights, std::get<world::light_id>(selected));

         if (light and not light->hidden) {
            bundle.push_back(edits::make_set_value(&light->hidden, true));
         }
      }
      else if (std::holds_alternative<world::path_id_node_pair>(selected)) {
         const auto [id, node_index] = std::get<world::path_id_node_pair>(selected);

         world::path* path = world::find_entity(_world.paths, id);

         if (path and not path->hidden) {
            bundle.push_back(edits::make_set_value(&path->hidden, true));
         }
      }
      else if (std::holds_alternative<world::region_id>(selected)) {
         world::region* region =
            world::find_entity(_world.regions, std::get<world::region_id>(selected));

         if (region and not region->hidden) {
            bundle.push_back(edits::make_set_value(&region->hidden, true));
         }
      }
      else if (std::holds_alternative<world::sector_id>(selected)) {
         world::sector* sector =
            world::find_entity(_world.sectors, std::get<world::sector_id>(selected));

         if (sector and not sector->hidden) {
            bundle.push_back(edits::make_set_value(&sector->hidden, true));
         }
      }
      else if (std::holds_alternative<world::portal_id>(selected)) {
         world::portal* portal =
            world::find_entity(_world.portals, std::get<world::portal_id>(selected));

         if (portal and not portal->hidden) {
            bundle.push_back(edits::make_set_value(&portal->hidden, true));
         }
      }
      else if (std::holds_alternative<world::hintnode_id>(selected)) {
         world::hintnode* hintnode =
            world::find_entity(_world.hintnodes,
                               std::get<world::hintnode_id>(selected));

         if (hintnode and not hintnode->hidden) {
            bundle.push_back(edits::make_set_value(&hintnode->hidden, true));
         }
      }
      else if (std::holds_alternative<world::barrier_id>(selected)) {
         world::barrier* barrier =
            world::find_entity(_world.barriers, std::get<world::barrier_id>(selected));

         if (barrier and not barrier->hidden) {
            bundle.push_back(edits::make_set_value(&barrier->hidden, true));
         }
      }
      else if (std::holds_alternative<world::planning_hub_id>(selected)) {
         world::planning_hub* hub =
            world::find_entity(_world.planning_hubs,
                               std::get<world::planning_hub_id>(selected));

         if (hub and not hub->hidden) {
            bundle.push_back(edits::make_set_value(&hub->hidden, true));
         }
      }
      else if (std::holds_alternative<world::planning_connection_id>(selected)) {
         world::planning_connection* connection =
            world::find_entity(_world.planning_connections,
                               std::get<world::planning_connection_id>(selected));

         if (connection and not connection->hidden) {
            bundle.push_back(edits::make_set_value(&connection->hidden, true));
         }
      }
      else if (std::holds_alternative<world::boundary_id>(selected)) {
         world::boundary* boundary =
            world::find_entity(_world.boundaries,
                               std::get<world::boundary_id>(selected));

         if (boundary and not boundary->hidden) {
            bundle.push_back(edits::make_set_value(&boundary->hidden, true));
         }
      }
      else if (std::holds_alternative<world::measurement_id>(selected)) {
         world::measurement* measurement =
            world::find_entity(_world.measurements,
                               std::get<world::measurement_id>(selected));

         if (measurement and not measurement->hidden) {
            bundle.push_back(edits::make_set_value(&measurement->hidden, true));
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

void world_edit::ground_selection() noexcept
{
   edits::bundle_vector bundle;
   bundle.reserve(_interaction_targets.selection.size());

   for (const auto& selected : _interaction_targets.selection) {
      if (std::holds_alternative<world::object_id>(selected)) {
         world::object* object =
            world::find_entity(_world.objects, std::get<world::object_id>(selected));

         if (object) {
            if (const std::optional<float3> grounded_position =
                   world::ground_object(*object, _world, _object_classes,
                                        _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&object->position, *grounded_position));
            }
         }
      }
      else if (std::holds_alternative<world::light_id>(selected)) {
         world::light* light =
            world::find_entity(_world.lights, std::get<world::light_id>(selected));

         if (light) {
            if (const std::optional<float3> grounded_position =
                   world::ground_light(*light, _world, _object_classes,
                                       _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&light->position, *grounded_position));
            }
         }
      }
      else if (std::holds_alternative<world::path_id_node_pair>(selected)) {
         const auto [id, node_index] = std::get<world::path_id_node_pair>(selected);

         world::path* path = world::find_entity(_world.paths, id);

         if (path and node_index < path->nodes.size()) {
            if (const std::optional<float3> grounded_position =
                   world::ground_point(path->nodes[node_index].position, _world,
                                       _object_classes, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(edits::make_set_vector_value(&path->nodes, node_index,
                                                             &world::path::node::position,
                                                             *grounded_position));
            }
         }
      }
      else if (std::holds_alternative<world::region_id>(selected)) {
         world::region* region =
            world::find_entity(_world.regions, std::get<world::region_id>(selected));

         if (region) {
            if (const std::optional<float3> grounded_position =
                   world::ground_region(*region, _world, _object_classes,
                                        _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&region->position, *grounded_position));
            }
         }
      }
      else if (std::holds_alternative<world::sector_id>(selected)) {
         world::sector* sector =
            world::find_entity(_world.sectors, std::get<world::sector_id>(selected));

         if (sector) {
            if (const std::optional<float> grounded_base =
                   world::ground_sector(*sector, _world, _object_classes,
                                        _world_layers_hit_mask);
                grounded_base) {
               bundle.push_back(edits::make_set_value(&sector->base, *grounded_base));
            }
         }
      }
      else if (std::holds_alternative<world::portal_id>(selected)) {
         world::portal* portal =
            world::find_entity(_world.portals, std::get<world::portal_id>(selected));

         if (portal) {
            if (const std::optional<float3> grounded_position =
                   world::ground_portal(*portal, _world, _object_classes,
                                        _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&portal->position, *grounded_position));
            }
         }
      }
      else if (std::holds_alternative<world::hintnode_id>(selected)) {
         world::hintnode* hintnode =
            world::find_entity(_world.hintnodes,
                               std::get<world::hintnode_id>(selected));

         if (hintnode) {
            if (const std::optional<float3> grounded_position =
                   world::ground_point(hintnode->position, _world,
                                       _object_classes, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(edits::make_set_value(&hintnode->position,
                                                      *grounded_position));
            }
         }
      }
      else if (std::holds_alternative<world::barrier_id>(selected)) {
         world::barrier* barrier =
            world::find_entity(_world.barriers, std::get<world::barrier_id>(selected));

         if (barrier) {
            if (const std::optional<float3> grounded_position =
                   world::ground_point(barrier->position, _world,
                                       _object_classes, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&barrier->position, *grounded_position));
            }
         }
      }
      else if (std::holds_alternative<world::planning_hub_id>(selected)) {
         world::planning_hub* hub =
            world::find_entity(_world.planning_hubs,
                               std::get<world::planning_hub_id>(selected));

         if (hub) {
            if (const std::optional<float3> grounded_position =
                   world::ground_point(hub->position, _world, _object_classes,
                                       _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&hub->position, *grounded_position));
            }
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

   auto& selected = _interaction_targets.selection.view().front();

   if (std::holds_alternative<world::object_id>(selected)) {
      world::object* object =
         world::find_entity(_world.objects, std::get<world::object_id>(selected));

      if (not object) return;

      world::object new_object = *object;

      new_object.name = world::create_unique_name(_world.objects, new_object.name);
      new_object.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_object)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_light)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_path)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_region)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_sector)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_portal)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_hintnode)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_barrier)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_hub)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_connection)),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_boundary)),
                              _edit_context);
      _world_draw_mask.boundaries = true;
   }
   else if (std::holds_alternative<world::measurement_id>(selected)) {
      _measurement_tool_open = true;
      _edit_stack_world.apply(edits::make_creation_entity_set(world::measurement{}),
                              _edit_context);
   }

   _entity_creation_context = {};

   if (_entity_creation_config.placement_rotation == placement_rotation::manual_euler) {
      _entity_creation_config.placement_rotation = placement_rotation::manual_quaternion;
   }
}

void world_edit::focus_on_selection() noexcept
{
   if (_interaction_targets.selection.empty()) return;

   const math::bounding_box selection_bbox =
      world::selection_bbox_for_camera(_world, _interaction_targets.selection,
                                       _object_classes,
                                       _settings.graphics.path_node_size);

   constexpr float pitch = -0.7853982f;
   constexpr float yaw = 0.7853982f;

   const float3 bounding_sphere_centre =
      (selection_bbox.max + selection_bbox.min) / 2.0f;
   const float bounding_sphere_radius =
      distance(selection_bbox.max, selection_bbox.min) / 2.0f;

   _camera.pitch(pitch);
   _camera.yaw(yaw);

   const float camera_offset =
      bounding_sphere_radius /
      std::sin((_camera.fov() / _camera.aspect_ratio()) / 2.0f);

   _camera.position(bounding_sphere_centre + camera_offset * _camera.back());
}

void world_edit::unhide_all() noexcept
{
   edits::bundle_vector bundle;

   const auto unhide_entities = [&]<typename T>(pinned_vector<T>& entities) {
      for (auto& entity : entities) {
         if (entity.hidden) {
            bundle.push_back(edits::make_set_value(&entity.hidden, false));
         }
      }
   };

   unhide_entities(_world.objects);
   unhide_entities(_world.lights);
   unhide_entities(_world.paths);
   unhide_entities(_world.regions);
   unhide_entities(_world.sectors);
   unhide_entities(_world.portals);
   unhide_entities(_world.hintnodes);
   unhide_entities(_world.barriers);
   unhide_entities(_world.planning_hubs);
   unhide_entities(_world.planning_connections);
   unhide_entities(_world.boundaries);
   unhide_entities(_world.measurements);

   if (bundle.size() == 1) {
      _edit_stack_world.apply(std::move(bundle.back()), _edit_context,
                              {.closed = true});
   }
   else if (not bundle.empty()) {
      _edit_stack_world.apply(edits::make_bundle(std::move(bundle)),
                              _edit_context, {.closed = true});
   }
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

   if (not _project_dir.empty()) {
      _renderer->async_save_thumbnail_disk_cache(
         (_project_dir / L".WorldEdit/thumbnails.bin").c_str());
   }

   _project_dir = path;
   _asset_libraries.source_directory(_project_dir);
   _project_world_paths.clear();

   const std::filesystem::path world_edit_project_files_path =
      _project_dir / L".WorldEdit";

   if (not std::filesystem::exists(world_edit_project_files_path)) {
      if (std::error_code ec;
          std::filesystem::create_directory(world_edit_project_files_path, ec)) {
         SetFileAttributesW(world_edit_project_files_path.c_str(), FILE_ATTRIBUTE_HIDDEN);
      }
      else {
         MessageBoxW(_window, L"Unable to .WorldEdit folder in selected folder. This folder is used to store things like the thumbnail disk cache. You can proceed but features that depend on this folder will be broken.",
                     L"Unable to create folder.", MB_OK);
      }
   }

   _renderer->async_load_thumbnail_disk_cache(
      (_project_dir / L".WorldEdit/thumbnails.bin").c_str());

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

void world_edit::close_project() noexcept
{
   _renderer->async_save_thumbnail_disk_cache(
      (_project_dir / L".WorldEdit/thumbnails.bin").c_str());

   _project_dir.clear();

   close_world();
}

void world_edit::load_world(std::filesystem::path path) noexcept
{
   if (not std::filesystem::exists(path)) return;

   close_world();

   try {
      _world = world::load_world(path, _stream);
      _world_path = path;

      _camera.position({0.0f, 0.0f, 0.0f});
      _camera.pitch(0.0f);
      _camera.yaw(0.0f);

      SetWindowTextA(_window,
                     fmt::format("WorldEdit - {}", _world_path.filename().string())
                        .c_str());
      _window_unsaved_star = false;
   }
   catch (std::exception& e) {
      _stream.write(fmt::format("Failed to load world '{}'!\n   Reason: \n{}",
                                path.filename().string(),
                                string::indent(2, e.what())));
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

      world::save_world(path, _world,
                        world::gather_terrain_cuts(_world.objects, _object_classes));

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

   _edit_stack_world.clear();
   _edit_stack_world.clear_modified_flag();

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

void world_edit::show_odf_in_explorer(const lowercase_string& asset_name) noexcept
{
   const auto asset_path = _asset_libraries.odfs.query_path(asset_name);

   if (asset_path.empty()) return;

   utility::try_show_in_explorer(asset_path);
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
   _display_scale.value = (_current_dpi / 96.0f) * _settings.ui.extra_scaling;
   _applied_user_display_scale = _settings.ui.extra_scaling;

   initialize_imgui_font();
   initialize_imgui_style();

   try {
      _renderer->display_scale_changed(_display_scale.value);
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }
}

void world_edit::initialize_imgui_font() noexcept
{
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
                                              16.0f * _display_scale, &font_config);

   try {
      _renderer->recreate_imgui_font_atlas();
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }
}

void world_edit::initialize_imgui_style() noexcept
{
   ImGui::GetStyle() = ImGuiStyle{};
   ImGui::GetStyle().ScaleAllSizes(_display_scale.value);
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
         _renderer = graphics::make_renderer(
            {.window = _window,
             .thread_pool = _thread_pool,
             .asset_libraries = _asset_libraries,
             .error_output = _stream,
             .display_scale = _display_scale.value,
             .use_debug_layer = _renderer_use_debug_layer,
             .use_legacy_barriers = _renderer_use_legacy_barriers,
             .never_use_shader_model_6_6 = _renderer_never_use_shader_model_6_6});

         _renderer->recreate_imgui_font_atlas();

         _stream.write("GPU Device was removed and has been recreated.\n");

         _world.terrain.untracked_fill_dirty_rects();
      }
      catch (graphics::gpu::exception& e) {
         if (e.error() != error::device_removed) {
            return handle_gpu_error(e);
         }

         switch (MessageBoxA(_window,
                             "Failed to recover from GPU device "
                             "removal.\n\nThe editor will now exit."
                             "\n\nSave world?",
                             "GPU Removed", MB_YESNO | MB_ICONERROR)) {
         case IDYES:
            save_world_with_picker();
            [[fallthrough]];
         case IDNO:
         default:
            std::terminate();
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
