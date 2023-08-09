#pragma once

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "commands.hpp"
#include "container/ring_set.hpp"
#include "edits/stack.hpp"
#include "gizmo.hpp"
#include "graphics/camera.hpp"
#include "graphics/gpu/exception.hpp"
#include "graphics/renderer.hpp"
#include "hotkeys.hpp"
#include "output_stream.hpp"
#include "settings/io.hpp"
#include "settings/settings.hpp"
#include "utility/command_line.hpp"
#include "world/object_class.hpp"
#include "world/object_class_library.hpp"
#include "world/tool_visualizers.hpp"
#include "world/world.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>

#include <Windows.h>

struct ImGuiContext;

namespace we {

enum class select_method : uint8 { single, multi };

enum class placement_mode : uint8 { manual, cursor };

enum class placement_alignment : uint8 { none, grid, snapping };

enum class placement_ground : uint8 { origin, bbox };

enum class placement_rotation : uint8 {
   manual_euler,
   manual_quaternion,
   surface
};

enum class placement_node_insert : uint8 { nearest, append };

enum class placement_resize_mode : uint8 { off, extend, shrink };

enum class selection_edit_tool : uint8 {
   none,
   move,
   move_path,
   move_sector_point,
   rotate,
   rotate_around_centre
};

enum class gizmo_object_placement : uint8 { position, bbox_centre };

constexpr float tool_window_start_x = 264.0f;

class world_edit {
public:
   world_edit(const HWND window, utility::command_line command_line);

   void update();

   void resized(uint16 width, uint16 height);

   void focused();

   void unfocused();

   bool idling() const noexcept;

   bool can_close() const noexcept;

   void dpi_changed(const int new_dpi) noexcept;

   void key_down(const key key) noexcept;

   void key_up(const key key) noexcept;

   void mouse_movement(const int32 x_movement, const int32 y_movement) noexcept;

private:
   void wait_for_swap_chain_ready() noexcept;

   void update_window_text() noexcept;

   void update_input() noexcept;

   void update_hovered_entity() noexcept;

   void update_object_classes() noexcept;

   void update_camera(const float delta_time);

   void update_ui() noexcept;

   void ui_show_main_menu_bar() noexcept;

   void ui_show_world_active_context() noexcept;

   void ui_show_hotkeys_view() noexcept;

   void ui_show_world_selection_editor() noexcept;

   void ui_show_world_creation_editor() noexcept;

   void ui_show_world_global_lights_editor() noexcept;

   void ui_show_world_layers_editor() noexcept;

   void ui_show_world_game_mode_editor() noexcept;

   void ui_show_world_requirements_editor() noexcept;

   void ui_show_world_explorer() noexcept;

   void ui_show_world_stats() noexcept;

   void ui_show_camera_controls() noexcept;

   void ui_show_world_selection_move() noexcept;

   void ui_show_world_selection_move_path() noexcept;

   void ui_show_world_selection_move_sector_point() noexcept;

   void ui_show_world_selection_rotate() noexcept;

   void ui_show_world_selection_rotate_around_centre() noexcept;

   void select_hovered_entity(const select_method method) noexcept;

   void deselect_hovered_entity() noexcept;

   void place_creation_entity() noexcept;

   void command_post_auto_place_meta_entities(const world::object& object) noexcept;

   void add_object_to_sectors(const world::object& object) noexcept;

   void cycle_creation_entity_object_class() noexcept;

   void undo() noexcept;

   void redo() noexcept;

   void delete_selected() noexcept;

   void align_selection() noexcept;

   void new_entity_from_selection() noexcept;

   void ask_to_save_world() noexcept;

   void open_project(std::filesystem::path path) noexcept;

   void open_project_with_picker() noexcept;

   void load_world(std::filesystem::path path) noexcept;

   void load_world_with_picker() noexcept;

   void save_world(std::filesystem::path path) noexcept;

   void save_world_with_picker() noexcept;

   void close_world() noexcept;

   void enumerate_project_worlds() noexcept;

   void open_odfs_for_selected() noexcept;

   void open_odf_in_text_editor(const lowercase_string& asset_name) noexcept;

   void initialize_commands() noexcept;

   void initialize_hotkeys() noexcept;

   void handle_gpu_error(graphics::gpu::exception& e) noexcept;

   standard_output_stream _stream;
   HWND _window{};
   settings::settings _settings;
   std::shared_ptr<async::thread_pool> _thread_pool = async::thread_pool::make();

   bool _focused = true;
   bool _window_unsaved_star = false;
   float _applied_user_display_scale = 1.0f;
   int _current_dpi = 96;
   float _display_scale = 1.0f;
   std::chrono::steady_clock::time_point _last_update =
      std::chrono::steady_clock::now();
   std::chrono::steady_clock::time_point _sprint_start =
      std::chrono::steady_clock::now();

   int32 _queued_mouse_movement_x = 0;
   int32 _queued_mouse_movement_y = 0;
   int32 _mouse_movement_x = 0;
   int32 _mouse_movement_y = 0;

   std::unique_ptr<ImGuiContext, void (*)(ImGuiContext*)> _imgui_context;

   std::filesystem::path _project_dir;
   std::filesystem::path _world_path;

   std::vector<std::filesystem::path> _project_world_paths;

   assets::libraries_manager _asset_libraries{_stream, _thread_pool};
   world::object_class_library _object_classes{_asset_libraries};
   world::world _world;
   world::interaction_targets _interaction_targets;
   world::active_entity_types _world_draw_mask;
   world::active_entity_types _world_hit_mask;
   world::active_layers _world_layers_draw_mask{true};
   world::active_layers _world_layers_hit_mask{true};
   world::terrain_collision _terrain_collision;
   world::tool_visualizers _tool_visualizers;

