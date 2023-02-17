
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

   _commands.add("edit.select"s, [this]() { select_hovered_entity(); });
   _commands.add("edit.deselect"s, [this]() {
      if (_interaction_targets.selection.empty()) return;

      _interaction_targets.selection.pop_back();
   });

   _commands.add("edit.undo"s, [this]() { _undo_stack.revert(_world); });
   _commands.add("edit.redo"s, [this]() { _undo_stack.reapply(_world); });

   _commands.add("hotkeys.show"s, _hotkeys_show);

   _commands.add("save"s, [this]() { save_world(_world_path); });

   _commands.add("entity_creation.cycle_rotation_mode"s, [this] {
      switch (_entity_creation_context.placement_rotation) {
      case placement_rotation::manual_euler:
         _entity_creation_context.placement_rotation =
            placement_rotation::manual_quaternion;
         return;
      case placement_rotation::manual_quaternion:
         _entity_creation_context.placement_rotation = placement_rotation::surface;
         return;
      case placement_rotation::surface:
         _entity_creation_context.placement_rotation = placement_rotation::manual_euler;
         return;
      }
   });
   _commands.add("entity_creation.cycle_placement_mode"s, [this] {
      switch (_entity_creation_context.placement_mode) {
      case placement_mode::manual:
         _entity_creation_context.placement_mode = placement_mode::cursor;
         return;
      case placement_mode::cursor:
         _entity_creation_context.placement_mode = placement_mode::manual;
         return;
      }
   });
   _commands.add("entity_creation.cycle_alignment_mode"s, [this] {
      switch (_entity_creation_context.placement_alignment) {
      case placement_alignment::none:
         _entity_creation_context.placement_alignment = placement_alignment::grid;
         return;
      case placement_alignment::grid:
         _entity_creation_context.placement_alignment = placement_alignment::snapping;
         return;
      case placement_alignment::snapping:
         _entity_creation_context.placement_alignment = placement_alignment::none;
         return;
      }
   });
   _commands.add("entity_creation.cycle_ground_mode"s, [this] {
      switch (_entity_creation_context.placement_ground) {
      case placement_ground::origin:
         _entity_creation_context.placement_ground = placement_ground::bbox;
         return;
      case placement_ground::bbox:
         _entity_creation_context.placement_ground = placement_ground::origin;
         return;
      }
   });

   _commands.add("entity_creation.toggle_point_at"s, [this] {
      _entity_creation_context.using_point_at =
         not _entity_creation_context.using_point_at;

      if (_entity_creation_context.using_point_at) {
         _entity_creation_context.placement_rotation =
            placement_rotation::manual_quaternion;
      }
   });
   _commands.add("entity_creation.deactivate_point_at"s,
                 [this] { _entity_creation_context.using_point_at = false; });
   _commands.add("entity_creation.toggle_extend_to"s, [this] {
      _entity_creation_context.using_extend_to =
         not _entity_creation_context.using_extend_to;
      _entity_creation_context.using_shrink_to = false;
   });
   _commands.add("entity_creation.toggle_shrink_to"s, [this] {
      _entity_creation_context.using_shrink_to =
         not _entity_creation_context.using_shrink_to;
      _entity_creation_context.using_extend_to = false;
   });
   _commands.add("entity_creation.deactivate_resize_to"s, [this] {
      _entity_creation_context.using_shrink_to = false;
      _entity_creation_context.using_extend_to = false;
   });
   _commands.add("entity_creation.toggle_from_object_bbox"s,
                 _entity_creation_context.using_from_object_bbox);

   _commands.add("entity_creation.lock_x_axis"s, _entity_creation_context.lock_x_axis);
   _commands.add("entity_creation.lock_y_axis"s, _entity_creation_context.lock_y_axis);
   _commands.add("entity_creation.lock_z_axis"s, _entity_creation_context.lock_z_axis);

   _commands.add("entity_creation.finish_path"s,
                 _entity_creation_context.finish_current_path);

   _commands.add("entity_creation.place"s, [this] { place_creation_entity(); });
   _commands.add("entity_creation.cancel"s,
                 [this] { _interaction_targets.creation_entity = std::nullopt; });
}

