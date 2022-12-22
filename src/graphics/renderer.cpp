
#include "renderer.hpp"
#include "assets/asset_libraries.hpp"
#include "async/thread_pool.hpp"
#include "camera.hpp"
#include "copy_command_list_pool.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "frustrum.hpp"
#include "geometric_shapes.hpp"
#include "gpu/barrier.hpp"
#include "gpu/rhi.hpp"
#include "imgui/imgui.h"
#include "imgui_renderer.hpp"
#include "light_clusters.hpp"
#include "line_drawer.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "model_manager.hpp"
#include "output_stream.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "sampler_list.hpp"
#include "settings/graphics.hpp"
#include "shader_library.hpp"
#include "shader_list.hpp"
#include "terrain.hpp"
#include "texture_manager.hpp"
#include "triangle_drawer.hpp"
#include "utility/look_for.hpp"
#include "utility/overload.hpp"
#include "world/world.hpp"
#include "world/world_utilities.hpp"

#include <range/v3/view.hpp>

namespace we::graphics {

namespace {

// TODO: Put this somewhere.
struct alignas(256) frame_constant_buffer {
   float4x4 view_projection_matrix;

   float3 view_positionWS;
   uint32 pad0 = 0;

   float2 viewport_size;
   float2 viewport_topleft;

   float line_width;
};

static_assert(sizeof(frame_constant_buffer) == 256);

// TODO: Put this somewhere.
struct alignas(256) wireframe_constant_buffer {
   float3 color;
};

static_assert(sizeof(wireframe_constant_buffer) == 256);

struct alignas(256) meta_outlined_constant_buffer {
   float4 color;
   float4 outline_color;
};

static_assert(sizeof(meta_outlined_constant_buffer) == 256);

// TODO: Put this somewhere.
const std::array<std::array<float3, 2>, 18> path_node_arrow_wireframe = [] {
   constexpr std::array<std::array<uint16, 2>, 18> arrow_indices{{{2, 1},
                                                                  {4, 3},
                                                                  {4, 5},
                                                                  {3, 5},
                                                                  {1, 5},
                                                                  {2, 5},
                                                                  {8, 6},
                                                                  {6, 7},
                                                                  {7, 9},
                                                                  {10, 12},
                                                                  {13, 11},
                                                                  {11, 10},
                                                                  {6, 10},
                                                                  {11, 7},
                                                                  {2, 9},
                                                                  {1, 8},
                                                                  {4, 13},
                                                                  {3, 12}}};

   constexpr std::array<float3, 15> arrow_vertices{
      {{0.0f, 0.0f, 0.0f},
       {0.611469f, -0.366881f, 0.085396f},
       {0.611469f, 0.366881f, 0.085396f},
       {-0.611469f, -0.366881f, 0.085396f},
       {-0.611469f, 0.366881f, 0.085396f},
       {0.000000f, 0.000000f, 1.002599f},
       {0.305735f, -0.366881f, -0.984675f},
       {0.305735f, 0.366881f, -0.984675f},
       {0.305735f, -0.366881f, 0.085396f},
       {0.305735f, 0.366881f, 0.085396f},
       {-0.305735f, -0.366881f, -0.984675f},
       {-0.305735f, 0.366881f, -0.984675f},
       {-0.305735f, -0.366881f, 0.085396f},
       {-0.305735f, 0.366881f, 0.085396f},
       {-0.305735f, 0.366881f, 0.085396f}}};

   std::array<std::array<float3, 2>, 18> arrow;

   for (std::size_t i = 0; i < arrow.size(); ++i) {
      arrow[i] = {arrow_vertices[arrow_indices[i][0]] * 0.25f + float3{0.0f, 0.0f, 0.8f},
                  arrow_vertices[arrow_indices[i][1]] * 0.25f +
                     float3{0.0f, 0.0f, 0.8f}};
   }

   return arrow;
}();
}

struct renderer_config {
   bool use_raytracing = false;
};

class renderer_impl {
public:
   renderer_impl(const renderer::window_handle window,
                 std::shared_ptr<settings::graphics> settings,
                 std::shared_ptr<async::thread_pool> thread_pool,
                 assets::libraries_manager& asset_libraries,
                 output_stream& error_output);

   void wait_for_swap_chain_ready();

   void draw_frame(const camera& camera, const world::world& world,
                   const world::interaction_targets& interaction_targets,
                   const world::active_entity_types active_entity_types,
                   const world::active_layers active_layers,
                   const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes);

   void window_resized(uint16 width, uint16 height);

   void mark_dirty_terrain() noexcept;

   void recreate_imgui_font_atlas()
   {
      _imgui_renderer.recreate_font_atlas(_copy_command_list_pool);
   }

   void reload_shaders() noexcept
   {
      _device.wait_for_idle();
      _shaders.reload(shader_list);
      _pipelines.reload(_device, _shaders, _root_signatures);
   }

private:
   struct render_list_item {
      float distance;
      gpu::pipeline_handle pipeline;
      gpu::pipeline_handle depth_prepass_pipeline;
      gpu::index_buffer_view index_buffer_view;
      std::array<gpu::vertex_buffer_view, 2> vertex_buffer_views;
      gpu_virtual_address object_constants_address;
      gpu_virtual_address material_cbv;
      uint32 index_count;
      uint32 start_index;
      uint32 start_vertex;
   };

   void update_frame_constant_buffer(const camera& camera,
                                     gpu::copy_command_list& command_list);

   void draw_world(const frustrum& view_frustrum,
                   gpu::graphics_command_list& command_list);

   void draw_world_render_list_depth_prepass(const std::vector<render_list_item>& list,
                                             gpu::graphics_command_list& command_list);

   void draw_world_render_list(const std::vector<render_list_item>& list,
                               gpu::graphics_command_list& command_list);

   void draw_world_meta_objects(const frustrum& view_frustrum, const world::world& world,
                                const world::active_entity_types active_entity_types,
                                const world::active_layers active_layers,
                                gpu::graphics_command_list& command_list);

   void draw_interaction_targets(
      const frustrum& view_frustrum, const world::world& world,
      const world::interaction_targets& interaction_targets,
      const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes,
      gpu::graphics_command_list& command_list);

   void build_world_mesh_list(
      gpu::copy_command_list& command_list, const world::world& world,
      const world::active_layers active_layers,
      const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes);

   void build_object_render_list(const frustrum& view_frustrum);

   void update_textures(gpu::copy_command_list& command_list);

   bool _terrain_dirty = true; // ughhhh, this feels so ugly

   std::shared_ptr<async::thread_pool> _thread_pool;
   output_stream& _error_output;

   gpu::device _device{gpu::device_desc{.enable_debug_layer = false}};
   gpu::swap_chain _swap_chain;
   gpu::copy_command_list _pre_render_command_list = _device.create_copy_command_list(
      {.allocator_name = "World Allocator", .debug_name = "Pre-Render Copy Command List"});
   gpu::graphics_command_list _world_command_list = _device.create_graphics_command_list(
      {.allocator_name = "World Allocator", .debug_name = "World Command List"});

   dynamic_buffer_allocator _dynamic_buffer_allocator{1024 * 1024 * 4, _device};
   copy_command_list_pool _copy_command_list_pool{_device};

   gpu::unique_resource_handle _camera_constant_buffer =
      {_device.create_buffer({.size = sizeof(frame_constant_buffer),
                              .debug_name = "Frame Constant Buffer"},
                             gpu::heap_type::default_),
       _device.direct_queue};
   gpu_virtual_address _camera_constant_buffer_view =
      _device.get_gpu_virtual_address(_camera_constant_buffer.get());

