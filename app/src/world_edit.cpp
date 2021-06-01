
#include "world_edit.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/odf/default_object_class_definition.hpp"
#include "hresult_error.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"
#include "world/world_io_load.hpp"

#include "graphics/frustrum.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

using namespace std::literals;

namespace sk {

namespace {

constexpr float camera_movement_sensitivity = 20.0f;
constexpr float camera_look_sensitivity = 0.18f;

}

world_edit::world_edit(const HWND window)
   : _imgui_context{ImGui::CreateContext(), &ImGui::DestroyContext}, _window{window}, _renderer{window, _asset_libraries}
{
   ImGui_ImplWin32_Init(window);
   imgui_keymap_init(ImGui::GetIO());

   std::filesystem::current_path(_project_dir);
   _asset_libraries.source_directory(_project_dir);

   try {
      _world = world::load_world("Worlds/SPT/World1/spt.wld", _stream);
   }
   catch (std::exception&) {
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

   // Logic!
   update_object_classes();

   _asset_load_queue.execute();

   if (not _focused) return true;

   ImGui_ImplDX12_NewFrame();
   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();
   imgui_update_io(mouse_state, keyboard_state, ImGui::GetIO());
   ImGui::ShowDemoWindow(nullptr);

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

}