void world_edit::initialize_hotkeys() noexcept
{
   _hotkeys.add_set("", [] { return true; },
                    {{"camera.move_forward", {.key = key::w}, {.toggle = true}},
                     {"camera.move_back", {.key = key::s}, {.toggle = true}},
                     {"camera.move_left", {.key = key::a}, {.toggle = true}},
                     {"camera.move_right", {.key = key::d}, {.toggle = true}},
                     {"camera.move_up", {.key = key::r}, {.toggle = true}},
                     {"camera.move_down", {.key = key::f}, {.toggle = true}},
                     {"camera.rotate_with_mouse", {.key = key::mouse2}, {.toggle = true}},

                     {"edit.select", {.key = key::mouse1}},
                     {"edit.deselect", {.key = key::escape}},

                     {"edit.undo", {.key = key::z, .modifiers = {.ctrl = true}}},
                     {"edit.redo", {.key = key::y, .modifiers = {.ctrl = true}}},

                     {"hotkeys.show", {.key = key::f1}},

                     {"save",
                      {.key = key::s, .modifiers = {.ctrl = true}},
                      {.ignore_imgui_focus = true}}});

   _hotkeys.add_set("Entity Creation",
                    [this] {
                       return _interaction_targets.creation_entity.has_value();
                    },
                    {
                       {"entity_creation.cycle_rotation_mode",
                        {.key = key::q, .modifiers = {.ctrl = true}}},
                       {"entity_creation.cycle_placement_mode",
                        {.key = key::w, .modifiers = {.ctrl = true}}},
                       {"entity_creation.cycle_alignment_mode",
                        {.key = key::e, .modifiers = {.ctrl = true}}},
                       {"entity_creation.cycle_ground_mode",
                        {.key = key::r, .modifiers = {.ctrl = true}}},

                       {"entity_creation.toggle_point_at", {.key = key::v}},
                       {"entity_creation.toggle_extend_to", {.key = key::t}},
                       {"entity_creation.toggle_shrink_to",
                        {.key = key::t, .modifiers = {.ctrl = true}}},
                       {"entity_creation.toggle_from_object_bbox", {.key = key::b}},

                       {"entity_creation.lock_x_axis", {.key = key::z}},
                       {"entity_creation.lock_y_axis", {.key = key::x}},
                       {"entity_creation.lock_z_axis", {.key = key::c}},

                       {"entity_creation.place", {.key = key::mouse1}},
                       {"entity_creation.cancel", {.key = key::escape}},

                       {"entity_creation.finish_path", {.key = key::g}},
                    });

   _hotkeys.add_set("Entity Creation (Point At)",
                    [this] {
                       return _interaction_targets.creation_entity and
                              _entity_creation_context.using_point_at;
                    },
                    {
                       {"entity_creation.deactivate_point_at", {.key = key::mouse1}},
                       {"entity_creation.deactivate_point_at", {.key = key::escape}},
                    });

   _hotkeys.add_set("Entity Creation (Resize To)",
                    [this] {
                       return _interaction_targets.creation_entity and
                              (_entity_creation_context.using_extend_to or
                               _entity_creation_context.using_shrink_to);
                    },
                    {
                       {"entity_creation.deactivate_resize_to", {.key = key::mouse1}},
                       {"entity_creation.deactivate_resize_to", {.key = key::escape}},
                    });

   _hotkeys.add_set("Entity Creation (From BBOX)",
                    [this] {
                       return _interaction_targets.creation_entity and
                              _entity_creation_context.using_from_object_bbox;
                    },
                    {
                       {"entity_creation.toggle_from_object_bbox", {.key = key::mouse1}},
                       {"entity_creation.toggle_from_object_bbox", {.key = key::escape}},
                    });
}

}