   gpu::unique_resource_handle _depth_stencil_texture =
      {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                               .flags = {.allow_depth_stencil = true},
                               .format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                               .width = _swap_chain.width(),
                               .height = _swap_chain.height(),
                               .optimized_clear_value =
                                  {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                   .depth_stencil = {.depth = 1.0f, .stencil = 0x0}}},
                              gpu::resource_state::depth_write),
       _device.direct_queue};
   gpu::unique_dsv_handle _depth_stencil_view =
      {_device.create_depth_stencil_view(_depth_stencil_texture.get(),
                                         {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                          .dimension = gpu::dsv_dimension::texture2d}),
       _device.direct_queue};

   gpu::unique_sampler_heap_handle _sampler_heap = {_device.create_sampler_heap(
                                                       sampler_descriptions()),
                                                    _device.direct_queue};

   shader_library _shaders{shader_list, _thread_pool};
   root_signature_library _root_signatures{_device};
   pipeline_library _pipelines{_device, _shaders, _root_signatures};

   texture_manager _texture_manager;
   model_manager _model_manager;
   geometric_shapes _geometric_shapes{_device, _copy_command_list_pool};
   light_clusters _light_clusters{_device, _copy_command_list_pool,
                                  _swap_chain.width(), _swap_chain.height()};
   terrain _terrain{_device, _texture_manager};

   constexpr static std::size_t max_drawn_objects = 2048;
   constexpr static std::size_t objects_constants_buffer_size =
      max_drawn_objects * sizeof(world_mesh_constants);

   std::array<gpu::unique_resource_handle, gpu::frame_pipeline_length> _object_constants_upload_buffers;
   std::array<std::byte*, gpu::frame_pipeline_length> _object_constants_upload_cpu_ptrs;
   gpu::unique_resource_handle _object_constants_buffer =
      {_device.create_buffer({.size = objects_constants_buffer_size,
                              .debug_name = "Object Constant Buffers"},
                             gpu::heap_type::default_),
       _device.direct_queue};

   world_mesh_list _world_mesh_list;
   std::vector<render_list_item> _opaque_object_render_list;
   std::vector<render_list_item> _transparent_object_render_list;

   imgui_renderer _imgui_renderer{_device, _copy_command_list_pool};

   renderer_config _config;
   std::shared_ptr<settings::graphics> _settings;
};

renderer_impl::renderer_impl(const renderer::window_handle window,
                             std::shared_ptr<settings::graphics> settings,
                             std::shared_ptr<async::thread_pool> thread_pool,
                             assets::libraries_manager& asset_libraries,
                             output_stream& error_output)
   : _thread_pool{thread_pool},
     _error_output{error_output},
     _swap_chain{_device.create_swap_chain({.window = window})},
     _texture_manager{_device, _copy_command_list_pool, thread_pool,
                      asset_libraries.textures},
     _model_manager{_device,          _copy_command_list_pool,
                    _texture_manager, asset_libraries.models,
                    thread_pool,      _error_output},
     _settings{settings}
{
   // create object constants upload buffers
   for (auto [buffer, cpu_ptr] :
        ranges::views::zip(_object_constants_upload_buffers,
                           _object_constants_upload_cpu_ptrs)) {
      buffer = {_device.create_buffer({.size = objects_constants_buffer_size,
                                       .debug_name =
                                          "Object Constant Upload Buffers"},
                                      gpu::heap_type::upload),
                _device.direct_queue};

      cpu_ptr = static_cast<std::byte*>(_device.map(buffer.get(), 0, {}));
   }

   // Sync with background uploads being done to initialize resources.
   _device.direct_queue.sync_with(_device.background_copy_queue);
}

void renderer_impl::wait_for_swap_chain_ready()
{
   _swap_chain.wait_for_ready();
}

void renderer_impl::draw_frame(
   const camera& camera, const world::world& world,
   const world::interaction_targets& interaction_targets,
   const world::active_entity_types active_entity_types,
   const world::active_layers active_layers,
   const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes)
{
   const frustrum view_frustrum{camera.inv_view_projection_matrix()};

   auto [back_buffer, back_buffer_rtv] = _swap_chain.current_back_buffer();
   _dynamic_buffer_allocator.reset(_device.frame_index());

   _model_manager.update_models();

   // Pre-Render Work
   {
      _pre_render_command_list.reset();

      if (std::exchange(_terrain_dirty, false)) {
         _terrain.init(world.terrain, _pre_render_command_list,
                       _dynamic_buffer_allocator);
      }

      update_textures(_pre_render_command_list);
      build_world_mesh_list(_pre_render_command_list, world, active_layers,
                            world_classes);
      update_frame_constant_buffer(camera, _pre_render_command_list);

      _light_clusters.prepare_lights(camera, view_frustrum, world,
                                     _pre_render_command_list,
                                     _dynamic_buffer_allocator);

      _pre_render_command_list.close();

      _device.copy_queue.execute_command_lists(_pre_render_command_list);
      _device.direct_queue.sync_with(_device.copy_queue);
   }

   build_object_render_list(view_frustrum);

   auto& command_list = _world_command_list;

   command_list.reset(_sampler_heap.get());

   _light_clusters.tile_lights(_root_signatures, _pipelines, command_list,
                               _dynamic_buffer_allocator);
   _light_clusters.draw_shadow_maps(_world_mesh_list, _root_signatures, _pipelines,
                                    command_list, _dynamic_buffer_allocator);

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(back_buffer, gpu::resource_state::present,
                              gpu::resource_state::render_target));
   command_list.flush_resource_barriers();

   command_list.clear_render_target_view(back_buffer_rtv,
                                         float4{0.0f, 0.0f, 0.0f, 1.0f});
   command_list.clear_depth_stencil_view(_depth_stencil_view.get(),
                                         {.clear_depth = true}, 1.0f, 0x0);

   command_list.rs_set_viewports(
      gpu::viewport{.width = static_cast<float>(_swap_chain.width()),
                    .height = static_cast<float>(_swap_chain.height())});
   command_list.rs_set_scissor_rects(
      {.right = _swap_chain.width(), .bottom = _swap_chain.height()});
   command_list.om_set_render_targets(back_buffer_rtv, _depth_stencil_view.get());

   // Render World
   if (active_entity_types.objects) draw_world(view_frustrum, command_list);

   // Render World Meta Objects
   draw_world_meta_objects(view_frustrum, world, active_entity_types,
                           active_layers, command_list);

   draw_interaction_targets(view_frustrum, world, interaction_targets,
                            world_classes, command_list);

   // Render ImGui
   ImGui::Render();
   _imgui_renderer.render_draw_data(ImGui::GetDrawData(), _root_signatures,
                                    _pipelines, command_list);

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(back_buffer, gpu::resource_state::render_target,
                              gpu::resource_state::present));
   command_list.flush_resource_barriers();

   command_list.close();

   _device.direct_queue.execute_command_lists(command_list);

   _swap_chain.present(false);

   _device.end_frame();
   _model_manager.trim_models();
}

void renderer_impl::window_resized(uint16 width, uint16 height)
{
   if (width == _swap_chain.width() and height == _swap_chain.height()) {
      return;
   }

   _swap_chain.resize(width, height);
   _depth_stencil_texture =
      {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                               .flags = {.allow_depth_stencil = true},
                               .format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                               .width = _swap_chain.width(),
                               .height = _swap_chain.height(),
                               .optimized_clear_value =
                                  {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                   .depth_stencil = {.depth = 1.0f, .stencil = 0x0}}},
                              gpu::resource_state::depth_write),
       _device.direct_queue};
   _depth_stencil_view =
      {_device.create_depth_stencil_view(_depth_stencil_texture.get(),
                                         {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                          .dimension = gpu::dsv_dimension::texture2d}),
       _device.direct_queue};

   _light_clusters.update_render_resolution(width, height);
}

void renderer_impl::mark_dirty_terrain() noexcept
{
   _terrain_dirty = true;
}

