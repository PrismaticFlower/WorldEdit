
#include "world_edit.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/odf/default_object_class_definition.hpp"
#include "edits/insert_entity.hpp"
#include "edits/insert_node.hpp"
#include "edits/insert_point.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "math/vector_funcs.hpp"
#include "utility/file_pickers.hpp"
#include "utility/overload.hpp"
#include "utility/string_ops.hpp"
#include "world/raycast.hpp"
#include "world/utility/world_utilities.hpp"
#include "world/world_io_load.hpp"
#include "world/world_io_save.hpp"

#include "utility/stopwatch.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

using namespace std::literals;

namespace we {

namespace {

constexpr float camera_movement_sensitivity = 20.0f;
constexpr float camera_look_sensitivity = 0.18f;

}

world_edit::world_edit(const HWND window, utility::command_line command_line)
   : _imgui_context{ImGui::CreateContext(), &ImGui::DestroyContext},
     _window{window},
     _renderer{graphics::make_renderer(window, _thread_pool, _asset_libraries, _stream)}
{
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
}

bool world_edit::update()
{
   const float delta_time =
      std::chrono::duration<float>(
         std::chrono::steady_clock::now() -
         std::exchange(_last_update, std::chrono::steady_clock::now()))
         .count();

   if (not _focused) return true;

   wait_for_swap_chain_ready();

   // Input!
   update_input();

   // UI!
   update_ui();

   // Logic!
   update_object_classes();
   update_hovered_entity();

   _asset_libraries.update_loaded();
   _asset_load_queue.execute();

   // Render!
   update_camera(delta_time);

   try {
      _renderer->draw_frame(_camera, _world, _interaction_targets, _world_draw_mask,
                            _world_layers_draw_mask, _tool_visualizers,
                            _object_classes, _settings.graphics);
   }
   catch (graphics::gpu::exception& e) {
      handle_gpu_error(e);
   }

   // Garbage Collection! (not memory)
   garbage_collect_assets();

   return true;
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

void world_edit::update_object_classes()
{
   for (const auto& object : _world.objects) {
      if (_object_classes.contains(object.class_name)) continue;

      auto definition = _asset_libraries.odfs[object.class_name];

      _object_classes.emplace(object.class_name,
                              world::object_class{_asset_libraries, definition});
   }

   if (_interaction_targets.creation_entity and
       std::holds_alternative<world::object>(*_interaction_targets.creation_entity)) {
      const lowercase_string& class_name =
         std::get<world::object>(*_interaction_targets.creation_entity).class_name;

      if (not _object_classes.contains(class_name)) {
         auto definition = _asset_libraries.odfs[class_name];

         _object_classes.emplace(class_name,
                                 world::object_class{_asset_libraries, definition});
      }
   }
}

void world_edit::update_input() noexcept
{
   _hotkeys.update(ImGui::GetIO().WantCaptureMouse, ImGui::GetIO().WantCaptureKeyboard);

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

   if (ImGui::GetIO().WantCaptureMouse) return;

   // TODO: interaction mask instead of reusing draw mask

   if (_world_draw_mask.objects) {
      if (std::optional<world::raycast_result<world::object>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_draw_mask,
                            _world.objects, _object_classes);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            _cursor_surface_normalWS = hit->normalWS;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (_world_draw_mask.lights) {
      if (std::optional<world::raycast_result<world::light>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_draw_mask,
                            _world.lights);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (_world_draw_mask.paths) {
      if (std::optional<world::raycast_result<world::path>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_draw_mask,
                            _world.paths);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (_world_draw_mask.regions) {
      if (std::optional<world::raycast_result<world::region>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_draw_mask,
                            _world.regions);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (_world_draw_mask.sectors) {
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

   if (_world_draw_mask.portals) {
      if (std::optional<world::raycast_result<world::portal>> hit =
             world::raycast(ray.origin, ray.direction, _world.portals);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (_world_draw_mask.hintnodes) {
      if (std::optional<world::raycast_result<world::hintnode>> hit =
             world::raycast(ray.origin, ray.direction, _world_layers_draw_mask,
                            _world.hintnodes);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (_world_draw_mask.barriers) {
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

   if (_world_draw_mask.boundaries) {
      if (std::optional<world::raycast_result<world::boundary>> hit =
             world::raycast(ray.origin, ray.direction, _world.boundaries,
                            _world.paths, _settings.graphics.boundary_height);
          hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = hit->id;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   // TODO: Unique terrain mask.
   if (_world_draw_mask.objects) {
      if (auto hit = _terrain_collision.raycast(ray.origin, ray.direction); hit) {
         if (hit->distance < hovered_entity_distance) {
            _interaction_targets.hovered_entity = std::nullopt;
            hovered_entity_distance = hit->distance;
         }
      }
   }

   if (hovered_entity_distance != std::numeric_limits<float>::max()) {
      _cursor_positionWS = ray.origin + ray.direction * hovered_entity_distance;
   }

   if (_interaction_targets.creation_entity and _interaction_targets.hovered_entity) {
      const bool tool_wants_hover =
         _entity_creation_context.using_from_object_bbox and
         std::holds_alternative<world::object_id>(*_interaction_targets.hovered_entity);

      if (not tool_wants_hover) {
         _interaction_targets.hovered_entity = std::nullopt;
      }
   }
}

void world_edit::update_camera(const float delta_time)
{
   float3 camera_position = _camera.position();

   const float camera_movement_scale = delta_time * camera_movement_sensitivity;

   if (_move_camera_forward) {
      camera_position += (_camera.forward() * camera_movement_scale);
   }
   if (_move_camera_back) {
      camera_position += (_camera.back() * camera_movement_scale);
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

   _camera.position(camera_position);

   if (_rotate_camera) {
      const float camera_look_scale = delta_time * camera_look_sensitivity;

      _camera.yaw(_camera.yaw() + (-_mouse_movement_x * camera_look_scale));
      _camera.pitch(_camera.pitch() + (-_mouse_movement_y * camera_look_scale));

      SetCursor(nullptr);
      SetCursorPos(_rotate_camera_cursor_position.x,
                   _rotate_camera_cursor_position.y);
   }
}

void world_edit::garbage_collect_assets() noexcept
{
   for (auto& [name, object_class] : _object_classes) {
      object_class.world_frame_references = 0;
   }

   for (const auto& object : _world.objects) {
      if (auto name_object_class = _object_classes.find(object.class_name);
          name_object_class != _object_classes.end()) {
         auto& [name, object_class] = *name_object_class;

         object_class.world_frame_references += 1;
      }

      continue; // Don't care about missing object classes here, leave that for update_object_classes().
   }

   if (_interaction_targets.creation_entity and
       std::holds_alternative<world::object>(*_interaction_targets.creation_entity)) {
      const lowercase_string& class_name =
         std::get<world::object>(*_interaction_targets.creation_entity).class_name;

      if (auto name_object_class = _object_classes.find(class_name);
          name_object_class != _object_classes.end()) {
         auto& [name, object_class] = *name_object_class;

         object_class.world_frame_references += 1;
      }
   }

   absl::erase_if(_object_classes, [](const auto& name_object_class) {
      auto& [name, object_class] = name_object_class;

      return object_class.world_frame_references == 0;
   });
}

void world_edit::select_hovered_entity() noexcept
{
   _interaction_targets.selection.clear();

   if (not _interaction_targets.hovered_entity) return;

   _interaction_targets.selection.push_back(*_interaction_targets.hovered_entity);
}

void world_edit::place_creation_entity() noexcept
{
   if (not _interaction_targets.creation_entity) return;

   std::visit(
      overload{
         [&](world::object& object) {
            world::object new_object = object;

            new_object.name =
               world::create_unique_name(_world.objects, new_object.name);
            new_object.id = _world.next_id.objects.aquire();

            _entity_creation_context.last_object = new_object.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_object)),
                                    _edit_context);

            object.name = world::create_unique_name(_world.objects, object.name);
         },
         [&](world::light& light) {
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

            _entity_creation_context.last_light = new_light.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_light)),
                                    _edit_context);

            light.name = world::create_unique_name(_world.objects, light.name);

            if (world::is_region_light(light)) {
               light.region_name =
                  world::create_unique_light_region_name(_world.lights, _world.regions,
                                                         light.region_name.empty()
                                                            ? light.name
                                                            : light.region_name);
            }
         },
         [&](world::path& path) {
            if (const world::path* existing_path =
                   world::find_entity(_world.paths, path.name);
                existing_path) {
               if (path.nodes.empty()) std::terminate();

               if (_entity_creation_context.placement_node_insert ==
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

               _edit_stack_world.apply(edits::make_insert_entity(std::move(new_path)),
                                       _edit_context);
            }
         },
         [&](world::region& region) {
            world::region new_region = region;

            new_region.name =
               world::create_unique_name(_world.regions, new_region.name);
            new_region.id = _world.next_id.regions.aquire();

            _entity_creation_context.last_region = new_region.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_region)),
                                    _edit_context);

            region.name = world::create_unique_name(_world.regions, region.name);
         },
         [&](world::sector& sector) {
            if (sector.points.empty()) std::terminate();

            if (const world::sector* existing_sector =
                   world::find_entity(_world.sectors, sector.name);
                existing_sector) {
               _edit_stack_world
                  .apply(edits::make_insert_point(existing_sector->id,
                                                  existing_sector->points.size(),
                                                  sector.points[0]),
                         _edit_context);
            }
            else {
               world::sector new_sector = sector;

               new_sector.name = sector.name;
               new_sector.id = _world.next_id.sectors.aquire();

               _edit_stack_world.apply(edits::make_insert_entity(std::move(new_sector)),
                                       _edit_context);
            }
         },
         [&](world::portal& portal) {
            world::portal new_portal = portal;

            new_portal.name =
               world::create_unique_name(_world.portals, new_portal.name);
            new_portal.id = _world.next_id.portals.aquire();

            _entity_creation_context.last_portal = new_portal.id;

            _edit_stack_world.apply(edits::make_insert_entity(std::move(new_portal)),
                                    _edit_context);

            portal.name = world::create_unique_name(_world.portals, portal.name);
         },
         [&](world::barrier& barrier) { (void)barrier; },
         [&](world::planning_hub& planning_hub) { (void)planning_hub; },
         [&](world::planning_connection& planning_connection) {
            (void)planning_connection;
         },
         [&](world::boundary& boundary) { (void)boundary; },
      },
      *_interaction_targets.creation_entity);
}

void world_edit::object_definition_loaded(const lowercase_string& name,
                                          asset_ref<assets::odf::definition> asset,
                                          asset_data<assets::odf::definition> data)
{
   _object_classes[name].update_definition(_asset_libraries, asset);
}

void world_edit::model_loaded(const lowercase_string& name,
                              asset_ref<assets::msh::flat_model> asset,
                              asset_data<assets::msh::flat_model> data)
{
   for (auto& [object_class_name, object_class] : _object_classes) {
      if (object_class.model_name != name) continue;

      object_class.model_asset = asset;
      object_class.model = data;
   }
}

void world_edit::open_project(std::filesystem::path path) noexcept
{
   if (not std::filesystem::exists(path / L"Worlds")) {
      if (MessageBoxW(_window, L"The selected folder does not appear to be a project folder. Are you sure you wish to open it?",
                      L"Not a Project Folder", MB_YESNO) != IDYES) {
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

   auto path = utility::show_folder_picker({.title = L"Open Project",
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
   }
   catch (std::exception& e) {
      auto message =
         fmt::format("Failed to save world!\n   Reason: \n{}\n"
                     "   Incomplete save data maybe present on disk.\n",
                     utility::string::indent(2, e.what()));

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
   _object_classes.clear();
   _world = {};
   _interaction_targets = {};
   _entity_creation_context = {};
   _world_draw_mask = {};
   _world_layers_draw_mask = {true};
   _world_path.clear();

   _terrain_collision = {};

   _edit_stack_world.clear();

   _renderer->mark_dirty_terrain();

   SetWindowTextA(_window, "WorldEdit");
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
      MessageBoxW(_window, L"Unable to enumerate project worlds. Loading worlds will require manual navigation.",
                  L"Error", MB_OK);
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
   _hotkeys.release_toggles();
}

bool world_edit::idling() const noexcept
{
   return not _focused;
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
   const float old_dpi = std::exchange(_current_dpi, static_cast<float>(new_dpi));

   _display_scale = _current_dpi / 96.0f;

   ImGui::GetIO().Fonts->Clear();
   ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf",
                                            std::floor(16.0f * _display_scale));
   ImGui::GetStyle().ScaleAllSizes(new_dpi / old_dpi);

   try {
      _renderer->recreate_imgui_font_atlas();
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

      case IDNO:
      default:
         std::terminate();
      }
   } break;
   case error::device_removed: {
      _renderer = nullptr;

      _renderer =
         graphics::make_renderer(_window, _thread_pool, _asset_libraries, _stream);

   } break;
   }
}

}
