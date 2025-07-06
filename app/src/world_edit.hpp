#pragma once

#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "commands.hpp"
#include "container/ring_set.hpp"
#include "edits/stack.hpp"
#include "gizmos.hpp"
#include "graphics/camera.hpp"
#include "graphics/gpu/exception.hpp"
#include "graphics/renderer.hpp"
#include "hotkeys.hpp"
#include "io/path.hpp"
#include "output_stream.hpp"
#include "scale_factor.hpp"
#include "settings/io.hpp"
#include "settings/settings.hpp"
#include "utility/command_line.hpp"
#include "utility/stopwatch.hpp"
#include "world/object_class.hpp"
#include "world/object_class_library.hpp"
#include "world/tool_visualizers.hpp"
#include "world/utility/animation.hpp"
#include "world/utility/terrain_light_map_baker.hpp"
#include "world/world.hpp"

#include <memory>
#include <vector>

#include <Windows.h>

struct ImGuiContext;

namespace we {

enum class mouse_cursor {
   none,
   arrow,
   ibeam,
   wait,
   cross,
   size_nwse,
   size_nesw,
   size_we,
   size_ns,
   size_all,
   no,
   hand,
   app_starting,
   pen,

   rotate_cw,
   enlarge_texture,
   shrink_texture,
   paintbrush,

   COUNT
};

enum class select_method : uint8 { replace, add, remove };

enum class placement_mode : uint8 { manual, cursor };

enum class placement_ground : uint8 { origin, bbox };

enum class placement_rotation : uint8 {
   manual_euler,
   manual_quaternion,
   surface
};

enum class surface_rotation_axis : uint8 { x, y, z, neg_x, neg_y, neg_z };

enum class placement_node_insert : uint8 { nearest, append };

enum class placement_resize_mode : uint8 { off, extend, shrink };

enum class entity_creation_tool : uint8 {
   none,
   point_at,
   extend_to,
   shrink_to,
   from_object_bbox,
   from_line,
   draw,
   pick_sector
};

enum class draw_light_step : uint8 {
   start,
   direction,
   point_radius,
   cone_length,
   cone_radius,
   region_box_depth,
   region_box_width,
   region_box_height,
   region_box_direction,
   region_sphere_radius,
   region_sphere_direction,
   region_cylinder_radius,
   region_cylinder_height,
   region_cylinder_direction
};

enum class draw_region_step : uint8 {
   start,
   box_depth,
   box_width,
   box_height,
   sphere_radius,
   cylinder_radius,
   cylinder_height
};

enum class draw_boundary_step : uint8 { start, end_x, radius_z };

enum class selection_edit_tool : uint8 {
   none,
   move,
   move_with_cursor,
   move_path,
   move_sector_point,
   rotate,
   rotate_around_centre,
   rotate_light_region,
   set_layer,
   match_transform,
   pick_sector,
   add_branch_weight,
   resize_entity
};

enum class add_branch_weight_step : uint8 { connection, target };

enum class gizmo_transform_space : uint8 { world, local };

enum class selection_pick_sector_index : uint8 { _1, _2 };

enum class gizmo_object_placement : uint8 { position, bbox_centre };

enum class terrain_edit_tool : uint8 {
   none,
   editor,
   import_heightmap,
   import_texture_weight_map,
   import_color_map,
   resize,
   crop,
   extend,
   light_baker,
   water_editor,
   foliage_editor
};

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

enum class animation_key_type : uint8 { position, rotation };

enum class animation_playback_state : uint8 { stopped, paused, play };

enum class block_edit_tool : uint8 {
   none,
   draw,
   rotate_texture,
   scale_texture,
   paint_material,
   set_texture_mode,
   offset_texture,
   resize_block,
};

enum class draw_block_type {
   box,
   ramp,
   quad,
   cylinder,
   stairway,
   cone,
   hemisphere,
   pyramid,
   ring,
   beveled_box,
   curve,
};

enum class draw_block_step : uint8 {
   start,
   box_depth,
   box_width,
   box_height,
   ramp_width,
   ramp_length,
   ramp_height,
   quad_v1,
   quad_v2,
   quad_v3,
   cylinder_radius,
   cylinder_height,
   stairway_width,
   stairway_length,
   stairway_height,
   cone_radius,
   cone_height,
   hemisphere_radius,
   pyramid_depth,
   pyramid_width,
   pyramid_height,
   ring_inner_radius,
   ring_outer_radius,
   ring_height,
   beveled_box_depth,
   beveled_box_width,
   beveled_box_height,
   curve_p3,
   curve_finalize,
};

enum class draw_block_cursor_plane : uint8 { none, x, y, z };

enum class draw_block_quad_split : uint8 { longest, shortest };

constexpr float tool_window_start_x = 264.0f;

class world_edit {
public:
   world_edit(const HWND window, utility::command_line command_line);