void renderer_impl::update_frame_constant_buffer(const camera& camera,
                                                 gpu::copy_command_list& command_list)
{
   frame_constant_buffer
      constants{.view_projection_matrix = camera.view_projection_matrix(),

                .view_positionWS = camera.position(),

                .viewport_size = {static_cast<float>(_swap_chain.width()),
                                  static_cast<float>(_swap_chain.height())},
                .viewport_topleft = {0.0f, 0.0f},

                .line_width = _settings->line_width()};

   auto allocation = _dynamic_buffer_allocator.allocate_and_copy(constants);

   command_list.copy_buffer_region(_camera_constant_buffer.get(), 0,
                                   _dynamic_buffer_allocator.resource(),
                                   allocation.offset, sizeof(frame_constant_buffer));
}

void renderer_impl::draw_world(const frustrum& view_frustrum,
                               gpu::graphics_command_list& command_list)
{
   draw_world_render_list_depth_prepass(_opaque_object_render_list, command_list);

   _terrain.draw(terrain_draw::depth_prepass, view_frustrum, _camera_constant_buffer_view,
                 _light_clusters.lights_constant_buffer_view(), command_list,
                 _root_signatures, _pipelines, _dynamic_buffer_allocator);

   draw_world_render_list(_opaque_object_render_list, command_list);

   _terrain.draw(terrain_draw::main, view_frustrum, _camera_constant_buffer_view,
                 _light_clusters.lights_constant_buffer_view(), command_list,
                 _root_signatures, _pipelines, _dynamic_buffer_allocator);

   draw_world_render_list(_transparent_object_render_list, command_list);
}

void renderer_impl::draw_world_render_list(const std::vector<render_list_item>& list,
                                           gpu::graphics_command_list& command_list)
{
   command_list.set_graphics_root_signature(_root_signatures.mesh.get());
   command_list.set_graphics_cbv(rs::mesh::frame_cbv, _camera_constant_buffer_view);
   command_list.set_graphics_cbv(rs::mesh::lights_cbv,
                                 _light_clusters.lights_constant_buffer_view());
   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

   gpu::pipeline_handle pipeline_state = gpu::null_pipeline_handle;

   for (auto& object : list) {
      if (pipeline_state != object.pipeline) {
         command_list.set_pipeline_state(object.pipeline);
      }

      command_list.set_graphics_cbv(rs::mesh::object_cbv,
                                    object.object_constants_address);
      command_list.set_graphics_cbv(rs::mesh::material_cbv, object.material_cbv);
      command_list.ia_set_vertex_buffers(0, object.vertex_buffer_views);
      command_list.ia_set_index_buffer(object.index_buffer_view);
      command_list.draw_indexed_instanced(object.index_count, 1, object.start_index,
                                          object.start_vertex, 0);
   }
}

void renderer_impl::draw_world_render_list_depth_prepass(
   const std::vector<render_list_item>& list, gpu::graphics_command_list& command_list)
{
   command_list.set_graphics_root_signature(_root_signatures.mesh_depth_prepass.get());
   command_list.set_graphics_cbv(rs::mesh_depth_prepass::frame_cbv,
                                 _camera_constant_buffer_view);
   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

   gpu::pipeline_handle pipeline_state = gpu::null_pipeline_handle;

   for (auto& object : list) {
      if (pipeline_state != object.depth_prepass_pipeline) {
         command_list.set_pipeline_state(object.depth_prepass_pipeline);
      }

      command_list.set_graphics_cbv(rs::mesh_depth_prepass::object_cbv,
                                    object.object_constants_address);
      command_list.set_graphics_cbv(rs::mesh_depth_prepass::material_cbv,
                                    object.material_cbv);
      command_list.ia_set_vertex_buffers(0, object.vertex_buffer_views);
      command_list.ia_set_index_buffer(object.index_buffer_view);
      command_list.draw_indexed_instanced(object.index_count, 1, object.start_index,
                                          object.start_vertex, 0);
   }
}

