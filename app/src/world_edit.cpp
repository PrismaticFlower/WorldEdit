
#include "world_edit.hpp"
#include "resource.h"

#include "assets/asset_libraries.hpp"
#include "assets/odf/default_object_class_definition.hpp"
#include "assets/texture/save_env_map.hpp"

#include "edits/add_block.hpp"
#include "edits/add_sector_object.hpp"
#include "edits/bundle.hpp"
#include "edits/creation_entity_set.hpp"
#include "edits/delete_block.hpp"
#include "edits/delete_entity.hpp"
#include "edits/insert_entity.hpp"
#include "edits/insert_node.hpp"
#include "edits/insert_point.hpp"
#include "edits/set_block.hpp"
#include "edits/set_class_name.hpp"
#include "edits/set_terrain_area.hpp"
#include "edits/set_value.hpp"

#include "math/frustum.hpp"
#include "math/plane_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"

#include "utility/file_pickers.hpp"
#include "utility/os_execute.hpp"
#include "utility/overload.hpp"
#include "utility/show_in_explorer.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include "world/blocks/utility/accessors.hpp"
#include "world/blocks/utility/bounding_box.hpp"
#include "world/blocks/utility/drag_select.hpp"
#include "world/blocks/utility/find.hpp"
#include "world/blocks/utility/grounding.hpp"
#include "world/blocks/utility/raycast.hpp"
#include "world/io/load.hpp"
#include "world/io/load_entity_group.hpp"
#include "world/io/save.hpp"
#include "world/io/save_entity_group.hpp"
#include "world/utility/entity_group_utilities.hpp"
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

#include <bit>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <backends/imgui_impl_win32.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <world/blocks/mesh_generate.hpp>

using namespace std::literals;

namespace we {

world_edit::world_edit(const HWND window, utility::command_line command_line)
   : _imgui_context{ImGui::CreateContext(), &ImGui::DestroyContext},
     _window{window},
     _renderer_prefer_high_performance_gpu{
        command_line.get_flag("-gpu_prefer_high_performance")},
     _renderer_use_debug_layer{command_line.get_flag("-gpu_debug_layer")},
     _renderer_use_legacy_barriers{
        command_line.get_flag("-gpu_legacy_barriers")},
     _renderer_never_use_shader_model_6_6{
        command_line.get_flag("-gpu_no_shader_model_6_6")},
     _renderer_never_use_open_existing_heap{
        command_line.get_flag("-gpu_no_open_existing_heap")},
     _renderer_never_use_write_buffer_immediate{
        command_line.get_flag("-gpu_no_write_buffer_immediate")},
     _renderer_never_use_relaxed_format_casting{
        command_line.get_flag("-gpu_no_relaxed_format_casting")},
     _renderer_never_use_target_independent_rasterization{
        command_line.get_flag("-gpu_no_target_independent_rasterization")}
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
      _renderer = graphics::make_renderer(get_renderer_init_params());
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }

   initialize_commands();
   initialize_hotkeys();

   ImGui_ImplWin32_Init(window);
   ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

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

   _window_width = static_cast<uint16>(rect.right - rect.left);
   _window_height = static_cast<uint16>(rect.bottom - rect.top);

   _camera.aspect_ratio(static_cast<float>(_window_width) /
                        static_cast<float>(_window_height));

   _settings = settings_load.get();
}

world_edit::~world_edit()
{
   if (not _project_dir.empty()) close_project();

   ImGui_ImplWin32_Shutdown();
}

auto world_edit::get_swap_chain_waitable_object() noexcept -> HANDLE
{
   return _renderer->get_swap_chain_waitable_object();
}

void world_edit::update()
{
   const float delta_time = _last_update_timer.elapsed();

   _last_update_timer.restart();

   if (_window_resized) {
      RECT rect{};
      GetClientRect(_window, &rect);

      const uint16 new_width = static_cast<uint16>(rect.right - rect.left);
      const uint16 new_height = static_cast<uint16>(rect.bottom - rect.top);

      if (new_width != _window_width or new_height != _window_height) {
         _window_width = new_width;
         _window_height = new_height;

         _camera.aspect_ratio(static_cast<float>(_window_width) /
                              static_cast<float>(_window_height));

         try {
            _renderer->window_resized(_window_width, _window_height);
         }
         catch (graphics::gpu::exception& e) {
            handle_gpu_error(e);
         }
      }
   }

   if (_applied_user_display_scale != _settings.ui.extra_scaling) {
      dpi_changed(_current_dpi);
   }

   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();

   _tool_visualizers.clear();

   _gizmos.update(make_camera_ray(_camera,
                                  {ImGui::GetMousePos().x, ImGui::GetMousePos().y},
                                  {ImGui::GetMainViewport()->Size.x,
                                   ImGui::GetMainViewport()->Size.y}),
                  {.left_mouse_down = ImGui::IsKeyDown(ImGuiKey_MouseLeft) and
                                      not ImGui::GetIO().WantCaptureMouse,
                   .ctrl_down = (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) or
                                 ImGui::IsKeyDown(ImGuiKey_RightCtrl)) and
                                not ImGui::GetIO().WantCaptureKeyboard},
                  _camera, _settings.ui.gizmo_scale);

   // Input!
   update_input();
   update_hovered_entity();

   // UI!
   update_window_text();
   update_ui();

   // Logic!
   _asset_libraries.update_loaded();
   _object_classes.update();

   // Render!
   update_camera(delta_time);

   ui_draw_select_box();

   try {
      _renderer->draw_frame(_camera, _world, _interaction_targets, _world_draw_mask,
                            _world_layers_draw_mask, _tool_visualizers,
                            _object_classes, _gizmos.get_draw_lists(),
                            {
                               .draw_terrain_grid = _draw_terrain_grid,
                               .draw_overlay_grid = _draw_overlay_grid,
                               .draw_foliage_map_overlay =
                                  _terrain_edit_tool == terrain_edit_tool::foliage_editor,
                               .delta_time = delta_time,
                               .overlay_grid_height = _editor_floor_height,
                               .overlay_grid_size = _editor_grid_size,
                            },
                            _settings.graphics);

      if (_env_map_render_requested) {
         _env_map_render_requested = false;

         graphics::env_map_result env_map =
            _renderer->draw_env_map(_env_map_render_params, _world, _world_draw_mask,
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

      _world_blocks_bvh_library.update(_world.blocks.custom_meshes, *_thread_pool);

      _world.terrain.untracked_clear_dirty_rects();
      _world.blocks.untracked_clear_dirty_ranges();
      _world.blocks.custom_meshes.clear_events();
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);

      ImGui::EndFrame();
   }
}

void world_edit::idle_exit() noexcept
{
   _last_update_timer.restart();
}

void world_edit::recreate_renderer_pending() noexcept
{
   if (_recreate_renderer_pending) recreate_renderer();
}

void world_edit::recreate_renderer() noexcept
{
   _recreate_renderer_pending = false;

   try {
      _renderer = nullptr;
      _renderer = graphics::make_renderer(get_renderer_init_params());
      _renderer->recreate_imgui_font_atlas();
   }
   catch (graphics::gpu::exception& e) {
      if (e.error() != graphics::gpu::error::device_removed) {
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

   _stream->write("GPU Device was removed and has been recreated.\n");

   _world.terrain.untracked_fill_dirty_rects();
   _world.blocks.untracked_fill_dirty_ranges();
   _world.blocks.custom_meshes.issue_restore_events();
}

void world_edit::update_window_text() noexcept
{
   if (_window_unsaved_star != _edit_stack_world.modified_flag()) {
      SetWindowTextA(_window, fmt::format("WorldEdit - {}{}",
                                          _edit_stack_world.modified_flag() ? "*" : "",
                                          _world_path.filename())
                                 .c_str());
      _window_unsaved_star = _edit_stack_world.modified_flag();
   }
}

void world_edit::update_input() noexcept
{
   const bool gizmos_want_mouse =
      _gizmos.want_capture_mouse() and not _rotate_camera and not _pan_camera;

   _hotkeys.update(ImGui::GetIO().WantCaptureMouse or gizmos_want_mouse,
                   ImGui::GetIO().WantCaptureKeyboard or
                      _gizmos.want_capture_keyboard());

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
   float cursor_distance = std::numeric_limits<float>::max();

   if (ImGui::GetIO().WantCaptureMouse or _gizmos.want_capture_mouse()) return;
   if (_rotate_camera or _pan_camera) return;
   if (_terrain_edit_tool != terrain_edit_tool::none) return;
   if (_block_editor_open) {
      if (_block_editor_context.tool != block_edit_tool::draw) return;
   }

   world::active_entity_types raycast_mask = _world_hit_mask;

   if (_interaction_targets.creation_entity.holds_entity()) {
      if (_entity_creation_context.tool == entity_creation_tool::from_object_bbox) {
         raycast_mask = world::active_entity_types{.objects = true, .terrain = false};
      }
      else if (_interaction_targets.creation_entity.is<world::planning_connection>()) {
         raycast_mask = world::active_entity_types{.objects = false,
                                                   .planning_hubs = true,
                                                   .terrain = false};
      }
      else if (_entity_creation_context.tool == entity_creation_tool::pick_sector) {
         raycast_mask = world::active_entity_types{.objects = false,
                                                   .sectors = true,
                                                   .terrain = false};
      }
   }
   else if (_selection_edit_tool == selection_edit_tool::pick_sector) {
      raycast_mask =
         world::active_entity_types{.objects = false, .sectors = true, .terrain = false};
   }
   else if (_selection_edit_tool == selection_edit_tool::add_branch_weight) {
      switch (_selection_add_branch_weight_context.step) {
      case add_branch_weight_step::connection:
         raycast_mask = world::active_entity_types{.objects = false,
                                                   .planning_connections = true,
                                                   .terrain = false};
         break;
      case add_branch_weight_step::target:
         raycast_mask = world::active_entity_types{.objects = false,
                                                   .planning_hubs = true,
                                                   .terrain = false};
         break;
      }
   }

   if (raycast_mask.objects) {
      if (std::optional<world::raycast_result<world::object>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                            _world.objects, _object_classes);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }

         if (hit->distance < cursor_distance) {
            _cursor_surface_normalWS = hit->normalWS;
            cursor_distance = hit->distance;
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
               world::make_path_id_node_mask(hit->id, hit->node_index);
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
            hovered_entity_distance = hit->distance;
         }

         if (hit->distance < cursor_distance and
             _interaction_targets.creation_entity.is<world::portal>()) {
            _cursor_surface_normalWS = hit->normalWS;
            cursor_distance = hit->distance;
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
                               _camera.projection_from_world(), _world.measurements);
          hit) {
         _interaction_targets.hovered_entity = *hit;
         hovered_entity_distance = 0.0f;
      }
   }

   if (raycast_mask.terrain and _world.terrain.active_flags.terrain) {
      if (auto hit = world::raycast(ray.origin, ray.direction, _world.terrain); hit) {
         if (*hit < hovered_entity_distance or *hit < cursor_distance) {
            if (not raycast_mask.objects or
                not world::point_inside_terrain_cut(ray.origin + ray.direction * *hit,
                                                    ray.direction, _world_layers_hit_mask,
                                                    _world.objects, _object_classes)) {
               if (*hit < hovered_entity_distance) {
                  _interaction_targets.hovered_entity = std::nullopt;
                  hovered_entity_distance = *hit;
               }

               if (*hit < cursor_distance) {
                  _cursor_surface_normalWS = std::nullopt;
                  cursor_distance = *hit;
               }
            }
         }
      }
   }

   if (raycast_mask.blocks) {
      if (std::optional<world::raycast_block_result> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_hit_mask,
                            _world.blocks, _world_blocks_bvh_library);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }

         if (hit->distance < cursor_distance) {
            cursor_distance = hit->distance;
         }
      }
   }

   if (cursor_distance == std::numeric_limits<float>::max()) {
      if (float hit = -(dot(ray.origin, float3{0.0f, 1.0f, 0.0f}) - _editor_floor_height) /
                      dot(ray.direction, float3{0.0f, 1.0f, 0.0f});
          hit > 0.0f) {
         cursor_distance = hit;
      }
   }

   if (cursor_distance != std::numeric_limits<float>::max()) {
      _cursor_positionWS = ray.origin + ray.direction * cursor_distance;
   }

   if (_interaction_targets.creation_entity.holds_entity() and
       _interaction_targets.hovered_entity) {
      const bool from_bbox_wants_hover =
         (_entity_creation_context.tool == entity_creation_tool::from_object_bbox and
          _interaction_targets.hovered_entity->is<world::object_id>());

      const bool connection_placement_wants_hover =
         _interaction_targets.creation_entity.is<world::planning_connection>() and
         _interaction_targets.hovered_entity->is<world::planning_hub_id>();

      const bool pick_sector_wants_hover =
         _entity_creation_context.tool == entity_creation_tool::pick_sector;

      const bool pick_sector_object_wants_hover =
         (_selection_edit_context.using_add_object_to_sector or
          _selection_edit_context.add_hovered_object) and
         _interaction_targets.hovered_entity->is<world::object_id>();

      const bool tool_wants_hover =
         from_bbox_wants_hover or connection_placement_wants_hover or
         pick_sector_wants_hover or pick_sector_object_wants_hover;

      if (not tool_wants_hover) {
         _interaction_targets.hovered_entity = std::nullopt;
      }
   }

   if (_animation_editor_open or _animation_group_editor_open or
       _animation_hierarchy_editor_open) {
      const bool pick_object_wants_hover =
         (_animation_editor_open and _animation_editor_context.pick_object.active) or
         (_animation_group_editor_open and
          _animation_group_editor_context.pick_object.active) or
         (_animation_hierarchy_editor_open and
          _animation_hierarchy_editor_context.pick_object.active);

      if (not pick_object_wants_hover) {
         _interaction_targets.hovered_entity = std::nullopt;
      }
   }

   if (_block_editor_open) {
      if (_block_editor_context.tool == block_edit_tool::draw) {
         _interaction_targets.hovered_entity = std::nullopt;
      }
   }
}