   ~world_edit();

   void update();

   void idle_exit() noexcept;

   void recreate_renderer_pending() noexcept;

   void save_world_with_picker() noexcept;

   void resized(uint16 width, uint16 height);

   void focused();

   void unfocused();

   void mouse_enter();

   void mouse_leave();

   bool idling() const noexcept;

   bool mouse_over() const noexcept;

   bool can_close() const noexcept;

   void dpi_changed(const int new_dpi) noexcept;

   void key_down(const key key) noexcept;

   void key_up(const key key) noexcept;

   void mouse_movement(const int32 x_movement, const int32 y_movement) noexcept;

   auto get_mouse_cursor() const noexcept -> mouse_cursor;

   auto get_swap_chain_waitable_object() noexcept -> HANDLE;

private:
   void recreate_renderer() noexcept;

   void update_window_text() noexcept;

   void update_input() noexcept;

   void update_hovered_entity() noexcept;

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

   void ui_show_world_selection_rotate_light_region() noexcept;

   void ui_show_world_selection_set_layer() noexcept;

   void ui_show_world_selection_match_transform() noexcept;

   void ui_show_world_selection_pick_sector() noexcept;

   void ui_show_world_selection_add_branch_weight() noexcept;

   void ui_show_world_selection_resize_entity() noexcept;

   void ui_show_terrain_editor() noexcept;

   void ui_show_terrain_import_height_map() noexcept;

   void ui_show_terrain_import_texture_weight_map() noexcept;

   void ui_show_terrain_import_color_map() noexcept;

   void ui_show_terrain_resize() noexcept;

   void ui_show_terrain_crop() noexcept;

   void ui_show_terrain_extend() noexcept;

   void ui_show_water_editor() noexcept;

   void ui_show_foliage_editor() noexcept;

   void ui_show_terrain_light_baker() noexcept;

   void ui_show_terrain_light_bake_progress() noexcept;

   void ui_show_about_window() noexcept;

   void ui_show_object_class_browser() noexcept;

   void ui_show_render_env_map() noexcept;

   void ui_show_measurement_tool() noexcept;

   void ui_show_animation_editor() noexcept;

   void ui_show_animation_group_editor() noexcept;

   void ui_show_animation_hierarchy_editor() noexcept;

   void ui_show_block_editor() noexcept;

   void ui_show_block_material_editor() noexcept;

   void ui_draw_select_box() noexcept;

   bool ui_object_class_pick_widget(world::object* object) noexcept;

   void setup_orbit_camera() noexcept;

   bool edit_stack_gizmo_position(const gizmo_position_desc& desc, float3* value) noexcept;

   void start_entity_select() noexcept;

   void finish_entity_select(const select_method method) noexcept;

   void lock_entity_select_size() noexcept;

   void place_creation_entity() noexcept;

   void place_creation_entity_at_camera() noexcept;

   void command_post_auto_place_meta_entities(world::object& object) noexcept;

   void add_object_to_sectors(const world::object& object) noexcept;

   void cycle_creation_entity_object_class() noexcept;

   void toggle_planning_entity_type() noexcept;

   void undo() noexcept;

   void redo() noexcept;

   void paste() noexcept;

   void copy_selected() noexcept;

   void cut_selected() noexcept;

   void delete_selected() noexcept;

   void align_selection(const float alignment) noexcept;

   void toggle_hide_selection() noexcept;

   void ground_selection() noexcept;

   void new_entity_from_selection() noexcept;

