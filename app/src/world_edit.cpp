
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

using namespace std::literals;

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
   imgui_keymap_init(ImGui::GetIO());

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

   const auto keyboard_state = get_keyboard_state();
   const auto mouse_state = get_mouse_state(_window);

   if (not _focused) return true;

   // UI!
   update_ui(mouse_state, keyboard_state);

   // Logic!
   update_object_classes();

   _asset_libraries.update_modified();
   _asset_load_queue.execute();

   // Render!
   update_camera(delta_time, mouse_state, keyboard_state);

   _renderer.draw_frame(_camera, _world, _object_classes);

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

void world_edit::update_camera(const float delta_time, const mouse_state& mouse_state,
                               const keyboard_state& keyboard_state)
{

   if (not ImGui::GetIO().WantCaptureKeyboard) {
      float3 camera_position = _camera.position();

      const float camera_movement_scale = delta_time * camera_movement_sensitivity;

      if (keyboard_state[keyboard_keys::w]) {
         camera_position += (_camera.forward() * camera_movement_scale);
      }
      if (keyboard_state[keyboard_keys::s]) {
         camera_position += (_camera.back() * camera_movement_scale);
      }
      if (keyboard_state[keyboard_keys::a]) {
         camera_position += (_camera.left() * camera_movement_scale);
      }
      if (keyboard_state[keyboard_keys::d]) {
         camera_position += (_camera.right() * camera_movement_scale);
      }
      if (keyboard_state[keyboard_keys::r]) {
         camera_position += (_camera.up() * camera_movement_scale);
      }
      if (keyboard_state[keyboard_keys::f]) {
         camera_position += (_camera.down() * camera_movement_scale);
      }

      _camera.position(camera_position);
   }

   if (mouse_state.over_window and mouse_state.right_button and
       not ImGui::GetIO().WantCaptureMouse) {
      const float camera_look_scale = delta_time * camera_look_sensitivity;

      _camera.yaw(_camera.yaw() + (mouse_state.x_movement * camera_look_scale));
      _camera.pitch(_camera.pitch() + (mouse_state.y_movement * camera_look_scale));
   }
}

void world_edit::update_ui(const mouse_state& mouse_state,
                           const keyboard_state& keyboard_state) noexcept
{
   ImGui_ImplDX12_NewFrame();
   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();
   imgui_update_io(mouse_state, keyboard_state, ImGui::GetIO());
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

   if (not path && not std::filesystem::exists(*path)) return;

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
}

bool world_edit::idling() const noexcept
{
   return not _focused;
}

void world_edit::mouse_wheel_movement(const float movement) noexcept
{
   ImGui::GetIO().MouseWheel += movement;
}

void world_edit::update_cursor() noexcept
{
   SetCursor(LoadCursorW(nullptr, [] {
      switch (ImGui::GetMouseCursor()) {
      default:
      case ImGuiMouseCursor_Arrow:
         return IDC_ARROW;
      case ImGuiMouseCursor_TextInput:
         return IDC_IBEAM;
      case ImGuiMouseCursor_ResizeAll:
         return IDC_SIZEALL;
      case ImGuiMouseCursor_ResizeNS:
         return IDC_SIZEWE;
      case ImGuiMouseCursor_ResizeEW:
         return IDC_SIZEWE;
      case ImGuiMouseCursor_ResizeNESW:
         return IDC_SIZENESW;
      case ImGuiMouseCursor_ResizeNWSE:
         return IDC_SIZENWSE;
      case ImGuiMouseCursor_Hand:
         return IDC_HAND;
      case ImGuiMouseCursor_NotAllowed:
         return IDC_NO;
      }
   }()));
}

void world_edit::char_input(const char16_t c) noexcept
{
   ImGui::GetIO().AddInputCharacterUTF16(static_cast<ImWchar16>(c));
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