void world_edit::update_camera(const float delta_time)
{
   float3 camera_position = _camera.position();

   float sprint_factor = 1.0f;

   if (_move_sprint) {
      const float sprint_time = _sprint_timer.elapsed();

      sprint_factor = std::pow(sprint_time + 1.0f, _settings.camera.sprint_power);
   }
   else {
      _sprint_timer = _last_update_timer;
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
            _object_classes[object.class_handle].model->bounding_box;

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

bool world_edit::edit_stack_gizmo_position(const gizmo_position_desc& desc,
                                           float3* value) noexcept
{
   assert(_edit_context.is_memory_valid(value));

   float3 position = *value;

   const bool activated = _gizmos.gizmo_position(desc, position);

   if (activated) {
      _edit_stack_world.apply(edits::make_set_value(value, position), _edit_context);
   }

   if (_gizmos.can_close_last_edit()) _edit_stack_world.close_last();

   return activated;
}

void world_edit::ui_draw_select_box() noexcept
{
   if (not _selecting_entity) return;

   const float2 current_cursor_position = {ImGui::GetIO().MousePos.x,
                                           ImGui::GetIO().MousePos.y};

   float2 rect_min;
   float2 rect_max;

   if (not _selecting_entity_locked_size) {
      rect_min = min(current_cursor_position, _select_start_position);
      rect_max = max(current_cursor_position, _select_start_position);
   }
   else {
      float2 start = current_cursor_position;
      float2 end = current_cursor_position + _select_locked_sign * _select_locked_size;

      rect_min = min(start, end);
      rect_max = max(start, end);
   }

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
   _selecting_entity_locked_size = false;
}

void world_edit::finish_entity_select(const select_method method) noexcept
{
   _selecting_entity = false;
   _selection_edit_tool = selection_edit_tool::none;

   const float2 current_cursor_position =
      std::bit_cast<float2>(ImGui::GetMousePos());

   float2 rect_min;
   float2 rect_max;

   if (not _selecting_entity_locked_size) {
      rect_min = min(current_cursor_position, _select_start_position);
      rect_max = max(current_cursor_position, _select_start_position);
   }
   else {
      float2 start = current_cursor_position;
      float2 end = current_cursor_position + _select_locked_sign * _select_locked_size;

      rect_min = min(start, end);
      rect_max = max(start, end);
   }

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

      if (method == select_method::replace) {
         _interaction_targets.selection.clear();
      }

      frustum frustumWS{_camera.world_from_projection(),
                        {min_ndc_pos.x, min_ndc_pos.y, 0.0f},
                        {max_ndc_pos.x, max_ndc_pos.y, 1.0f}};

      if (_world_hit_mask.objects) {
         for (auto& object : _world.objects) {
            if (not _world_layers_hit_mask[object.layer] or object.hidden) {
               continue;
            }

            const quaternion inverse_rotation = conjugate(object.rotation);
            const float3 inverse_position = inverse_rotation * -object.position;

            const frustum frustumOS =
               transform(frustumWS, inverse_rotation, inverse_position);

            if (_object_classes[object.class_handle].model->bvh.intersects(frustumOS)) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(object.id);
               }
               else {
                  _interaction_targets.selection.add(object.id);
               }
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
                  return intersects(frustumWS, light.position, 2.8284f);
               case world::light_type::point:
                  return intersects(frustumWS, light.position, light.range);
               case world::light_type::spot: {
                  const float outer_cone_radius =
                     light.range * std::tan(light.outer_cone_angle * 0.5f);
                  const float inner_cone_radius =
                     light.range * std::tan(light.inner_cone_angle * 0.5f);
                  const float radius = std::min(outer_cone_radius, inner_cone_radius);

                  math::bounding_box bbox{.min = {-radius, -radius, 0.0f},
                                          .max = {radius, radius, light.range}};

                  bbox = light.rotation * bbox + light.position;

                  return intersects(frustumWS, bbox);
               }
               case world::light_type::directional_region_box: {
                  math::bounding_box bbox{.min = {-light.region_size},
                                          .max = {light.region_size}};

                  bbox = light.region_rotation * bbox + light.position;

                  return intersects(frustumWS, bbox);
               }
               case world::light_type::directional_region_sphere:
                  return intersects(frustumWS, light.position,
                                    length(light.region_size));
               case world::light_type::directional_region_cylinder: {
                  const float cylinder_length =
                     length(float2{light.region_size.x, light.region_size.z});

                  math::bounding_box bbox{.min = {-cylinder_length,
                                                  -light.region_size.y, -cylinder_length},
                                          .max = {cylinder_length,
                                                  light.region_size.y, cylinder_length}};

                  bbox = light.region_rotation * bbox + light.position;

                  return intersects(frustumWS, bbox);
               }
               default:
                  return false;
               }
            }();

            if (inside) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(light.id);
               }
               else {
                  _interaction_targets.selection.add(light.id);
               }
            }
         }
      }

      if (_world_hit_mask.paths) {
         for (auto& path : _world.paths) {
            if (not _world_layers_hit_mask[path.layer] or path.hidden) {
               continue;
            }

            for (uint32 i = 0; i < path.nodes.size(); ++i) {
               if (intersects(frustumWS, path.nodes[i].position,
                              0.707f * (_settings.graphics.path_node_size / 0.5f))) {
                  if (method == select_method::remove) {
                     _interaction_targets.selection.remove(
                        world::make_path_id_node_mask(path.id, i));
                  }
                  else {
                     _interaction_targets.selection.add(
                        world::make_path_id_node_mask(path.id, i));
                  }
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

                  return intersects(frustumWS, bbox);
               }
               case world::region_shape::sphere:
                  return intersects(frustumWS, region.position, length(region.size));
               case world::region_shape::cylinder: {
                  const float cylinder_length =
                     length(float2{region.size.x, region.size.z});

                  math::bounding_box bbox{.min = {-cylinder_length, -region.size.y,
                                                  -cylinder_length},
                                          .max = {cylinder_length, region.size.y,
                                                  cylinder_length}};

                  bbox = region.rotation * bbox + region.position;

                  return intersects(frustumWS, bbox);
               }
               default:
                  return false;
               }
            }();

            if (inside) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(region.id);
               }
               else {
                  _interaction_targets.selection.add(region.id);
               }
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

            if (intersects(frustumWS, bbox)) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(sector.id);
               }
               else {
                  _interaction_targets.selection.add(sector.id);
               }
            }
         }
      }

      if (_world_hit_mask.portals) {
         for (auto& portal : _world.portals) {
            if (portal.hidden) continue;

            if (intersects(frustumWS, portal.position,
                           std::max(portal.height, portal.width))) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(portal.id);
               }
               else {
                  _interaction_targets.selection.add(portal.id);
               }
            }
         }
      }

      if (_world_hit_mask.hintnodes) {
         for (auto& hintnode : _world.hintnodes) {
            if (not _world_layers_hit_mask[hintnode.layer] or hintnode.hidden) {
               continue;
            }

            if (intersects(frustumWS, hintnode.position, 2.0f)) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(hintnode.id);
               }
               else {
                  _interaction_targets.selection.add(hintnode.id);
               }
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

            if (intersects(frustumWS, bbox)) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(barrier.id);
               }
               else {
                  _interaction_targets.selection.add(barrier.id);
               }
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

            if (intersects(frustumWS, bbox)) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(hub.id);
               }
               else {
                  _interaction_targets.selection.add(hub.id);
               }
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

            const math::bounding_box start_bbox{
               .min = float3{-start.radius, -_settings.graphics.planning_connection_height,
                             -start.radius} +
                      start.position,
               .max = float3{start.radius, _settings.graphics.planning_connection_height,
                             start.radius} +
                      start.position};
            const math::bounding_box end_bbox{
               .min = float3{-end.radius, -_settings.graphics.planning_connection_height,
                             -end.radius} +
                      end.position,
               .max = float3{end.radius, _settings.graphics.planning_connection_height,
                             end.radius} +
                      end.position};

            const math::bounding_box bbox = math::combine(start_bbox, end_bbox);

            if (not intersects(frustumWS, bbox)) continue;

            const float3 normal =
               normalize(float3{-(start.position.z - end.position.z), 0.0f,
                                start.position.x - end.position.x});

            const std::array<float3, 4> quadWS = {start.position +
                                                     normal * start.radius,
                                                  start.position -
                                                     normal * start.radius,
                                                  end.position + normal * end.radius,
                                                  end.position - normal * end.radius};

            const float3 height_offset = {0.0f, _settings.graphics.planning_connection_height,
                                          0.0f};

            const std::array<float3, 8> cornersWS = {quadWS[0] + height_offset,
                                                     quadWS[1] + height_offset,
                                                     quadWS[2] + height_offset,
                                                     quadWS[3] + height_offset,
                                                     quadWS[0] - height_offset,
                                                     quadWS[1] - height_offset,
                                                     quadWS[2] - height_offset,
                                                     quadWS[3] - height_offset};

            constexpr static std::array<std::array<int8, 3>, 12> tris = {
               // Top
               3, 2, 0, //
               3, 0, 1, //

               // Bottom
               4, 6, 7, //
               5, 4, 7, //

               // Side 0
               0, 6, 4, //
               0, 2, 6, //

               // Side 1
               1, 5, 7, //
               7, 3, 1, //

               // Back
               4, 1, 0, //
               5, 1, 4, //

               // Front
               2, 3, 6, //
               6, 3, 7  //
            };

            for (const std::array<int8, 3>& tri : tris) {
               if (intersects(frustumWS, cornersWS[tri[0]], cornersWS[tri[1]],
                              cornersWS[tri[2]])) {
                  if (method == select_method::remove) {
                     _interaction_targets.selection.remove(connection.id);
                  }
                  else {
                     _interaction_targets.selection.add(connection.id);
                  }

                  break;
               }
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

            if (intersects(frustumWS, bbox)) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(boundary.id);
               }
               else {
                  _interaction_targets.selection.add(boundary.id);
               }
            }
         }
      }

      if (_world_hit_mask.measurements) {
         for (auto& measurement : _world.measurements) {
            if (measurement.hidden) continue;

            math::bounding_box bbox{min(measurement.start, measurement.end),
                                    max(measurement.start, measurement.end)};

            if (intersects(frustumWS, bbox)) {
               if (method == select_method::remove) {
                  _interaction_targets.selection.remove(measurement.id);
               }
               else {
                  _interaction_targets.selection.add(measurement.id);
               }
            }
         }
      }

      if (_world_hit_mask.blocks) {
         world::drag_select(_world.blocks, _world_blocks_bvh_library, frustumWS,
                            method == select_method::remove
                               ? world::block_drag_select_op::remove
                               : world::block_drag_select_op::add,
                            _interaction_targets.selection);
      }
   }
   else {
      if (method == select_method::replace) {
         _interaction_targets.selection.clear();
      }

      if (not _interaction_targets.hovered_entity) return;

      if (method == select_method::remove) {
         _interaction_targets.selection.remove(*_interaction_targets.hovered_entity);
      }
      else {
         _interaction_targets.selection.add(*_interaction_targets.hovered_entity);
      }
   }
}

