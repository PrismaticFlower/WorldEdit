
#include "world_edit.hpp"

using namespace std::literals;

namespace we {

void world_edit::initialize_commands() noexcept
{
   _commands.add("camera.move_forward"s, _move_camera_forward);
   _commands.add("camera.move_back"s, _move_camera_back);
   _commands.add("camera.move_left"s, _move_camera_left);
   _commands.add("camera.move_right"s, _move_camera_right);
   _commands.add("camera.move_up"s, _move_camera_up);
   _commands.add("camera.move_down"s, _move_camera_down);
   _commands.add("camera.rotate_with_mouse"s, [this]() {
      _rotate_camera = not _rotate_camera;
      GetCursorPos(&_rotate_camera_cursor_position);
   });

   _commands.add("selection.add"s, [this]() { select_hovered_entity(); });

   _commands.add("edit.undo"s, [this]() { _undo_stack.revert(_world); });
   _commands.add("edit.redo"s, [this]() { _undo_stack.reapply(_world); });

   _commands_binder.set_default_bindings({
      {"camera.move_forward"s, {.key = key::w}, {.toggle = true}},
      {"camera.move_back"s, {.key = key::s}, {.toggle = true}},
      {"camera.move_left"s, {.key = key::a}, {.toggle = true}},
      {"camera.move_right"s, {.key = key::d}, {.toggle = true}},
      {"camera.move_up"s, {.key = key::r}, {.toggle = true}},
      {"camera.move_down"s, {.key = key::f}, {.toggle = true}},
      {"camera.rotate_with_mouse"s, {.key = key::mouse2}, {.toggle = true}},

      {"selection.add"s, {.key = key::mouse1}},

      {"edit.undo"s, {.key = key::z, .modifiers = {.ctrl = true}}},
      {"edit.redo"s, {.key = key::y, .modifiers = {.ctrl = true}}},
   });
}
}