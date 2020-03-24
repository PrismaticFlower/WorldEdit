
#include "world_edit.hpp"
#include "assets/asset_libraries.hpp"
#include "hresult_error.hpp"
#include "world/world_io_load.hpp"

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace sk {

namespace {

constexpr float camera_movement_sensitivity = 20.0f;
constexpr float camera_look_sensitivity = 0.18f;

}

world_edit::world_edit(const HWND window) : _window{window}, _renderer{window}
{
   std::filesystem::current_path(_project_dir); // TODO: Decide on project dir handling design.
   _asset_libraries.source_directory(_project_dir);

   try {
      _world = world::load_world("Worlds/SPT/World1/SPT.wld", _stream);
   }
   catch (std::exception&) {
   }

   RECT rect{};
   GetWindowRect(window, &rect);

   _camera.aspect_ratio(static_cast<float>(rect.right - rect.left) /
                        static_cast<float>(rect.bottom - rect.top));
   _camera.position({0.0f, 0.0f, 10.0f});
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

   // Render!
   update_camera(delta_time, mouse_state, keyboard_state);

   _renderer.draw_frame(_camera, _world, _object_classes);

   return true;
}

void world_edit::update_object_classes()
{
   for (const auto& object : _world.objects) {
      if (_object_classes.contains(object.class_name)) continue;

      auto definition = _asset_libraries.odfs.aquire_if(object.class_name);

      if (not definition) continue; // TODO: Default ODF handling.

      _object_classes.emplace(object.class_name,
                              world::object_class{*definition, _asset_libraries});
   }
}

void world_edit::update_camera(const float delta_time, const mouse_state& mouse_state,
                               const keyboard_state& keyboard_state)
{

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

   if (mouse_state.over_window and mouse_state.right_button) {
      const float camera_look_scale = delta_time * camera_look_sensitivity;

      _camera.yaw(_camera.yaw() + (mouse_state.x_movement * camera_look_scale));
      _camera.pitch(_camera.pitch() + (mouse_state.y_movement * camera_look_scale));
   }

   _camera.position(camera_position);
}

void world_edit::resized(uint16 width, uint16 height)
{
   _camera.aspect_ratio(float(width) / float(height));
   _renderer.window_resized(width, height);
}

void world_edit::focused() {}

void world_edit::unfocused() {}

}