   void focus_on_selection() noexcept;

   void unhide_all() noexcept;

   void floor_terrain() noexcept;

   void ask_to_save_world() noexcept;

   void open_project(const io::path& path) noexcept;

   void open_project_with_picker() noexcept;

   void close_project() noexcept;

   void load_world(const io::path& path) noexcept;

   void load_world_with_picker() noexcept;

   void save_world(const io::path& path) noexcept;

   void close_world() noexcept;

   void save_entity_group_with_picker(const world::entity_group& group) noexcept;

   void enumerate_project_worlds() noexcept;

   void open_odfs_for_selected() noexcept;

   void open_odf_in_text_editor(const lowercase_string& asset_name) noexcept;

   void show_odf_in_explorer(const lowercase_string& asset_name) noexcept;

   void initialize_commands() noexcept;

   void initialize_hotkeys() noexcept;

   void initialize_imgui_font() noexcept;

   void initialize_imgui_style() noexcept;

   void handle_gpu_error(graphics::gpu::exception& e) noexcept;

   auto get_renderer_init_params() noexcept -> graphics::renderer_init;

   std::unique_ptr<output_stream> _stream = make_async_output_stream_stdout();
   HWND _window{};
   settings::settings _settings;
   std::shared_ptr<async::thread_pool> _thread_pool = async::thread_pool::make();

   bool _focused = true;
   bool _mouse_over = false;
   bool _window_unsaved_star = false;
   bool _recreate_renderer_pending = false;
   bool _window_resized = false;
   float _applied_user_display_scale = 1.0f;
   int _current_dpi = 96;
   uint16 _window_width = 0;
   uint16 _window_height = 0;
   scale_factor _display_scale{.value = 1.0f};
   utility::stopwatch _last_update_timer;
   utility::stopwatch _sprint_timer;

   int32 _queued_mouse_movement_x = 0;
   int32 _queued_mouse_movement_y = 0;
   int32 _mouse_movement_x = 0;
   int32 _mouse_movement_y = 0;

   float3 _camera_orbit_centre = {0.0f, 0.0f, 0.0f};
   float _camera_orbit_distance = 0.0f;

   std::unique_ptr<ImGuiContext, void (*)(ImGuiContext*)> _imgui_context;

   io::path _project_dir;
   io::path _world_path;

   std::vector<io::path> _project_world_paths;

   assets::libraries_manager _asset_libraries{*_stream, _thread_pool};
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

   bool _renderer_prefer_high_performance_gpu = false;
   bool _renderer_use_debug_layer = false;
   bool _renderer_use_legacy_barriers = false;
   bool _renderer_never_use_shader_model_6_6 = false;
   bool _renderer_never_use_open_existing_heap = false;
   bool _renderer_never_use_write_buffer_immediate = false;
   bool _renderer_never_use_relaxed_format_casting = false;
   bool _renderer_never_use_target_independent_rasterization = false;

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
   bool _selecting_entity_locked_size = false;
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
   bool _animation_editor_open = false;
   bool _animation_group_editor_open = false;
   bool _animation_hierarchy_editor_open = false;
   bool _block_editor_open = false;
   bool _block_material_editor_open = false;
   terrain_edit_tool _terrain_edit_tool = terrain_edit_tool::none;
   selection_edit_tool _selection_edit_tool = selection_edit_tool::none;
   gizmo_object_placement _gizmo_object_placement = gizmo_object_placement::position;
   gizmo_transform_space _selection_move_space = gizmo_transform_space::world;
   gizmo_transform_space _selection_rotate_space = gizmo_transform_space::world;

   bool _terrain_light_map_baking = false;
   bool _env_map_render_requested = false;

   std::string _entity_group_filter;
   container::ring_set<lowercase_string, 4> _recent_entity_groups;

   std::string _layer_editor_new_name;
   std::string _game_mode_editor_new_name;
   std::string _req_editor_new_name;
   std::string _req_editor_add_entry;

   std::string _world_explorer_filter;
   std::string _world_explorer_class_filter;
   std::vector<uint32> _world_explorer_sort_map;
   std::string _object_class_pick_filter;
   int32 _object_class_pick_keyboard_hover = -1;

