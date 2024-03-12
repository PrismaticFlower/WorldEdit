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
#include "scale_factor.hpp"
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

enum class select_method : uint8 { replace, add };

enum class placement_mode : uint8 { manual, cursor };

enum class placement_alignment : uint8 { none, grid, snapping };

enum class placement_ground : uint8 { origin, bbox };

enum class placement_rotation : uint8 {
   manual_euler,
   manual_quaternion,
   surface
};

enum class surface_rotation_axis : uint8 { x, y, z, neg_x, neg_y, neg_z };

enum class placement_node_insert : uint8 { nearest, append };

enum class placement_resize_mode : uint8 { off, extend, shrink };

enum class selection_edit_tool : uint8 {
   none,
   move,
   move_with_cursor,
   move_path,
   move_sector_point,
   rotate,
   rotate_around_centre,
   set_layer
};

enum class selection_move_space : uint8 { world, local };

enum class gizmo_object_placement : uint8 { position, bbox_centre };

enum class terrain_edit_target : uint8 { height, texture, color };

enum class terrain_brush_mode : uint8 {
   raise,
   lower,
   overwrite,
   pull_towards,
   blend
};

enum class terrain_texture_brush_mode : uint8 { paint, spray, erase, soften };

enum class terrain_color_brush_mode : uint8 { paint, spray, blur };

enum class terrain_brush_falloff : uint8 {
   none,
   cone,
   smooth,
   bell,
   ramp,
   custom
};

enum class terrain_brush_rotation : uint8 { r0, r90, r180, r270 };

enum class water_brush_mode : uint8 { paint, erase };

enum class foliage_brush_mode : uint8 { paint, erase };

constexpr float tool_window_start_x = 264.0f;

class world_edit {
public:
   world_edit(const HWND window, utility::command_line command_line);

   ~world_edit();

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

   void ui_show_create_menu_items() noexcept;

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

   void ui_show_world_selection_move_with_cursor() noexcept;

   void ui_show_world_selection_move_path() noexcept;

   void ui_show_world_selection_move_sector_point() noexcept;

   void ui_show_world_selection_rotate() noexcept;

   void ui_show_world_selection_rotate_around_centre() noexcept;

   void ui_show_world_selection_set_layer() noexcept;

   void ui_show_terrain_editor() noexcept;

   void ui_show_terrain_import_height_map() noexcept;

   void ui_show_terrain_import_texture_weight_map() noexcept;

   void ui_show_terrain_resize() noexcept;

   void ui_show_terrain_crop() noexcept;

   void ui_show_terrain_extend() noexcept;

   void ui_show_water_editor() noexcept;

   void ui_show_foliage_editor() noexcept;

   void ui_show_about_window() noexcept;

   void ui_show_object_class_browser() noexcept;

   void ui_show_render_env_map() noexcept;

   void ui_show_measurement_tool() noexcept;

   void ui_draw_select_box() noexcept;

   void setup_orbit_camera() noexcept;

   void start_entity_select() noexcept;

   void finish_entity_select(const select_method method) noexcept;

   void start_entity_deselect() noexcept;

   void finish_entity_deselect() noexcept;

   void place_creation_entity() noexcept;

   void command_post_auto_place_meta_entities(world::object& object) noexcept;

   void add_object_to_sectors(const world::object& object) noexcept;

   void cycle_creation_entity_object_class() noexcept;

   void toggle_planning_entity_type() noexcept;

   void undo() noexcept;

   void redo() noexcept;

   void delete_selected() noexcept;

   void align_selection(const float alignment) noexcept;

   void hide_selection() noexcept;

   void ground_selection() noexcept;

   void new_entity_from_selection() noexcept;

   void focus_on_selection() noexcept;

   void unhide_all() noexcept;

   void ask_to_save_world() noexcept;

   void open_project(std::filesystem::path path) noexcept;

   void open_project_with_picker() noexcept;

   void close_project() noexcept;

   void load_world(std::filesystem::path path) noexcept;

   void load_world_with_picker() noexcept;

   void save_world(std::filesystem::path path) noexcept;

   void save_world_with_picker() noexcept;

   void close_world() noexcept;

   void enumerate_project_worlds() noexcept;

   void open_odfs_for_selected() noexcept;

   void open_odf_in_text_editor(const lowercase_string& asset_name) noexcept;

   void show_odf_in_explorer(const lowercase_string& asset_name) noexcept;

   void initialize_commands() noexcept;

   void initialize_hotkeys() noexcept;

   void initialize_imgui_font() noexcept;

   void initialize_imgui_style() noexcept;

   void handle_gpu_error(graphics::gpu::exception& e) noexcept;

   standard_output_stream _stream;
   HWND _window{};
   settings::settings _settings;
   std::shared_ptr<async::thread_pool> _thread_pool = async::thread_pool::make();