   edits::stack<world::edit_context> _edit_stack_world;
   world::edit_context _edit_context{.world = _world,
                                     .creation_entity =
                                        _interaction_targets.creation_entity};

   bool _renderer_use_debug_layer = false;

   std::unique_ptr<graphics::renderer> _renderer;
   graphics::camera _camera;

   bool _move_camera_forward = false;
   bool _move_camera_back = false;
   bool _move_camera_left = false;
   bool _move_camera_right = false;
   bool _move_camera_up = false;
   bool _move_camera_down = false;
   bool _move_sprint = false;
   bool _rotate_camera = false;
   bool _pan_camera = false;

   bool _hotkeys_view_show = false;
   bool _imgui_demo_open = false;
   bool _hotkeys_editor_open = false;
   bool _clear_edit_stack_confirm_open = false;
   bool _world_global_lights_editor_open = false;
   bool _world_layers_editor_open = false;
   bool _world_game_mode_editor_open = false;
   bool _world_requirements_editor_open = false;
   bool _world_explorer_open = false;
   bool _world_stats_open = false;
   bool _camera_controls_open = false;
   bool _settings_editor_open = false;
   selection_edit_tool _selection_edit_tool = selection_edit_tool::none;
   gizmo_object_placement _gizmo_object_placement = gizmo_object_placement::position;

   std::string _layer_editor_new_name;
   std::string _game_mode_editor_new_name;
   std::string _req_editor_new_name;
   std::string _req_editor_add_entry;

   std::string _world_explorer_filter;
   std::string _world_explorer_class_filter;

   std::vector<std::string_view> _world_explorer_object_classes;

   bool _world_explorer_path_show_all_nodes = false;

   POINT _rotate_camera_cursor_position = {0, 0};

   struct last_created_entities {
      world::object_id last_object = world::max_id;
      world::light_id last_light = world::max_id;
      world::path_id last_path = world::max_id;
      world::region_id last_region = world::max_id;
      world::sector_id last_sector = world::max_id;
      world::portal_id last_portal = world::max_id;
      world::hintnode_id last_hintnode = world::max_id;
      world::barrier_id last_barrier = world::max_id;
      world::planning_hub_id last_planning_hub = world::max_id;
      world::planning_connection_id last_planning_connection = world::max_id;
      world::boundary_id last_boundary = world::max_id;

      int last_layer = 0;

      container::ring_set<lowercase_string, 8> last_used_object_classes;
   } _last_created_entities;

   struct entity_creation_context {
      bool lock_x_axis = false;
      bool lock_y_axis = false;
      bool lock_z_axis = false;

      bool rotate_forward = false;
      bool rotate_back = false;

      bool activate_point_at = false;
      bool activate_extend_to = false;
      bool activate_shrink_to = false;
      bool activate_from_object_bbox = false;
      bool activate_from_line = false;
      bool activate_draw_barrier = false;
      bool activate_pick_sector = false;

      bool using_point_at = false;
      bool using_extend_to = false;
      bool using_shrink_to = false;
      bool using_from_object_bbox = false;
      bool using_from_line = false;
      bool using_draw_barrier = false;
      bool using_pick_sector = false;

      bool finish_current_path = false;
      bool finish_current_sector = false;
      bool finish_from_object_bbox = false;

      bool from_line_click = false;
      bool draw_barrier_click = false;

      bool hub_sizing_started = false;
      bool connection_link_started = false;

      int pick_sector_index = 0;

      std::optional<float3> resize_start_size;
      std::optional<float3> resize_barrier_start_position;
      std::optional<float2> resize_barrier_start_size;
      std::optional<float2> resize_boundary_start_size;
      std::optional<float> resize_portal_start_width;
      std::optional<float> resize_portal_start_height;

      std::optional<float3> from_line_start;

      std::optional<float3> draw_barrier_start;
      std::optional<float3> draw_barrier_mid;
   } _entity_creation_context;

   struct entity_creation_config {
      placement_rotation placement_rotation = placement_rotation::manual_euler;
      placement_mode placement_mode = placement_mode::cursor;
      placement_alignment placement_alignment = placement_alignment::none;
      placement_ground placement_ground = placement_ground::origin;
      placement_node_insert placement_node_insert = placement_node_insert::append;

      float alignment = 4.0f;
      float snap_distance = 0.25f;

      bool command_post_auto_place_meta_entities = true;
      bool auto_fill_sector = true;
      bool auto_add_object_to_sectors = true;

      float command_post_capture_radius = 8.0f;
      float command_post_control_radius = 16.0f;
      float command_post_control_height = 8.0f;
      float command_post_spawn_radius = 8.0f;

      uint32 cycle_object_class_index = 0;
   } _entity_creation_config;

   struct selection_edit_context {
      bool using_add_object_to_sector = false;
      bool add_hovered_object = false;

      bool ground_objects = false;

      world::sector_id sector_to_add_object_to = world::max_id;
   } _selection_edit_context;

   float3 _cursor_positionWS = {0.0f, 0.0f, 0.0f};
   std::optional<float3> _cursor_surface_normalWS;

   float _editor_floor_height = 0.0f;

   float3 _move_selection_amount = {0.0f, 0.0f, 0.0f};
   float3 _rotate_selection_amount = {0.0f, 0.0f, 0.0f};
   float3 _rotate_selection_centre = {0.0f, 0.0f, 0.0f};
   world::path_id _move_entire_path_id = {};
   world::sector_id _move_sector_point_id = {};
   std::size_t _move_sector_point_index = 0;

   gizmo _gizmo;
   commands _commands;
   hotkeys _hotkeys{_commands, _stream};

   settings::saver _settings_saver{".settings", _settings};
};

}