void renderer_impl::draw_world_meta_objects(
   const frustrum& view_frustrum, const world::world& world,
   const world::active_entity_types active_entity_types,
   const world::active_layers active_layers, gpu::graphics_command_list& command_list)
{
   (void)view_frustrum; // TODO: Frustrum Culling (Is it worth it for meta objects?)

   if (active_entity_types.paths and not world.paths.empty()) {
      static bool draw_nodes = true;
      static bool draw_connections = true;
      static bool draw_orientation = false;

      // TODO: Move theses to _settings
      ImGui::Indent();
      ImGui::Checkbox("Draw Paths Nodes", &draw_nodes);
      ImGui::Checkbox("Draw Paths Connections", &draw_connections);
      ImGui::Checkbox("Draw Paths Orientation", &draw_orientation);
      ImGui::Unindent();

      if (draw_nodes) {
         command_list.set_graphics_root_signature(
            _root_signatures.meta_mesh_wireframe.get());
         command_list.set_graphics_cbv(rs::meta_mesh_wireframe::frame_cbv,
                                       _camera_constant_buffer_view);
         command_list.set_graphics_cbv(
            rs::meta_mesh_wireframe::wireframe_cbv,
            _dynamic_buffer_allocator
               .allocate_and_copy(meta_outlined_constant_buffer{
                  .color = float4{_settings->path_node_color(), 1.0f},
                  .outline_color = float4{_settings->path_node_outline_color(), 1.0f}})
               .gpu_address);

         command_list.set_pipeline_state(_pipelines.meta_mesh_outlined.get());
         command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

         for (auto& path : world.paths) {
            if (not active_layers[path.layer]) continue;

            for (auto& node : path.nodes) {
               // TEMP constants setup
               {
                  float4x4 transform = to_matrix(node.rotation) *
                                       float4x4{{0.5f, 0.0f, 0.0f, 0.0f},
                                                {0.0f, 0.5f, 0.0f, 0.0f},
                                                {0.0f, 0.0f, 0.5f, 0.0f},
                                                {0.0f, 0.0f, 0.0f, 1.0f}};

                  transform[3] = {node.position, 1.0f};

                  command_list.set_graphics_cbv(rs::meta_mesh::object_cbv,
                                                _dynamic_buffer_allocator
                                                   .allocate_and_copy(transform)
                                                   .gpu_address);
               }

               const geometric_shape shape = _geometric_shapes.octahedron();

               command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
               command_list.ia_set_index_buffer(shape.index_buffer_view);
               command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
            }
         }
      }

      if (draw_connections) {
         draw_lines(command_list, _root_signatures, _pipelines, _dynamic_buffer_allocator,
                    {.line_color = _settings->path_node_connection_color(),

                     .camera_constant_buffer_view = _camera_constant_buffer_view,

                     .connect_mode = line_connect_mode::linear},
                    [&](line_draw_context& draw_context) {
                       for (auto& path : world.paths) {
                          if (not active_layers[path.layer]) continue;

                          using namespace ranges::views;

                          const auto get_position = [](const world::path::node& node) {
                             return node.position;
                          };

                          for (const auto [a, b] :
                               zip(path.nodes | transform(get_position),
                                   path.nodes | drop(1) | transform(get_position))) {
                             draw_context.add(a, b);
                          }
                       }
                    });
      }

      if (draw_orientation) {
         draw_lines(command_list, _root_signatures, _pipelines, _dynamic_buffer_allocator,
                    {.line_color = _settings->path_node_orientation_color(),

                     .camera_constant_buffer_view = _camera_constant_buffer_view,

                     .connect_mode = line_connect_mode::linear},
                    [&](line_draw_context& draw_context) {
                       for (auto& path : world.paths) {
                          using namespace ranges::views;

                          const auto get_position = [](const world::path::node& node) {
                             return node.position;
                          };

                          for (auto& node : path.nodes) {
                             if (not active_layers[path.layer]) continue;

                             float4x4 transform = to_matrix(node.rotation);

                             transform[3] = {node.position, 1.0f};

                             for (const auto line : path_node_arrow_wireframe) {
                                const float3 a = transform * line[0];
                                const float3 b = transform * line[1];

                                draw_context.add(a, b);
                             }
                          }
                       }
                    });
      }
   }

   // Draws a region, requires camera CBV, colour CBV, IA topplogy, pipeline state and root signature to be set.
   //
   // Shared between light volume drawing and region drawing.
   //
   // Should probably be moved to a proper function once draw_world_meta_objects
   // isn't just for debugging and get's the love it deserves.
   const auto draw_region = [&](const world::region& region) {
      // TEMP constants setup
      {
         const float3 scale = [&] {
            switch (region.shape) {
            default:
            case world::region_shape::box: {
               return region.size;
            }
            case world::region_shape::sphere: {
               const float sphere_radius = length(region.size);

               return float3{sphere_radius, sphere_radius, sphere_radius};
            }
            case world::region_shape::cylinder: {
               const float cylinder_length =
                  length(float2{region.size.x, region.size.z});
               return float3{cylinder_length, region.size.y, cylinder_length};
            }
            }
         }();

         float4x4 transform =
            to_matrix(region.rotation) * float4x4{{scale.x, 0.0f, 0.0f, 0.0f},
                                                  {0.0f, scale.y, 0.0f, 0.0f},
                                                  {0.0f, 0.0f, scale.z, 0.0f},
                                                  {0.0f, 0.0f, 0.0f, 1.0f}};

         transform[3] = {region.position, 1.0f};

         command_list.set_graphics_cbv(
            rs::meta_mesh::object_cbv,
            _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);
      }

      const geometric_shape shape = [&] {
         switch (region.shape) {
         default:
         case world::region_shape::box:
            return _geometric_shapes.cube();
         case world::region_shape::sphere:
            return _geometric_shapes.icosphere();
         case world::region_shape::cylinder:
            return _geometric_shapes.cylinder();
         }
      }();

      command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
      command_list.ia_set_index_buffer(shape.index_buffer_view);
      command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
   };

   if (active_entity_types.regions and not world.regions.empty()) {
      command_list.set_pipeline_state(_pipelines.meta_mesh.get());
      command_list.set_graphics_root_signature(_root_signatures.meta_mesh.get());

      command_list.set_graphics_cbv(rs::meta_mesh::frame_cbv,
                                    _camera_constant_buffer_view);
      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      command_list.set_graphics_cbv(rs::meta_mesh::color_cbv,
                                    _dynamic_buffer_allocator
                                       .allocate_and_copy(_settings->region_color())
                                       .gpu_address);

      for (auto& region : world.regions) {
         if (not active_layers[region.layer]) continue;

         draw_region(region);
      }
   }

   if (active_entity_types.barriers and not world.barriers.empty()) {
      command_list.set_pipeline_state(_pipelines.meta_mesh.get());
      command_list.set_graphics_root_signature(_root_signatures.meta_mesh.get());

      command_list.set_graphics_cbv(rs::meta_mesh::frame_cbv,
                                    _camera_constant_buffer_view);
      command_list.set_graphics_cbv(rs::meta_mesh::color_cbv,
                                    _dynamic_buffer_allocator
                                       .allocate_and_copy(_settings->barrier_color())
                                       .gpu_address);

      const geometric_shape shape = _geometric_shapes.cube();

      command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
      command_list.ia_set_index_buffer(shape.index_buffer_view);
      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      const float barrier_height = _settings->barrier_height();

      for (auto& barrier : world.barriers) {
         // TEMP constants setup
         {
            const float2 position = (barrier.corners[0] + barrier.corners[2]) / 2.0f;
            const float2 size{distance(barrier.corners[0], barrier.corners[3]),
                              distance(barrier.corners[0], barrier.corners[1])};
            const float angle =
               std::atan2(barrier.corners[1].x - barrier.corners[0].x,
                          barrier.corners[1].y - barrier.corners[0].y);

            float4x4 transform =
               make_rotation_matrix_from_euler({0.0f, angle, 0.0f}) *
               float4x4{{size.x / 2.0f, 0.0f, 0.0f, 0.0f},
                        {0.0f, barrier_height, 0.0f, 0.0f},
                        {0.0f, 0.0f, size.y / 2.0f, 0.0f},
                        {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {position.x, 0.0f, position.y, 1.0f};

            command_list.set_graphics_cbv(
               rs::meta_mesh::object_cbv,
               _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);
         }

         command_list.draw_indexed_instanced(_geometric_shapes.cube().index_count,
                                             1, 0, 0, 0);
      }
   }

   if (active_entity_types.lights and not world.lights.empty()) {
      command_list.set_pipeline_state(_pipelines.meta_mesh.get());
      command_list.set_graphics_root_signature(_root_signatures.meta_mesh.get());

      command_list.set_graphics_cbv(rs::meta_mesh::frame_cbv,
                                    _camera_constant_buffer_view);
      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      const float volume_alpha = _settings->light_volume_alpha();

      for (auto& light : world.lights) {
         if (not active_layers[light.layer]) continue;

         // Set Color
         {
            const float4 color{light.color, light.light_type == world::light_type::spot
                                               ? volume_alpha * 0.5f
                                               : volume_alpha};

            command_list.set_graphics_cbv(
               rs::meta_mesh::color_cbv,
               _dynamic_buffer_allocator.allocate_and_copy(color).gpu_address);
         }

         switch (light.light_type) {
         case world::light_type::directional: {
            if (light.directional_region.empty()) break;

            if (const world::region* const region =
                   world::find_region_by_description(world, light.directional_region);
                region) {
               draw_region(*region);
            }

            break;
         }
         case world::light_type::point: {
            // TEMP constants setup
            {
               float4x4 transform = float4x4{{light.range, 0.0f, 0.0f, 0.0f},
                                             {0.0f, light.range, 0.0f, 0.0f},
                                             {0.0f, 0.0f, light.range, 0.0f},
                                             {0.0f, 0.0f, 0.0f, 1.0f}};

               transform[3] = {light.position, 1.0f};

               command_list.set_graphics_cbv(rs::meta_mesh::object_cbv,
                                             _dynamic_buffer_allocator
                                                .allocate_and_copy(transform)
                                                .gpu_address);
            }

            auto shape = _geometric_shapes.icosphere();

            command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
            command_list.ia_set_index_buffer(shape.index_buffer_view);
            command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);

            break;
         }
         case world::light_type::spot: {
            // TEMP constants setup
            const auto bind_cone_transform = [&](const float angle) {
               const float half_range = light.range / 2.0f;
               const float cone_radius = half_range * std::tan(angle);

               float4x4 transform =
                  to_matrix(light.rotation) *
                  to_matrix(quaternion{0.707107f, -0.707107f, 0.0f, 0.0f}) *
                  float4x4{{cone_radius, 0.0f, 0.0f, 0.0f},
                           {0.0f, half_range, 0.0f, 0.0f},
                           {0.0f, 0.0f, cone_radius, 0.0f},
                           {0.0f, -half_range, 0.0f, 1.0f}};

               transform[3] += float4{light.position, 0.0f};

               command_list.set_graphics_cbv(rs::meta_mesh::object_cbv,
                                             _dynamic_buffer_allocator
                                                .allocate_and_copy(transform)
                                                .gpu_address);
            };

            bind_cone_transform(light.outer_cone_angle);

            auto shape = _geometric_shapes.cone();

            command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
            command_list.ia_set_index_buffer(shape.index_buffer_view);
            command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);

            bind_cone_transform(light.inner_cone_angle);

            command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);

            break;
         }
         }
      }
   }

   triangle_drawer triangle_drawer{command_list, _dynamic_buffer_allocator, 1024};

   if (active_entity_types.sectors and not world.sectors.empty()) {
      command_list.set_pipeline_state(_pipelines.meta_mesh.get());
      command_list.set_graphics_root_signature(_root_signatures.meta_mesh.get());

      command_list.set_graphics_cbv(rs::meta_mesh::frame_cbv,
                                    _camera_constant_buffer_view);
      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      command_list.set_graphics_cbv(rs::meta_mesh::object_cbv,
                                    _dynamic_buffer_allocator
                                       .allocate_and_copy(
                                          float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                   {0.0f, 1.0f, 0.0f, 0.0f},
                                                   {0.0f, 0.0f, 1.0f, 0.0f},
                                                   {0.0f, 0.0f, 0.0f, 1.0f}})
                                       .gpu_address);
      command_list.set_graphics_cbv(rs::meta_mesh::color_cbv,
                                    _dynamic_buffer_allocator
                                       .allocate_and_copy(_settings->sector_color())
                                       .gpu_address);

      for (auto& sector : world.sectors) {
         using namespace ranges::views;

         for (const auto [a, b] :
              zip(sector.points,
                  concat(sector.points | drop(1), sector.points | take(1)))) {
            const std::array quad = {float3{a.x, sector.base, a.y},
                                     float3{b.x, sector.base, b.y},
                                     float3{a.x, sector.base + sector.height, a.y},
                                     float3{b.x, sector.base + sector.height, b.y}};

            triangle_drawer.add(quad[0], quad[1], quad[2]);
            triangle_drawer.add(quad[2], quad[1], quad[3]);
            triangle_drawer.add(quad[0], quad[2], quad[1]);
            triangle_drawer.add(quad[2], quad[3], quad[1]);
         }
      }

      triangle_drawer.submit();
   }

   if (active_entity_types.portals and not world.portals.empty()) {
      command_list.set_pipeline_state(_pipelines.meta_mesh.get());
      command_list.set_graphics_root_signature(_root_signatures.meta_mesh.get());

      command_list.set_graphics_cbv(rs::meta_mesh::frame_cbv,
                                    _camera_constant_buffer_view);
      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      command_list.set_graphics_cbv(rs::meta_mesh::object_cbv,
                                    _dynamic_buffer_allocator
                                       .allocate_and_copy(
                                          float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                   {0.0f, 1.0f, 0.0f, 0.0f},
                                                   {0.0f, 0.0f, 1.0f, 0.0f},
                                                   {0.0f, 0.0f, 0.0f, 1.0f}})
                                       .gpu_address);
      command_list.set_graphics_cbv(rs::meta_mesh::color_cbv,
                                    _dynamic_buffer_allocator
                                       .allocate_and_copy(_settings->portal_color())
                                       .gpu_address);

      for (auto& portal : world.portals) {
         using namespace ranges::views;

         const float half_width = portal.width * 0.5f;
         const float half_height = portal.height * 0.5f;

         std::array quad = {float3{-half_width, -half_height, 0.0f},
                            float3{half_width, -half_height, 0.0f},
                            float3{-half_width, half_height, 0.0f},
                            float3{half_width, half_height, 0.0f}};

         for (auto& v : quad) {
            v = portal.rotation * v;
            v += portal.position;
         }

         triangle_drawer.add(quad[0], quad[1], quad[2]);
         triangle_drawer.add(quad[2], quad[1], quad[3]);
         triangle_drawer.add(quad[0], quad[2], quad[1]);
         triangle_drawer.add(quad[2], quad[3], quad[1]);
      }

      triangle_drawer.submit();
   }

   if (active_entity_types.hintnodes and not world.hintnodes.empty()) {
      command_list.set_graphics_root_signature(
         _root_signatures.meta_mesh_wireframe.get());

      command_list.set_graphics_cbv(rs::meta_mesh::frame_cbv,
                                    _camera_constant_buffer_view);
      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      command_list.set_pipeline_state(_pipelines.meta_mesh_outlined.get());

      command_list.set_graphics_cbv(
         rs::meta_mesh::color_cbv,
         _dynamic_buffer_allocator
            .allocate_and_copy(meta_outlined_constant_buffer{
               .color = float4{_settings->hintnode_color(), 1.0f},
               .outline_color = float4{_settings->hintnode_outline_color(), 1.0f}})
            .gpu_address);

      for (auto& hintnode : world.hintnodes) {
         if (not active_layers[hintnode.layer]) continue;

         // TEMP constants setup
         {
            float4x4 transform = to_matrix(hintnode.rotation);

            transform[3] = {hintnode.position, 1.0f};

            command_list.set_graphics_cbv(
               rs::meta_mesh::object_cbv,
               _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);
         }

         const geometric_shape shape = _geometric_shapes.octahedron();

         command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
         command_list.ia_set_index_buffer(shape.index_buffer_view);
         command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
      }
   }

   if (active_entity_types.boundaries and not world.boundaries.empty()) {
      command_list.set_pipeline_state(_pipelines.meta_mesh.get());
      command_list.set_graphics_root_signature(_root_signatures.meta_mesh.get());

      command_list.set_graphics_cbv(rs::meta_mesh::frame_cbv,
                                    _camera_constant_buffer_view);
      command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

      const float boundary_height = _settings->boundary_height();

      command_list.set_graphics_cbv(rs::meta_mesh::object_cbv,
                                    _dynamic_buffer_allocator
                                       .allocate_and_copy(
                                          float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                   {0.0f, 1.0f, 0.0f, 0.0f},
                                                   {0.0f, 0.0f, 1.0f, 0.0f},
                                                   {0.0f, 0.0f, 0.0f, 1.0f}})
                                       .gpu_address);
      command_list.set_graphics_cbv(rs::meta_mesh::color_cbv,
                                    _dynamic_buffer_allocator
                                       .allocate_and_copy(_settings->boundary_color())
                                       .gpu_address);

      for (auto& boundary : world.boundaries) {
         using namespace ranges::views;

         const world::path* path = look_for(world.paths, [&](const world::path& path) {
            return path.name == boundary.name;
         });

         if (not path) continue;

         for (const auto [a, b] :
              zip(path->nodes, concat(path->nodes | drop(1), path->nodes | take(1)))) {

            const std::array quad = {a.position, b.position,
                                     a.position + float3{0.0f, boundary_height, 0.0f},
                                     b.position + float3{0.0f, boundary_height, 0.0f}};

            triangle_drawer.add(quad[0], quad[1], quad[2]);
            triangle_drawer.add(quad[2], quad[1], quad[3]);
            triangle_drawer.add(quad[0], quad[2], quad[1]);
            triangle_drawer.add(quad[2], quad[3], quad[1]);
         }
      }

      triangle_drawer.submit();
   }
}

