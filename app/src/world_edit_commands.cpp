
#include "edits/creation_entity_set.hpp"
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
   _commands.add("camera.move_sprint_forward"s, [this] {
      _move_camera_forward = not _move_camera_forward;
      _move_sprint = not _move_sprint;
   });
   _commands.add("camera.move_sprint_back"s, [this] {
      _move_camera_back = not _move_camera_back;
      _move_sprint = not _move_sprint;
   });
   _commands.add("camera.move_sprint_left"s, [this] {
      _move_camera_left = not _move_camera_left;
      _move_sprint = not _move_sprint;
   });
   _commands.add("camera.move_sprint_right"s, [this] {
      _move_camera_right = not _move_camera_right;
      _move_sprint = not _move_sprint;
   });
   _commands.add("camera.move_sprint_up"s, [this] {
      _move_camera_up = not _move_camera_up;
      _move_sprint = not _move_sprint;
   });
   _commands.add("camera.move_sprint_down"s, [this] {
      _move_camera_down = not _move_camera_down;
      _move_sprint = not _move_sprint;
   });
   _commands.add("camera.rotate_with_mouse"s, [this]() {
      _rotate_camera = not _rotate_camera;
      GetCursorPos(&_rotate_camera_cursor_position);
   });

   _commands.add("edit.select"s, [this]() { select_hovered_entity(); });
   _commands.add("edit.deselect"s, [this]() {
      if (_interaction_targets.selection.empty()) return;

      _interaction_targets.selection.pop_back();
   });

   _commands.add("edit.undo"s, [this]() { undo(); });
   _commands.add("edit.redo"s, [this]() { redo(); });
   _commands.add("edit.delete"s, [this]() { delete_selected(); });

   _commands.add("hotkeys.show"s, _hotkeys_view_show);

   _commands.add("save"s, [this]() { save_world(_world_path); });

   _commands.add("entity_creation.cycle_rotation_mode"s, [this] {
      switch (_entity_creation_config.placement_rotation) {
      case placement_rotation::manual_euler:
         _entity_creation_config.placement_rotation =
            placement_rotation::manual_quaternion;
         return;
      case placement_rotation::manual_quaternion:
         _entity_creation_config.placement_rotation = placement_rotation::surface;
         return;
      case placement_rotation::surface:
         _entity_creation_config.placement_rotation = placement_rotation::manual_euler;
         return;
      }
   });
   _commands.add("entity_creation.cycle_placement_mode"s, [this] {
      switch (_entity_creation_config.placement_mode) {
      case placement_mode::manual:
         _entity_creation_config.placement_mode = placement_mode::cursor;
         return;
      case placement_mode::cursor:
         _entity_creation_config.placement_mode = placement_mode::manual;
         return;
      }
   });
   _commands.add("entity_creation.cycle_alignment_mode"s, [this] {
      switch (_entity_creation_config.placement_alignment) {
      case placement_alignment::none:
         _entity_creation_config.placement_alignment = placement_alignment::grid;
         return;
      case placement_alignment::grid:
         _entity_creation_config.placement_alignment = placement_alignment::snapping;
         return;
      case placement_alignment::snapping:
         _entity_creation_config.placement_alignment = placement_alignment::none;
         return;
      }
   });
   _commands.add("entity_creation.cycle_ground_mode"s, [this] {
      switch (_entity_creation_config.placement_ground) {
      case placement_ground::origin:
         _entity_creation_config.placement_ground = placement_ground::bbox;
         return;
      case placement_ground::bbox:
         _entity_creation_config.placement_ground = placement_ground::origin;
         return;
      }
   });
   _commands.add("entity_creation.cycle_object_class"s,
                 [this] { cycle_creation_entity_object_class(); });

   _commands.add("entity_creation.activate_point_at"s,
                 _entity_creation_context.activate_point_at);
   _commands.add("entity_creation.deactivate_point_at"s,
                 [this] { _entity_creation_context.using_point_at = false; });
   _commands.add("entity_creation.activate_extend_to"s,
                 _entity_creation_context.activate_extend_to);
   _commands.add("entity_creation.activate_shrink_to"s,
                 _entity_creation_context.activate_shrink_to);
   _commands.add("entity_creation.deactivate_resize_to"s, [this] {
      _entity_creation_context.using_shrink_to = false;
      _entity_creation_context.using_extend_to = false;
   });
   _commands.add("entity_creation.activate_from_object_bbox"s,
                 _entity_creation_context.activate_from_object_bbox);
   _commands.add("entity_creation.deactivate_from_object_bbox"s, [this] {
      _entity_creation_context.using_from_object_bbox = false;
   });

   _commands.add("entity_creation.activate_from_line"s,
                 _entity_creation_context.activate_from_line);
   _commands.add("entity_creation.deactivate_from_line"s, [this] {
      _entity_creation_context.using_from_line = false;
      _entity_creation_context.from_line_click = false;
   });
   _commands.add("entity_creation.from_line_click"s,
                 _entity_creation_context.from_line_click);

   _commands.add("entity_creation.lock_x_axis"s, _entity_creation_context.lock_x_axis);
   _commands.add("entity_creation.lock_y_axis"s, _entity_creation_context.lock_y_axis);
   _commands.add("entity_creation.lock_z_axis"s, _entity_creation_context.lock_z_axis);

   _commands.add("entity_creation.finish_path"s,
                 _entity_creation_context.finish_current_path);

   _commands.add("entity_creation.place"s, [this] { place_creation_entity(); });
   _commands.add("entity_creation.cancel"s, [this] {
      if (not _interaction_targets.creation_entity) return;

      _edit_stack_world
         .apply(edits::make_creation_entity_set(std::nullopt,
                                                _interaction_targets.creation_entity),
                _edit_context);
   });
}