   bool _focused = true;
   bool _window_unsaved_star = false;
   float _applied_user_display_scale = 1.0f;
   int _current_dpi = 96;
   scale_factor _display_scale{.value = 1.0f};
   std::chrono::steady_clock::time_point _last_update =
      std::chrono::steady_clock::now();
   std::chrono::steady_clock::time_point _sprint_start =
      std::chrono::steady_clock::now();

   int32 _queued_mouse_movement_x = 0;
   int32 _queued_mouse_movement_y = 0;
   int32 _mouse_movement_x = 0;
   int32 _mouse_movement_y = 0;

   float3 _camera_orbit_centre = {0.0f, 0.0f, 0.0f};
   float _camera_orbit_distance = 0.0f;

   std::unique_ptr<ImGuiContext, void (*)(ImGuiContext*)> _imgui_context;

   std::filesystem::path _project_dir;
   std::filesystem::path _world_path;

   std::vector<std::filesystem::path> _project_world_paths;

   assets::libraries_manager _asset_libraries{_stream, _thread_pool};
   world::object_class_library _object_classes{_asset_libraries};
   world::world _world;
   world::interaction_targets _interaction_targets;
   world::active_entity_types _world_draw_mask;
   world::active_entity_types _world_hit_mask{};
   world::active_layers _world_layers_draw_mask{true};
   world::active_layers _world_layers_hit_mask{true};
   world::tool_visualizers _tool_visualizers;

   edits::stack<world::edit_context> _edit_stack_world;
   world::edit_context _edit_context{.world = _world,
                                     .creation_entity =
                                        _interaction_targets.creation_entity};

   bool _renderer_use_debug_layer = false;
   bool _renderer_use_legacy_barriers = false;
   bool _renderer_never_use_shader_model_6_6 = false;

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
   bool _orbit_camera_active = false;
   bool _selecting_entity = false;
   bool _cursor_placement_undo_lock = false;
   bool _draw_overlay_grid = false;
   bool _draw_terrain_grid = false;

   bool _hotkeys_view_show = false;
   bool _imgui_demo_open = false;
   bool _hotkeys_editor_open = false;
   bool _world_global_lights_editor_open = false;
   bool _world_layers_editor_open = false;
   bool _world_game_mode_editor_open = false;
   bool _world_requirements_editor_open = false;
   bool _world_explorer_open = false;
   bool _world_stats_open = false;
   bool _object_class_browser_open = false;
   bool _camera_controls_open = false;
   bool _settings_editor_open = false;
   bool _about_window_open = false;
   bool _render_env_map_open = false;
   bool _measurement_tool_open = false;
   bool _terrain_editor_open = false;
   bool _terrain_import_heightmap_open = false;
   bool _terrain_import_texture_weight_map_open = false;
   bool _terrain_resize_open = false;
   bool _terrain_crop_open = false;
   bool _terrain_extend_open = false;
   bool _water_editor_open = false;
   bool _foliage_editor_open = false;
   selection_edit_tool _selection_edit_tool = selection_edit_tool::none;
   gizmo_object_placement _gizmo_object_placement = gizmo_object_placement::position;
   selection_move_space _selection_move_space = selection_move_space::world;

   bool _env_map_render_requested = false;

   std::string _layer_editor_new_name;
   std::string _game_mode_editor_new_name;
   std::string _req_editor_new_name;
   std::string _req_editor_add_entry;

   std::string _world_explorer_filter;
   std::string _world_explorer_class_filter;

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

      int16 last_layer = 0;

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

      bool measurement_started = false;

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
      surface_rotation_axis surface_rotation_axis = surface_rotation_axis::y;
      placement_mode placement_mode = placement_mode::cursor;
      placement_alignment placement_alignment = placement_alignment::none;
      placement_ground placement_ground = placement_ground::origin;
      placement_node_insert placement_node_insert = placement_node_insert::append;

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

