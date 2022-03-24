
#include "world_edit.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/odf/default_object_class_definition.hpp"
#include "hresult_error.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"
#include "utility/file_pickers.hpp"
#include "world/world_io_load.hpp"
#include "world/world_io_save.hpp"

#include "graphics/frustrum.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

#include <range/v3/view/enumerate.hpp>

using namespace std::literals;
using ranges::views::enumerate;

namespace we {

namespace {

constexpr float camera_movement_sensitivity = 20.0f;
constexpr float camera_look_sensitivity = 0.18f;

}

world_edit::world_edit(const HWND window, utility::command_line command_line)
   : _imgui_context{ImGui::CreateContext(), &ImGui::DestroyContext},
     _window{window},
     _renderer{window, _settings, _thread_pool, _asset_libraries}
{
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

   // TEMP: Hotkey initialization.
   _key_input_manager.bind({.key = key::w}, {.toggle = true}, [&] {
      _move_camera_forward = not _move_camera_forward;
   });
   _key_input_manager.bind({.key = key::s}, {.toggle = true},
                           [&] { _move_camera_back = not _move_camera_back; });
   _key_input_manager.bind({.key = key::a}, {.toggle = true},
                           [&] { _move_camera_left = not _move_camera_left; });
   _key_input_manager.bind({.key = key::d}, {.toggle = true},
                           [&] { _move_camera_right = not _move_camera_right; });
   _key_input_manager.bind({.key = key::r}, {.toggle = true},
                           [&] { _move_camera_up = not _move_camera_up; });
   _key_input_manager.bind({.key = key::f}, {.toggle = true},
                           [&] { _move_camera_down = not _move_camera_down; });
   _key_input_manager.bind({.key = key::mouse2}, {.toggle = true},
                           [&] { _rotate_camera = not _rotate_camera; });
}

bool world_edit::update()
{
   const float delta_time =
      std::chrono::duration<float>(
         std::chrono::steady_clock::now() -
         std::exchange(_last_update, std::chrono::steady_clock::now()))
         .count();

   [[maybe_unused]] const auto keyboard_state = get_keyboard_state();
   [[maybe_unused]] const auto mouse_state = get_mouse_state(_window);

   if (not _focused) return true;

   // Input!
   update_input();

   // UI!
   update_ui();

   // Logic!
   update_object_classes();

   _asset_load_queue.execute();

   // Render!
   update_camera(delta_time);

   _renderer.draw_frame(_camera, _world, _interaction_targets, _world_draw_mask,
                        _world_layers_draw_mask, _object_classes);

   return true;
}

void world_edit::update_object_classes()
{
   for (const auto& object : _world.objects) {
      if (_object_classes.contains(object.class_name)) continue;

      auto definition = _asset_libraries.odfs[object.class_name];

      _object_classes.emplace(object.class_name,
                              std::make_shared<world::object_class>(_asset_libraries,
                                                                    definition));
   }
}

void world_edit::update_input() noexcept
{
   _imgui_wants_input_capture =
      ImGui::GetIO().WantCaptureMouse or ImGui::GetIO().WantCaptureKeyboard;

   if (_imgui_wants_input_capture) {
      _key_input_manager.release_unmodified_toggles();
   }

   _key_input_manager.update(_imgui_wants_input_capture);

   _mouse_movement_x = std::exchange(_queued_mouse_movement_x, 0);
   _mouse_movement_y = std::exchange(_queued_mouse_movement_y, 0);
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
   }
}

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

   ImGui::Begin("Highlight TEMP");

   const auto build_temp_highlight_selection =
      [&]<typename T>(const char* name, std::vector<T>& entities) {
         if (ImGui::CollapsingHeader(name)) {
            for (auto& entity : entities) {
               ImGui::PushID(std::to_underlying(entity.id));

               if (ImGui::Selectable(entity.name.c_str(),
                                     _interaction_targets.hovered_entity ==
                                        world::hovered_entity{entity.id})) {
                  _interaction_targets.hovered_entity = entity.id;
               }

               ImGui::PopID();
            }
         }
      };

   build_temp_highlight_selection("Objects", _world.objects);
   build_temp_highlight_selection("Lights", _world.lights);
   build_temp_highlight_selection("Paths", _world.paths);

   if (ImGui::CollapsingHeader("Paths Nodes")) {
      for (auto& path : _world.paths) {
         ImGui::PushID(std::to_underlying(path.id));
         ImGui::PushID(path.name.c_str());

         ImGui::TextUnformatted(path.name.c_str());
         ImGui::Indent();

         for (std::size_t i = 0; i < path.nodes.size(); ++i) {

            if (ImGui::Selectable(fmt::format("Node {}", i).c_str(),
                                  _interaction_targets.hovered_entity ==
                                     world::hovered_entity{
                                        world::path_id_node_pair{path.id, i}})) {
               _interaction_targets.hovered_entity =
                  world::path_id_node_pair{path.id, i};
            }
         }

         ImGui::Unindent();
         ImGui::PopID();
         ImGui::PopID();
      }
   }

   build_temp_highlight_selection("Regions", _world.regions);
   build_temp_highlight_selection("Sectors", _world.sectors);
   build_temp_highlight_selection("Portals", _world.portals);
   build_temp_highlight_selection("Hintnodes", _world.hintnodes);
   build_temp_highlight_selection("Barriers", _world.barriers);
   build_temp_highlight_selection("Boundaries", _world.boundaries);

   ImGui::End();
}

void world_edit::object_definition_loaded(const lowercase_string& name,
                                          asset_ref<assets::odf::definition> asset,
                                          asset_data<assets::odf::definition> data)
{
   _object_classes[name]->update_definition(_asset_libraries, asset);
}

void world_edit::model_loaded(const lowercase_string& name,
                              asset_ref<assets::msh::flat_model> asset,
                              asset_data<assets::msh::flat_model> data)
{
   for (auto& [object_class_name, object_class] : _object_classes) {
      if (object_class->model_name != name) continue;

      object_class->model_asset = asset;
      object_class->model = data;
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
   _world_draw_mask = {};
   _world_layers_draw_mask = {true};
   _world_path.clear();

   _renderer.mark_dirty_terrain();

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
   _renderer.window_resized(width, height);
}

void world_edit::focused()
{
   _focused = true;
}

void world_edit::unfocused()
{
   _focused = false;
   _key_input_manager.release_toggles();
}

bool world_edit::idling() const noexcept
{
   return not _focused;
}

void world_edit::key_down(const key key) noexcept
{
   _key_input_manager.notify_key_down(key);
}

void world_edit::key_up(const key key) noexcept
{
   _key_input_manager.notify_key_up(key);
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

   ImGui_ImplDX12_InvalidateDeviceObjects();
}

}