void world_edit::initialize_hotkeys() noexcept
{
   _hotkeys.add_set(
      "", [] { return true; },
      {
         {"Move Forward", "camera.move_forward", {.key = key::w}, {.toggle = true}},
         {"Move Back", "camera.move_back", {.key = key::s}, {.toggle = true}},
         {"Move Left", "camera.move_left", {.key = key::a}, {.toggle = true}},
         {"Move Right", "camera.move_right", {.key = key::d}, {.toggle = true}},
         {"Move Up", "camera.move_up", {.key = key::r}, {.toggle = true}},
         {"Move Down", "camera.move_down", {.key = key::f}, {.toggle = true}},
         {"Move Sprint Forward",
          "camera.move_sprint_forward",
          {.key = key::w, .modifiers = {.shift = true}},
          {.toggle = true}},
         {"Move Sprint Back",
          "camera.move_sprint_back",
          {.key = key::s, .modifiers = {.shift = true}},
          {.toggle = true}},
         {"Move Sprint Left",
          "camera.move_sprint_left",
          {.key = key::a, .modifiers = {.shift = true}},
          {.toggle = true}},
         {"Move Sprint Right",
          "camera.move_sprint_right",
          {.key = key::d, .modifiers = {.shift = true}},
          {.toggle = true}},
         {"Move Sprint Up",
          "camera.move_sprint_up",
          {.key = key::r, .modifiers = {.shift = true}},
          {.toggle = true}},
         {"Move Sprint Down",
          "camera.move_sprint_down",
          {.key = key::f, .modifiers = {.shift = true}},
          {.toggle = true}},
         {"Rotate Camera", "camera.rotate_with_mouse", {.key = key::mouse2}, {.toggle = true}},

         {"Select", "edit.select", {.key = key::mouse1}},
         {"Deselect", "edit.deselect", {.key = key::escape}},

         {"Undo", "edit.undo", {.key = key::z, .modifiers = {.ctrl = true}}},
         {"Redo", "edit.redo", {.key = key::y, .modifiers = {.ctrl = true}}},
         {"Delete", "edit.delete", {.key = key::del}},

         {"Show Hotkeys", "hotkeys.show", {.key = key::f1}},

         {"Save", "save", {.key = key::s, .modifiers = {.ctrl = true}}, {.ignore_imgui_focus = true}},
      });

   _hotkeys.add_set(
      "Entity Creation",
      [this] { return _interaction_targets.creation_entity.has_value(); },
      {
         {"Change Rotation Mode",
          "entity_creation.cycle_rotation_mode",
          {.key = key::q, .modifiers = {.ctrl = true}}},
         {"Change Placement Mode",
          "entity_creation.cycle_placement_mode",
          {.key = key::w, .modifiers = {.ctrl = true}}},
         {"Change Alignment Mode",
          "entity_creation.cycle_alignment_mode",
          {.key = key::e, .modifiers = {.ctrl = true}}},
         {"Change Grounding Mode",
          "entity_creation.cycle_ground_mode",
          {.key = key::r, .modifiers = {.ctrl = true}}},
         {"Cycle Object Class", "entity_creation.cycle_object_class", {.key = key::q}},

         {"Start Point At", "entity_creation.activate_point_at", {.key = key::v}},
         {"Start Extend To", "entity_creation.activate_extend_to", {.key = key::t}},
         {"Start Shrink To",
          "entity_creation.activate_shrink_to",
          {.key = key::t, .modifiers = {.ctrl = true}}},
         {"Start From Object BBOX",
          "entity_creation.activate_from_object_bbox",
          {.key = key::b}},
         {"Start From Line",
          "entity_creation.activate_from_line",
          {.key = key::f, .modifiers = {.ctrl = true}}},

         {"Lock X Axis", "entity_creation.lock_x_axis", {.key = key::z}},
         {"Lock Y Axis", "entity_creation.lock_y_axis", {.key = key::x}},
         {"Lock Z Axis", "entity_creation.lock_z_axis", {.key = key::c}},

         {"Place Entity", "entity_creation.place", {.key = key::mouse1}},
         {"Cancel", "entity_creation.cancel", {.key = key::escape}},

         {"Finish Path", "entity_creation.finish_path", {.key = key::g}},
      });

   _hotkeys.add_set("Entity Creation (Point At)",
                    [this] {
                       return _interaction_targets.creation_entity and
                              _entity_creation_context.using_point_at;
                    },
                    {
                       {"Stop Point At",
                        "entity_creation.deactivate_point_at",
                        {.key = key::mouse1}},
                       {"Stop Point At (Escape)",
                        "entity_creation.deactivate_point_at",
                        {.key = key::escape}},
                    });

   _hotkeys.add_set("Entity Creation (Resize To)",
                    [this] {
                       return _interaction_targets.creation_entity and
                              (_entity_creation_context.using_extend_to or
                               _entity_creation_context.using_shrink_to);
                    },
                    {
                       {"Stop Resize To",
                        "entity_creation.deactivate_resize_to",
                        {.key = key::mouse1}},
                       {"Stop Resize To (Escape)",
                        "entity_creation.deactivate_resize_to",
                        {.key = key::escape}},
                    });

   _hotkeys.add_set("Entity Creation (From BBOX)",
                    [this] {
                       return _interaction_targets.creation_entity and
                              _entity_creation_context.using_from_object_bbox;
                    },
                    {
                       {"Stop From Object BBOX",
                        "entity_creation.deactivate_from_object_bbox",
                        {.key = key::mouse1}},
                       {"Stop From Object BBOX (Escape)",
                        "entity_creation.deactivate_from_object_bbox",
                        {.key = key::escape}},
                    });

   _hotkeys.add_set("Entity Creation (From Line)",
                    [this] {
                       return _interaction_targets.creation_entity and
                              _entity_creation_context.using_from_line;
                    },
                    {
                       {"Stop From Line",
                        "entity_creation.from_line_click",
                        {.key = key::mouse1}},
                       {"Stop From Line (Escape)",
                        "entity_creation.deactivate_from_line",
                        {.key = key::escape}},
                    });
}

}