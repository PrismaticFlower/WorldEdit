
#include "renderer.hpp"
#include "async/thread_pool.hpp"
#include "camera.hpp"
#include "copy_command_list_pool.hpp"
#include "cull_objects.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "frustum.hpp"
#include "geometric_shapes.hpp"
#include "gpu/rhi.hpp"
#include "imgui/imgui.h"
#include "imgui_renderer.hpp"
#include "light_clusters.hpp"
#include "math/matrix_funcs.hpp"
#include "math/quaternion_funcs.hpp"
#include "math/vector_funcs.hpp"
#include "meta_draw_batcher.hpp"
#include "model_manager.hpp"
#include "output_stream.hpp"
#include "pipeline_library.hpp"
#include "profiler.hpp"
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
#include "utility/srgb_conversion.hpp"
#include "utility/stopwatch.hpp"
#include "world/utility/world_utilities.hpp"
#include "world/world.hpp"

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

}

struct renderer_config {
   bool use_raytracing = false;
};

struct renderer_impl final : renderer {
   renderer_impl(const renderer::window_handle window,
                 std::shared_ptr<async::thread_pool> thread_pool,
                 assets::libraries_manager& asset_libraries,
                 output_stream& error_output);

   void wait_for_swap_chain_ready() override;

   void draw_frame(const camera& camera, const world::world& world,
                   const world::interaction_targets& interaction_targets,
                   const world::active_entity_types active_entity_types,
                   const world::active_layers active_layers,
                   const world::tool_visualizers& tool_visualizers,
                   const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes,
                   const settings::graphics& settings) override;

   void window_resized(uint16 width, uint16 height) override;

   void mark_dirty_terrain() noexcept override;

   void recreate_imgui_font_atlas() override
   {
      _imgui_renderer.recreate_font_atlas(_copy_command_list_pool);
   }

   void reload_shaders() noexcept override
   {
      _device.wait_for_idle();
      _shaders.reload(shader_list);
      _pipelines.reload(_device, _shaders, _root_signatures);
   }

private:
   void update_frame_constant_buffer(const camera& camera,
                                     const settings::graphics& settings,
                                     gpu::copy_command_list& command_list);

   void draw_world(const frustum& view_frustum, gpu::graphics_command_list& command_list);

   void draw_world_render_list_depth_prepass(const std::vector<uint16>& list,
                                             gpu::graphics_command_list& command_list);

   void draw_world_render_list(const std::vector<uint16>& list,
                               gpu::graphics_command_list& command_list);

   void draw_world_meta_objects(const frustum& view_frustum, const world::world& world,
                                const world::interaction_targets& interaction_targets,
                                const world::active_entity_types active_entity_types,
                                const world::active_layers active_layers,
                                const world::tool_visualizers& tool_visualizers,
                                const settings::graphics& settings,
                                gpu::graphics_command_list& command_list);

   void draw_interaction_targets(
      const frustum& view_frustum, const world::world& world,
      const world::interaction_targets& interaction_targets,
      const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes,
      const settings::graphics& settings, gpu::graphics_command_list& command_list);

   void build_world_mesh_list(
      gpu::copy_command_list& command_list, const world::world& world,
      const world::active_layers active_layers,
      const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes,
      const world::object* const creation_object);

   void build_object_render_list(const frustum& view_frustum);

   void clear_depth_minmax(gpu::copy_command_list& command_list);

   void reduce_depth_minmax(gpu::graphics_command_list& command_list);

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
                               .format = DXGI_FORMAT_R24G8_TYPELESS,
                               .width = _swap_chain.width(),
                               .height = _swap_chain.height(),
                               .optimized_clear_value =
                                  {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                   .depth_stencil = {.depth = 1.0f, .stencil = 0x0}}},
                              gpu::barrier_layout::direct_queue_shader_resource),
       _device.direct_queue};
   gpu::unique_dsv_handle _depth_stencil_view =
      {_device.create_depth_stencil_view(_depth_stencil_texture.get(),
                                         {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                          .dimension = gpu::dsv_dimension::texture2d}),
       _device.direct_queue};
   gpu::unique_resource_view _depth_stencil_srv =
      {_device.create_shader_resource_view(_depth_stencil_texture.get(),
                                           {.format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS}),
       _device.direct_queue};
   gpu::unique_resource_handle _depth_minmax_buffer =
      {_device.create_buffer({.size = sizeof(float4),
                              .flags = {.allow_unordered_access = true}},
                             gpu::heap_type::default_),
       _device.direct_queue};
   gpu::unique_resource_handle _depth_minmax_readback_buffer =
      {_device.create_buffer({.size = sizeof(float4) * gpu::frame_pipeline_length},
                             gpu::heap_type::readback),
       _device.direct_queue};
   std::array<const float4*, gpu::frame_pipeline_length> _depth_minmax_readback_buffer_ptrs;

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
   std::vector<uint16> _opaque_object_render_list;
   std::vector<uint16> _transparent_object_render_list;

   meta_draw_batcher _meta_draw_batcher;

   imgui_renderer _imgui_renderer{_device, _copy_command_list_pool};

   profiler _profiler{_device, 256};

   renderer_config _config;
};