      world::sector_id sector_to_add_object_to = world::max_id;
   } _selection_edit_context;

   struct terrain_editor_config {
      terrain_edit_target edit_target = terrain_edit_target::height;
      int32 brush_size_x = 1;
      int32 brush_size_y = 1;
      int32 custom_brush_index = 0;

      struct height_config {
         terrain_brush_mode brush_mode = terrain_brush_mode::raise;
         terrain_brush_falloff brush_falloff = terrain_brush_falloff::none;
         terrain_brush_rotation brush_rotation = terrain_brush_rotation::r0;
         float brush_height = 0.0f;
         float brush_speed = 0.5f;
         float brush_rate = 2.5f;
      } height;

      struct texture_config {
         terrain_texture_brush_mode brush_mode = terrain_texture_brush_mode::paint;
         terrain_brush_falloff brush_falloff = terrain_brush_falloff::none;
         terrain_brush_rotation brush_rotation = terrain_brush_rotation::r0;
         float brush_texture_weight = 255.0f;
         float brush_rate = 200.0f;
         float brush_speed = 0.5f;
         uint32 edit_texture = 0;
      } texture;

      struct color_config {
         terrain_color_brush_mode brush_mode = terrain_color_brush_mode::paint;
         terrain_brush_falloff brush_falloff = terrain_brush_falloff::none;
         terrain_brush_rotation brush_rotation = terrain_brush_rotation::r0;
         float3 brush_color = {1.0f, 1.0f, 1.0f};
         float brush_rate = 1.0f;
         float brush_speed = 0.5f;
      } color;

      struct custom_brush {
         std::string name;
         container::dynamic_array_2d<uint8> falloff_map;
      };

      std::vector<custom_brush> custom_brushes;
      std::string custom_brush_error;
   } _terrain_editor_config;

   struct terrain_editor_context {
      bool brush_held = false;
      bool brush_active = false;

      float2 brush_start_mouse_position;
      float2 locked_terrain_point;

      float brush_plane_height = 0.0f;
      std::chrono::steady_clock::time_point last_brush_update =
         std::chrono::steady_clock::now();
   } _terrain_editor_context;

   struct terrain_editor_maps {
      constexpr static int active_mask_factor = 4;
      constexpr static int mask_word_bits = 64;

      int32 length = 256;

      container::dynamic_array_2d<uint64> active_mask{((length / active_mask_factor) +
                                                       (mask_word_bits - 1)) /
                                                         mask_word_bits,
                                                      length / active_mask_factor};

      container::dynamic_array_2d<float> height{length, length};
      container::dynamic_array_2d<float3> color{length, length};
   } _terrain_editor_maps;

   struct terrain_import_heightmap_context {
      container::dynamic_array_2d<uint8> loaded_heightmap_u8;
      container::dynamic_array_2d<uint16> loaded_heightmap_u16;
      std::string error_message;

      float heightmap_peak_height = 0.0f;
      float heightmap_terrain_world_size = 0.0f;
      bool start_from_bottom = false;
      bool start_from_midpoint = false;
   } _terrain_import_heightmap_context;

   struct terrain_import_texture_weight_map_context {
      container::dynamic_array_2d<uint8> loaded_weight_map;
      std::string error_message;

      int weight_map_index = -1;
   } _terrain_import_texture_weight_map_context;

   struct terrain_resize_context {
      int32 new_length = 0;
      bool maintain_world_proportions = true;
      bool cubic_interpolation = false;
   } _terrain_resize_context;

   struct terrain_crop_context {
      int32 new_length = 0;
   } _terrain_crop_context;

   struct terrain_extend_context {
      int32 new_length = 0;
      bool fill_from_edges = true;
   } _terrain_extend_context;

   struct water_editor_config {
      water_brush_mode brush_mode = water_brush_mode::paint;
      int32 brush_size_x = 1;
      int32 brush_size_y = 1;
   } _water_editor_config;

   struct water_editor_context {
      bool flood_fill_active = false;
      bool brush_held = false;
      bool brush_active = false;
      bool flooded = false;
   } _water_editor_context;

   struct foliage_editor_config {
      foliage_brush_mode brush_mode = foliage_brush_mode::paint;
      int32 brush_size_x = 1;
      int32 brush_size_y = 1;
      int32 layer = 0;
   } _foliage_editor_config;

   struct foliage_editor_context {
      bool brush_held = false;
      bool brush_active = false;
   } _foliage_editor_context;

   float3 _cursor_positionWS = {0.0f, 0.0f, 0.0f};
   std::optional<float3> _cursor_surface_normalWS;

   float _editor_floor_height = 0.0f;
   float _editor_grid_size = 4.0f;

   float3 _move_selection_amount = {0.0f, 0.0f, 0.0f};
   float3 _rotate_selection_amount = {0.0f, 0.0f, 0.0f};
   float3 _rotate_selection_centre = {0.0f, 0.0f, 0.0f};
   world::path_id _move_entire_path_id = {};
   world::sector_id _move_sector_point_id = {};
   uint32 _move_sector_point_index = 0;
   int16 _selection_set_layer = 0;
   bool _selection_cursor_move_ground_with_bbox = true;
   world::active_entity_types _selection_cursor_move_hit_mask;

   float2 _select_start_position;
   float2 _cursor_placement_lock_position;

   graphics::env_map_params _env_map_render_params;
   float3 _env_map_render_offset;
   std::filesystem::path _env_map_save_path;
   std::string _env_map_save_error;

   gizmo _gizmo;
   commands _commands;
   hotkeys _hotkeys{_commands, _stream};

   settings::saver _settings_saver{".settings", _settings};
};

}