void renderer_impl::draw_interaction_targets(
   const frustrum& view_frustrum, const world::world& world,
   const world::interaction_targets& interaction_targets,
   const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes,
   gpu::graphics_command_list& command_list)
{
   (void)view_frustrum; // TODO: Frustrum Culling (Is it worth it for interaction targets?)

   triangle_drawer triangle_drawer{command_list, _dynamic_buffer_allocator, 1024};

   const auto draw_target = [&](world::interaction_target target,
                                gpu_virtual_address wireframe_constants) {
      const auto meta_mesh_common_setup = [&] {
         command_list.set_graphics_root_signature(
            _root_signatures.mesh_wireframe.get());
         command_list.set_graphics_cbv(rs::meta_mesh_wireframe::wireframe_cbv,
                                       wireframe_constants);
         command_list.set_graphics_cbv(rs::meta_mesh_wireframe::frame_cbv,
                                       _camera_constant_buffer_view);

         command_list.set_pipeline_state(_pipelines.meta_mesh_wireframe.get());

         command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);
      };

      const auto draw_path_node = [&](const world::path::node& node) {
         float4x4 transform =
            to_matrix(node.rotation) * float4x4{{0.5f, 0.0f, 0.0f, 0.0f},
                                                {0.0f, 0.5f, 0.0f, 0.0f},
                                                {0.0f, 0.0f, 0.5f, 0.0f},
                                                {0.0f, 0.0f, 0.0f, 1.0f}};

         transform[3] = {node.position, 1.0f};

         command_list.set_graphics_cbv(
            rs::meta_mesh_wireframe::object_cbv,
            _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

         const geometric_shape shape = _geometric_shapes.octahedron();

         command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
         command_list.ia_set_index_buffer(shape.index_buffer_view);
         command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
      };

      std::visit(
         overload{
            [&](world::object_id id) {
               const world::object* object =
                  look_for(world.objects, [id](const world::object& object) {
                     return id == object.id;
                  });

               if (not object) return;

               gpu_virtual_address object_constants = [&] {
                  auto allocation =
                     _dynamic_buffer_allocator.allocate(sizeof(world_mesh_constants));

                  world_mesh_constants constants{};

                  constants.object_to_world = to_matrix(object->rotation);
                  constants.object_to_world[3] = float4{object->position, 1.0f};

                  std::memcpy(allocation.cpu_address, &constants,
                              sizeof(world_mesh_constants));

                  return allocation.gpu_address;
               }();

               model& model =
                  _model_manager[world_classes.at(object->class_name).model_name];

               command_list.set_graphics_root_signature(
                  _root_signatures.mesh_wireframe.get());
               command_list.set_graphics_cbv(rs::mesh_wireframe::object_cbv,
                                             object_constants);
               command_list.set_graphics_cbv(rs::mesh_wireframe::wireframe_cbv,
                                             wireframe_constants);
               command_list.set_graphics_cbv(rs::mesh_wireframe::frame_cbv,
                                             _camera_constant_buffer_view);

               command_list.set_pipeline_state(_pipelines.mesh_wireframe.get());

               command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

               command_list.ia_set_index_buffer(model.gpu_buffer.index_buffer_view);
               command_list.ia_set_vertex_buffers(0, model.gpu_buffer.position_vertex_buffer_view);

               for (auto& part : model.parts) {
                  command_list.draw_indexed_instanced(part.index_count, 1,
                                                      part.start_index,
                                                      part.start_vertex, 0);
               }
            },
            [&](world::light_id id) {
               const world::light* light =
                  look_for(world.lights, [id](const world::light& light) {
                     return id == light.id;
                  });

               if (not light) return;

               meta_mesh_common_setup();

               if (light->light_type == world::light_type::directional) {
                  if (light->directional_region.empty()) {
                     return; // TODO: Directional light visualizers.
                  }

                  const world::region* const region =
                     world::find_region_by_description(world, light->directional_region);

                  if (not region) return;

                  const float3 scale = [&] {
                     switch (region->shape) {
                     default:
                     case world::region_shape::box: {
                        return region->size;
                     }
                     case world::region_shape::sphere: {
                        const float sphere_radius = length(region->size);

                        return float3{sphere_radius, sphere_radius, sphere_radius};
                     }
                     case world::region_shape::cylinder: {
                        const float cylinder_length =
                           length(float2{region->size.x, region->size.z});
                        return float3{cylinder_length, region->size.y, cylinder_length};
                     }
                     }
                  }();

                  float4x4 transform = to_matrix(region->rotation) *
                                       float4x4{{scale.x, 0.0f, 0.0f, 0.0f},
                                                {0.0f, scale.y, 0.0f, 0.0f},
                                                {0.0f, 0.0f, scale.z, 0.0f},
                                                {0.0f, 0.0f, 0.0f, 1.0f}};

                  transform[3] = {region->position, 1.0f};

                  command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                                _dynamic_buffer_allocator
                                                   .allocate_and_copy(transform)
                                                   .gpu_address);

                  const geometric_shape shape = [&] {
                     switch (region->shape) {
                     default:
                     case world::region_shape::box:
                        return _geometric_shapes.cube();
                     case world::region_shape::sphere:
                        return _geometric_shapes.icosphere();
                     case world::region_shape::cylinder:
                        return _geometric_shapes.cylinder();
                     }
                  }();

                  command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
                  command_list.ia_set_index_buffer(shape.index_buffer_view);
                  command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
               }
               else if (light->light_type == world::light_type::point) {
                  float4x4 transform = float4x4{{light->range, 0.0f, 0.0f, 0.0f},
                                                {0.0f, light->range, 0.0f, 0.0f},
                                                {0.0f, 0.0f, light->range, 0.0f},
                                                {0.0f, 0.0f, 0.0f, 1.0f}};

                  transform[3] = {light->position, 1.0f};

                  command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                                _dynamic_buffer_allocator
                                                   .allocate_and_copy(transform)
                                                   .gpu_address);

                  auto shape = _geometric_shapes.icosphere();

                  command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
                  command_list.ia_set_index_buffer(shape.index_buffer_view);
                  command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
               }
               else if (light->light_type == world::light_type::spot) {
                  const float half_range = light->range / 2.0f;
                  const float cone_radius =
                     half_range * std::tan(light->outer_cone_angle);

                  float4x4 transform =
                     to_matrix(light->rotation) *
                     to_matrix(quaternion{0.707107f, -0.707107f, 0.0f, 0.0f}) *
                     float4x4{{cone_radius, 0.0f, 0.0f, 0.0f},
                              {0.0f, half_range, 0.0f, 0.0f},
                              {0.0f, 0.0f, cone_radius, 0.0f},
                              {0.0f, -half_range, 0.0f, 1.0f}};

                  transform[3] += float4{light->position, 0.0f};

                  command_list.set_graphics_cbv(rs::meta_mesh::object_cbv,
                                                _dynamic_buffer_allocator
                                                   .allocate_and_copy(transform)
                                                   .gpu_address);

                  auto shape = _geometric_shapes.cone();

                  command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
                  command_list.ia_set_index_buffer(shape.index_buffer_view);
                  command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
               }
            },
            [&](world::path_id id) {
               const world::path* path =
                  look_for(world.paths, [id](const world::path& path) {
                     return id == path.id;
                  });

               if (not path) return;

               meta_mesh_common_setup();

               for (auto& node : path->nodes) draw_path_node(node);

               draw_lines(command_list, _root_signatures, _pipelines,
                          _dynamic_buffer_allocator,
                          {.line_color = _settings->hover_color(),

                           .camera_constant_buffer_view = _camera_constant_buffer_view,

                           .connect_mode = line_connect_mode::linear},
                          [&](line_draw_context& draw_context) {
                             using namespace ranges::views;

                             const auto get_position = [](const world::path::node& node) {
                                return node.position;
                             };

                             for (const auto [a, b] :
                                  zip(path->nodes | transform(get_position),
                                      path->nodes | drop(1) | transform(get_position))) {
                                draw_context.add(a, b);
                             }
                          });
            },
            [&](world::path_id_node_pair id_node) {
               auto [id, node_index] = id_node;

               const world::path* path =
                  look_for(world.paths, [id](const world::path& path) {
                     return id == path.id;
                  });

               if (not path) return;

               if (node_index >= path->nodes.size()) return;

               meta_mesh_common_setup();

               draw_path_node(path->nodes[node_index]);
            },
            [&](world::region_id id) {
               const world::region* region =
                  look_for(world.regions, [id](const world::region& region) {
                     return id == region.id;
                  });

               if (not region) return;

               meta_mesh_common_setup();

               const float3 scale = [&] {
                  switch (region->shape) {
                  default:
                  case world::region_shape::box: {
                     return region->size;
                  }
                  case world::region_shape::sphere: {
                     const float sphere_radius = length(region->size);

                     return float3{sphere_radius, sphere_radius, sphere_radius};
                  }
                  case world::region_shape::cylinder: {
                     const float cylinder_length =
                        length(float2{region->size.x, region->size.z});
                     return float3{cylinder_length, region->size.y, cylinder_length};
                  }
                  }
               }();

               float4x4 transform = to_matrix(region->rotation) *
                                    float4x4{{scale.x, 0.0f, 0.0f, 0.0f},
                                             {0.0f, scale.y, 0.0f, 0.0f},
                                             {0.0f, 0.0f, scale.z, 0.0f},
                                             {0.0f, 0.0f, 0.0f, 1.0f}};

               transform[3] = {region->position, 1.0f};

               command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                             _dynamic_buffer_allocator
                                                .allocate_and_copy(transform)
                                                .gpu_address);

               const geometric_shape shape = [&] {
                  switch (region->shape) {
                  default:
                  case world::region_shape::box:
                     return _geometric_shapes.cube();
                  case world::region_shape::sphere:
                     return _geometric_shapes.icosphere();
                  case world::region_shape::cylinder:
                     return _geometric_shapes.cylinder();
                  }
               }();

               command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
               command_list.ia_set_index_buffer(shape.index_buffer_view);
               command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
            },
            [&](world::sector_id id) {
               const world::sector* sector =
                  look_for(world.sectors, [id](const world::sector& sector) {
                     return id == sector.id;
                  });

               if (not sector) return;

               meta_mesh_common_setup();

               command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                             _dynamic_buffer_allocator
                                                .allocate_and_copy(
                                                   float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                            {0.0f, 1.0f, 0.0f, 0.0f},
                                                            {0.0f, 0.0f, 1.0f, 0.0f},
                                                            {0.0f, 0.0f, 0.0f, 1.0f}})
                                                .gpu_address);

               using namespace ranges::views;

               for (auto& points = sector->points;
                    const auto [a, b] :
                    zip(points, concat(points | drop(1), points | take(1)))) {
                  const std::array quad =
                     {float3{a.x, sector->base, a.y}, float3{b.x, sector->base, b.y},
                      float3{a.x, sector->base + sector->height, a.y},
                      float3{b.x, sector->base + sector->height, b.y}};

                  triangle_drawer.add(quad[0], quad[1], quad[2]);
                  triangle_drawer.add(quad[2], quad[1], quad[3]);
                  triangle_drawer.add(quad[0], quad[2], quad[1]);
                  triangle_drawer.add(quad[2], quad[3], quad[1]);
               }

               triangle_drawer.submit();
            },
            [&](world::portal_id id) {
               const world::portal* portal =
                  look_for(world.portals, [id](const world::portal& portal) {
                     return id == portal.id;
                  });

               if (not portal) return;

               meta_mesh_common_setup();

               command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                             _dynamic_buffer_allocator
                                                .allocate_and_copy(
                                                   float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                            {0.0f, 1.0f, 0.0f, 0.0f},
                                                            {0.0f, 0.0f, 1.0f, 0.0f},
                                                            {0.0f, 0.0f, 0.0f, 1.0f}})
                                                .gpu_address);

               const float half_width = portal->width * 0.5f;
               const float half_height = portal->height * 0.5f;

               std::array quad = {float3{-half_width, -half_height, 0.0f},
                                  float3{half_width, -half_height, 0.0f},
                                  float3{-half_width, half_height, 0.0f},
                                  float3{half_width, half_height, 0.0f}};

               for (auto& v : quad) {
                  v = portal->rotation * v;
                  v += portal->position;
               }

               triangle_drawer.add(quad[0], quad[1], quad[2]);
               triangle_drawer.add(quad[2], quad[1], quad[3]);
               triangle_drawer.add(quad[0], quad[2], quad[1]);
               triangle_drawer.add(quad[2], quad[3], quad[1]);

               triangle_drawer.submit();
            },
            [&](world::hintnode_id id) {
               const world::hintnode* hintnode =
                  look_for(world.hintnodes, [id](const world::hintnode& hintnode) {
                     return id == hintnode.id;
                  });

               if (not hintnode) return;

               meta_mesh_common_setup();

               float4x4 transform = to_matrix(hintnode->rotation);
               transform[3] = {hintnode->position, 1.0f};

               command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                             _dynamic_buffer_allocator
                                                .allocate_and_copy(transform)
                                                .gpu_address);

               const geometric_shape shape = _geometric_shapes.octahedron();

               command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
               command_list.ia_set_index_buffer(shape.index_buffer_view);
               command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
            },
            [&](world::barrier_id id) {
               const world::barrier* barrier =
                  look_for(world.barriers, [id](const world::barrier& barrier) {
                     return id == barrier.id;
                  });

               if (not barrier) return;

               meta_mesh_common_setup();

               const geometric_shape shape = _geometric_shapes.cube();

               const float2 position =
                  (barrier->corners[0] + barrier->corners[2]) / 2.0f;
               const float2 size{distance(barrier->corners[0], barrier->corners[3]),
                                 distance(barrier->corners[0], barrier->corners[1])};
               const float angle =
                  std::atan2(barrier->corners[1].x - barrier->corners[0].x,
                             barrier->corners[1].y - barrier->corners[0].y);

               const float barrier_height = _settings->barrier_height();

               float4x4 transform =
                  make_rotation_matrix_from_euler({0.0f, angle, 0.0f}) *
                  float4x4{{size.x / 2.0f, 0.0f, 0.0f, 0.0f},
                           {0.0f, barrier_height, 0.0f, 0.0f},
                           {0.0f, 0.0f, size.y / 2.0f, 0.0f},
                           {0.0f, 0.0f, 0.0f, 1.0f}};
               transform[3] = {position.x, 0.0f, position.y, 1.0f};

               command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                             _dynamic_buffer_allocator
                                                .allocate_and_copy(transform)
                                                .gpu_address);

               command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
               command_list.ia_set_index_buffer(shape.index_buffer_view);
               command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
            },
            [&](world::planning_hub_id id) { (void)id; },
            [&](world::planning_connection_id id) { (void)id; },
            [&](world::boundary_id id) {
               const world::boundary* boundary =
                  look_for(world.boundaries, [id](const world::boundary& boundary) {
                     return id == boundary.id;
                  });

               if (not boundary) return;

               const world::path* path =
                  look_for(world.paths, [&](const world::path& path) {
                     return path.name == boundary->name;
                  });

               if (not path) return;

               meta_mesh_common_setup();

               command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                             _dynamic_buffer_allocator
                                                .allocate_and_copy(
                                                   float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                            {0.0f, 1.0f, 0.0f, 0.0f},
                                                            {0.0f, 0.0f, 1.0f, 0.0f},
                                                            {0.0f, 0.0f, 0.0f, 1.0f}})
                                                .gpu_address);

               const float boundary_height = _settings->boundary_height();

               using namespace ranges::views;

               for (const auto [a, b] :
                    zip(path->nodes,
                        concat(path->nodes | drop(1), path->nodes | take(1)))) {

                  const std::array quad = {a.position, b.position,
                                           a.position +
                                              float3{0.0f, boundary_height, 0.0f},
                                           b.position +
                                              float3{0.0f, boundary_height, 0.0f}};

                  triangle_drawer.add(quad[0], quad[1], quad[2]);
                  triangle_drawer.add(quad[2], quad[1], quad[3]);
                  triangle_drawer.add(quad[0], quad[2], quad[1]);
                  triangle_drawer.add(quad[2], quad[3], quad[1]);
               }

               triangle_drawer.submit();
            },
         },
         target);
   };

   if (interaction_targets.hovered_entity) {
      gpu_virtual_address wireframe_constants = [&] {
         auto allocation =
            _dynamic_buffer_allocator.allocate(sizeof(wireframe_constant_buffer));

         wireframe_constant_buffer constants{.color = _settings->hover_color()};

         std::memcpy(allocation.cpu_address, &constants,
                     sizeof(wireframe_constant_buffer));

         return allocation.gpu_address;
      }();

      draw_target(*interaction_targets.hovered_entity, wireframe_constants);
   }

   if (not interaction_targets.selection.empty()) {
      gpu_virtual_address wireframe_constants = [&] {
         auto allocation =
            _dynamic_buffer_allocator.allocate(sizeof(wireframe_constant_buffer));

         wireframe_constant_buffer constants{.color = _settings->selected_color()};

         std::memcpy(allocation.cpu_address, &constants,
                     sizeof(wireframe_constant_buffer));

         return allocation.gpu_address;
      }();

      for (auto target : interaction_targets.selection) {
         draw_target(target, wireframe_constants);
      }
   }
}