renderer_impl::renderer_impl(const renderer::window_handle window,
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
                    thread_pool,      _error_output}
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

   // map depth minmax readback buffer
   {
      float4* address = static_cast<float4*>(
         _device.map(_depth_minmax_readback_buffer.get(), 0,
                     {0, sizeof(float4) * gpu::frame_pipeline_length}));

      for (auto& ptr : _depth_minmax_readback_buffer_ptrs) {
         ptr = address;
         address += 1;
      }
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
   const world::tool_visualizers& tool_visualizers,
   const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes,
   const settings::graphics& settings)
{
   const frustum view_frustum{camera.inv_view_projection_matrix()};

   auto [back_buffer, back_buffer_rtv] = _swap_chain.current_back_buffer();
   _dynamic_buffer_allocator.reset(_device.frame_index());

   _model_manager.update_models();

   if (settings.show_profiler) _profiler.show();

   // Pre-Render Work
   {
      _pre_render_command_list.reset();

      if (std::exchange(_terrain_dirty, false)) {
         _terrain.init(world.terrain, _pre_render_command_list,
                       _dynamic_buffer_allocator);
      }

      update_textures(_pre_render_command_list);
      build_world_mesh_list(_pre_render_command_list, world, active_layers, world_classes,
                            interaction_targets.creation_entity
                               ? std::get_if<world::object>(
                                    &(*interaction_targets.creation_entity))
                               : nullptr);
      update_frame_constant_buffer(camera, settings, _pre_render_command_list);
      clear_depth_minmax(_pre_render_command_list);

      const float4 scene_depth_min_max =
         *_depth_minmax_readback_buffer_ptrs[_device.frame_index()];

      _light_clusters
         .prepare_lights(camera, view_frustum, world,
                         interaction_targets.creation_entity
                            ? std::get_if<world::light>(
                                 &interaction_targets.creation_entity.value())
                            : nullptr,
                         {scene_depth_min_max.x, scene_depth_min_max.y},
                         _pre_render_command_list, _dynamic_buffer_allocator);

      _pre_render_command_list.close();

      _device.copy_queue.execute_command_lists(_pre_render_command_list);
      _device.direct_queue.sync_with(_device.copy_queue);
   }

   build_object_render_list(view_frustum);

   auto& command_list = _world_command_list;

   command_list.reset(_sampler_heap.get());

   _light_clusters.tile_lights(_root_signatures, _pipelines, command_list,
                               _dynamic_buffer_allocator, _profiler);
   _light_clusters.draw_shadow_maps(_world_mesh_list, _root_signatures,
                                    _pipelines, command_list,
                                    _dynamic_buffer_allocator, _profiler);

   command_list.deferred_barrier(
      gpu::texture_barrier{.sync_before = gpu::barrier_sync::none,
                           .sync_after = gpu::barrier_sync::render_target,
                           .access_before = gpu::barrier_access::no_access,
                           .access_after = gpu::barrier_access::render_target,
                           .layout_before = gpu::barrier_layout::present,
                           .layout_after = gpu::barrier_layout::render_target,
                           .resource = back_buffer});

   command_list.deferred_barrier(
      gpu::texture_barrier{.sync_before = gpu::barrier_sync::none,
                           .sync_after = gpu::barrier_sync::depth_stencil,
                           .access_before = gpu::barrier_access::no_access,
                           .access_after = gpu::barrier_access::depth_stencil_write,
                           .layout_before = gpu::barrier_layout::direct_queue_shader_resource,
                           .layout_after = gpu::barrier_layout::depth_stencil_write,
                           .resource = _depth_stencil_texture.get()});
   command_list.flush_barriers();

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
   if (active_entity_types.objects) draw_world(view_frustum, command_list);

   // Render World Meta Objects
   draw_world_meta_objects(view_frustum, world, interaction_targets,
                           active_entity_types, active_layers, tool_visualizers,
                           settings, command_list);

   draw_interaction_targets(view_frustum, world, interaction_targets,
                            world_classes, settings, command_list);

   // Render ImGui
   ImGui::Render();
   _imgui_renderer.render_draw_data(ImGui::GetDrawData(), _root_signatures,
                                    _pipelines, command_list);

   reduce_depth_minmax(command_list);

   _profiler.end_frame(command_list);

   command_list.deferred_barrier(
      gpu::texture_barrier{.sync_before = gpu::barrier_sync::render_target,
                           .sync_after = gpu::barrier_sync::none,
                           .access_before = gpu::barrier_access::render_target,
                           .access_after = gpu::barrier_access::no_access,
                           .layout_before = gpu::barrier_layout::render_target,
                           .layout_after = gpu::barrier_layout::present,
                           .resource = back_buffer});

   command_list.flush_barriers();

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
                              gpu::barrier_layout::direct_queue_shader_resource),
       _device.direct_queue};
   _depth_stencil_view =
      {_device.create_depth_stencil_view(_depth_stencil_texture.get(),
                                         {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                          .dimension = gpu::dsv_dimension::texture2d}),
       _device.direct_queue};
   _depth_stencil_srv =
      {_device.create_shader_resource_view(_depth_stencil_texture.get(),
                                           {.format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS}),
       _device.direct_queue};

   _light_clusters.update_render_resolution(width, height);
}

void renderer_impl::mark_dirty_terrain() noexcept
{
   _terrain_dirty = true;
}

void renderer_impl::update_frame_constant_buffer(const camera& camera,
                                                 const settings::graphics& settings,
                                                 gpu::copy_command_list& command_list)
{
   frame_constant_buffer
      constants{.view_projection_matrix = camera.view_projection_matrix(),

                .view_positionWS = camera.position(),

                .viewport_size = {static_cast<float>(_swap_chain.width()),
                                  static_cast<float>(_swap_chain.height())},
                .viewport_topleft = {0.0f, 0.0f},

                .line_width = settings.line_width};

   auto allocation = _dynamic_buffer_allocator.allocate_and_copy(constants);

   command_list.copy_buffer_region(_camera_constant_buffer.get(), 0,
                                   _dynamic_buffer_allocator.resource(),
                                   allocation.offset, sizeof(frame_constant_buffer));
}

void renderer_impl::draw_world(const frustum& view_frustum,
                               gpu::graphics_command_list& command_list)
{
   {
      profile_section profile{"World - Draw Render List Depth Prepass",
                              command_list, _profiler, profiler_queue::direct};

      draw_world_render_list_depth_prepass(_opaque_object_render_list, command_list);
   }

   {
      profile_section profile{"Terrain - Draw Depth Prepass", command_list,
                              _profiler, profiler_queue::direct};

      _terrain.draw(terrain_draw::depth_prepass, view_frustum,
                    _camera_constant_buffer_view,
                    _light_clusters.lights_constant_buffer_view(), command_list,
                    _root_signatures, _pipelines, _dynamic_buffer_allocator);
   }

   {
      profile_section profile{"World - Draw Opaque", command_list, _profiler,
                              profiler_queue::direct};

      draw_world_render_list(_opaque_object_render_list, command_list);
   }

   {
      profile_section profile{"Terrain - Draw", command_list, _profiler,
                              profiler_queue::direct};

      _terrain.draw(terrain_draw::main, view_frustum, _camera_constant_buffer_view,
                    _light_clusters.lights_constant_buffer_view(), command_list,
                    _root_signatures, _pipelines, _dynamic_buffer_allocator);
   }

   {
      profile_section profile{"World - Draw Transparent", command_list,
                              _profiler, profiler_queue::direct};

      draw_world_render_list(_transparent_object_render_list, command_list);
   }
}

void renderer_impl::draw_world_render_list(const std::vector<uint16>& list,
                                           gpu::graphics_command_list& command_list)
{

   command_list.set_graphics_root_signature(_root_signatures.mesh.get());
   command_list.set_graphics_cbv(rs::mesh::frame_cbv, _camera_constant_buffer_view);
   command_list.set_graphics_cbv(rs::mesh::lights_cbv,
                                 _light_clusters.lights_constant_buffer_view());
   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

   gpu::pipeline_handle pipeline_state = gpu::null_pipeline_handle;

   auto& meshes = _world_mesh_list;

   for (auto& i : list) {
      if (std::exchange(pipeline_state, meshes.pipeline[i]) != meshes.pipeline[i]) {
         command_list.set_pipeline_state(meshes.pipeline[i]);
      }

      command_list.set_graphics_cbv(rs::mesh::object_cbv, meshes.gpu_constants[i]);
      command_list.set_graphics_cbv(rs::mesh::material_cbv,
                                    meshes.material_constant_buffer[i]);

      auto& mesh = meshes.mesh[i];

      command_list.ia_set_index_buffer(mesh.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, mesh.vertex_buffer_views);
      command_list.draw_indexed_instanced(mesh.index_count, 1, mesh.start_index,
                                          mesh.start_vertex, 0);
   }
}

