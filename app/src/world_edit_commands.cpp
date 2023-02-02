
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

   _commands.add("edit.world_click"s, [this]() { world_clicked(); });

   _commands.add("edit.undo"s, [this]() { _undo_stack.revert(_world); });
   _commands.add("edit.redo"s, [this]() { _undo_stack.reapply(_world); });
}

void world_edit::initialize_hotkeys() noexcept
{
   _hotkeys.add_set("", [] { return true; },
                    {
                       {"camera.move_forward", {.key = key::w}, {.toggle = true}},
                       {"camera.move_back", {.key = key::s}, {.toggle = true}},
                       {"camera.move_left", {.key = key::a}, {.toggle = true}},
                       {"camera.move_right", {.key = key::d}, {.toggle = true}},
                       {"camera.move_up", {.key = key::r}, {.toggle = true}},
                       {"camera.move_down", {.key = key::f}, {.toggle = true}},
                       {"camera.rotate_with_mouse", {.key = key::mouse2}, {.toggle = true}},

                       {"edit.world_click", {.key = key::mouse1}},

                       {"edit.undo", {.key = key::z, .modifiers = {.ctrl = true}}},
                       {"edit.redo", {.key = key::y, .modifiers = {.ctrl = true}}},
                    });
}

}