   bool _world_explorer_path_show_all_nodes = false;
   world::active_layers _world_explorer_layers_mask{true};

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

      int8 last_layer = 0;

      container::ring_set<lowercase_string, 8> last_used_object_classes;
   } _last_created_entities;

   struct entity_creation_context {
      bool lock_x_axis = false;
      bool lock_y_axis = false;
      bool lock_z_axis = false;

      bool rotate_forward = false;
      bool rotate_back = false;

      entity_creation_tool activate_tool = entity_creation_tool::none;
      entity_creation_tool tool = entity_creation_tool::none;

      bool finish_current_path = false;
      bool finish_current_sector = false;
      bool finish_from_object_bbox = false;

      bool from_line_click = false;
      bool draw_click = false;

      bool hub_sizing_started = false;
      bool connection_link_started = false;

      bool measurement_started = false;

      draw_light_step draw_light_step = draw_light_step::start;
      draw_region_step draw_region_step = draw_region_step::start;
      draw_boundary_step draw_boundary_step = draw_boundary_step::start;

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

      float3 draw_light_start;
      float3 draw_light_depth;
      float3 draw_light_width;
      float draw_light_region_rotation_angle = 0.0f;
      float3 draw_light_region_position;
      float3 draw_light_region_size;

      float3 draw_region_start;
      float3 draw_region_depth;
      float3 draw_region_width;
      float draw_region_rotation_angle = 0.0f;

      float3 draw_boundary_start;
      float draw_boundary_end_x;
   } _entity_creation_context;

   struct entity_creation_config {
      placement_rotation placement_rotation = placement_rotation::manual_euler;
      surface_rotation_axis surface_rotation_axis = surface_rotation_axis::y;
      placement_mode placement_mode = placement_mode::cursor;
      placement_ground placement_ground = placement_ground::origin;
      placement_node_insert placement_node_insert = placement_node_insert::append;
      gizmo_transform_space gizmo_position_space = gizmo_transform_space::world;

      float snap_distance = 0.25f;
      float from_bbox_padding = 0.0f;

      bool command_post_auto_place_meta_entities = true;
      bool auto_fill_sector = true;
      bool auto_add_object_to_sectors = true;
      bool placement_cursor_align = false;
      bool placement_cursor_snapping = false;
      bool snap_to_corners = true;
      bool snap_to_edge_midpoints = true;
      bool snap_to_face_midpoints = true;

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

   struct selection_match_transform_config {
      bool match_rotation = true;
      bool match_position = true;
   } _selection_match_transform_config;

   struct selection_match_transform_context {
      bool clicked = false;
   } _selection_match_transform_context;

   struct selection_pick_sector_context {
      bool clicked = false;
      selection_pick_sector_index index = selection_pick_sector_index::_1;
      world::portal_id id = {};
   } _selection_pick_sector_context;

   struct selection_add_branch_weight_context {
      bool clicked = false;
      add_branch_weight_step step = add_branch_weight_step::connection;
      world::planning_hub_id from_hub_id = {};
      world::planning_connection_id connection_id = {};
   } _selection_add_branch_weight_context;

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
      utility::stopwatch last_brush_update_timer;
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
      int heightmap_precision_steps = 100;
      bool start_from_bottom = false;
      bool start_from_midpoint = false;
   } _terrain_import_heightmap_context;

   struct terrain_import_texture_weight_map_context {
      std::string error_message;

      int weight_map_index = -1;
   } _terrain_import_texture_weight_map_context;

   struct terrain_import_color_map_context {
      std::string error_message;

      bool as_light_map = false;
   } _terrain_import_color_map_context;

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

   world::terrain_light_map_baker_config _terrain_light_baker_config;

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

   struct animation_editor_config {
      bool match_tangents = true;
      bool auto_tangents = false;
      float auto_tangent_smoothness = 1.0f;

      std::string new_animation_name;
   } _animation_editor_config;

   struct animation_editor_context {
      struct selected {
         world::animation_id id = world::max_id;

         int32 key = 0;
         animation_key_type key_type = animation_key_type::position;

         float new_position_key_time = 0.0f;
         float new_rotation_key_time = 0.0f;