void renderer_impl::build_world_mesh_list(
   gpu::copy_command_list& command_list, const world::world& world,
   const world::active_layers active_layers,
   const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes)
{
   _world_mesh_list.clear();
   _world_mesh_list.reserve(1024 * 16);

   auto& upload_buffer = _object_constants_upload_buffers[_device.frame_index()];

   const gpu_virtual_address constants_upload_gpu_address =
      _device.get_gpu_virtual_address(_object_constants_buffer.get());
   std::byte* const constants_upload_data =
      _object_constants_upload_cpu_ptrs[_device.frame_index()];
   std::size_t constants_data_size = 0;

   for (std::size_t i = 0; i < std::min(world.objects.size(), max_drawn_objects); ++i) {
      const auto& object = world.objects[i];
      auto& model = _model_manager[world_classes.at(object.class_name).model_name];

      if (not active_layers[object.layer]) continue;

      const auto object_bbox = object.rotation * model.bbox + object.position;

      const std::size_t object_constants_offset = constants_data_size;
      const gpu_virtual_address object_constants_address =
         constants_upload_gpu_address + object_constants_offset;

      world_mesh_constants constants;

      constants.object_to_world = to_matrix(object.rotation);
      constants.object_to_world[3] = float4{object.position, 1.0f};

      std::memcpy(constants_upload_data + object_constants_offset,
                  &constants.object_to_world, sizeof(world_mesh_constants));

      constants_data_size += sizeof(world_mesh_constants);

      for (auto& mesh : model.parts) {
         const gpu::pipeline_handle pipeline =
            _pipelines.mesh_normal[mesh.material.flags].get();

         _world_mesh_list.push_back(
            object_bbox, object_constants_address, object.position, pipeline,
            mesh.material.flags, mesh.material.constant_buffer_view,
            world_mesh{.index_buffer_view = model.gpu_buffer.index_buffer_view,
                       .vertex_buffer_views = {model.gpu_buffer.position_vertex_buffer_view,
                                               model.gpu_buffer.attributes_vertex_buffer_view},
                       .index_count = mesh.index_count,
                       .start_index = mesh.start_index,
                       .start_vertex = mesh.start_vertex});
      }
   }

   command_list.copy_buffer_region(_object_constants_buffer.get(), 0,
                                   upload_buffer.get(), 0, constants_data_size);
}