void renderer_impl::draw_world_render_list_depth_prepass(
   const std::vector<uint16>& list, gpu::graphics_command_list& command_list)
{
   command_list.set_graphics_root_signature(_root_signatures.mesh_depth_prepass.get());
   command_list.set_graphics_cbv(rs::mesh_depth_prepass::frame_cbv,
                                 _camera_constant_buffer_view);
   command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

   material_pipeline_flags pipeline_flags = material_pipeline_flags::none;

   command_list.set_pipeline_state(_pipelines.mesh_depth_prepass.get());

   auto& meshes = _world_mesh_list;

   for (auto& i : list) {
      if (std::exchange(pipeline_flags, meshes.pipeline_flags[i]) !=
          meshes.pipeline_flags[i]) {

         if (are_flags_set(pipeline_flags, material_pipeline_flags::alpha_cutout |
                                              material_pipeline_flags::doublesided)) {
            command_list.set_pipeline_state(
               _pipelines.mesh_depth_prepass_alpha_cutout_doublesided.get());
         }
         else if (are_flags_set(pipeline_flags, material_pipeline_flags::alpha_cutout)) {
            command_list.set_pipeline_state(
               _pipelines.mesh_depth_prepass_alpha_cutout.get());
         }
         else if (are_flags_set(pipeline_flags, material_pipeline_flags::doublesided)) {
            command_list.set_pipeline_state(
               _pipelines.mesh_depth_prepass_doublesided.get());
         }
         else {
            command_list.set_pipeline_state(_pipelines.mesh_depth_prepass.get());
         }
      }

      command_list.set_graphics_cbv(rs::mesh_depth_prepass::object_cbv,
                                    meshes.gpu_constants[i]);

      if (are_flags_set(pipeline_flags, material_pipeline_flags::alpha_cutout)) {
         command_list.set_graphics_cbv(rs::mesh_depth_prepass::material_cbv,
                                       meshes.material_constant_buffer[i]);
      }

      auto& mesh = meshes.mesh[i];

      command_list.ia_set_index_buffer(mesh.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, mesh.vertex_buffer_views);
      command_list.draw_indexed_instanced(mesh.index_count, 1, mesh.start_index,
                                          mesh.start_vertex, 0);
   }
}