         bool delete_key = false;
         bool toggle_playback = false;
         bool stop_playback = false;

         animation_playback_state playback_state = animation_playback_state::stopped;
         float playback_time = 0.0f;

         utility::stopwatch playback_tick_timer;
      } selected;

      bool select = false;
      bool select_behind = false;

      struct place {
         bool active = false;
         bool finish = false;
      } place;

      struct pick_object {
         bool active = false;
         bool finish = false;
      } pick_object;

      std::string add_new_group = "";
      std::string add_new_object = "";
   } _animation_editor_context;

   struct animation_group_editor_config {
      std::string new_group_name;

      std::string new_entry_animation_name;
      std::string new_entry_object_name;
   } _animation_group_editor_config;

   struct animation_group_editor_context {
      struct selected {
         world::animation_group_id id = world::max_id;

         animation_playback_state playback_state = animation_playback_state::stopped;
         float playback_time = 0.0f;

         utility::stopwatch playback_tick_timer;

         struct playback_cache_entry {
            std::string animation_name;
            std::string object_name;

            world::animation_id animation = world::max_id;
            world::animation_hierarchy_id animation_hierarchy = world::max_id;
            world::object_id object = world::max_id;

            quaternion base_rotation;
            float3 base_position;
            quaternion inverse_base_rotation;
            float3 inverse_base_position;

            float animation_runtime = 0.0f;
            bool animation_loops = false;

            struct child {
               std::string name;
               float4x4 transform;
               world::object_id id;
            };

            std::vector<child> children;
         };

         std::vector<playback_cache_entry> playback_cache;
      } selected;

      struct pick_object {
         bool active = false;
         bool finish = false;
      } pick_object;
   } _animation_group_editor_context;

   struct animation_hierarchy_editor_config {
      std::string new_root_object_name;
      std::string new_child_object_name;
   } _animation_hierarchy_editor_config;

   struct animation_hierarchy_editor_context {
      struct selected {
         world::animation_hierarchy_id id = world::max_id;
      } selected;

      struct pick_object {
         enum class target : uint8 { root, child };

         target target = target::root;
         bool active = false;
         bool finish = false;
      } pick_object;
   } _animation_hierarchy_editor_context;

   world::animation_solver _animation_solver;

   struct class_browser_context {
      std::string filter;

      std::vector<uint32> selected_stack;
      lowercase_string selected_name;

      std::vector<uint32> traversal_stack;

      std::vector<const assets::library_tree_branch*> branch_stack;
   } _class_browser_context;

   struct block_editor_config {
      int xz_alignment_exponent = 0;
      int y_alignment_exponent = 0;
      int snap_edge_points = 3;

      draw_block_type draw_type = draw_block_type::box;

      draw_block_quad_split quad_split = draw_block_quad_split::longest;

      float step_height = 0.125f;
      float first_step_offset = 0.0f;

      struct ring {
         uint16 segments = 16;
         bool flat_shading = false;
         float texture_loops = 1.0f;
      } ring;

      struct beveled_box {
         float amount = 0.125f;

         bool bevel_top = true;
         bool bevel_sides = true;
         bool bevel_bottom = true;
      } beveled_box;

      struct curve {
         float width = 1.0f;
         float height = 1.0f;
         uint16 segments = 16;
         float texture_loops = 1.0f;
      } curve;

      bool enable_alignment = true;
      bool enable_snapping = true;

      int8 new_block_layer = 0;

      bool scale_texture_u = true;
      bool scale_texture_v = true;

      world::block_texture_mode texture_mode = {};

      uint8 paint_material_index = 0;
      std::string paint_material_filter;
   } _block_editor_config;

   struct block_editor_context {
      block_edit_tool activate_tool = block_edit_tool::none;
      block_edit_tool tool = block_edit_tool::none;

      bool tool_click = false;
      bool tool_ctrl_click = false;

      struct draw_block {
         draw_block_step step = draw_block_step::start;
         std::optional<float> height_plane = std::nullopt;

         draw_block_cursor_plane toggle_plane = draw_block_cursor_plane::none;
         draw_block_cursor_plane cursor_plane = draw_block_cursor_plane::none;
         float3 cursor_plane_positionWS;