void world_edit::lock_entity_select_size() noexcept
{
   if (not _selecting_entity) return;

   _selecting_entity_locked_size = true;

   const float2 current_cursor_position = {ImGui::GetIO().MousePos.x,
                                           ImGui::GetIO().MousePos.y};
   const float2 rect_min = min(current_cursor_position, _select_start_position);
   const float2 rect_max = max(current_cursor_position, _select_start_position);

   _select_locked_sign = sign(_select_start_position - current_cursor_position);
   _select_locked_size = abs(rect_max - rect_min);
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
         *_object_classes[object.class_handle].definition,
         new_object.instance_properties);
      new_object.class_handle = _object_classes.null_handle();
      new_object.id = _world.next_id.objects.aquire();

      _last_created_entities.last_object = new_object.id;
      _last_created_entities.last_used_object_classes.insert(new_object.class_name);

      if (_world.objects.size() == _world.objects.max_size()) {
         report_limit_reached("Max Objects ({}) Reached", _world.objects.max_size());

         return;
      }

      _edit_stack_world.apply(edits::make_insert_entity(std::move(new_object),
                                                        _object_classes),
                              _edit_context);

      if (not object.name.empty()) {
         _edit_stack_world
            .apply(edits::make_set_value(&object.name,
                                         world::create_unique_name(_world.objects,
                                                                   object.name)),
                   _edit_context, {.transparent = true});
      }

      if (_entity_creation_config.command_post_auto_place_meta_entities and
          string::iequals(_object_classes[object.class_handle]
                             .definition->header.class_label,
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

      _edit_stack_world
         .apply(edits::make_set_value(&light.name,
                                      world::create_unique_name(_world.lights,
                                                                light.name)),
                _edit_context, {.transparent = true});

      if (world::is_region_light(light)) {
         _edit_stack_world.apply(edits::make_set_value(&light.region_name,
                                                       world::create_unique_light_region_name(
                                                          _world.lights, _world.regions,
                                                          light.region_name.empty()
                                                             ? light.name
                                                             : light.region_name)),
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

         if (existing_path->nodes.size() == world::max_path_nodes) {
            report_limit_reached("Max Path Nodes ({}) Reached", world::max_path_nodes);

            return;
         }

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

      new_region.name =
         world::create_unique_name(_world.regions, _world.lights, new_region.name);
      new_region.id = _world.next_id.regions.aquire();

      _last_created_entities.last_region = new_region.id;

      if (_world.regions.size() == _world.regions.max_size()) {
         report_limit_reached("Max Regions ({}) Reached", _world.regions.max_size());

         return;
      }

      _edit_stack_world.apply(edits::make_insert_entity(std::move(new_region)),
                              _edit_context);

      _edit_stack_world
         .apply(edits::make_set_value(&region.name,
                                      world::create_unique_name(_world.regions,
                                                                _world.lights,
                                                                region.name)),
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

         const std::size_t insert_index =
            _entity_creation_config.placement_node_insert == placement_node_insert::nearest
               ? world::find_closest_edge(sector.points[0], *existing_sector) + 1
               : existing_sector->points.size();

         _edit_stack_world.apply(edits::make_insert_point(existing_sector->id, insert_index,
                                                          sector.points[0]),
                                 _edit_context);

         if (sector.base < existing_sector->base or
             sector.base + sector.height >
                existing_sector->base + existing_sector->height) {
            const float new_base = std::min(sector.base, existing_sector->base);
            const float new_height =
               std::max((existing_sector->base + existing_sector->height),
                        (sector.base + sector.height)) -
               new_base;

            _edit_stack_world.apply(edits::make_set_multi_value(&existing_sector->height,
                                                                new_height,
                                                                &existing_sector->base,
                                                                new_base),
                                    _edit_context, {.transparent = true});
         }

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
            _edit_stack_world.apply(edits::make_set_value(&sector.points,
                                                          {sector.points[0]}),
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
         .apply(edits::make_set_value(&portal.name,
                                      world::create_unique_name(_world.portals,
                                                                portal.name)),
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

      _edit_stack_world
         .apply(edits::make_set_value(&hintnode.name,
                                      world::create_unique_name(_world.hintnodes,
                                                                hintnode.name)),
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
         .apply(edits::make_set_value(&barrier.name,
                                      world::create_unique_name(_world.barriers,
                                                                barrier.name)),
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

         _edit_stack_world
            .apply(edits::make_set_value(&hub.name,
                                         world::create_unique_name(_world.planning_hubs,
                                                                   hub.name)),
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
              _interaction_targets.hovered_entity->is<world::planning_hub_id>())) {
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
                  _edit_stack_world.apply(edits::make_set_value(&connection.start_hub_index,
                                                                uint32{0}),
                                          _edit_context);

                  _edit_stack_world.apply(edits::make_set_value(&connection.end_hub_index,
                                                                uint32{0}),
                                          _edit_context, {.transparent = true});
               }
               else {
                  _edit_stack_world.apply(edits::make_creation_entity_set(world::creation_entity_none,
                                                                          _object_classes),
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

         _edit_stack_world
            .apply(edits::make_set_value(&connection.name,
                                         world::create_unique_name(_world.planning_connections,
                                                                   connection.name)),
                   _edit_context, {.transparent = true});

         _entity_creation_context.connection_link_started = false;
      }
      else {
         _edit_stack_world
            .apply(edits::make_set_value(
                      &connection.start_hub_index,
                      get_hub_index(_world.planning_hubs,
                                    _interaction_targets.hovered_entity
                                       ->get<world::planning_hub_id>())),
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

      _edit_stack_world
         .apply(edits::make_set_value(&boundary.name,
                                      world::create_unique_name(_world.boundaries,
                                                                boundary.name)),
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
   else if (_interaction_targets.creation_entity.is<world::entity_group>()) {

      const world::entity_group& group =
         _interaction_targets.creation_entity.get<world::entity_group>();

      if (_world.objects.size() + group.objects.size() > _world.objects.max_size()) {
         report_limit_reached("Max objects ({}) Reached", _world.objects.max_size());

         return;
      }
      else if (_world.lights.size() + group.lights.size() > _world.lights.max_size()) {
         report_limit_reached("Max lights ({}) Reached", _world.lights.max_size());

         return;
      }
      else if (_world.paths.size() + group.paths.size() > _world.paths.max_size()) {
         report_limit_reached("Max paths ({}) Reached", _world.paths.max_size());

         return;
      }
      else if (_world.regions.size() + group.regions.size() >
               _world.regions.max_size()) {
         report_limit_reached("Max regions ({}) Reached", _world.regions.max_size());

         return;
      }
      else if (_world.sectors.size() + group.sectors.size() >
               _world.sectors.max_size()) {
         report_limit_reached("Max sectors ({}) Reached", _world.sectors.max_size());

         return;
      }
      else if (_world.portals.size() + group.portals.size() >
               _world.portals.max_size()) {
         report_limit_reached("Max portals ({}) Reached", _world.portals.max_size());

         return;
      }
      else if (_world.hintnodes.size() + group.hintnodes.size() >
               _world.hintnodes.max_size()) {
         report_limit_reached("Max hintnodes ({}) Reached",
                              _world.hintnodes.max_size());

         return;
      }
      else if (_world.barriers.size() + group.barriers.size() >
               _world.barriers.max_size()) {
         report_limit_reached("Max barriers ({}) Reached", _world.barriers.max_size());

         return;
      }
      else if (_world.planning_hubs.size() + group.planning_hubs.size() >
               _world.planning_hubs.max_size()) {
         report_limit_reached("Max AI planning hubs ({}) Reached",
                              _world.planning_hubs.max_size());

         return;
      }
      else if (_world.planning_connections.size() + group.planning_connections.size() >
               _world.planning_connections.max_size()) {
         report_limit_reached("Max AI planning connections ({}) Reached",
                              _world.planning_connections.max_size());

         return;
      }
      else if (_world.boundaries.size() + group.boundaries.size() >
               _world.boundaries.max_size()) {
         report_limit_reached("Max boundarys ({}) Reached",
                              _world.boundaries.max_size());

         return;
      }
      else if (_world.measurements.size() + group.measurements.size() >
               _world.measurements.max_size()) {
         report_limit_reached("Max measurements ({}) Reached",
                              _world.measurements.max_size());

         return;
      }
      else if (_world.blocks.boxes.size() + group.blocks.boxes.size() > world::max_blocks) {
         report_limit_reached("Max blocks (boxes, {}) Reached", world::max_blocks);

         return;
      }
      else if (_world.blocks.ramps.size() + group.blocks.ramps.size() > world::max_blocks) {
         report_limit_reached("Max blocks (ramps, {}) Reached", world::max_blocks);

         return;
      }
      else if (_world.blocks.quads.size() + group.blocks.quads.size() > world::max_blocks) {
         report_limit_reached("Max blocks (quads, {}) Reached", world::max_blocks);

         return;
      }
      else if (_world.blocks.custom.size() + group.blocks.custom.description.size() >
               world::max_blocks) {
         report_limit_reached("Max blocks (custom, {}) Reached", world::max_blocks);

         return;
      }
      else if (_world.blocks.hemispheres.size() + group.blocks.hemispheres.size() >
               world::max_blocks) {
         report_limit_reached("Max blocks (hemispheres, {}) Reached", world::max_blocks);

         return;
      }

      const uint32 object_base_index = static_cast<uint32>(_world.objects.size());
      const uint32 path_base_index = static_cast<uint32>(_world.paths.size());
      const uint32 region_base_index = static_cast<uint32>(_world.regions.size());
      const uint32 sector_base_index = static_cast<uint32>(_world.sectors.size());
      const uint32 planning_hub_base_index =
         static_cast<uint32>(_world.planning_hubs.size());
      const uint32 planning_connection_base_index =
         static_cast<uint32>(_world.planning_connections.size());

      bool is_transparent_edit = false;

      for (const world::object& object : group.objects) {
         world::object new_object = object;

         new_object.layer = group.layer;
         new_object.rotation = group.rotation * new_object.rotation;
         new_object.position = group.rotation * new_object.position + group.position;
         new_object.name = world::create_unique_name(_world.objects, new_object.name);
         new_object.id = _world.next_id.objects.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_object),
                                                           _object_classes),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::light& light : group.lights) {
         world::light new_light = light;

         new_light.layer = group.layer;
         new_light.rotation = group.rotation * new_light.rotation;
         new_light.position = group.rotation * new_light.position + group.position;
         new_light.region_rotation = group.rotation * new_light.region_rotation;
         new_light.name = world::create_unique_name(_world.lights, new_light.name);
         new_light.id = _world.next_id.lights.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_light)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::path& path : group.paths) {
         world::path new_path = path;

         for (world::path::node& node : new_path.nodes) {
            node.rotation = group.rotation * node.rotation;
            node.position = group.rotation * node.position + group.position;
         }

         new_path.layer = group.layer;
         new_path.name = world::create_unique_name(_world.paths, new_path.name);
         new_path.id = _world.next_id.paths.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_path)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::region& region : group.regions) {
         world::region new_region = region;

         new_region.layer = group.layer;
         new_region.rotation = group.rotation * new_region.rotation;
         new_region.position = group.rotation * new_region.position + group.position;
         new_region.name = world::create_unique_name(_world.regions, _world.lights,
                                                     new_region.name);
         new_region.id = _world.next_id.regions.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_region)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::sector& sector : group.sectors) {
         world::sector new_sector = sector;

         float2 sector_centre = {0.0f, 0.0f};

         for (const float2& point : new_sector.points) {
            sector_centre += (point);
         }

         sector_centre /= static_cast<float>(new_sector.points.size());

         const float3 rotated_sector_centre =
            group.rotation * float3{sector_centre.x, 0.0f, sector_centre.y};

         for (float2& point : new_sector.points) {
            const float3 rotated_point =
               group.rotation *
               float3{point.x - sector_centre.x, 0.0f, point.y - sector_centre.y};

            point = float2{rotated_point.x, rotated_point.z} +
                    float2{rotated_sector_centre.x, rotated_sector_centre.z} +
                    float2{group.position.x, group.position.z};
         }

         for (std::string& object : new_sector.objects) {
            object = world::get_placed_entity_name(object, _world.objects,
                                                   group, object_base_index);
         }

         new_sector.base += group.position.y;
         new_sector.name = world::create_unique_name(_world.sectors, new_sector.name);
         new_sector.id = _world.next_id.sectors.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_sector)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::portal& portal : group.portals) {
         world::portal new_portal = portal;

         if (not new_portal.sector1.empty()) {
            new_portal.sector1 =
               world::get_placed_entity_name(new_portal.sector1, _world.sectors,
                                             group, sector_base_index);
         }

         if (not new_portal.sector2.empty()) {
            new_portal.sector2 =
               world::get_placed_entity_name(new_portal.sector2, _world.sectors,
                                             group, sector_base_index);
         }

         new_portal.rotation = group.rotation * new_portal.rotation;
         new_portal.position = group.rotation * new_portal.position + group.position;
         new_portal.name = world::create_unique_name(_world.portals, new_portal.name);
         new_portal.id = _world.next_id.portals.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_portal)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::hintnode& hintnode : group.hintnodes) {
         world::hintnode new_hintnode = hintnode;

         if (not new_hintnode.command_post.empty()) {
            new_hintnode.command_post =
               world::get_placed_entity_name(new_hintnode.command_post, _world.objects,
                                             group, object_base_index);
         }

         new_hintnode.layer = group.layer;
         new_hintnode.rotation = group.rotation * new_hintnode.rotation;
         new_hintnode.position = group.rotation * new_hintnode.position + group.position;
         new_hintnode.name =
            world::create_unique_name(_world.hintnodes, new_hintnode.name);
         new_hintnode.id = _world.next_id.hintnodes.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_hintnode)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::barrier& barrier : group.barriers) {
         world::barrier new_barrier = barrier;

         new_barrier.rotation_angle = group.rotation_angle + new_barrier.rotation_angle;
         new_barrier.position = group.rotation * new_barrier.position + group.position;
         new_barrier.name =
            world::create_unique_name(_world.barriers, new_barrier.name);
         new_barrier.id = _world.next_id.barriers.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_barrier)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::planning_hub& hub : group.planning_hubs) {
         world::planning_hub new_hub = hub;

         for (world::planning_branch_weights& weight : new_hub.weights) {
            weight.hub_index += planning_hub_base_index;
            weight.connection_index += planning_connection_base_index;
         }

         new_hub.position = group.rotation * new_hub.position + group.position;
         new_hub.name = world::create_unique_name(_world.planning_hubs, new_hub.name);
         new_hub.id = _world.next_id.planning_hubs.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_hub)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::planning_connection& connection : group.planning_connections) {
         world::planning_connection new_connection = connection;

         new_connection.start_hub_index += planning_hub_base_index;
         new_connection.end_hub_index += planning_hub_base_index;
         new_connection.name = world::create_unique_name(_world.planning_connections,
                                                         new_connection.name);
         new_connection.id = _world.next_id.planning_connections.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_connection)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::boundary& boundary : group.boundaries) {
         world::boundary new_boundary = boundary;

         new_boundary.position = group.rotation * new_boundary.position + group.position;
         new_boundary.name =
            world::create_unique_name(_world.boundaries, new_boundary.name);
         new_boundary.id = _world.next_id.boundaries.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_boundary)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      for (const world::measurement& measurement : group.measurements) {
         world::measurement new_measurement = measurement;

         new_measurement.start = group.rotation * new_measurement.start + group.position;
         new_measurement.end = group.rotation * new_measurement.end + group.position;
         new_measurement.id = _world.next_id.measurements.aquire();

         _edit_stack_world.apply(edits::make_insert_entity(std::move(new_measurement)),
                                 _edit_context,
                                 {.transparent =
                                     std::exchange(is_transparent_edit, true)});
      }

      std::array<std::optional<uint8>, world::max_block_materials> block_material_remap;

      for (uint32 material_index = 0;
           material_index <
           std::min(group.blocks.materials.size(), world::max_block_materials);
           ++material_index) {
         if (std::optional<uint8> equivalent_material =
                world::find_block_equivalent_material(_world.blocks,
                                                      group.blocks.materials[material_index]);
             equivalent_material) {
            block_material_remap[material_index] = *equivalent_material;

            continue;
         }

         if (std::optional<uint8> empty_material =
                world::find_block_empty_material(_world.blocks);
             empty_material) {
            block_material_remap[material_index] = *empty_material;

            _edit_stack_world.apply(
               edits::make_set_block_material(&_world.blocks.materials[*empty_material],
                                              group.blocks.materials[material_index],
                                              *empty_material,
                                              &_world.blocks.materials_dirty),
               _edit_context,
               {.transparent = std::exchange(is_transparent_edit, true)});
         }
      }

      for (const world::block_description_box& box : group.blocks.boxes) {
         world::block_description_box new_box = box;

         new_box.rotation = group.rotation * new_box.rotation;
         new_box.position = group.rotation * new_box.position + group.position;

         for (uint8& material : new_box.surface_materials) {
            if (block_material_remap[material]) {
               material = *block_material_remap[material];
            }
            else {
               material = 0;
            }
         }

         _edit_stack_world
            .apply(edits::make_add_block(new_box, group.layer,
                                         _world.blocks.next_id.boxes.aquire()),
                   _edit_context,
                   {.transparent = std::exchange(is_transparent_edit, true)});
      }

      for (const world::block_description_ramp& ramp : group.blocks.ramps) {
         world::block_description_ramp new_ramp = ramp;

         new_ramp.rotation = group.rotation * new_ramp.rotation;
         new_ramp.position = group.rotation * new_ramp.position + group.position;

         for (uint8& material : new_ramp.surface_materials) {
            if (block_material_remap[material]) {
               material = *block_material_remap[material];
            }
            else {
               material = 0;
            }
         }

         _edit_stack_world
            .apply(edits::make_add_block(new_ramp, group.layer,
                                         _world.blocks.next_id.ramps.aquire()),
                   _edit_context,
                   {.transparent = std::exchange(is_transparent_edit, true)});
      }

      for (const world::block_description_quad& quad : group.blocks.quads) {
         world::block_description_quad new_quad = quad;

         new_quad.vertices = {
            group.rotation * quad.vertices[0] + group.position,
            group.rotation * quad.vertices[1] + group.position,
            group.rotation * quad.vertices[2] + group.position,
            group.rotation * quad.vertices[3] + group.position,
         };

         for (uint8& material : new_quad.surface_materials) {
            if (block_material_remap[material]) {
               material = *block_material_remap[material];
            }
            else {
               material = 0;
            }
         }

         _edit_stack_world
            .apply(edits::make_add_block(new_quad, group.layer,
                                         _world.blocks.next_id.quads.aquire()),
                   _edit_context,
                   {.transparent = std::exchange(is_transparent_edit, true)});
      }

      for (const world::block_description_custom& block :
           group.blocks.custom.description) {
         world::block_description_custom new_block = block;

         new_block.rotation = group.rotation * new_block.rotation;
         new_block.position = group.rotation * new_block.position + group.position;

         for (uint8& material : new_block.surface_materials) {
            if (block_material_remap[material]) {
               material = *block_material_remap[material];
            }
            else {
               material = 0;
            }
         }

         _edit_stack_world
            .apply(edits::make_add_block(new_block, group.layer,
                                         _world.blocks.next_id.custom.aquire()),
                   _edit_context,
                   {.transparent = std::exchange(is_transparent_edit, true)});
      }

      for (const world::block_description_hemisphere& hemisphere :
           group.blocks.hemispheres) {
         world::block_description_hemisphere new_hemisphere = hemisphere;

         new_hemisphere.rotation = group.rotation * new_hemisphere.rotation;
         new_hemisphere.position =
            group.rotation * new_hemisphere.position + group.position;

         for (uint8& material : new_hemisphere.surface_materials) {
            if (block_material_remap[material]) {
               material = *block_material_remap[material];
            }
            else {
               material = 0;
            }
         }

         _edit_stack_world
            .apply(edits::make_add_block(new_hemisphere, group.layer,
                                         _world.blocks.next_id.hemispheres.aquire()),
                   _edit_context,
                   {.transparent = std::exchange(is_transparent_edit, true)});
      }

      for (uint32 object_index = object_base_index;
           object_index < _world.objects.size(); ++object_index) {
         world::object& object = _world.objects[object_index];

         for (uint32 prop_index = 0;
              prop_index < object.instance_properties.size(); ++prop_index) {
            const world::instance_property& prop =
               object.instance_properties[prop_index];

            if (prop.value.empty()) continue;

            std::string_view new_value;

            if (string::iequals("ControlZone", prop.key)) {
               new_value = world::get_placed_entity_name(prop.value, _world.objects,
                                                         group, object_base_index);
            }
            else if (string::icontains(prop.key, "Path")) {
               new_value = world::get_placed_entity_name(prop.value, _world.paths,
                                                         group, path_base_index);
            }
            else if (string::icontains(prop.key, "Region")) {
               for (uint32 i = 0; i < group.regions.size(); ++i) {
                  if (not string::iequals(prop.value, group.regions[i].description)) {
                     continue;
                  }

                  world::region& region = _world.regions[region_base_index + i];

                  if (not string::iequals(region.name, region.description)) {
                     _edit_stack_world.apply(edits::make_set_value(&region.description,
                                                                   region.name),
                                             _edit_context, {.transparent = true});
                  }

                  new_value = region.description;
               }
            }

            if (not new_value.empty()) {
               _edit_stack_world
                  .apply(edits::make_set_vector_value(&object.instance_properties, prop_index,
                                                      &world::instance_property::value,
                                                      std::string{new_value}),
                         _edit_context, {.transparent = true});
            }
         }
      }

      _last_created_entities.last_layer = group.layer;
   }

   _entity_creation_config.placement_mode = placement_mode::cursor;
}