void renderer_impl::build_object_render_list(const frustrum& view_frustrum)
{
   auto& meshes = _world_mesh_list;

   _opaque_object_render_list.clear();
   _transparent_object_render_list.clear();
   _opaque_object_render_list.reserve(meshes.size());
   _transparent_object_render_list.reserve(meshes.size());

   for (std::size_t i = 0; i < meshes.size(); ++i) {
      if (not intersects(view_frustrum, meshes.bbox[i])) continue;

      auto& render_list = are_flags_set(meshes.pipeline_flags[i],
                                        material_pipeline_flags::transparent)
                             ? _transparent_object_render_list
                             : _opaque_object_render_list;

      const gpu::pipeline_handle depth_prepass_pipeline = [&]() {
         if (are_flags_set(meshes.pipeline_flags[i],
                           material_pipeline_flags::alpha_cutout |
                              material_pipeline_flags::doublesided)) {
            return _pipelines.mesh_depth_prepass_alpha_cutout_doublesided.get();
         }
         if (are_flags_set(meshes.pipeline_flags[i],
                           material_pipeline_flags::alpha_cutout)) {
            return _pipelines.mesh_depth_prepass_alpha_cutout.get();
         }
         if (are_flags_set(meshes.pipeline_flags[i],
                           material_pipeline_flags::doublesided)) {
            return _pipelines.mesh_depth_prepass_doublesided.get();
         }

         return _pipelines.mesh_depth_prepass.get();
      }();

      render_list.push_back({
         .distance = dot(view_frustrum.planes[frustrum_planes::near_],
                         float4{meshes.position[i], 1.0f}),

         .pipeline = meshes.pipeline[i],
         .depth_prepass_pipeline = depth_prepass_pipeline,

         .index_buffer_view = meshes.mesh[i].index_buffer_view,
         .vertex_buffer_views = {meshes.mesh[i].vertex_buffer_views},

         .object_constants_address = meshes.gpu_constants[i],
         .material_cbv = meshes.material_constant_buffer[i],

         .index_count = meshes.mesh[i].index_count,
         .start_index = meshes.mesh[i].start_index,
         .start_vertex = meshes.mesh[i].start_vertex,
      });
   }

   std::sort(_opaque_object_render_list.begin(), _opaque_object_render_list.end(),
             [](const render_list_item& l, const render_list_item& r) {
                return std::tie(l.distance, l.pipeline) <
                       std::tie(r.distance, r.pipeline);
             });
   std::sort(_transparent_object_render_list.begin(),
             _transparent_object_render_list.end(),
             [](const render_list_item& l, const render_list_item& r) {
                return l.distance > r.distance;
             });
}