         struct box {
            float3 start;
            float depth_x = 0.0f;
            float depth_z = 0.0f;
            float width_x = 0.0f;
            float width_z = 0.0f;
            quaternion rotation;
         } box;

         struct ramp {
            float3 start;
            float width_x = 0.0f;
            float width_z = 0.0f;
            float length_x = 0.0f;
            float length_z = 0.0f;
            quaternion rotation;
         } ramp;

         struct quad {
            std::array<float3, 3> vertices;
         } quad;

         struct cylinder {
            float3 start;
            float radius;
         } cylinder;

         struct stairway {
            float3 start;
            float width_x = 0.0f;
            float width_z = 0.0f;
            float length_x = 0.0f;
            float length_z = 0.0f;
            quaternion rotation;
         } stairway;

         struct cone {
            float3 start;
            float radius;
         } cone;

         struct hemisphere {
            float3 start;
         } hemisphere;

         struct pyramid {
            float3 start;
            float depth_x = 0.0f;
            float depth_z = 0.0f;
            float width_x = 0.0f;
            float width_z = 0.0f;
            quaternion rotation;
         } pyramid;

         struct ring {
            float3 start;
            float inner_radius;
            float outer_radius;
         } ring;

         struct beveled_box {
            float3 start;
            float depth_x = 0.0f;
            float depth_z = 0.0f;
            float width_x = 0.0f;
            float width_z = 0.0f;
            quaternion rotation;
         } beveled_box;

         struct curve {
            float3 p0;
            float3 p1;
            float3 p2;
            float3 p3;
         } curve;

         uint32 index = 0;
         world::block_id block_id = world::block_id::none;
      } draw_block;

      struct offset_texture {
         world::block_id block_id = world::block_id::none;
         uint32 surface_index = 0;
      } offset_texture;

      struct resize_block {
         world::block_id block_id = world::block_id::none;
      } resize_block;
   } _block_editor_context;

   struct block_material_editor_config {
      std::string filter;
   } _block_material_editor_config;

   struct block_material_editor_context {
      uint32 selected_index = UINT32_MAX;
   } _block_material_editor_context;

   float3 _cursor_positionWS = {0.0f, 0.0f, 0.0f};
   std::optional<float3> _cursor_surface_normalWS;

   float _editor_floor_height = 0.0f;
   float _editor_grid_size = 4.0f;

   float3 _rotate_selection_amount = {0.0f, 0.0f, 0.0f};
   float3 _rotate_selection_centre = {0.0f, 0.0f, 0.0f};
   world::path_id _move_entire_path_id = {};
   world::sector_id _move_sector_point_id = {};
   uint32 _move_sector_point_index = 0;
   world::light_id _rotate_light_region_id = {};
   int8 _selection_set_layer = 0;
   bool _selection_cursor_move_ground_with_bbox = true;
   bool _selection_cursor_move_align_cursor = false;
   bool _selection_cursor_move_snap_cursor = false;
   bool _selection_cursor_move_snap_to_corners = true;
   bool _selection_cursor_move_snap_to_edge_midpoints = true;
   bool _selection_cursor_move_snap_to_face_midpoints = true;
   bool _selection_cursor_move_lock_x_axis = false;
   bool _selection_cursor_move_lock_y_axis = false;
   bool _selection_cursor_move_lock_z_axis = false;
   bool _selection_cursor_move_rotate_forward = false;
   bool _selection_cursor_move_rotate_back = false;
   world::active_entity_types _selection_cursor_move_hit_mask;
   float _selection_cursor_move_snap_distance = 0.125f;

   float2 _select_start_position;
   float2 _select_locked_sign;
   float2 _select_locked_size;
   float2 _cursor_placement_lock_position;

   std::optional<world::terrain_light_map_baker> _terrain_light_map_baker;

   graphics::env_map_params _env_map_render_params;
   float3 _env_map_render_offset;
   io::path _env_map_save_path;
   std::string _env_map_save_error;

   gizmos _gizmos;
   commands _commands;
   hotkeys _hotkeys{_commands, *_stream};

   settings::saver _settings_saver{".settings", _settings};
};

}