void world_edit::place_creation_entity_at_camera() noexcept
{
   if (not _interaction_targets.creation_entity.holds_entity()) return;

   const quaternion rotation = make_quat_from_matrix(_camera.world_from_view()) *
                               quaternion{0.0f, 0.0f, 1.0f, 0.0f};

   if (_interaction_targets.creation_entity.is<world::object>()) {
      world::object& object =
         _interaction_targets.creation_entity.get<world::object>();

      _edit_stack_world.apply(edits::make_set_multi_value(&object.rotation,
                                                          rotation, &object.position,
                                                          _camera.position()),
                              _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::light>()) {
      world::light& light = _interaction_targets.creation_entity.get<world::light>();

      _edit_stack_world.apply(edits::make_set_multi_value(&light.rotation,
                                                          rotation, &light.position,
                                                          _camera.position()),
                              _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::path>()) {
      world::path& path = _interaction_targets.creation_entity.get<world::path>();

      if (path.nodes.empty()) std::terminate();

      _edit_stack_world.apply(edits::make_set_creation_path_node_location(
                                 rotation, _camera.position(), _edit_context.euler_rotation),
                              _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::region>()) {
      world::region& region =
         _interaction_targets.creation_entity.get<world::region>();

      _edit_stack_world.apply(edits::make_set_multi_value(&region.rotation,
                                                          rotation, &region.position,
                                                          _camera.position()),
                              _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::sector>()) {
      world::sector& sector =
         _interaction_targets.creation_entity.get<world::sector>();

      if (sector.points.empty()) std::terminate();

      _edit_stack_world
         .apply(edits::make_set_vector_value(&sector.points, 0,
                                             float2{_camera.position().x,
                                                    _camera.position().z}),
                _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::portal>()) {
      world::portal& portal =
         _interaction_targets.creation_entity.get<world::portal>();

      _edit_stack_world.apply(edits::make_set_multi_value(&portal.rotation,
                                                          rotation, &portal.position,
                                                          _camera.position()),
                              _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::hintnode>()) {
      world::hintnode& hintnode =
         _interaction_targets.creation_entity.get<world::hintnode>();

      _edit_stack_world.apply(edits::make_set_multi_value(&hintnode.rotation,
                                                          rotation, &hintnode.position,
                                                          _camera.position()),
                              _edit_context, {.transparent = true});
   }
   else if (_interaction_targets.creation_entity.is<world::barrier>()) {
      return;
   }
   else if (_interaction_targets.creation_entity.is<world::planning_hub>()) {
      return;
   }
   else if (_interaction_targets.creation_entity.is<world::planning_connection>()) {
      return;
   }
   else if (_interaction_targets.creation_entity.is<world::boundary>()) {
      return;
   }
   else if (_interaction_targets.creation_entity.is<world::measurement>()) {
      return;
   }
   else if (_interaction_targets.creation_entity.is<world::entity_group>()) {
      return;
   }

   place_creation_entity();
}

void world_edit::command_post_auto_place_meta_entities(world::object& object) noexcept
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
         _world, _object_classes);

   for (uint32 i = 0; i < object.instance_properties.size(); ++i) {
      const std::string_view key = object.instance_properties[i].key;

      if (string::iequals(key, "CaptureRegion")) {
         _edit_stack_world.apply(edits::make_set_vector_value(
                                    &object.instance_properties, i,
                                    &world::instance_property::value,
                                    command_post_linked_entities.capture_region.description),
                                 _edit_context,
                                 {.closed = true, .transparent = true});
      }
      else if (string::iequals(key, "ControlRegion")) {
         _edit_stack_world.apply(edits::make_set_vector_value(
                                    &object.instance_properties, i,
                                    &world::instance_property::value,
                                    command_post_linked_entities.control_region.description),
                                 _edit_context,
                                 {.closed = true, .transparent = true});
      }
      else if (string::iequals(key, "SpawnPath")) {
         _edit_stack_world.apply(
            edits::make_set_vector_value(&object.instance_properties, i,
                                         &world::instance_property::value,
                                         command_post_linked_entities.spawn_path.name),
            _edit_context, {.closed = true, .transparent = true});
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

   _edit_stack_world.apply(edits::make_set_class_name(
                              &_interaction_targets.creation_entity.get<world::object>(),
                              class_name, _object_classes),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_connection),
                                                              _object_classes),
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

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_hub),
                                                              _object_classes),
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

void world_edit::paste() noexcept
{
   const char* clipboard_text_cstr = ImGui::GetClipboardText();

   if (not clipboard_text_cstr) return;

   std::string_view clipboard_text = clipboard_text_cstr;

   if (clipboard_text.empty()) return;

   try {
      world::entity_group group =
         world::load_entity_group_from_string(clipboard_text, *_stream);

      if (world::is_entity_group_empty(group)) return;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(group),
                                                              _object_classes),
                              _edit_context);
   }
   catch (std::exception& e) {
      _stream->write("Couldn't paste text from clipboard into "
                     "world as entity group.\n   Reason: \n{}",
                     string::indent(2, e.what()));
   }
}

void world_edit::copy_selected() noexcept
{
   world::entity_group group =
      world::make_entity_group_from_selection(_world, _interaction_targets.selection);

   world::centre_entity_group(group);

   ImGui::SetClipboardText(world::save_entity_group_to_string(group).c_str());
}

void world_edit::cut_selected() noexcept
{
   copy_selected();
   delete_selected();
}

void world_edit::delete_selected() noexcept
{
   if (_interaction_targets.creation_entity.holds_entity()) {
      _edit_stack_world.apply(edits::make_creation_entity_set(world::creation_entity_none,
                                                              _object_classes),
                              _edit_context);

      return;
   }

   bool first_delete = true;

   while (not _interaction_targets.selection.empty()) {
      const auto& selected = _interaction_targets.selection.view().back();

      if (not is_valid(selected, _world)) continue;

      const bool transparent_edit = not std::exchange(first_delete, false);

      if (selected.is<world::object_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::object_id>(),
                                                           _world, _object_classes),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::light_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::light_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::path_id_node_mask>()) {
         const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

         world::path* path = world::find_entity(_world.paths, id);

         const int32 node_count =
            static_cast<int32>(std::min(path->nodes.size(), world::max_path_nodes));

         for (int32 i = node_count - 1; i >= 0; --i) {
            if (not node_mask[i]) continue;

            _edit_stack_world.apply(edits::make_delete_entity(path->id,
                                                              static_cast<uint32>(i),
                                                              _world),
                                    _edit_context, {.transparent = transparent_edit});
         }
      }
      else if (selected.is<world::region_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::region_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::sector_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::sector_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::portal_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::portal_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::hintnode_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::hintnode_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::barrier_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::barrier_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::planning_hub_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::planning_hub_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::planning_connection_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::planning_connection_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::boundary_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::boundary_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::measurement_id>()) {
         _edit_stack_world.apply(edits::make_delete_entity(selected.get<world::measurement_id>(),
                                                           _world),
                                 _edit_context, {.transparent = transparent_edit});
      }
      else if (selected.is<world::block_id>()) {
         if (const std::optional<uint32> index =
                world::find_block(_world.blocks, selected.get<world::block_id>());
             index) {
            _edit_stack_world
               .apply(edits::make_delete_block(selected.get<world::block_id>().type(),
                                               *index),
                      _edit_context, {.transparent = transparent_edit});
         }
      }

      _interaction_targets.selection.remove(selected);
   }
}

void world_edit::align_selection(const float alignment) noexcept
{
   if (_interaction_targets.selection.empty()) return;

   edits::bundle_vector bundle;
   bundle.reserve(_interaction_targets.selection.size());

   const auto align_position = [=](const float3 position) {
      return float3{std::round(position.x / alignment) * alignment, position.y,
                    std::round(position.z / alignment) * alignment};
   };

   for (const auto& selected : _interaction_targets.selection) {
      if (selected.is<world::object_id>()) {
         world::object* object =
            world::find_entity(_world.objects, selected.get<world::object_id>());

         if (object) {
            bundle.push_back(edits::make_set_value(&object->position,
                                                   align_position(object->position)));
         }
      }
      else if (selected.is<world::path_id_node_mask>()) {
         const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

         world::path* path = world::find_entity(_world.paths, id);

         if (path) {
            const std::size_t node_count =
               std::min(path->nodes.size(), world::max_path_nodes);

            for (uint32 node_index = 0; node_index < node_count; ++node_index) {
               if (not node_mask[node_index]) continue;

               const float3 position = path->nodes[node_index].position;

               bundle.push_back(
                  edits::make_set_vector_value(&path->nodes, node_index,
                                               &world::path::node::position,
                                               align_position(position)));
            }
         }
      }
      else if (selected.is<world::light_id>()) {
         world::light* light =
            world::find_entity(_world.lights, selected.get<world::light_id>());

         if (light) {
            bundle.push_back(edits::make_set_value(&light->position,
                                                   align_position(light->position)));
         }
      }
      else if (selected.is<world::region_id>()) {
         world::region* region =
            world::find_entity(_world.regions, selected.get<world::region_id>());

         if (region) {
            bundle.push_back(edits::make_set_value(&region->position,
                                                   align_position(region->position)));
         }
      }
      else if (selected.is<world::sector_id>()) {
         world::sector* sector =
            world::find_entity(_world.sectors, selected.get<world::sector_id>());

         if (sector) {
            std::vector<float2> new_points = sector->points;

            for (auto& point : new_points) {
               point = round(point / alignment) * alignment;
            }

            bundle.push_back(
               edits::make_set_value(&sector->points, std::move(new_points)));
         }
      }
      else if (selected.is<world::portal_id>()) {
         world::portal* portal =
            world::find_entity(_world.portals, selected.get<world::portal_id>());

         if (portal) {
            bundle.push_back(edits::make_set_value(&portal->position,
                                                   align_position(portal->position)));
         }
      }
      else if (selected.is<world::hintnode_id>()) {
         world::hintnode* hintnode =
            world::find_entity(_world.hintnodes, selected.get<world::hintnode_id>());

         if (hintnode) {
            bundle.push_back(edits::make_set_value(&hintnode->position,
                                                   align_position(hintnode->position)));
         }
      }
      else if (selected.is<world::barrier_id>()) {
         world::barrier* barrier =
            world::find_entity(_world.barriers, selected.get<world::barrier_id>());

         if (barrier) {
            bundle.push_back(edits::make_set_value(&barrier->position,
                                                   align_position(barrier->position)));
         }
      }
      else if (selected.is<world::planning_hub_id>()) {
         world::planning_hub* planning_hub =
            world::find_entity(_world.planning_hubs,
                               selected.get<world::planning_hub_id>());

         if (planning_hub) {
            bundle.push_back(
               edits::make_set_value(&planning_hub->position,
                                     align_position(planning_hub->position)));
         }
      }
      else if (selected.is<world::boundary_id>()) {
         world::boundary* boundary =
            world::find_entity(_world.boundaries, selected.get<world::boundary_id>());

         if (boundary) {
            bundle.push_back(
               edits::make_set_value(&boundary->position,
                                     round(boundary->position / alignment) * alignment));
         }
      }
      else if (selected.is<world::measurement_id>()) {
         world::measurement* measurement =
            world::find_entity(_world.measurements,
                               selected.get<world::measurement_id>());

         if (measurement) {
            bundle.push_back(
               edits::make_set_value(&measurement->start,
                                     round(measurement->start / alignment) * alignment));
            bundle.push_back(
               edits::make_set_value(&measurement->end,
                                     round(measurement->end / alignment) * alignment));
         }
      }
      else if (selected.is<world::block_id>()) {
         _Pragma("warning(push)");
         _Pragma("warning(default : 4061)"); // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
         _Pragma("warning(default : 4062)"); // enumerator 'identifier' in switch of enum 'enumeration' is not handled

         const world::block_id block_id = selected.get<world::block_id>();
         const std::optional<uint32> block_index =
            world::find_block(_world.blocks, block_id);

         if (block_index) {
            switch (block_id.type()) {
            case world::block_type::box: {
               const world::block_description_box& box =
                  _world.blocks.boxes.description[*block_index];

               bundle.push_back(
                  edits::make_set_block_box_metrics(*block_index, box.rotation,
                                                    align_position(box.position),
                                                    box.size));
            } break;
            case world::block_type::ramp: {
               const world::block_description_ramp& ramp =
                  _world.blocks.ramps.description[*block_index];

               bundle.push_back(
                  edits::make_set_block_ramp_metrics(*block_index, ramp.rotation,
                                                     align_position(ramp.position),
                                                     ramp.size));
            } break;
            case world::block_type::quad: {
               const world::block_description_quad& quad =
                  _world.blocks.quads.description[*block_index];

               bundle.push_back(edits::make_set_block_quad_metrics(
                  *block_index, {
                                   align_position(quad.vertices[0]),
                                   align_position(quad.vertices[1]),
                                   align_position(quad.vertices[2]),
                                   align_position(quad.vertices[3]),
                                }));
            } break;
            case world::block_type::custom: {
               const world::block_description_custom& block =
                  _world.blocks.custom.description[*block_index];

               bundle.push_back(
                  edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                       align_position(block.position),
                                                       block.mesh_description));
            } break;
            case world::block_type::hemisphere: {
               const world::block_description_hemisphere& hemisphere =
                  _world.blocks.hemispheres.description[*block_index];

               bundle.push_back(edits::make_set_block_hemisphere_metrics(
                  *block_index, hemisphere.rotation,
                  align_position(hemisphere.position), hemisphere.size));
            } break;
            case world::block_type::pyramid: {
               const world::block_description_pyramid& pyramid =
                  _world.blocks.pyramids.description[*block_index];

               bundle.push_back(
                  edits::make_set_block_pyramid_metrics(*block_index, pyramid.rotation,
                                                        align_position(pyramid.position),
                                                        pyramid.size));
            } break;
            }
         }

         _Pragma("warning(pop)");
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

void world_edit::toggle_hide_selection() noexcept
{
   edits::bundle_vector bundle;
   bundle.reserve(_interaction_targets.selection.size());

   world::selection new_selection;

   for (const auto& selected : _interaction_targets.selection) {
      if (selected.is<world::object_id>()) {
         world::object* object =
            world::find_entity(_world.objects, selected.get<world::object_id>());

         if (object) {
            bundle.push_back(edits::make_set_value(&object->hidden, not object->hidden));

            if (object->hidden) new_selection.add(object->id);
         }
      }
      else if (selected.is<world::light_id>()) {
         world::light* light =
            world::find_entity(_world.lights, selected.get<world::light_id>());

         if (light) {
            bundle.push_back(edits::make_set_value(&light->hidden, not light->hidden));

            if (light->hidden) new_selection.add(light->id);
         }
      }
      else if (selected.is<world::path_id_node_mask>()) {
         const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

         world::path* path = world::find_entity(_world.paths, id);

         if (path) {
            bundle.push_back(edits::make_set_value(&path->hidden, not path->hidden));

            if (path->hidden) {
               new_selection.add(world::path_id_node_mask{path->id, node_mask});
            }
         }
      }
      else if (selected.is<world::region_id>()) {
         world::region* region =
            world::find_entity(_world.regions, selected.get<world::region_id>());

         if (region) {
            bundle.push_back(edits::make_set_value(&region->hidden, not region->hidden));

            if (region->hidden) new_selection.add(region->id);
         }
      }
      else if (selected.is<world::sector_id>()) {
         world::sector* sector =
            world::find_entity(_world.sectors, selected.get<world::sector_id>());

         if (sector) {
            bundle.push_back(edits::make_set_value(&sector->hidden, not sector->hidden));

            if (sector->hidden) new_selection.add(sector->id);
         }
      }
      else if (selected.is<world::portal_id>()) {
         world::portal* portal =
            world::find_entity(_world.portals, selected.get<world::portal_id>());

         if (portal) {
            bundle.push_back(edits::make_set_value(&portal->hidden, not portal->hidden));

            if (portal->hidden) new_selection.add(portal->id);
         }
      }
      else if (selected.is<world::hintnode_id>()) {
         world::hintnode* hintnode =
            world::find_entity(_world.hintnodes, selected.get<world::hintnode_id>());

         if (hintnode) {
            bundle.push_back(
               edits::make_set_value(&hintnode->hidden, not hintnode->hidden));

            if (hintnode->hidden) new_selection.add(hintnode->id);
         }
      }
      else if (selected.is<world::barrier_id>()) {
         world::barrier* barrier =
            world::find_entity(_world.barriers, selected.get<world::barrier_id>());

         if (barrier) {
            bundle.push_back(
               edits::make_set_value(&barrier->hidden, not barrier->hidden));

            if (barrier->hidden) new_selection.add(barrier->id);
         }
      }
      else if (selected.is<world::planning_hub_id>()) {
         world::planning_hub* hub =
            world::find_entity(_world.planning_hubs,
                               selected.get<world::planning_hub_id>());

         if (hub) {
            bundle.push_back(edits::make_set_value(&hub->hidden, not hub->hidden));

            if (hub->hidden) new_selection.add(hub->id);
         }
      }
      else if (selected.is<world::planning_connection_id>()) {
         world::planning_connection* connection =
            world::find_entity(_world.planning_connections,
                               selected.get<world::planning_connection_id>());

         if (connection) {
            bundle.push_back(edits::make_set_value(&connection->hidden,
                                                   not connection->hidden));

            if (connection->hidden) new_selection.add(connection->id);
         }
      }
      else if (selected.is<world::boundary_id>()) {
         world::boundary* boundary =
            world::find_entity(_world.boundaries, selected.get<world::boundary_id>());

         if (boundary) {
            bundle.push_back(
               edits::make_set_value(&boundary->hidden, not boundary->hidden));

            if (boundary->hidden) new_selection.add(boundary->id);
         }
      }
      else if (selected.is<world::measurement_id>()) {
         world::measurement* measurement =
            world::find_entity(_world.measurements,
                               selected.get<world::measurement_id>());

         if (measurement) {
            bundle.push_back(edits::make_set_value(&measurement->hidden,
                                                   not measurement->hidden));

            if (measurement->hidden) new_selection.add(measurement->id);
         }
      }
      else if (selected.is<world::block_id>()) {
         const std::optional<uint32> block_index =
            world::find_block(_world.blocks, selected.get<world::block_id>());

         if (block_index) {
            bool& block_hidden =
               world::get_block_hidden(_world.blocks,
                                       selected.get<world::block_id>().type(),
                                       *block_index);

            bundle.push_back(edits::make_set_value(&block_hidden, not block_hidden));

            if (block_hidden) {
               new_selection.add(selected.get<world::block_id>());
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

   _interaction_targets.selection = new_selection;
}

void world_edit::ground_selection() noexcept
{
   edits::bundle_vector bundle;
   bundle.reserve(_interaction_targets.selection.size());

   for (const auto& selected : _interaction_targets.selection) {
      if (selected.is<world::object_id>()) {
         world::object* object =
            world::find_entity(_world.objects, selected.get<world::object_id>());

         if (object) {
            if (const std::optional<float3> grounded_position =
                   world::ground_object(*object, _world, _object_classes,
                                        _world_blocks_bvh_library, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&object->position, *grounded_position));
            }
         }
      }
      else if (selected.is<world::light_id>()) {
         world::light* light =
            world::find_entity(_world.lights, selected.get<world::light_id>());

         if (light) {
            if (const std::optional<float3> grounded_position =
                   world::ground_light(*light, _world, _object_classes,
                                       _world_blocks_bvh_library, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&light->position, *grounded_position));
            }
         }
      }
      else if (selected.is<world::path_id_node_mask>()) {
         const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

         world::path* path = world::find_entity(_world.paths, id);

         if (path) {
            const std::size_t node_count =
               std::min(path->nodes.size(), world::max_path_nodes);

            for (uint32 node_index = 0; node_index < node_count; ++node_index) {
               if (not node_mask[node_index]) continue;

               if (const std::optional<float3> grounded_position =
                      world::ground_point(path->nodes[node_index].position, _world,
                                          _object_classes, _world_blocks_bvh_library,
                                          _world_layers_hit_mask);
                   grounded_position) {
                  bundle.push_back(
                     edits::make_set_vector_value(&path->nodes, node_index,
                                                  &world::path::node::position,
                                                  *grounded_position));
               }
            }
         }
      }
      else if (selected.is<world::region_id>()) {
         world::region* region =
            world::find_entity(_world.regions, selected.get<world::region_id>());

         if (region) {
            if (const std::optional<float3> grounded_position =
                   world::ground_region(*region, _world, _object_classes,
                                        _world_blocks_bvh_library, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&region->position, *grounded_position));
            }
         }
      }
      else if (selected.is<world::sector_id>()) {
         world::sector* sector =
            world::find_entity(_world.sectors, selected.get<world::sector_id>());

         if (sector) {
            if (const std::optional<float> grounded_base =
                   world::ground_sector(*sector, _world, _object_classes,
                                        _world_blocks_bvh_library, _world_layers_hit_mask);
                grounded_base) {
               bundle.push_back(edits::make_set_value(&sector->base, *grounded_base));
            }
         }
      }
      else if (selected.is<world::portal_id>()) {
         world::portal* portal =
            world::find_entity(_world.portals, selected.get<world::portal_id>());

         if (portal) {
            if (const std::optional<float3> grounded_position =
                   world::ground_portal(*portal, _world, _object_classes,
                                        _world_blocks_bvh_library, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&portal->position, *grounded_position));
            }
         }
      }
      else if (selected.is<world::hintnode_id>()) {
         world::hintnode* hintnode =
            world::find_entity(_world.hintnodes, selected.get<world::hintnode_id>());

         if (hintnode) {
            if (const std::optional<float3> grounded_position =
                   world::ground_point(hintnode->position, _world, _object_classes,
                                       _world_blocks_bvh_library, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(edits::make_set_value(&hintnode->position,
                                                      *grounded_position));
            }
         }
      }
      else if (selected.is<world::barrier_id>()) {
         world::barrier* barrier =
            world::find_entity(_world.barriers, selected.get<world::barrier_id>());

         if (barrier) {
            if (const std::optional<float3> grounded_position =
                   world::ground_point(barrier->position, _world, _object_classes,
                                       _world_blocks_bvh_library, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&barrier->position, *grounded_position));
            }
         }
      }
      else if (selected.is<world::planning_hub_id>()) {
         world::planning_hub* hub =
            world::find_entity(_world.planning_hubs,
                               selected.get<world::planning_hub_id>());

         if (hub) {
            if (const std::optional<float3> grounded_position =
                   world::ground_point(hub->position, _world, _object_classes,
                                       _world_blocks_bvh_library, _world_layers_hit_mask);
                grounded_position) {
               bundle.push_back(
                  edits::make_set_value(&hub->position, *grounded_position));
            }
         }
      }
      else if (selected.is<world::block_id>()) {
         _Pragma("warning(push)");
         _Pragma("warning(default : 4061)"); // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
         _Pragma("warning(default : 4062)"); // enumerator 'identifier' in switch of enum 'enumeration' is not handled

         const world::block_id block_id = selected.get<world::block_id>();
         const std::optional<uint32> block_index =
            world::find_block(_world.blocks, block_id);

         if (block_index) {
            if (const std::optional<float3> grounded_position =
                   world::ground_block(block_id, *block_index, _world,
                                       _object_classes, _world_blocks_bvh_library,
                                       _world_layers_hit_mask);
                grounded_position) {
               switch (block_id.type()) {
               case world::block_type::box: {
                  const world::block_description_box& box =
                     _world.blocks.boxes.description[*block_index];

                  bundle.push_back(
                     edits::make_set_block_box_metrics(*block_index, box.rotation,
                                                       *grounded_position, box.size));
               } break;
               case world::block_type::ramp: {
                  const world::block_description_ramp& ramp =
                     _world.blocks.ramps.description[*block_index];

                  bundle.push_back(
                     edits::make_set_block_ramp_metrics(*block_index, ramp.rotation,
                                                        *grounded_position, ramp.size));
               } break;
               case world::block_type::quad: {
                  const world::block_description_quad& quad =
                     _world.blocks.quads.description[*block_index];

                  const math::bounding_box bbox = world::get_bounding_box(quad);

                  const float3 quad_centreWS = (bbox.min + bbox.max) / 2.0f;

                  bundle.push_back(edits::make_set_block_quad_metrics(
                     *block_index,
                     {
                        quad.vertices[0] - quad_centreWS + *grounded_position,
                        quad.vertices[1] - quad_centreWS + *grounded_position,
                        quad.vertices[2] - quad_centreWS + *grounded_position,
                        quad.vertices[3] - quad_centreWS + *grounded_position,
                     }));
               } break;
               case world::block_type::custom: {
                  const world::block_description_custom& block =
                     _world.blocks.custom.description[*block_index];

                  bundle.push_back(
                     edits::make_set_block_custom_metrics(*block_index, block.rotation,
                                                          *grounded_position,
                                                          block.mesh_description));
               } break;
               case world::block_type::hemisphere: {
                  const world::block_description_hemisphere& hemisphere =
                     _world.blocks.hemispheres.description[*block_index];

                  bundle.push_back(
                     edits::make_set_block_hemisphere_metrics(*block_index,
                                                              hemisphere.rotation,
                                                              *grounded_position,
                                                              hemisphere.size));
               } break;
               case world::block_type::pyramid: {
                  const world::block_description_pyramid& pyramid =
                     _world.blocks.pyramids.description[*block_index];

                  bundle.push_back(
                     edits::make_set_block_pyramid_metrics(*block_index, pyramid.rotation,
                                                           *grounded_position,
                                                           pyramid.size));
               } break;
               }
            }
         }

         _Pragma("warning(pop)");
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

   if (selected.is<world::object_id>()) {
      world::object* object =
         world::find_entity(_world.objects, selected.get<world::object_id>());

      if (not object) return;

      world::object new_object = *object;

      new_object.name = world::create_unique_name(_world.objects, new_object.name);
      new_object.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_object),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.objects = true;
   }
   else if (selected.is<world::light_id>()) {
      world::light* light =
         world::find_entity(_world.lights, selected.get<world::light_id>());

      if (not light) return;

      world::light new_light = *light;

      new_light.name = world::create_unique_name(_world.lights, new_light.name);
      new_light.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_light),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.lights = true;
   }
   else if (selected.is<world::path_id_node_mask>()) {
      const auto& [id, node_mask] = selected.get<world::path_id_node_mask>();

      world::path* path = world::find_entity(_world.paths, id);

      if (not path) return;

      world::path new_path = *path;

      new_path.name = world::create_unique_name(_world.paths, new_path.name);
      new_path.nodes = {world::path::node{}};
      new_path.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_path),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.paths = true;
   }
   else if (selected.is<world::region_id>()) {
      world::region* region =
         world::find_entity(_world.regions, selected.get<world::region_id>());

      if (not region) return;

      world::region new_region = *region;

      new_region.name =
         world::create_unique_name(_world.regions, _world.lights, new_region.name);
      new_region.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_region),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.regions = true;
   }
   else if (selected.is<world::sector_id>()) {
      world::sector* sector =
         world::find_entity(_world.sectors, selected.get<world::sector_id>());

      if (not sector) return;

      world::sector new_sector = *sector;

      new_sector.name = world::create_unique_name(_world.sectors, new_sector.name);
      new_sector.points = {{0.0f, 0.0f}};
      new_sector.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_sector),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.sectors = true;
   }
   else if (selected.is<world::portal_id>()) {
      world::portal* portal =
         world::find_entity(_world.portals, selected.get<world::portal_id>());

      if (not portal) return;

      world::portal new_portal = *portal;

      new_portal.name = world::create_unique_name(_world.portals, new_portal.name);
      new_portal.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_portal),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.portals = true;
   }
   else if (selected.is<world::hintnode_id>()) {
      world::hintnode* hintnode =
         world::find_entity(_world.hintnodes, selected.get<world::hintnode_id>());

      if (not hintnode) return;

      world::hintnode new_hintnode = *hintnode;

      new_hintnode.name =
         world::create_unique_name(_world.hintnodes, new_hintnode.name);
      new_hintnode.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_hintnode),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.hintnodes = true;
   }
   else if (selected.is<world::barrier_id>()) {
      world::barrier* barrier =
         world::find_entity(_world.barriers, selected.get<world::barrier_id>());

      if (not barrier) return;

      world::barrier new_barrier = *barrier;

      new_barrier.name = world::create_unique_name(_world.barriers, new_barrier.name);
      new_barrier.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_barrier),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.barriers = true;
   }
   else if (selected.is<world::planning_hub_id>()) {
      world::planning_hub* hub =
         world::find_entity(_world.planning_hubs,
                            selected.get<world::planning_hub_id>());

      if (not hub) return;

      world::planning_hub new_hub = *hub;

      new_hub.name = world::create_unique_name(_world.planning_hubs, new_hub.name);
      new_hub.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_hub),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.planning_hubs = true;
      _world_draw_mask.planning_connections = true;
   }
   else if (selected.is<world::planning_connection_id>()) {
      world::planning_connection* connection =
         world::find_entity(_world.planning_connections,
                            selected.get<world::planning_connection_id>());

      if (not connection) return;

      world::planning_connection new_connection = *connection;

      new_connection.name =
         world::create_unique_name(_world.planning_connections, new_connection.name);
      new_connection.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_connection),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.planning_hubs = true;
      _world_draw_mask.planning_connections = true;
   }
   else if (selected.is<world::boundary_id>()) {
      world::boundary* boundary =
         world::find_entity(_world.boundaries, selected.get<world::boundary_id>());

      if (not boundary) return;

      world::boundary new_boundary = *boundary;

      new_boundary.name =
         world::create_unique_name(_world.boundaries, new_boundary.name);
      new_boundary.id = world::max_id;

      _edit_stack_world.apply(edits::make_creation_entity_set(std::move(new_boundary),
                                                              _object_classes),
                              _edit_context);
      _world_draw_mask.boundaries = true;
   }
   else if (selected.is<world::measurement_id>()) {
      _measurement_tool_open = true;
      _edit_stack_world.apply(edits::make_creation_entity_set(world::measurement{},
                                                              _object_classes),
                              _edit_context);
   }
   else if (selected.is<world::block_id>()) {
      _edit_stack_world.apply(edits::make_creation_entity_set(
                                 world::make_entity_group_from_block_id(
                                    _world.blocks, selected.get<world::block_id>()),
                                 _object_classes),
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

   if (_orbit_camera_active) {
      setup_orbit_camera();

      return;
   }

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

   const auto unhide_blocks = [&](pinned_vector<bool>& blocks_hidden) {
      for (bool& hidden : blocks_hidden) {
         if (hidden) {
            bundle.push_back(edits::make_set_value(&hidden, false));
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

   unhide_blocks(_world.blocks.boxes.hidden);
   unhide_blocks(_world.blocks.ramps.hidden);
   unhide_blocks(_world.blocks.quads.hidden);
   unhide_blocks(_world.blocks.custom.hidden);
   unhide_blocks(_world.blocks.hemispheres.hidden);
   unhide_blocks(_world.blocks.pyramids.hidden);

   if (bundle.size() == 1) {
      _edit_stack_world.apply(std::move(bundle.back()), _edit_context,
                              {.closed = true});
   }
   else if (not bundle.empty()) {
      _edit_stack_world.apply(edits::make_bundle(std::move(bundle)),
                              _edit_context, {.closed = true});
   }
}

void world_edit::floor_terrain() noexcept
{
   container::dynamic_array_2d<int16> new_height_map = _world.terrain.height_map;

   int16 min_height = INT16_MAX;

   for (int16 v : new_height_map) {
      if (v < min_height) min_height = v;
   }

   for (int16& v : new_height_map) v = INT16_MIN + (v - min_height);

   _edit_stack_world.apply(edits::make_set_terrain_area(0, 0, std::move(new_height_map)),
                           _edit_context, {.closed = true});
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

void world_edit::open_project(const io::path& path) noexcept
{
   if (not io::exists(io::compose_path(path, "Worlds"))) {
      if (MessageBoxW(_window, L"The selected folder does not appear to be a mod data folder. Are you sure you wish to open it?",
                      L"Not a Data Folder", MB_YESNO) != IDYES) {
         return;
      }
   }

   if (not _project_dir.empty()) {
      _renderer->async_save_thumbnail_disk_cache(
         io::compose_path(_project_dir, ".WorldEdit/thumbnails.bin").c_str());
   }

   _project_dir = path;
   _asset_libraries.source_directory(_project_dir);
   _project_world_paths.clear();

   const io::path world_edit_project_files_path =
      io::compose_path(_project_dir, ".WorldEdit");

   if (not io::exists(world_edit_project_files_path)) {
      if (io::create_directory(world_edit_project_files_path)) {
         SetFileAttributesW(io::wide_path{world_edit_project_files_path}.c_str(),
                            FILE_ATTRIBUTE_HIDDEN);
      }
      else {
         MessageBoxW(_window, L"Unable to .WorldEdit folder in selected folder. This folder is used to store things like the thumbnail disk cache. You can proceed but features that depend on this folder will be broken.",
                     L"Unable to create folder.", MB_OK);
      }
   }

   _renderer->async_load_thumbnail_disk_cache(
      io::compose_path(_project_dir, ".WorldEdit/thumbnails.bin").c_str());

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

   if (not path or not io::exists(*path)) return;

   open_project(*path);
}

void world_edit::close_project() noexcept
{
   _renderer->async_save_thumbnail_disk_cache(
      io::compose_path(_project_dir, ".WorldEdit/thumbnails.bin").c_str());

   _project_dir = "";

   close_world();
}

void world_edit::load_world(const io::path& path) noexcept
{
   if (not io::exists(path)) return;

   close_world();

   try {
      _world = world::load_world(path, *_stream);
      _world_path = path;

      for (world::object& object : _world.objects) {
         object.class_handle = _object_classes.acquire(object.class_name);
      }

      _camera.position({0.0f, 0.0f, 0.0f});
      _camera.pitch(0.0f);
      _camera.yaw(0.0f);

      SetWindowTextA(_window,
                     fmt::format("WorldEdit - {}", _world_path.filename()).c_str());
      _window_unsaved_star = false;
   }
   catch (std::exception& e) {
      _stream->write(fmt::format("Failed to load world '{}'!\n   Reason: \n{}",
                                 path.filename(), string::indent(2, e.what())));
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

void world_edit::save_world(const io::path& path) noexcept
{
   try {
      const world::save_flags flags =
         {.save_gamemodes = not _settings.preferences.dont_save_world_gamemodes,
          .save_boundary_bf1_format = _settings.preferences.save_world_boundary_bf1_format,
          .save_effects = not _settings.preferences.dont_save_world_effects};

      world::save_world(path, _world,
                        world::gather_terrain_cuts(_world.objects, _object_classes),
                        flags);

      _edit_stack_world.clear_modified_flag();
   }
   catch (std::exception& e) {
      auto message =
         fmt::format("Failed to save world!\n   Reason: \n{}\n"
                     "   Incomplete save data maybe present on disk.\n",
                     string::indent(2, e.what()));

      _stream->write(message);

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
   _world_path = "";

   _edit_stack_world.clear();
   _edit_stack_world.clear_modified_flag();

   SetWindowTextA(_window, "WorldEdit");
   _window_unsaved_star = false;
}

void world_edit::save_entity_group_with_picker(const world::entity_group& group) noexcept
{
   static constexpr GUID save_entity_group_picker_guid =
      {0xd9b928cb, 0xdb72, 0x4c25, {0xa5, 0xf, 0xb2, 0xd3, 0x54, 0xe2, 0xeb, 0xbb}};

   auto path = utility::show_file_save_picker(
      {.title = L"Save Entity Group"s,
       .ok_button_label = L"Save"s,
       .forced_start_folder = _world_path,
       .filters = {utility::file_picker_filter{.name = L"Entity Group"s, .filter = L"*.eng"s}},
       .picker_guid = save_entity_group_picker_guid,
       .window = _window,
       .must_exist = true});

   if (not path) return;
   if (not path->extension().empty()) *path += ".eng";

   try {
      world::save_entity_group(*path, group);
   }
   catch (std::exception& e) {
      auto message =
         fmt::format("Failed to save entity group!\n   Reason: \n{}",
                     string::indent(2, e.what()));

      _stream->write(message);

      MessageBoxA(_window, message.data(), "Failed to save entity group!", MB_OK);
   }
}

void world_edit::enumerate_project_worlds() noexcept
{
   const io::path worlds_path = io::compose_path(_project_dir, "Worlds");

   if (not io::exists(worlds_path)) {
      MessageBoxW(_window, L"Unable to enumerate data folder worlds. Loading worlds will require manual navigation.",
                  L"Error", MB_OK);

      return;
   }

   for (const io::directory_entry& entry : io::directory_iterator{worlds_path}) {
      if (not entry.is_file) continue;

      if (string::iequals(entry.path.extension(), ".wld")) {
         _project_world_paths.push_back(entry.path);
      }
   }
}

void world_edit::open_odfs_for_selected() noexcept
{
   for (auto& selected : _interaction_targets.selection) {
      if (not selected.is<world::object_id>()) continue;

      const world::object* object =
         world::find_entity(_world.objects, selected.get<world::object_id>());

      if (not object) continue;

      open_odf_in_text_editor(object->class_name);
   }
}

void world_edit::open_odf_in_text_editor(const lowercase_string& asset_name) noexcept
{
   const auto asset_path = _asset_libraries.odfs.query_path(asset_name);

   if (asset_path.empty()) return;

   const std::string command_line =
      fmt::format("{} {}", _settings.preferences.text_editor,
                  asset_path.string_view());

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
   if (width == _window_width or height == _window_height) return;

   _window_resized = true;
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
   return not _focused and not _mouse_over;
}

bool world_edit::mouse_over() const noexcept
{
   return _mouse_over;
}

void world_edit::mouse_enter()
{
   _mouse_over = true;
}

void world_edit::mouse_leave()
{
   _mouse_over = false;
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

auto world_edit::get_mouse_cursor() const noexcept -> mouse_cursor
{
   if (_rotate_camera or _pan_camera) return mouse_cursor::none;

   if (ImGui::GetIO().WantCaptureMouse) {
      switch (ImGui::GetMouseCursor()) {
      case ImGuiMouseCursor_Arrow:
         return mouse_cursor::arrow;
      case ImGuiMouseCursor_TextInput:
         return mouse_cursor::ibeam;
      case ImGuiMouseCursor_ResizeAll:
         return mouse_cursor::size_all;
      case ImGuiMouseCursor_ResizeNS:
         return mouse_cursor::size_ns;
      case ImGuiMouseCursor_ResizeEW:
         return mouse_cursor::size_we;
      case ImGuiMouseCursor_ResizeNESW:
         return mouse_cursor::size_nesw;
      case ImGuiMouseCursor_ResizeNWSE:
         return mouse_cursor::size_nwse;
      case ImGuiMouseCursor_Hand:
         return mouse_cursor::hand;
      case ImGuiMouseCursor_NotAllowed:
         return mouse_cursor::no;
      }
   }

   if (_gizmos.want_capture_mouse()) return mouse_cursor::arrow;

   if (_interaction_targets.creation_entity.holds_entity()) {
      if (_interaction_targets.creation_entity.is<world::planning_hub>()) {
         return mouse_cursor::pen;
      }
      else if (_interaction_targets.creation_entity.is<world::planning_connection>()) {
         return mouse_cursor::cross;
      }

      switch (_entity_creation_context.tool) {
      case entity_creation_tool::none:
         return mouse_cursor::arrow;
      case entity_creation_tool::point_at:
         return mouse_cursor::arrow;
      case entity_creation_tool::extend_to:
      case entity_creation_tool::shrink_to:
         return mouse_cursor::size_all;
      case entity_creation_tool::from_object_bbox:
         return mouse_cursor::cross;
      case entity_creation_tool::from_line:
      case entity_creation_tool::draw:
         return mouse_cursor::pen;
      case entity_creation_tool::pick_sector:
         return mouse_cursor::cross;
      }
   }

   if (not _interaction_targets.selection.empty()) {
      if (_selection_edit_context.using_add_object_to_sector) {
         return mouse_cursor::cross;
      }

      switch (_selection_edit_tool) {
      case selection_edit_tool::none:
      case selection_edit_tool::move:
         return mouse_cursor::arrow;
      case selection_edit_tool::move_with_cursor:
         return mouse_cursor::size_all;
      case selection_edit_tool::move_path:
      case selection_edit_tool::move_sector_point:
      case selection_edit_tool::rotate:
      case selection_edit_tool::rotate_around_centre:
      case selection_edit_tool::rotate_light_region:
      case selection_edit_tool::set_layer:
         return mouse_cursor::arrow;
      case selection_edit_tool::match_transform:
      case selection_edit_tool::pick_sector:
      case selection_edit_tool::add_branch_weight:
         return mouse_cursor::cross;
      }
   }

   if (_animation_editor_open) {
      if (_animation_editor_context.pick_object.active) {
         return mouse_cursor::cross;
      }
   }

   if (_animation_group_editor_open) {
      if (_animation_group_editor_context.pick_object.active) {
         return mouse_cursor::cross;
      }
   }

   if (_animation_hierarchy_editor_open) {
      if (_animation_hierarchy_editor_context.pick_object.active) {
         return mouse_cursor::cross;
      }
   }

   if (_block_editor_open) {
      switch (_block_editor_context.tool) {
      case block_edit_tool::none:
         return mouse_cursor::arrow;
      case block_edit_tool::draw:
         return mouse_cursor::pen;
      case block_edit_tool::rotate_texture:
         return mouse_cursor::rotate_cw;
      case block_edit_tool::scale_texture:
         return (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) or
                 ImGui::IsKeyDown(ImGuiKey_RightCtrl))
                   ? mouse_cursor::shrink_texture
                   : mouse_cursor::enlarge_texture;
      case block_edit_tool::paint_material:
         return mouse_cursor::paintbrush;
      case block_edit_tool::set_texture_mode:
         return mouse_cursor::cross;
      case block_edit_tool::offset_texture:
         return mouse_cursor::size_all;
      case block_edit_tool::resize_block:
         return mouse_cursor::cross;
      }
   }

   return mouse_cursor::arrow;
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
      _recreate_renderer_pending = true;
   } break;
   case error::device_hung: {
      if (MessageBoxA(_window, "The GPU has hung during rendering. This often (but not always) indicates a bug in the editor's renderer.\n\nIt may or may not be possible to recover, attempt?",
                      "GPU Hung", MB_YESNO | MB_ICONERROR) == IDYES) {
         _recreate_renderer_pending = true;

         break;
      }

      if (not _edit_stack_world.modified_flag()) std::terminate();

      switch (MessageBoxA(_window, "You have unsaved changes, save world?",
                          "GPU Hung", MB_YESNO | MB_ICONERROR)) {
      case IDYES:
         save_world_with_picker();
         [[fallthrough]];
      case IDNO:
      default:
         std::terminate();
      }
   } break;
   case error::driver_internal_error: {
      switch (MessageBoxA(_window, "An unknown error has occured inside the GPU driver. The editor can not safely recover, save world?",
                          "GPU Driver Internal Error", MB_YESNO | MB_ICONERROR)) {
      case IDYES:
         save_world_with_picker();
         [[fallthrough]];
      case IDNO:
      default:
         std::terminate();
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

auto world_edit::get_renderer_init_params() noexcept -> graphics::renderer_init
{
   return {.window = _window,
           .thread_pool = _thread_pool,
           .asset_libraries = _asset_libraries,
           .error_output = *_stream,
           .display_scale = _display_scale.value,
           .prefer_high_performance_gpu = _renderer_prefer_high_performance_gpu,
           .use_debug_layer = _renderer_use_debug_layer,
           .use_legacy_barriers = _renderer_use_legacy_barriers,
           .never_use_shader_model_6_6 = _renderer_never_use_shader_model_6_6,
           .never_use_open_existing_heap = _renderer_never_use_open_existing_heap,
           .never_use_write_buffer_immediate = _renderer_never_use_write_buffer_immediate,
           .never_use_relaxed_format_casting = _renderer_never_use_relaxed_format_casting,
           .never_use_target_independent_rasterization =
              _renderer_never_use_target_independent_rasterization};
}
}