void renderer_impl::draw_world_meta_objects(
   const frustum& view_frustum, const world::world& world,
   const world::interaction_targets& interaction_targets,
   const world::active_entity_types active_entity_types,
   const world::active_layers active_layers,
   const world::tool_visualizers& tool_visualizers,
   const settings::graphics& settings, gpu::graphics_command_list& command_list)
{
   profile_section profile{"World - Draw Meta Objects", command_list, _profiler,
                           profiler_queue::direct};

   _meta_draw_batcher.clear();

   if (active_entity_types.paths and not world.paths.empty()) {
      static bool draw_nodes = true;
      static bool draw_connections = true;
      static bool draw_orientation = false;

      // TODO: Move theses to _settings
      ImGui::Indent();
      ImGui::Checkbox("Draw Paths Connections", &draw_connections);
      ImGui::Checkbox("Draw Paths Orientation", &draw_orientation);
      ImGui::Unindent();

      const float4 path_node_color = {settings.path_node_color, 1.0f};
      const float4 path_node_outline_color = {settings.path_node_outline_color, 1.0f};

      for (auto& path : world.paths) {
         if (not active_layers[path.layer]) continue;

         for (auto& node : path.nodes) {
            if (not intersects(view_frustum, node.position, 0.5f)) continue;

            const float4x4 rotation = to_matrix(node.rotation);
            float4x4 transform = rotation * float4x4{{0.5f, 0.0f, 0.0f, 0.0f},
                                                     {0.0f, 0.5f, 0.0f, 0.0f},
                                                     {0.0f, 0.0f, 0.5f, 0.0f},
                                                     {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {node.position, 1.0f};

            _meta_draw_batcher.add_octahedron_outlined(transform, path_node_color,
                                                       path_node_outline_color);

            if (draw_orientation) {
               float4x4 orientation_transform =
                  rotation * float4x4{{0.25f, 0.0f, 0.0f, 0.0f},
                                      {0.0f, 0.25f, 0.0f, 0.0f},
                                      {0.0f, 0.0f, 0.25f, 0.0f},
                                      {0.0f, 0.0f, 0.0f, 1.0f}};
               orientation_transform[3] = {node.position, 1.0f};

               _meta_draw_batcher.add_arrow_outline_solid(orientation_transform,
                                                          0.8f / 0.25f,
                                                          utility::pack_srgb_bgra(
                                                             float4{settings.path_node_orientation_color,
                                                                    1.0f}));
            }
         }

         if (draw_connections) {
            const uint32 path_node_connection_color = utility::pack_srgb_bgra(
               float4{settings.path_node_connection_color, 1.0f});

            using namespace ranges::views;

            const auto get_position = [](const world::path::node& node) {
               return node.position;
            };

            for (const auto [a, b] :
                 zip(path.nodes | transform(get_position),
                     path.nodes | drop(1) | transform(get_position))) {
               _meta_draw_batcher.add_line_solid(a, b, path_node_connection_color);
            }
         }
      }
   }

   // Adds a region to _meta_draw_batcher. Shared between light volume drawing and region drawing.
   const auto add_region = [&](const quaternion rotation, const float3 position,
                               const float3 size, const world::region_shape shape,
                               const float4 color) {
      const auto make_region_transform = [&](const float3 scale) {
         float4x4 transform =
            to_matrix(rotation) * float4x4{{scale.x, 0.0f, 0.0f, 0.0f},
                                           {0.0f, scale.y, 0.0f, 0.0f},
                                           {0.0f, 0.0f, scale.z, 0.0f},
                                           {0.0f, 0.0f, 0.0f, 1.0f}};

         transform[3] = {position, 1.0f};

         return transform;
      };

      switch (shape) {
      default:
      case world::region_shape::box: {
         math::bounding_box bbox{.min = {-size}, .max = {size}};

         bbox = rotation * bbox + position;

         if (not intersects(view_frustum, bbox)) {
            return;
         }

         const float4x4 transform = make_region_transform(size);

         _meta_draw_batcher.add_box(transform, color);
      } break;
      case world::region_shape::sphere: {
         const float sphere_radius = length(size);

         if (not intersects(view_frustum, position, sphere_radius)) {
            return;
         }

         _meta_draw_batcher.add_sphere(position, sphere_radius, color);
      } break;
      case world::region_shape::cylinder: {
         const float cylinder_length = length(float2{size.x, size.z});

         math::bounding_box bbox{.min = {-cylinder_length, -size.y, -cylinder_length},
                                 .max = {cylinder_length, size.y, cylinder_length}};

         bbox = rotation * bbox + position;

         if (not intersects(view_frustum, bbox)) {
            return;
         }

         const float4x4 transform =
            make_region_transform(float3{cylinder_length, size.y, cylinder_length});

         _meta_draw_batcher.add_cylinder(transform, color);
      } break;
      }
   };

   if (active_entity_types.regions and not world.regions.empty()) {
      const float4 region_color = settings.region_color;

      for (auto& region : world.regions) {
         if (not active_layers[region.layer]) continue;

         add_region(region.rotation, region.position, region.size, region.shape,
                    region_color);
      }
   }

   if (active_entity_types.barriers and not world.barriers.empty()) {
      const float barrier_height = settings.barrier_height;
      const float4 barrier_color = settings.barrier_color;

      for (auto& barrier : world.barriers) {

         const float2 position = (barrier.corners[0] + barrier.corners[2]) / 2.0f;
         const float2 size{distance(barrier.corners[0], barrier.corners[3]),
                           distance(barrier.corners[0], barrier.corners[1])};
         const float angle =
            std::atan2(barrier.corners[1].x - barrier.corners[0].x,
                       barrier.corners[1].y - barrier.corners[0].y);

         const float4x4 rotation =
            make_rotation_matrix_from_euler({0.0f, angle, 0.0f});

         math::bounding_box bbox{.min = {position.x - size.x, -barrier_height,
                                         position.y - size.y},
                                 .max = {position.x + size.x, barrier_height,
                                         position.y + size.y}};

         bbox.min = rotation * bbox.min;
         bbox.max = rotation * bbox.max;

         float4x4 transform =
            rotation * float4x4{{size.x / 2.0f, 0.0f, 0.0f, 0.0f},
                                {0.0f, barrier_height, 0.0f, 0.0f},
                                {0.0f, 0.0f, size.y / 2.0f, 0.0f},
                                {0.0f, 0.0f, 0.0f, 1.0f}};

         transform[3] = {position.x, 0.0f, position.y, 1.0f};

         if (intersects(view_frustum, bbox)) {
            _meta_draw_batcher.add_box(transform, barrier_color);
         }
      }
   }

   if (active_entity_types.lights and not world.lights.empty()) {
      const float volume_alpha = settings.light_volume_alpha;

      const auto add_light = [&](const world::light& light) {
         if (not active_layers[light.layer]) return;

         const float4 color{light.color, light.light_type == world::light_type::spot
                                            ? volume_alpha * 0.5f
                                            : volume_alpha};

         switch (light.light_type) {
         case world::light_type::directional: {
            const float4x4 rotation = to_matrix(light.rotation);
            float4x4 transform = rotation * float4x4{{2.0f, 0.0f, 0.0f, 0.0f},
                                                     {0.0f, 2.0f, 0.0f, 0.0f},
                                                     {0.0f, 0.0f, 2.0f, 0.0f},
                                                     {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {light.position, 1.0f};

            _meta_draw_batcher.add_octahedron(transform, color);
            _meta_draw_batcher.add_arrow_outline_solid(transform, 2.2f,
                                                       utility::pack_srgb_bgra(
                                                          float4{light.color, 1.0f}));
         } break;
         case world::light_type::point: {
            if (not intersects(view_frustum, light.position, light.range)) {
               return;
            }

            _meta_draw_batcher.add_sphere(light.position, light.range, color);
         } break;
         case world::light_type::spot: {
            const float half_range = light.range / 2.0f;
            const float outer_cone_radius =
               half_range * std::tan(light.outer_cone_angle);
            const float inner_cone_radius =
               half_range * std::tan(light.outer_cone_angle);

            const float3 light_direction =
               normalize(light.rotation * float3{0.0f, 0.0f, -1.0f});

            const float light_bounds_radius = std::max(outer_cone_radius, half_range);
            const float3 light_centre =
               light.position - (light_direction * (half_range));

            // TODO: Better cone culling
            if (not intersects(view_frustum, light_centre, light_bounds_radius)) {
               return;
            }

            const float4x4 rotation = to_matrix(
               light.rotation * quaternion{0.707107f, -0.707107f, 0.0f, 0.0f});

            float4x4 outer_transform =
               rotation * float4x4{{outer_cone_radius, 0.0f, 0.0f, 0.0f},
                                   {0.0f, half_range, 0.0f, 0.0f},
                                   {0.0f, 0.0f, outer_cone_radius, 0.0f},
                                   {0.0f, -half_range, 0.0f, 1.0f}};

            outer_transform[3] += float4{light.position, 0.0f};

            float4x4 inner_transform =
               rotation * float4x4{{inner_cone_radius, 0.0f, 0.0f, 0.0f},
                                   {0.0f, half_range, 0.0f, 0.0f},
                                   {0.0f, 0.0f, inner_cone_radius, 0.0f},
                                   {0.0f, -half_range, 0.0f, 1.0f}};

            inner_transform[3] += float4{light.position, 0.0f};

            _meta_draw_batcher.add_cone(outer_transform, color);
            _meta_draw_batcher.add_cone(inner_transform, color);
         } break;
         case world::light_type::directional_region_box:
         case world::light_type::directional_region_sphere:
         case world::light_type::directional_region_cylinder: {
            switch (light.light_type) {
            case world::light_type::directional_region_box: {
               add_region(light.region_rotation, light.position,
                          light.region_size, world::region_shape::box, color);
            } break;
            case world::light_type::directional_region_sphere: {
               add_region(light.region_rotation, light.position,
                          light.region_size, world::region_shape::sphere, color);
            } break;
            case world::light_type::directional_region_cylinder: {
               add_region(light.region_rotation, light.position, light.region_size,
                          world::region_shape::cylinder, color);
            } break;
            }

            float4x4 transform = to_matrix(light.rotation);
            transform[3] = {light.position, 1.0f};

            _meta_draw_batcher.add_arrow_outline_solid(transform, 0.0f,
                                                       utility::pack_srgb_bgra(
                                                          float4{light.color, 1.0f}));
         } break;
         }
      };

      for (auto& light : world.lights) {
         add_light(light);
      }

      if (interaction_targets.creation_entity and
          std::holds_alternative<world::light>(*interaction_targets.creation_entity)) {
         add_light(std::get<world::light>(*interaction_targets.creation_entity));
      }
   }

   if (active_entity_types.sectors and not world.sectors.empty()) {
      const uint32 sector_color = utility::pack_srgb_bgra(settings.sector_color);

      for (auto& sector : world.sectors) {
         using namespace ranges::views;

         for (const auto [a, b] :
              zip(sector.points,
                  concat(sector.points | drop(1), sector.points | take(1)))) {
            const std::array quad = {float3{a.x, sector.base, a.y},
                                     float3{b.x, sector.base, b.y},
                                     float3{a.x, sector.base + sector.height, a.y},
                                     float3{b.x, sector.base + sector.height, b.y}};

            math::bounding_box bbox{.min = quad[0], .max = quad[0]};

            for (auto v : quad | drop(1)) bbox = integrate(bbox, v);

            if (not intersects(view_frustum, bbox)) continue;

            _meta_draw_batcher.add_triangle(quad[0], quad[1], quad[2], sector_color);
            _meta_draw_batcher.add_triangle(quad[2], quad[1], quad[3], sector_color);
            _meta_draw_batcher.add_triangle(quad[0], quad[2], quad[1], sector_color);
            _meta_draw_batcher.add_triangle(quad[2], quad[3], quad[1], sector_color);
         }
      }
   }

   if (active_entity_types.portals and not world.portals.empty()) {
      const uint32 portal_color = utility::pack_srgb_bgra(settings.portal_color);

      for (auto& portal : world.portals) {
         using namespace ranges::views;

         const float half_width = portal.width * 0.5f;
         const float half_height = portal.height * 0.5f;

         if (not intersects(view_frustum, portal.position,
                            std::max(half_width, half_height))) {
            continue;
         }

         std::array quad = {float3{-half_width, -half_height, 0.0f},
                            float3{half_width, -half_height, 0.0f},
                            float3{-half_width, half_height, 0.0f},
                            float3{half_width, half_height, 0.0f}};

         for (auto& v : quad) {
            v = portal.rotation * v;
            v += portal.position;
         }

         _meta_draw_batcher.add_triangle(quad[0], quad[1], quad[2], portal_color);
         _meta_draw_batcher.add_triangle(quad[2], quad[1], quad[3], portal_color);
         _meta_draw_batcher.add_triangle(quad[0], quad[2], quad[1], portal_color);
         _meta_draw_batcher.add_triangle(quad[2], quad[3], quad[1], portal_color);
      }
   }

   if (active_entity_types.hintnodes and not world.hintnodes.empty()) {
      const float4 hintnode_color = float4{settings.hintnode_color, 1.0f};
      const float4 hintnode_outline_color =
         float4{settings.hintnode_outline_color, 1.0f};

      for (auto& hintnode : world.hintnodes) {
         if (not active_layers[hintnode.layer]) continue;

         if (not intersects(view_frustum, hintnode.position, 1.0f)) continue;

         float4x4 transform = to_matrix(hintnode.rotation);

         transform[3] = {hintnode.position, 1.0f};

         _meta_draw_batcher.add_octahedron_outlined(transform, hintnode_color,
                                                    hintnode_outline_color);
      }
   }

   if (active_entity_types.boundaries and not world.boundaries.empty()) {
      const float boundary_height = settings.boundary_height;
      const uint32 boundary_color = utility::pack_srgb_bgra(settings.boundary_color);

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

            _meta_draw_batcher.add_triangle(quad[0], quad[1], quad[2], boundary_color);
            _meta_draw_batcher.add_triangle(quad[2], quad[1], quad[3], boundary_color);
            _meta_draw_batcher.add_triangle(quad[0], quad[2], quad[1], boundary_color);
            _meta_draw_batcher.add_triangle(quad[2], quad[3], quad[1], boundary_color);
         }
      }
   }

   for (const auto& line : tool_visualizers.lines) {
      _meta_draw_batcher.add_line_solid(line.v0, line.v1, line.color);
   }

   _meta_draw_batcher.draw(command_list, _camera_constant_buffer_view,
                           _root_signatures, _pipelines, _geometric_shapes,
                           _dynamic_buffer_allocator);
}

void renderer_impl::draw_interaction_targets(
   const frustum& view_frustum, const world::world& world,
   const world::interaction_targets& interaction_targets,
   const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes,
   const settings::graphics& settings, gpu::graphics_command_list& command_list)
{
   profile_section profile{"World - Draw Interaction Targets", command_list,
                           _profiler, profiler_queue::direct};

   (void)view_frustum; // TODO: frustum Culling (Is it worth it for interaction targets?)

   triangle_drawer triangle_drawer{command_list, _dynamic_buffer_allocator, 1024};

   const auto meta_mesh_common_setup = [&](gpu_virtual_address wireframe_constants) {
      command_list.set_graphics_root_signature(_root_signatures.mesh_wireframe.get());
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

   const auto draw_entity = overload{
      [&](const world::object& object, gpu_virtual_address wireframe_constants) {
         gpu_virtual_address object_constants = [&] {
            auto allocation =
               _dynamic_buffer_allocator.allocate(sizeof(world_mesh_constants));

            world_mesh_constants constants{};

            constants.object_to_world = to_matrix(object.rotation);
            constants.object_to_world[3] = float4{object.position, 1.0f};

            std::memcpy(allocation.cpu_address, &constants,
                        sizeof(world_mesh_constants));

            return allocation.gpu_address;
         }();

         model& model = _model_manager[world_classes.at(object.class_name).model_name];

         command_list.set_graphics_root_signature(
            _root_signatures.mesh_wireframe.get());
         command_list.set_graphics_cbv(rs::mesh_wireframe::object_cbv, object_constants);
         command_list.set_graphics_cbv(rs::mesh_wireframe::wireframe_cbv,
                                       wireframe_constants);
         command_list.set_graphics_cbv(rs::mesh_wireframe::frame_cbv,
                                       _camera_constant_buffer_view);

         command_list.set_pipeline_state(_pipelines.mesh_wireframe.get());

         command_list.ia_set_primitive_topology(gpu::primitive_topology::trianglelist);

         command_list.ia_set_index_buffer(model.gpu_buffer.index_buffer_view);
         command_list.ia_set_vertex_buffers(0, model.gpu_buffer.position_vertex_buffer_view);

         bool doublesided = false;

         for (auto& part : model.parts) {
            if (std::exchange(doublesided,
                              are_flags_set(part.material.flags,
                                            material_pipeline_flags::doublesided)) !=
                doublesided) {
               command_list.set_pipeline_state(
                  doublesided ? _pipelines.mesh_wireframe_doublesided.get()
                              : _pipelines.mesh_wireframe.get());
            }

            command_list.draw_indexed_instanced(part.index_count, 1, part.start_index,
                                                part.start_vertex, 0);
         }
      },
      [&](const world::light& light, gpu_virtual_address wireframe_constants) {
         meta_mesh_common_setup(wireframe_constants);

         if (light.light_type == world::light_type::directional) {
            const float4x4 rotation = to_matrix(light.rotation);
            float4x4 transform = rotation * float4x4{{2.0f, 0.0f, 0.0f, 0.0f},
                                                     {0.0f, 2.0f, 0.0f, 0.0f},
                                                     {0.0f, 0.0f, 2.0f, 0.0f},
                                                     {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {light.position, 1.0f};

            command_list.set_graphics_cbv(
               rs::meta_mesh_wireframe::object_cbv,
               _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

            auto shape = _geometric_shapes.octahedron();

            command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
            command_list.ia_set_index_buffer(shape.index_buffer_view);
            command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
         }
         else if (light.light_type == world::light_type::point) {
            float4x4 transform = float4x4{{light.range, 0.0f, 0.0f, 0.0f},
                                          {0.0f, light.range, 0.0f, 0.0f},
                                          {0.0f, 0.0f, light.range, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {light.position, 1.0f};

            command_list.set_graphics_cbv(
               rs::meta_mesh_wireframe::object_cbv,
               _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

            auto shape = _geometric_shapes.icosphere();

            command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
            command_list.ia_set_index_buffer(shape.index_buffer_view);
            command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
         }
         else if (light.light_type == world::light_type::spot) {
            const float half_range = light.range / 2.0f;
            const float cone_radius = half_range * std::tan(light.outer_cone_angle);

            float4x4 transform =
               to_matrix(light.rotation) *
               to_matrix(quaternion{0.707107f, -0.707107f, 0.0f, 0.0f}) *
               float4x4{{cone_radius, 0.0f, 0.0f, 0.0f},
                        {0.0f, half_range, 0.0f, 0.0f},
                        {0.0f, 0.0f, cone_radius, 0.0f},
                        {0.0f, -half_range, 0.0f, 1.0f}};

            transform[3] += float4{light.position, 0.0f};

            command_list.set_graphics_cbv(
               rs::meta_mesh_wireframe::object_cbv,
               _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

            auto shape = _geometric_shapes.cone();

            command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
            command_list.ia_set_index_buffer(shape.index_buffer_view);
            command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
         }
         else if (light.light_type == world::light_type::directional_region_box) {
            const float3 scale = light.region_size;

            float4x4 transform = to_matrix(light.region_rotation) *
                                 float4x4{{scale.x, 0.0f, 0.0f, 0.0f},
                                          {0.0f, scale.y, 0.0f, 0.0f},
                                          {0.0f, 0.0f, scale.z, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {light.position, 1.0f};

            command_list.set_graphics_cbv(
               rs::meta_mesh_wireframe::object_cbv,
               _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

            const geometric_shape shape = _geometric_shapes.cube();

            command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
            command_list.ia_set_index_buffer(shape.index_buffer_view);
            command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
         }
         else if (light.light_type == world::light_type::directional_region_sphere) {
            const float scale = length(light.region_size);

            float4x4 transform = float4x4{{scale, 0.0f, 0.0f, 0.0f},
                                          {0.0f, scale, 0.0f, 0.0f},
                                          {0.0f, 0.0f, scale, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {light.position, 1.0f};

            command_list.set_graphics_cbv(
               rs::meta_mesh_wireframe::object_cbv,
               _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

            const geometric_shape shape = _geometric_shapes.icosphere();

            command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
            command_list.ia_set_index_buffer(shape.index_buffer_view);
            command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
         }
         else if (light.light_type == world::light_type::directional_region_cylinder) {
            const float cylinder_length =
               length(float2{light.region_size.x, light.region_size.z});
            const float3 scale =
               float3{cylinder_length, light.region_size.y, cylinder_length};

            float4x4 transform = to_matrix(light.region_rotation) *
                                 float4x4{{scale.x, 0.0f, 0.0f, 0.0f},
                                          {0.0f, scale.y, 0.0f, 0.0f},
                                          {0.0f, 0.0f, scale.z, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {light.position, 1.0f};

            command_list.set_graphics_cbv(
               rs::meta_mesh_wireframe::object_cbv,
               _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

            const geometric_shape shape = _geometric_shapes.cylinder();

            command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
            command_list.ia_set_index_buffer(shape.index_buffer_view);
            command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
         }
      },
      [&](const world::path& path, gpu_virtual_address wireframe_constants) {
         meta_mesh_common_setup(wireframe_constants);

         for (auto& node : path.nodes) draw_path_node(node);

         _meta_draw_batcher.clear();

         using namespace ranges::views;

         const auto get_position = [](const world::path::node& node) {
            return node.position;
         };

         const uint32 hover_color =
            utility::pack_srgb_bgra({settings.hover_color, 1.0f});

         for (const auto [a, b] : zip(path.nodes | transform(get_position),
                                      path.nodes | drop(1) | transform(get_position))) {
            _meta_draw_batcher.add_line_solid(a, b, hover_color);
         }

         _meta_draw_batcher.draw(command_list, _camera_constant_buffer_view,
                                 _root_signatures, _pipelines,
                                 _geometric_shapes, _dynamic_buffer_allocator);
      },
      [&](const world::path::node& path_node, gpu_virtual_address wireframe_constants) {
         meta_mesh_common_setup(wireframe_constants);

         draw_path_node(path_node);
      },
      [&](const world::region& region, gpu_virtual_address wireframe_constants) {
         meta_mesh_common_setup(wireframe_constants);

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
            rs::meta_mesh_wireframe::object_cbv,
            _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

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
      },
      [&](const world::sector& sector, gpu_virtual_address wireframe_constants) {
         meta_mesh_common_setup(wireframe_constants);

         command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                       _dynamic_buffer_allocator
                                          .allocate_and_copy(
                                             float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                      {0.0f, 1.0f, 0.0f, 0.0f},
                                                      {0.0f, 0.0f, 1.0f, 0.0f},
                                                      {0.0f, 0.0f, 0.0f, 1.0f}})
                                          .gpu_address);

         using namespace ranges::views;

         for (auto& points = sector.points;
              const auto [a, b] :
              zip(points, concat(points | drop(1), points | take(1)))) {
            const std::array quad = {float3{a.x, sector.base, a.y},
                                     float3{b.x, sector.base, b.y},
                                     float3{a.x, sector.base + sector.height, a.y},
                                     float3{b.x, sector.base + sector.height, b.y}};

            triangle_drawer.add(quad[0], quad[1], quad[2]);
            triangle_drawer.add(quad[2], quad[1], quad[3]);
            triangle_drawer.add(quad[0], quad[2], quad[1]);
            triangle_drawer.add(quad[2], quad[3], quad[1]);
         }

         triangle_drawer.submit();
      },
      [&](const world::portal& portal, gpu_virtual_address wireframe_constants) {
         meta_mesh_common_setup(wireframe_constants);

         command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                       _dynamic_buffer_allocator
                                          .allocate_and_copy(
                                             float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                      {0.0f, 1.0f, 0.0f, 0.0f},
                                                      {0.0f, 0.0f, 1.0f, 0.0f},
                                                      {0.0f, 0.0f, 0.0f, 1.0f}})
                                          .gpu_address);

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

         triangle_drawer.submit();
      },
      [&](const world::hintnode& hintnode, gpu_virtual_address wireframe_constants) {
         meta_mesh_common_setup(wireframe_constants);

         float4x4 transform = to_matrix(hintnode.rotation);
         transform[3] = {hintnode.position, 1.0f};

         command_list.set_graphics_cbv(
            rs::meta_mesh_wireframe::object_cbv,
            _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

         const geometric_shape shape = _geometric_shapes.octahedron();

         command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
         command_list.ia_set_index_buffer(shape.index_buffer_view);
         command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
      },
      [&](const world::barrier& barrier, gpu_virtual_address wireframe_constants) {
         meta_mesh_common_setup(wireframe_constants);

         const geometric_shape shape = _geometric_shapes.cube();

         const float2 position = (barrier.corners[0] + barrier.corners[2]) / 2.0f;
         const float2 size{distance(barrier.corners[0], barrier.corners[3]),
                           distance(barrier.corners[0], barrier.corners[1])};
         const float angle =
            std::atan2(barrier.corners[1].x - barrier.corners[0].x,
                       barrier.corners[1].y - barrier.corners[0].y);

         const float barrier_height = settings.barrier_height;

         float4x4 transform = make_rotation_matrix_from_euler({0.0f, angle, 0.0f}) *
                              float4x4{{size.x / 2.0f, 0.0f, 0.0f, 0.0f},
                                       {0.0f, barrier_height, 0.0f, 0.0f},
                                       {0.0f, 0.0f, size.y / 2.0f, 0.0f},
                                       {0.0f, 0.0f, 0.0f, 1.0f}};
         transform[3] = {position.x, 0.0f, position.y, 1.0f};

         command_list.set_graphics_cbv(
            rs::meta_mesh_wireframe::object_cbv,
            _dynamic_buffer_allocator.allocate_and_copy(transform).gpu_address);

         command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
         command_list.ia_set_index_buffer(shape.index_buffer_view);
         command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
      },
      [&]([[maybe_unused]] const world::planning_hub& planning_hub,
          [[maybe_unused]] gpu_virtual_address wireframe_constants) {},
      [&]([[maybe_unused]] const world::planning_connection& planning_connection,
          [[maybe_unused]] gpu_virtual_address wireframe_constants) {},
      [&](const world::boundary& boundary, gpu_virtual_address wireframe_constants) {
         const world::path* path = look_for(world.paths, [&](const world::path& path) {
            return path.name == boundary.name;
         });

         if (not path) return;

         meta_mesh_common_setup(wireframe_constants);

         command_list.set_graphics_cbv(rs::meta_mesh_wireframe::object_cbv,
                                       _dynamic_buffer_allocator
                                          .allocate_and_copy(
                                             float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                      {0.0f, 1.0f, 0.0f, 0.0f},
                                                      {0.0f, 0.0f, 1.0f, 0.0f},
                                                      {0.0f, 0.0f, 0.0f, 1.0f}})
                                          .gpu_address);

         const float boundary_height = settings.boundary_height;

         using namespace ranges::views;

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

         triangle_drawer.submit();
      },
   };

   const auto draw_target = [&](world::interaction_target target,
                                gpu_virtual_address wireframe_constants) {
      std::visit(
         overload{
            [&](world::object_id id) {
               const world::object* object =
                  look_for(world.objects, [id](const world::object& object) {
                     return id == object.id;
                  });

               if (object) draw_entity(*object, wireframe_constants);
            },
            [&](world::light_id id) {
               const world::light* light =
                  look_for(world.lights, [id](const world::light& light) {
                     return id == light.id;
                  });

               if (light) draw_entity(*light, wireframe_constants);
            },
            [&](world::path_id id) {
               const world::path* path =
                  look_for(world.paths, [id](const world::path& path) {
                     return id == path.id;
                  });

               if (path) draw_entity(*path, wireframe_constants);
            },
            [&](world::path_id_node_pair id_node) {
               auto [id, node_index] = id_node;

               const world::path* path =
                  look_for(world.paths, [id](const world::path& path) {
                     return id == path.id;
                  });

               if (not path) return;

               if (node_index >= path->nodes.size()) return;

               if (path) {
                  draw_entity(path->nodes[node_index], wireframe_constants);
               }
            },
            [&](world::region_id id) {
               const world::region* region =
                  look_for(world.regions, [id](const world::region& region) {
                     return id == region.id;
                  });

               if (region) draw_entity(*region, wireframe_constants);
            },
            [&](world::sector_id id) {
               const world::sector* sector =
                  look_for(world.sectors, [id](const world::sector& sector) {
                     return id == sector.id;
                  });

               if (sector) draw_entity(*sector, wireframe_constants);
            },
            [&](world::portal_id id) {
               const world::portal* portal =
                  look_for(world.portals, [id](const world::portal& portal) {
                     return id == portal.id;
                  });

               if (portal) draw_entity(*portal, wireframe_constants);
            },
            [&](world::hintnode_id id) {
               const world::hintnode* hintnode =
                  look_for(world.hintnodes, [id](const world::hintnode& hintnode) {
                     return id == hintnode.id;
                  });

               if (hintnode) draw_entity(*hintnode, wireframe_constants);
            },
            [&](world::barrier_id id) {
               const world::barrier* barrier =
                  look_for(world.barriers, [id](const world::barrier& barrier) {
                     return id == barrier.id;
                  });

               if (barrier) draw_entity(*barrier, wireframe_constants);
            },
            [&](world::planning_hub_id id) {
               const world::planning_hub* planning_hub =
                  look_for(world.planning_hubs,
                           [id](const world::planning_hub& planning_hub) {
                              return id == planning_hub.id;
                           });

               if (planning_hub) {
                  draw_entity(*planning_hub, wireframe_constants);
               }
            },
            [&](world::planning_connection_id id) {
               const world::planning_connection* planning_connection =
                  look_for(world.planning_connections,
                           [id](const world::planning_connection& planning_connection) {
                              return id == planning_connection.id;
                           });

               if (planning_connection) {
                  draw_entity(*planning_connection, wireframe_constants);
               }
            },
            [&](world::boundary_id id) {
               const world::boundary* boundary =
                  look_for(world.boundaries, [id](const world::boundary& boundary) {
                     return id == boundary.id;
                  });

               if (boundary) draw_entity(*boundary, wireframe_constants);
            },
         },
         target);
   };

   if (interaction_targets.hovered_entity) {
      gpu_virtual_address wireframe_constants = [&] {
         auto allocation =
            _dynamic_buffer_allocator.allocate(sizeof(wireframe_constant_buffer));

         wireframe_constant_buffer constants{.color = settings.hover_color};

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

         wireframe_constant_buffer constants{.color = settings.selected_color};

         std::memcpy(allocation.cpu_address, &constants,
                     sizeof(wireframe_constant_buffer));

         return allocation.gpu_address;
      }();

      for (auto target : interaction_targets.selection) {
         draw_target(target, wireframe_constants);
      }
   }

   if (interaction_targets.creation_entity) {
      gpu_virtual_address wireframe_constants = [&] {
         auto allocation =
            _dynamic_buffer_allocator.allocate(sizeof(wireframe_constant_buffer));

         wireframe_constant_buffer constants{.color = settings.creation_color};

         std::memcpy(allocation.cpu_address, &constants,
                     sizeof(wireframe_constant_buffer));

         return allocation.gpu_address;
      }();

      std::visit(
         [&](const auto& entity) { draw_entity(entity, wireframe_constants); },
         *interaction_targets.creation_entity);
   }
}

void renderer_impl::build_world_mesh_list(
   gpu::copy_command_list& command_list, const world::world& world,
   const world::active_layers active_layers,
   const absl::flat_hash_map<lowercase_string, world::object_class>& world_classes,
   const world::object* const creation_object)
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

   if (creation_object and world.objects.size() < max_drawn_objects) {
      auto& model =
         _model_manager[world_classes.at(creation_object->class_name).model_name];

      const auto object_bbox =
         creation_object->rotation * model.bbox + creation_object->position;

      const std::size_t object_constants_offset = constants_data_size;
      const gpu_virtual_address object_constants_address =
         constants_upload_gpu_address + object_constants_offset;

      world_mesh_constants constants;

      constants.object_to_world = to_matrix(creation_object->rotation);
      constants.object_to_world[3] = float4{creation_object->position, 1.0f};

      std::memcpy(constants_upload_data + object_constants_offset,
                  &constants.object_to_world, sizeof(world_mesh_constants));

      constants_data_size += sizeof(world_mesh_constants);

      for (auto& mesh : model.parts) {
         const gpu::pipeline_handle pipeline =
            _pipelines.mesh_normal[mesh.material.flags].get();

         _world_mesh_list.push_back(
            object_bbox, object_constants_address, creation_object->position,
            pipeline, mesh.material.flags, mesh.material.constant_buffer_view,
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

void renderer_impl::build_object_render_list(const frustum& view_frustum)
{
   auto& meshes = _world_mesh_list;

   cull_objects_avx2(view_frustum, meshes.bbox.min.x, meshes.bbox.min.y,
                     meshes.bbox.min.z, meshes.bbox.max.x, meshes.bbox.max.y,
                     meshes.bbox.max.z, meshes.pipeline_flags,
                     _opaque_object_render_list, _transparent_object_render_list);

   std::sort(_opaque_object_render_list.begin(), _opaque_object_render_list.end(),
             [&](const uint16 l, const uint16 r) {
                return std::tuple{dot(view_frustum.planes[frustum_planes::near_],
                                      float4{meshes.position[l], 1.0f}),
                                  meshes.pipeline[l]} <
                       std::tuple{dot(view_frustum.planes[frustum_planes::near_],
                                      float4{meshes.position[r], 1.0f}),
                                  meshes.pipeline[r]};
             });
   std::sort(_transparent_object_render_list.begin(),
             _transparent_object_render_list.end(),
             [&](const uint16 l, const uint16 r) {
                return dot(view_frustum.planes[frustum_planes::near_],
                           float4{meshes.position[l], 1.0f}) >
                       dot(view_frustum.planes[frustum_planes::near_],
                           float4{meshes.position[r], 1.0f});
             });
}

void renderer_impl::clear_depth_minmax(gpu::copy_command_list& command_list)
{
   const gpu_virtual_address depth_minmax_buffer =
      _device.get_gpu_virtual_address(_depth_minmax_buffer.get());

   command_list.write_buffer_immediate(depth_minmax_buffer,
                                       std::bit_cast<uint32>(1.0f));
   command_list.write_buffer_immediate(depth_minmax_buffer + sizeof(float),
                                       std::bit_cast<uint32>(0.0f));
}

void renderer_impl::reduce_depth_minmax(gpu::graphics_command_list& command_list)
{
   profile_section profile{"Reduce Depth Minmax", command_list, _profiler,
                           profiler_queue::direct};

   command_list.deferred_barrier(
      gpu::texture_barrier{.sync_before = gpu::barrier_sync::depth_stencil,
                           .sync_after = gpu::barrier_sync::compute_shading,
                           .access_before = gpu::barrier_access::depth_stencil_write,
                           .access_after = gpu::barrier_access::shader_resource,
                           .layout_before = gpu::barrier_layout::depth_stencil_write,
                           .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
                           .resource = _depth_stencil_texture.get()});
   command_list.flush_barriers();

   const gpu_virtual_address depth_minmax_buffer =
      _device.get_gpu_virtual_address(_depth_minmax_buffer.get());

   std::array reduce_depth_inputs{_depth_stencil_srv.get().index,
                                  _swap_chain.width(), _swap_chain.height()};

   command_list.set_compute_root_signature(_root_signatures.depth_reduce_minmax.get());
   command_list.set_compute_32bit_constants(rs::depth_reduce_minmax::input_constants,
                                            std::as_bytes(std::span{reduce_depth_inputs}),
                                            0);
   command_list.set_compute_uav(rs::depth_reduce_minmax::output_uav, depth_minmax_buffer);

   command_list.set_pipeline_state(_pipelines.depth_reduce_minmax.get());

   command_list.dispatch(math::align_up(_swap_chain.width() / 8, 8),
                         math::align_up(_swap_chain.height() / 8, 8), 1);

   command_list.deferred_barrier(
      gpu::buffer_barrier{.sync_before = gpu::barrier_sync::compute_shading,
                          .sync_after = gpu::barrier_sync::copy,
                          .access_before = gpu::barrier_access::unordered_access,
                          .access_after = gpu::barrier_access::copy_source,
                          .resource = _depth_minmax_buffer.get()});
   command_list.flush_barriers();

   command_list.copy_buffer_region(_depth_minmax_readback_buffer.get(),
                                   sizeof(float4) * _device.frame_index(),
                                   _depth_minmax_buffer.get(), 0, sizeof(float4));
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

auto make_renderer(const renderer::window_handle window,
                   std::shared_ptr<async::thread_pool> thread_pool,
                   assets::libraries_manager& asset_libraries,
                   output_stream& error_output) -> std::unique_ptr<renderer>
{
   return std::make_unique<renderer_impl>(window, thread_pool, asset_libraries,
                                          error_output);
}

}