void renderer_impl::update_textures(gpu::copy_command_list& command_list)
{
   _texture_manager.eval_updated_textures(
      [&](const absl::flat_hash_map<lowercase_string, std::shared_ptr<const world_texture>>& updated) {
         _model_manager.for_each([&](model& model) {
            for (auto& part : model.parts) {
               part.material.process_updated_textures(command_list, updated, _device);
            }
         });

         _terrain.process_updated_texture(command_list, updated);
      });
}

renderer::renderer(const window_handle window,
                   std::shared_ptr<settings::graphics> settings,
                   std::shared_ptr<async::thread_pool> thread_pool,
                   assets::libraries_manager& asset_libraries,
                   output_stream& error_output)
   : _impl{std::make_unique<renderer_impl>(window, settings, thread_pool,
                                           asset_libraries, error_output)}
{
}

renderer::~renderer() = default;

void renderer::wait_for_swap_chain_ready()
{
   _impl->wait_for_swap_chain_ready();
}

void renderer::draw_frame(const camera& camera, const world::world& world,
                          const world::interaction_targets& interaction_targets,
                          const world::active_entity_types active_entity_types,
                          const world::active_layers active_layers,
                          const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes)
{
   _impl->draw_frame(camera, world, interaction_targets, active_entity_types,
                     active_layers, world_classes);
}

void renderer::window_resized(uint16 width, uint16 height)
{
   _impl->window_resized(width, height);
}

void renderer::mark_dirty_terrain() noexcept
{
   _impl->mark_dirty_terrain();
}

void renderer::recreate_imgui_font_atlas()
{
   _impl->recreate_imgui_font_atlas();
}

void renderer::reload_shaders() noexcept
{
   _impl->reload_shaders();
}
}
