
#include "actions/stack.hpp"
#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "commands.hpp"
#include "graphics/camera.hpp"
#include "graphics/gpu/exception.hpp"
#include "graphics/renderer.hpp"
#include "hotkeys.hpp"
#include "output_stream.hpp"
#include "settings/settings.hpp"
#include "utility/command_line.hpp"
#include "utility/synchronous_task_queue.hpp"
#include "world/object_class.hpp"
#include "world/tool_visualizers.hpp"
#include "world/world.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>

#include <Windows.h>

#include <absl/container/flat_hash_map.h>

struct ImGuiContext;

namespace we {

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

class world_edit {
public:
   world_edit(const HWND window, utility::command_line command_line);

   bool update();

   void resized(uint16 width, uint16 height);

   void focused();

   void unfocused();

   bool idling() const noexcept;

   void dpi_changed(const int new_dpi) noexcept;

   void key_down(const key key) noexcept;

   void key_up(const key key) noexcept;

   void mouse_movement(const int32 x_movement, const int32 y_movement) noexcept;

private:
   void wait_for_swap_chain_ready() noexcept;

   void update_object_classes();

   void update_input() noexcept;

   void update_hovered_entity() noexcept;

   void update_camera(const float delta_time);

   void update_ui() noexcept;

   void garbage_collect_assets() noexcept;

   void select_hovered_entity() noexcept;

   void place_creation_entity() noexcept;

   void object_definition_loaded(const lowercase_string& name,
                                 asset_ref<assets::odf::definition> asset,
                                 asset_data<assets::odf::definition> data);

   void model_loaded(const lowercase_string& name,
                     asset_ref<assets::msh::flat_model> asset,
                     asset_data<assets::msh::flat_model> data);

   void open_project(std::filesystem::path path) noexcept;

   void open_project_with_picker() noexcept;

   void load_world(std::filesystem::path path) noexcept;

   void load_world_with_picker() noexcept;

   void save_world(std::filesystem::path path) noexcept;

   void save_world_with_picker() noexcept;

   void close_world() noexcept;

   void enumerate_project_worlds() noexcept;

   void initialize_commands() noexcept;

   void initialize_hotkeys() noexcept;

   void handle_gpu_error(graphics::gpu::exception& e) noexcept;

   standard_output_stream _stream;
   HWND _window{};
   settings::settings _settings;
   std::shared_ptr<async::thread_pool> _thread_pool = async::thread_pool::make();

   bool _focused = true;
   float _current_dpi = 96.0f;
   float _display_scale = 1.0f;
   std::chrono::steady_clock::time_point _last_update =
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
   absl::flat_hash_map<lowercase_string, world::object_class> _object_classes;
   world::world _world;
   world::interaction_targets _interaction_targets;
   world::active_entity_types _world_draw_mask;
   world::active_layers _world_layers_draw_mask{true};
   world::terrain_collision _terrain_collision;
   world::tool_visualizers _tool_visualizers;

   edits::stack<world::world> _edit_stack_world;

   std::unique_ptr<graphics::renderer> _renderer;
   graphics::camera _camera;

   bool _move_camera_forward = false;
   bool _move_camera_back = false;
   bool _move_camera_left = false;
   bool _move_camera_right = false;
   bool _move_camera_up = false;
   bool _move_camera_down = false;
   bool _rotate_camera = false;
   bool _hotkeys_show = false;

   POINT _rotate_camera_cursor_position = {0, 0};

   struct entity_creation_context {
      world::object_id last_object = world::max_id;
      world::light_id last_light = world::max_id;
      world::path_id last_path = world::max_id;
      world::region_id last_region = world::max_id;
      world::sector_id last_sector = world::max_id;
      world::portal_id last_portal = world::max_id;
      world::barrier_id last_barrier = world::max_id;
      world::planning_hub_id last_planning_hub = world::max_id;
      world::planning_connection_id last_planning_connection = world::max_id;
      world::boundary_id last_boundary = world::max_id;

      placement_rotation placement_rotation = placement_rotation::manual_euler;
      placement_mode placement_mode = placement_mode::cursor;
      placement_alignment placement_alignment = placement_alignment::none;
      placement_ground placement_ground = placement_ground::origin;
      placement_node_insert placement_node_insert = placement_node_insert::nearest;

      bool lock_x_axis = false;
      bool lock_y_axis = false;
      bool lock_z_axis = false;

      bool using_point_at = false;
      bool using_extend_to = false;
      bool using_shrink_to = false;
      bool using_from_object_bbox = false;

      bool finish_current_path = false;

      float alignment = 4.0f;
      float snap_distance = 0.5f;

      float3 rotation{0.0f, 0.0f, 0.0f};
      float3 light_region_rotation{0.0f, 0.0f, 0.0f};

      std::optional<float3> resize_start_size;
   } _entity_creation_context;

   float3 _cursor_positionWS = {0.0f, 0.0f, 0.0f};
   std::optional<float3> _cursor_surface_normalWS;

   utility::synchronous_task_queue _asset_load_queue;
   event_listener<void(const lowercase_string&, asset_ref<assets::odf::definition>,
                       asset_data<assets::odf::definition>)>
      _object_definition_load_listener = _asset_libraries.odfs.listen_for_loads(
         [this](lowercase_string name, asset_ref<assets::odf::definition> asset,
                asset_data<assets::odf::definition> data) {
            _asset_load_queue.enqueue(
               [=] { object_definition_loaded(name, asset, data); });
         });
   event_listener<void(const lowercase_string&, asset_ref<assets::msh::flat_model>,
                       asset_data<assets::msh::flat_model>)>
      _model_load_listener = _asset_libraries.models.listen_for_loads(
         [this](lowercase_string name, asset_ref<assets::msh::flat_model> asset,
                asset_data<assets::msh::flat_model> data) {
            _asset_load_queue.enqueue([=] { model_loaded(name, asset, data); });
         });

   commands _commands;
   hotkeys _hotkeys{_commands, _stream};
};

}
