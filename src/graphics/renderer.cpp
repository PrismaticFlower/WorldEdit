
#include "renderer.hpp"
#include "ai_overlay_batches.hpp"
#include "async/thread_pool.hpp"
#include "camera.hpp"
#include "copy_command_list_pool.hpp"
#include "cull_objects.hpp"
#include "dynamic_buffer_allocator.hpp"
#include "frustum.hpp"
#include "geometric_shapes.hpp"
#include "gpu/rhi.hpp"
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
#include "settings/graphics.hpp"
#include "shader_library.hpp"
#include "shader_list.hpp"
#include "sky.hpp"
#include "terrain.hpp"
#include "texture_manager.hpp"
#include "thumbnail_manager.hpp"
#include "ui_texture_manager.hpp"
#include "utility/overload.hpp"
#include "utility/srgb_conversion.hpp"
#include "utility/stopwatch.hpp"
#include "water.hpp"
#include "world/object_class_library.hpp"
#include "world/utility/boundary_nodes.hpp"
#include "world/utility/world_utilities.hpp"
#include "world/world.hpp"

#include <imgui.h>

namespace we::graphics {

namespace {

// TODO: Put this somewhere.
struct alignas(256) frame_constant_buffer {
   float4x4 view_projection_matrix;

   float3 view_positionWS;
   float texture_scroll_duration;

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

struct renderer_impl final : renderer {
   renderer_impl(const renderer_init& init);

   void wait_for_swap_chain_ready() override;

   void draw_frame(const camera& camera, const world::world& world,
                   const world::interaction_targets& interaction_targets,
                   const world::active_entity_types active_entity_types,
                   const world::active_layers active_layers,
                   const world::tool_visualizers& tool_visualizers,
                   const world::object_class_library& world_classes,
                   const draw_frame_options frame_options,
                   const settings::graphics& settings) override;

   auto draw_env_map(const env_map_params& params, const world::world& world,
                     const world::active_layers active_layers,
                     const world::object_class_library& world_classes) -> env_map_result;

   void window_resized(uint16 width, uint16 height) override;

   void display_scale_changed(const float display_scale) override
   {
      _display_scale = display_scale;

      _thumbnail_manager.display_scale_changed(display_scale);
   }

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

   auto request_object_class_thumbnail(const std::string_view name)
      -> object_class_thumbnail override
   {
      return _thumbnail_manager.request_object_class_thumbnail(name);
   }

   void async_save_thumbnail_disk_cache(const wchar_t* path) noexcept override
   {
      _thumbnail_manager.async_save_disk_cache(path);
   }

   void async_load_thumbnail_disk_cache(const wchar_t* path) noexcept override
   {
      _thumbnail_manager.async_load_disk_cache(path);
   }

   void reset_thumbnails() noexcept override
   {
      _thumbnail_manager.reset();
   }

   auto request_imgui_texture_id(const std::string_view name,
                                 const fallback_imgui_texture fallback) noexcept
      -> void* override
   {
      return _ui_texture_manager.request(name, fallback, _texture_manager);
   }

private:
   void update_frame_constant_buffer(const camera& camera, const gpu::viewport viewport,
                                     const bool scroll_textures, const float line_width,
                                     gpu::copy_command_list& command_list);

   void draw_world(const frustum& view_frustum,
                   const world::active_entity_types active_entity_types,
                   gpu::graphics_command_list& command_list);

   void draw_world_render_list_depth_prepass(const std::vector<uint16>& list,
                                             gpu::graphics_command_list& command_list);

   void draw_world_render_list(const std::vector<uint16>& list,
                               gpu::graphics_command_list& command_list);

   void draw_world_meta_objects(const frustum& view_frustum,
                                const camera& camera, const world::world& world,
                                const world::interaction_targets& interaction_targets,
                                const world::active_entity_types active_entity_types,
                                const world::active_layers active_layers,
                                const world::tool_visualizers& tool_visualizers,
                                const settings::graphics& settings);

   void draw_terrain_cut_visualizers(const frustum& view_frustum,
                                     const settings::graphics& settings,
                                     gpu::graphics_command_list& command_list);

   void draw_grid_overlay(const float height, const float length,
                          const float grid_scale, float3 camera_positionWS,
                          const settings::graphics& settings,
                          gpu::graphics_command_list& command_list);

   void draw_ai_overlay(gpu::rtv_handle back_buffer_rtv,
                        const settings::graphics& settings,
                        gpu::graphics_command_list& command_list);

   void draw_interaction_targets(const frustum& view_frustum, const world::world& world,
                                 const world::interaction_targets& interaction_targets,
                                 const world::object_class_library& world_classes,
                                 const settings::graphics& settings,
                                 gpu::graphics_command_list& command_list);

   void build_world_mesh_list(gpu::copy_command_list& command_list,
                              const world::world& world,
                              const world::active_layers active_layers,
                              const world::object_class_library& world_classes,
                              const world::object* const creation_object,
                              std::span<const world::tool_visualizers_ghost> ghost_objects);

   void build_object_render_list(const frustum& view_frustum);

   void clear_depth_minmax(gpu::copy_command_list& command_list);

   void reduce_depth_minmax(gpu::graphics_command_list& command_list);

   void update_textures(gpu::copy_command_list& command_list);

   std::shared_ptr<async::thread_pool> _thread_pool;
   output_stream& _error_output;
   float _display_scale = 1.0f;

   gpu::device _device;
   gpu::swap_chain _swap_chain;
   gpu::copy_command_list _pre_render_command_list = _device.create_copy_command_list(
      {.debug_name = "Pre-Render Copy Command List"});
   gpu::graphics_command_list _world_command_list = _device.create_graphics_command_list(
      {.debug_name = "World Command List"});

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
                               .format = DXGI_FORMAT_R32G8X24_TYPELESS,
                               .width = _swap_chain.width(),
                               .height = _swap_chain.height(),
                               .optimized_clear_value =
                                  {.format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
                                   .depth_stencil = {.depth = 0.0f, .stencil = 0x0}}},
                              gpu::barrier_layout::direct_queue_shader_resource,
                              gpu::legacy_resource_state::all_shader_resource),
       _device.direct_queue};
   gpu::unique_dsv_handle _depth_stencil_view =
      {_device.create_depth_stencil_view(_depth_stencil_texture.get(),
                                         {.format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
                                          .dimension = gpu::dsv_dimension::texture2d}),
       _device.direct_queue};
   gpu::unique_resource_view _depth_stencil_srv =
      {_device.create_shader_resource_view(_depth_stencil_texture.get(),
                                           {.format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS}),
       _device.direct_queue};
   gpu::unique_resource_handle _depth_minmax_buffer =
      {_device.create_buffer({.size = sizeof(float4),
                              .flags = {.allow_unordered_access = true},
                              .debug_name = "Depth Min-Max Buffer"},
                             gpu::heap_type::default_),
       _device.direct_queue};
   gpu::unique_resource_handle _depth_minmax_readback_buffer =
      {_device.create_buffer({.size = sizeof(float4) * gpu::frame_pipeline_length,
                              .debug_name = "Depth Min-Max Readback Buffer"},
                             gpu::heap_type::readback),
       _device.direct_queue};
   std::array<const float4*, gpu::frame_pipeline_length> _depth_minmax_readback_buffer_ptrs;

   shader_library _shaders{shader_list, _device.supports_shader_model_6_6(),
                           _thread_pool, _error_output};
   root_signature_library _root_signatures{_device};
   pipeline_library _pipelines{_device, _shaders, _root_signatures};

   texture_manager _texture_manager;
   model_manager _model_manager;
   geometric_shapes _geometric_shapes{_device, _copy_command_list_pool};
   light_clusters _light_clusters{_device, _copy_command_list_pool,
                                  _swap_chain.width(), _swap_chain.height()};
   terrain _terrain{_device, _copy_command_list_pool, _dynamic_buffer_allocator,
                    _texture_manager};
   water _water{_device, _texture_manager};
   sky _sky;

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

   std::vector<terrain_cut> _terrain_cut_list;

   meta_draw_batcher _meta_draw_batcher;
   ai_overlay_batches _ai_overlay_batches;

   imgui_renderer _imgui_renderer{_device, _copy_command_list_pool};

   utility::stopwatch _texture_scroll_timer;

   profiler _profiler{_device, 256};

   thumbnail_manager _thumbnail_manager;
   ui_texture_manager _ui_texture_manager;
};

renderer_impl::renderer_impl(const renderer_init& init)
   : _thread_pool{init.thread_pool},
     _error_output{init.error_output},
     _display_scale{init.display_scale},
     _device{gpu::device_desc{.enable_debug_layer = init.use_debug_layer,
                              .enable_gpu_based_validation = init.use_debug_layer,
                              .force_legacy_barriers = init.use_legacy_barriers,
                              .force_no_shader_model_6_6 = init.never_use_shader_model_6_6,
                              .force_no_open_existing_heap = init.never_use_open_existing_heap,
                              .force_no_write_buffer_immediate =
                                 init.never_use_write_buffer_immediate}},
     _swap_chain{_device.create_swap_chain({.window = init.window})},
     _texture_manager{_device, _copy_command_list_pool, init.thread_pool,
                      init.asset_libraries.textures, init.error_output},
     _model_manager{_device,          _copy_command_list_pool,
                    _texture_manager, init.asset_libraries.models,
                    init.thread_pool, _error_output},
     _sky{_device, _model_manager, init.asset_libraries},
     _thumbnail_manager{
        thumbnail_manager_init{init.thread_pool, init.asset_libraries,
                               init.error_output, _device, init.display_scale}}
{
   // create object constants upload buffers
   for (std::size_t i = 0; i < _object_constants_upload_buffers.size(); ++i) {
      _object_constants_upload_buffers[i] = {
         _device.create_buffer({.size = objects_constants_buffer_size,
                                .debug_name = "Object Constant Upload Buffers"},
                               gpu::heap_type::upload),
         _device.direct_queue};

      _object_constants_upload_cpu_ptrs[i] = static_cast<std::byte*>(
         _device.map(_object_constants_upload_buffers[i].get(), 0, {}));
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

void renderer_impl::draw_frame(const camera& camera, const world::world& world,
                               const world::interaction_targets& interaction_targets,
                               const world::active_entity_types active_entity_types,
                               const world::active_layers active_layers,
                               const world::tool_visualizers& tool_visualizers,
                               const world::object_class_library& world_classes,
                               const draw_frame_options frame_options,
                               const settings::graphics& settings)
{
   _device.new_frame();
   _ui_texture_manager.new_frame();

   const frustum view_frustum{camera.inv_view_projection_matrix()};
   const gpu::viewport viewport{.width = static_cast<float>(_swap_chain.width()),
                                .height = static_cast<float>(_swap_chain.height())};

   auto [back_buffer, back_buffer_rtv] = _swap_chain.current_back_buffer();
   _dynamic_buffer_allocator.reset(_device.frame_index());

   _model_manager.update_models();
   _sky.update(world.name);
   _thumbnail_manager.update_cpu_cache();

   if (settings.show_profiler) _profiler.show();

   // Pre-Render Work
   {
      _pre_render_command_list.reset();

      update_textures(_pre_render_command_list);

      if (active_entity_types.objects) {
         build_world_mesh_list(_pre_render_command_list, world, active_layers,
                               world_classes,
                               interaction_targets.creation_entity.is<world::object>()
                                  ? &interaction_targets.creation_entity.get<world::object>()
                                  : nullptr,
                               tool_visualizers.ghost_objects());
      }
      else {
         _world_mesh_list.clear();
         _terrain_cut_list.clear();
      }

      update_frame_constant_buffer(camera, viewport, true, settings.line_width,
                                   _pre_render_command_list);
      clear_depth_minmax(_pre_render_command_list);

      const float4 scene_depth_min_max =
         *_depth_minmax_readback_buffer_ptrs[_device.frame_index()];

      _terrain.update(world.terrain, _pre_render_command_list,
                      _dynamic_buffer_allocator, _texture_manager, settings);
      _water.update(world, _pre_render_command_list, _dynamic_buffer_allocator,
                    _texture_manager);
      _light_clusters
         .prepare_lights(camera, view_frustum, world,
                         interaction_targets.creation_entity.is<world::light>()
                            ? &interaction_targets.creation_entity.get<world::light>()
                            : nullptr,
                         {scene_depth_min_max.x, scene_depth_min_max.y},
                         _pre_render_command_list, _dynamic_buffer_allocator);

      _pre_render_command_list.close();

      if (gpu::frame_pipeline_length > 2) {
         _device.copy_queue.sync_with(_device.direct_queue);
      }

      _device.copy_queue.execute_command_lists(_pre_render_command_list);
      _device.direct_queue.sync_with(_device.copy_queue);
   }

   build_object_render_list(view_frustum);

   auto& command_list = _world_command_list;

   command_list.reset();

   _thumbnail_manager.update_gpu(_model_manager, _root_signatures, _pipelines,
                                 _dynamic_buffer_allocator, command_list);

   _light_clusters.tile_lights(_root_signatures, _pipelines, command_list,
                               _dynamic_buffer_allocator, _profiler);
   _light_clusters.draw_shadow_maps(_world_mesh_list, _root_signatures,
                                    _pipelines, command_list,
                                    _dynamic_buffer_allocator, _profiler);

   [[likely]] if (_device.supports_enhanced_barriers()) {
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
   }
   else {
      command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
         .resource = back_buffer,
         .state_before = gpu::legacy_resource_state::present,
         .state_after = gpu::legacy_resource_state::render_target});

      command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
         .resource = _depth_stencil_texture.get(),
         .state_before = gpu::legacy_resource_state::all_shader_resource,
         .state_after = gpu::legacy_resource_state::depth_write});
   }
   command_list.flush_barriers();

   command_list.clear_render_target_view(back_buffer_rtv,
                                         float4{0.0f, 0.0f, 0.0f, 1.0f});
   command_list.clear_depth_stencil_view(_depth_stencil_view.get(),
                                         {.clear_depth = true, .clear_stencil = true},
                                         0.0f, 0x0);

   command_list.rs_set_viewports(viewport);
   command_list.rs_set_scissor_rects(
      {.right = _swap_chain.width(), .bottom = _swap_chain.height()});
   command_list.om_set_render_targets(back_buffer_rtv, _depth_stencil_view.get());

   // Render World
   draw_world(view_frustum, active_entity_types, command_list);

   // Render World Meta Objects
   _meta_draw_batcher.clear();
   _ai_overlay_batches.clear();

   if (frame_options.draw_foliage_map_overlay) {
      _terrain.draw(terrain_draw::foliage_map, view_frustum, _terrain_cut_list,
                    _camera_constant_buffer_view,
                    _light_clusters.lights_constant_buffer_view(), command_list,
                    _root_signatures, _pipelines, _dynamic_buffer_allocator);
   }

   if (frame_options.draw_terrain_grid) {
      _terrain.draw(terrain_draw::grid, view_frustum, _terrain_cut_list,
                    _camera_constant_buffer_view,
                    _light_clusters.lights_constant_buffer_view(), command_list,
                    _root_signatures, _pipelines, _dynamic_buffer_allocator);
   }

   if (frame_options.draw_overlay_grid) {
      const float grid_height_bias =
         camera.position().y >= frame_options.overlay_grid_height ? 0.002f : -0.002f;

      draw_grid_overlay(frame_options.overlay_grid_height + grid_height_bias,
                        16384.0f, frame_options.overlay_grid_size,
                        camera.position(), settings, command_list);
   }

   if (settings.visualize_terrain_cutters) {
      draw_terrain_cut_visualizers(view_frustum, settings, command_list);
   }

   draw_world_meta_objects(view_frustum, camera, world, interaction_targets,
                           active_entity_types, active_layers, tool_visualizers,
                           settings);

   draw_ai_overlay(back_buffer_rtv, settings, command_list);

   draw_interaction_targets(view_frustum, world, interaction_targets,
                            world_classes, settings, command_list);

   _meta_draw_batcher.draw(command_list, _camera_constant_buffer_view,
                           _root_signatures, _pipelines, _geometric_shapes,
                           _dynamic_buffer_allocator);

   // Render ImGui
   ImGui::Render();
   _imgui_renderer.render_draw_data(ImGui::GetDrawData(), _root_signatures,
                                    _pipelines, command_list);

   reduce_depth_minmax(command_list);

   _profiler.end_frame(command_list);

   [[likely]] if (_device.supports_enhanced_barriers()) {
      command_list.deferred_barrier(
         gpu::texture_barrier{.sync_before = gpu::barrier_sync::render_target,
                              .sync_after = gpu::barrier_sync::none,
                              .access_before = gpu::barrier_access::render_target,
                              .access_after = gpu::barrier_access::no_access,
                              .layout_before = gpu::barrier_layout::render_target,
                              .layout_after = gpu::barrier_layout::present,
                              .resource = back_buffer});
   }
   else {
      command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
         .resource = back_buffer,
         .state_before = gpu::legacy_resource_state::render_target,
         .state_after = gpu::legacy_resource_state::present});
   }

   command_list.flush_barriers();

   command_list.close();

   _device.direct_queue.execute_command_lists(command_list);

   _swap_chain.present(false);

   _device.end_frame();
   _model_manager.trim_models();
   _thumbnail_manager.end_frame();
}

auto renderer_impl::draw_env_map(const env_map_params& params, const world::world& world,
                                 const world::active_layers active_layers,
                                 const world::object_class_library& world_classes)
   -> env_map_result
{
   _device.new_frame();
   _dynamic_buffer_allocator.reset(_device.frame_index());

   const float pi = 3.1415927f;
   const float half_pi = 1.5707964f;

   const std::array<float2, 6> camera_angles{float2{-half_pi, 0.0f},
                                             float2{half_pi, 0.0f},
                                             float2{0.0f, half_pi},
                                             float2{0.0f, -half_pi},
                                             float2{0.0f, 0.0f},
                                             float2{pi, 0.0f}};

   const uint32 super_sample_length = params.length * 4;

   _light_clusters.update_render_resolution(super_sample_length, super_sample_length);

   gpu::unique_resource_handle env_map_super_sample_render_texture =
      {_device.create_texture(
          {.dimension = gpu::texture_dimension::t_2d,
           .flags = {.allow_render_target = true},
           .format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
           .width = super_sample_length,
           .height = super_sample_length,
           .optimized_clear_value = {.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                                     .color = float4{0.0f, 0.0f, 0.0f, 1.0f}},
           .debug_name = "Env Map Super Sample Render Target"},
          gpu::barrier_layout::render_target, gpu::legacy_resource_state::render_target),
       _device.direct_queue};
   gpu::unique_rtv_handle env_map_super_sample_rtv =
      {_device.create_render_target_view(env_map_super_sample_render_texture.get(),
                                         {.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                                          .dimension = gpu::rtv_dimension::texture2d}),
       _device.direct_queue};
   gpu::unique_resource_view env_map_super_sample_srv =
      {_device.create_shader_resource_view(env_map_super_sample_render_texture.get(),
                                           {.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB}),
       _device.direct_queue};

   gpu::unique_resource_handle env_map_render_texture =
      {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                               .flags = {.allow_render_target = true},
                               .format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                               .width = params.length,
                               .height = params.length,
                               .array_size = 6,
                               .optimized_clear_value =
                                  {.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                                   .color = float4{0.0f, 0.0f, 0.0f, 1.0f}},
                               .debug_name = "Env Map Render Target"},
                              gpu::barrier_layout::render_target,
                              gpu::legacy_resource_state::render_target),
       _device.direct_queue};
   gpu::unique_resource_view env_map_srv =
      {_device.create_shader_resource_view(env_map_render_texture.get(),
                                           {.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                                            .texture_cube_view = true}),
       _device.direct_queue};

   gpu::unique_resource_handle env_map_resample_render_texture =
      {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                               .flags = {.allow_render_target = true},
                               .format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                               .width = params.length,
                               .height = params.length,
                               .array_size = 6,
                               .optimized_clear_value =
                                  {.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                                   .color = float4{0.0f, 0.0f, 0.0f, 1.0f}},
                               .debug_name = "Env Map Resample Render Target"},
                              gpu::barrier_layout::render_target,
                              gpu::legacy_resource_state::render_target),
       _device.direct_queue};
   gpu::unique_rtv_handle env_map_resample_rtv =
      {_device.create_render_target_view(env_map_resample_render_texture.get(),
                                         {.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                                          .dimension = gpu::rtv_dimension::texture2d_array,
                                          .texture2d_array = {.first_array_slice = 0,
                                                              .array_size = 6}}),
       _device.direct_queue};

   gpu::unique_resource_handle env_map_depth_stencil_texture =
      {_device.create_texture({.dimension = gpu::texture_dimension::t_2d,
                               .flags = {.allow_depth_stencil = true},
                               .format = DXGI_FORMAT_R32G8X24_TYPELESS,
                               .width = super_sample_length,
                               .height = super_sample_length,
                               .optimized_clear_value =
                                  {.format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
                                   .depth_stencil = {.depth = 0.0f, .stencil = 0x0}},
                               .debug_name = "Env Map Depth Stencil Target"},
                              gpu::barrier_layout::depth_stencil_write,
                              gpu::legacy_resource_state::depth_write),
       _device.direct_queue};
   gpu::unique_dsv_handle env_map_depth_stencil_view =
      {_device.create_depth_stencil_view(env_map_depth_stencil_texture.get(),
                                         {.format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
                                          .dimension = gpu::dsv_dimension::texture2d}),
       _device.direct_queue};
   gpu::unique_resource_view env_map_depth_stencil_srv =
      {_device.create_shader_resource_view(env_map_depth_stencil_texture.get(),
                                           {.format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS}),
       _device.direct_queue};

   gpu::unique_resource_handle env_map_depth_min_max_readback_buffer =
      {_device.create_buffer({.size = sizeof(float4) * 6, .debug_name = "Env Map Depth Min-Max Readback Buffer"},
                             gpu::heap_type::readback),
       _device.direct_queue};

   const uint32 readback_row_pitch =
      math::align_up<uint32>(params.length * sizeof(uint32),
                             gpu::texture_data_pitch_alignment);
   const uint32 readback_item_pitch = readback_row_pitch * params.length;

   gpu::unique_resource_handle readback_buffer =
      {_device.create_buffer({.size = readback_item_pitch * 6,
                              .debug_name = "Env Map Readback Buffer"},
                             gpu::heap_type::readback),
       _device.direct_queue};

   gpu::copy_command_list pre_render_command_list = _device.create_copy_command_list(
      {.debug_name = "Pre-Env Map Render Copy Command List"});
   gpu::graphics_command_list command_list = _device.create_graphics_command_list(
      {.debug_name = "Env Map Command List"});

   // Pre-Render Work
   {
      pre_render_command_list.reset();

      build_world_mesh_list(pre_render_command_list, world, active_layers,
                            world_classes, nullptr, {});

      pre_render_command_list.close();

      _device.copy_queue.execute_command_lists(pre_render_command_list);
   }

   for (uint32 i = 0; i < 6; ++i) {
      camera camera;

      camera.position(params.positionWS);
      camera.fov(half_pi);
      camera.yaw(camera_angles[i].x);
      camera.pitch(camera_angles[i].y);

      const frustum view_frustum{camera.inv_view_projection_matrix()};
      const gpu::viewport viewport{.width = static_cast<float>(super_sample_length),
                                   .height = static_cast<float>(super_sample_length)};

      build_object_render_list(view_frustum);

      command_list.reset();

      clear_depth_minmax(command_list);

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::buffer_barrier{.sync_before = gpu::barrier_sync::copy,
                                .sync_after = gpu::barrier_sync::compute_shading,
                                .access_before = gpu::barrier_access::copy_dest,
                                .access_after = gpu::barrier_access::unordered_access,
                                .resource = _depth_minmax_buffer.get(),
                                .offset = 0,
                                .size = sizeof(float4)});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _depth_minmax_buffer.get(),
            .state_before = gpu::legacy_resource_state::copy_dest,
            .state_after = gpu::legacy_resource_state::unordered_access});
      }

      command_list.flush_barriers();

      command_list.clear_depth_stencil_view(env_map_depth_stencil_view.get(),
                                            {.clear_depth = true, .clear_stencil = true},
                                            0.0f, 0x0);

      command_list.rs_set_viewports(viewport);
      command_list.rs_set_scissor_rects(
         {.right = super_sample_length, .bottom = super_sample_length});
      command_list.om_set_render_targets(env_map_super_sample_rtv.get(),
                                         env_map_depth_stencil_view.get());

      _terrain.draw(terrain_draw::depth_prepass, view_frustum,
                    _terrain_cut_list, _camera_constant_buffer_view,
                    _light_clusters.lights_constant_buffer_view(), command_list,
                    _root_signatures, _pipelines, _dynamic_buffer_allocator);

      draw_world_render_list_depth_prepass(_opaque_object_render_list, command_list);

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::depth_stencil,
                                 .sync_after = gpu::barrier_sync::compute_shading,
                                 .access_before = gpu::barrier_access::depth_stencil_write,
                                 .access_after = gpu::barrier_access::shader_resource,
                                 .layout_before = gpu::barrier_layout::depth_stencil_write,
                                 .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
                                 .resource = env_map_depth_stencil_texture.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = env_map_depth_stencil_texture.get(),
            .state_before = gpu::legacy_resource_state::depth_write,
            .state_after = gpu::legacy_resource_state::all_shader_resource});
      }
      command_list.flush_barriers();

      const gpu_virtual_address depth_minmax_buffer =
         _device.get_gpu_virtual_address(_depth_minmax_buffer.get());

      std::array reduce_depth_inputs{_depth_stencil_srv.get().index,
                                     super_sample_length, super_sample_length};

      command_list.set_compute_root_signature(
         _root_signatures.depth_reduce_minmax.get());
      command_list.set_compute_32bit_constants(rs::depth_reduce_minmax::input_constants,
                                               std::as_bytes(std::span{reduce_depth_inputs}),
                                               0);
      command_list.set_compute_uav(rs::depth_reduce_minmax::output_uav,
                                   depth_minmax_buffer);

      command_list.set_pipeline_state(_pipelines.depth_reduce_minmax.get());

      command_list.dispatch(math::align_up(super_sample_length / 8, 8),
                            math::align_up(super_sample_length / 8, 8), 1);

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(
            gpu::buffer_barrier{.sync_before = gpu::barrier_sync::compute_shading,
                                .sync_after = gpu::barrier_sync::copy,
                                .access_before = gpu::barrier_access::unordered_access,
                                .access_after = gpu::barrier_access::copy_source,
                                .resource = _depth_minmax_buffer.get()});

         command_list.deferred_barrier(
            gpu::texture_barrier{.sync_before = gpu::barrier_sync::compute_shading,
                                 .sync_after = gpu::barrier_sync::none,
                                 .access_before = gpu::barrier_access::shader_resource,
                                 .access_after = gpu::barrier_access::no_access,
                                 .layout_before = gpu::barrier_layout::direct_queue_shader_resource,
                                 .layout_after = gpu::barrier_layout::depth_stencil_write,
                                 .resource = env_map_depth_stencil_texture.get()});
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = _depth_minmax_buffer.get(),
            .state_before = gpu::legacy_resource_state::unordered_access,
            .state_after = gpu::legacy_resource_state::copy_source});

         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = env_map_depth_stencil_texture.get(),
            .state_before = gpu::legacy_resource_state::all_shader_resource,
            .state_after = gpu::legacy_resource_state::depth_write});
      }
      command_list.flush_barriers();

      command_list.copy_buffer_region(env_map_depth_min_max_readback_buffer.get(),
                                      sizeof(float4) * i,
                                      _depth_minmax_buffer.get(), 0, sizeof(float4));

      command_list.close();

      _device.direct_queue.execute_command_lists(command_list);
   }

   _device.direct_queue.wait_for_idle();

   float shadow_min_depth = 1.0f;
   float shadow_max_depth = 0.0f;

   {
      const float4* min_maxes = static_cast<const float4*>(
         _device.map(env_map_depth_min_max_readback_buffer.get(), 0,
                     {0, sizeof(float4) * 6}));

      for (uint32 i = 0; i < 6; ++i) {
         shadow_min_depth = std::min(min_maxes[i].x, shadow_min_depth);
         shadow_max_depth = std::max(min_maxes[i].y, shadow_max_depth);
      }

      _device.unmap(env_map_depth_min_max_readback_buffer.get(), 0, {0, 0});
   }

   for (uint32 i = 0; i < 6; ++i) {
      gpu::unique_rtv_handle env_map_rtv =
         {_device.create_render_target_view(env_map_render_texture.get(),
                                            {.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                                             .dimension = gpu::rtv_dimension::texture2d_array,
                                             .texture2d_array = {.first_array_slice = i,
                                                                 .array_size = 1}}),
          _device.direct_queue};

      camera camera;

      camera.position(params.positionWS);
      camera.fov(half_pi);
      camera.yaw(camera_angles[i].x);
      camera.pitch(camera_angles[i].y);

      const frustum view_frustum{camera.inv_view_projection_matrix()};
      const gpu::viewport viewport{.width = static_cast<float>(super_sample_length),
                                   .height = static_cast<float>(super_sample_length)};
      // Pre-Render Work
      {
         pre_render_command_list.reset();

         update_frame_constant_buffer(camera, viewport, false, 1.0f,
                                      pre_render_command_list);

         _light_clusters.prepare_lights(camera, view_frustum, world, nullptr,
                                        {shadow_min_depth, shadow_max_depth},
                                        pre_render_command_list,
                                        _dynamic_buffer_allocator);

         pre_render_command_list.close();

         _device.copy_queue.sync_with(_device.direct_queue);
         _device.copy_queue.execute_command_lists(pre_render_command_list);
         _device.direct_queue.sync_with(_device.copy_queue);
      }

      build_object_render_list(view_frustum);

      command_list.reset();

      _thumbnail_manager.update_gpu(_model_manager, _root_signatures, _pipelines,
                                    _dynamic_buffer_allocator, command_list);

      _light_clusters.tile_lights(_root_signatures, _pipelines, command_list,
                                  _dynamic_buffer_allocator, _profiler);
      _light_clusters.draw_shadow_maps(_world_mesh_list, _root_signatures,
                                       _pipelines, command_list,
                                       _dynamic_buffer_allocator, _profiler);

      command_list.flush_barriers();

      command_list.clear_render_target_view(env_map_super_sample_rtv.get(),
                                            float4{0.0f, 0.0f, 0.0f, 1.0f});
      command_list.clear_depth_stencil_view(env_map_depth_stencil_view.get(),
                                            {.clear_depth = true, .clear_stencil = true},
                                            0.0f, 0x0);

      command_list.rs_set_viewports(viewport);
      command_list.rs_set_scissor_rects(
         {.right = super_sample_length, .bottom = super_sample_length});
      command_list.om_set_render_targets(env_map_super_sample_rtv.get(),
                                         env_map_depth_stencil_view.get());

      draw_world(view_frustum, {.objects = true, .terrain = true}, command_list);

      // Downsample to desired size.

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(gpu::texture_barrier{
            .sync_before = gpu::barrier_sync::render_target,
            .sync_after = gpu::barrier_sync::pixel_shading,
            .access_before = gpu::barrier_access::render_target,
            .access_after = gpu::barrier_access::shader_resource,
            .layout_before = gpu::barrier_layout::render_target,
            .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
            .resource = env_map_super_sample_render_texture.get(),
         });
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = env_map_super_sample_render_texture.get(),
            .state_before = gpu::legacy_resource_state::render_target,
            .state_after = gpu::legacy_resource_state::pixel_shader_resource});
      }

      command_list.flush_barriers();

      command_list.rs_set_viewports(
         gpu::viewport{.width = static_cast<float>(params.length),
                       .height = static_cast<float>(params.length)});
      command_list.rs_set_scissor_rects(
         {.right = params.length, .bottom = params.length});

      command_list.om_set_render_targets(env_map_rtv.get());

      command_list.set_graphics_root_signature(_root_signatures.resample_env_map.get());
      command_list.set_graphics_32bit_constant(rs::resample_env_map::env_map,
                                               env_map_super_sample_srv.get().index,
                                               0);

      command_list.set_pipeline_state(_pipelines.env_map_downsample.get());

      command_list.draw_instanced(3, 1, 0, 0);

      [[likely]] if (_device.supports_enhanced_barriers()) {
         command_list.deferred_barrier(gpu::texture_barrier{
            .sync_before = gpu::barrier_sync::pixel_shading,
            .sync_after = gpu::barrier_sync::none,
            .access_before = gpu::barrier_access::shader_resource,
            .access_after = gpu::barrier_access::no_access,
            .layout_before = gpu::barrier_layout::direct_queue_shader_resource,
            .layout_after = gpu::barrier_layout::render_target,
            .resource = env_map_super_sample_render_texture.get(),
         });
      }
      else {
         command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
            .resource = env_map_super_sample_render_texture.get(),
            .state_before = gpu::legacy_resource_state::pixel_shader_resource,
            .state_after = gpu::legacy_resource_state::render_target});
      }

      // Correct orientation and copy to CPU.
      if (i == 5) {
         [[likely]] if (_device.supports_enhanced_barriers()) {
            command_list.deferred_barrier(gpu::texture_barrier{
               .sync_before = gpu::barrier_sync::render_target,
               .sync_after = gpu::barrier_sync::pixel_shading,
               .access_before = gpu::barrier_access::render_target,
               .access_after = gpu::barrier_access::shader_resource,
               .layout_before = gpu::barrier_layout::render_target,
               .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
               .resource = env_map_render_texture.get(),
            });
         }
         else {
            command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
               .resource = env_map_render_texture.get(),
               .state_before = gpu::legacy_resource_state::render_target,
               .state_after = gpu::legacy_resource_state::pixel_shader_resource});
         }

         command_list.flush_barriers();

         command_list.om_set_render_targets(env_map_resample_rtv.get());

         command_list.set_graphics_32bit_constant(rs::resample_env_map::env_map,
                                                  env_map_srv.get().index, 0);

         command_list.set_pipeline_state(_pipelines.resample_env_map.get());

         command_list.draw_instanced(3 * 6, 1, 0, 0);

         [[likely]] if (_device.supports_enhanced_barriers()) {
            command_list.deferred_barrier(gpu::texture_barrier{
               .sync_before = gpu::barrier_sync::render_target,
               .sync_after = gpu::barrier_sync::copy,
               .access_before = gpu::barrier_access::render_target,
               .access_after = gpu::barrier_access::copy_source,
               .layout_before = gpu::barrier_layout::render_target,
               .layout_after = gpu::barrier_layout::direct_queue_copy_source,
               .resource = env_map_resample_render_texture.get(),
            });
         }
         else {
            command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
               .resource = env_map_resample_render_texture.get(),
               .state_before = gpu::legacy_resource_state::render_target,
               .state_after = gpu::legacy_resource_state::copy_source});
         }

         command_list.flush_barriers();

         for (uint32 subresource = 0; subresource < 6; ++subresource) {
            command_list.copy_texture_to_buffer(readback_buffer.get(),
                                                {.offset = readback_item_pitch * subresource,
                                                 .format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
                                                 .width = params.length,
                                                 .height = params.length,
                                                 .depth = 1,
                                                 .row_pitch = readback_row_pitch},
                                                0, 0, 0,
                                                env_map_resample_render_texture.get(),
                                                subresource);
         }
      }
      else {
         command_list.flush_barriers();
      }

      command_list.close();

      _device.direct_queue.execute_command_lists(command_list);
   }

   _device.end_frame();

   _light_clusters.update_render_resolution(_swap_chain.width(), _swap_chain.height());

   _device.direct_queue.wait_for_idle();

   const uint32 data_size = readback_item_pitch * 6;

   std::unique_ptr<std::byte[]> data =
      std::make_unique_for_overwrite<std::byte[]>(data_size);

   void* mapped_env_map = _device.map(readback_buffer.get(), 0, {0, data_size});

   std::memcpy(data.get(), mapped_env_map, data_size);

   _device.unmap(readback_buffer.get(), 0, {});

   return {.length = params.length,
           .row_pitch = readback_row_pitch,
           .item_pitch = readback_item_pitch,
           .data = std::move(data)};
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
                               .format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
                               .width = _swap_chain.width(),
                               .height = _swap_chain.height(),
                               .optimized_clear_value =
                                  {.format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
                                   .depth_stencil = {.depth = 0.0f, .stencil = 0x0}}},
                              gpu::barrier_layout::direct_queue_shader_resource,
                              gpu::legacy_resource_state::all_shader_resource),
       _device.direct_queue};
   _depth_stencil_view =
      {_device.create_depth_stencil_view(_depth_stencil_texture.get(),
                                         {.format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
                                          .dimension = gpu::dsv_dimension::texture2d}),
       _device.direct_queue};
   _depth_stencil_srv = {_device.create_shader_resource_view(
                            _depth_stencil_texture.get(),
                            {.format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS}),
                         _device.direct_queue};

   _light_clusters.update_render_resolution(width, height);
}

void renderer_impl::update_frame_constant_buffer(const camera& camera,
                                                 const gpu::viewport viewport,
                                                 const bool scroll_textures,
                                                 const float line_width,
                                                 gpu::copy_command_list& command_list)
{
   frame_constant_buffer constants{
      .view_projection_matrix = camera.view_projection_matrix(),

      .view_positionWS = camera.position(),
      .texture_scroll_duration =
         scroll_textures
            ? static_cast<float>(std::fmod(_texture_scroll_timer.elapsed_f64(), 255.0))
            : 0.0f,

      .viewport_size = {viewport.width, viewport.height},
      .viewport_topleft = {viewport.top_left_x, viewport.top_left_y},

      .line_width = line_width * _display_scale};

   auto allocation = _dynamic_buffer_allocator.allocate_and_copy(constants);

   command_list.copy_buffer_region(_camera_constant_buffer.get(), 0,
                                   _dynamic_buffer_allocator.resource(),
                                   allocation.offset, sizeof(frame_constant_buffer));
}

void renderer_impl::draw_world(const frustum& view_frustum,
                               const world::active_entity_types active_entity_types,
                               gpu::graphics_command_list& command_list)
{
   if (active_entity_types.terrain) {
      profile_section profile{"Terrain - Draw Depth Prepass", command_list,
                              _profiler, profiler_queue::direct};

      _terrain.draw(terrain_draw::depth_prepass, view_frustum,
                    _terrain_cut_list, _camera_constant_buffer_view,
                    _light_clusters.lights_constant_buffer_view(), command_list,
                    _root_signatures, _pipelines, _dynamic_buffer_allocator);
   }

   if (active_entity_types.objects) {
      profile_section profile{"World - Draw Render List Depth Prepass",
                              command_list, _profiler, profiler_queue::direct};

      draw_world_render_list_depth_prepass(_opaque_object_render_list, command_list);
   }

   if (active_entity_types.objects) {
      profile_section profile{"World - Draw Opaque", command_list, _profiler,
                              profiler_queue::direct};

      draw_world_render_list(_opaque_object_render_list, command_list);
   }

   if (active_entity_types.terrain) {
      profile_section profile{"Terrain - Draw", command_list, _profiler,
                              profiler_queue::direct};

      _terrain.draw(terrain_draw::main, view_frustum, _terrain_cut_list,
                    _camera_constant_buffer_view,
                    _light_clusters.lights_constant_buffer_view(), command_list,
                    _root_signatures, _pipelines, _dynamic_buffer_allocator);
   }

   {
      profile_section profile{"Sky - Draw", command_list, _profiler,
                              profiler_queue::direct};

      _sky.draw(_camera_constant_buffer_view, command_list, _root_signatures,
                _pipelines, _dynamic_buffer_allocator);
   }

   if (active_entity_types.terrain) {
      profile_section profile{"Water - Draw", command_list, _profiler,
                              profiler_queue::direct};

      _water.draw(view_frustum, _camera_constant_buffer_view, command_list,
                  _dynamic_buffer_allocator, _root_signatures, _pipelines);
   }

   if (active_entity_types.objects) {
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

   material_pipeline_flags pipeline_flags = material_pipeline_flags::count; // Initialize to count to the loop below sets the pipeline on the first iteration.

   auto& meshes = _world_mesh_list;

   for (auto& i : list) {
      [[unlikely]] if (pipeline_flags != meshes.pipeline_flags[i].material) {
         pipeline_flags = meshes.pipeline_flags[i].material;

         command_list.set_pipeline_state(_pipelines.mesh_normal[pipeline_flags].get());
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

   depth_prepass_pipeline_flags pipeline_flags = depth_prepass_pipeline_flags::count; // Initialize to count to the loop below sets the pipeline on the first iteration.

   auto& meshes = _world_mesh_list;

   for (auto& i : list) {
      [[unlikely]] if (pipeline_flags != meshes.pipeline_flags[i].depth_prepass) {
         pipeline_flags = meshes.pipeline_flags[i].depth_prepass;

         command_list.set_pipeline_state(
            _pipelines.mesh_depth_prepass[pipeline_flags].get());
      }

      command_list.set_graphics_cbv(rs::mesh_depth_prepass::object_cbv,
                                    meshes.gpu_constants[i]);

      [[unlikely]] if (are_flags_set(pipeline_flags,
                                     depth_prepass_pipeline_flags::alpha_cutout)) {
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
   const frustum& view_frustum, const camera& camera, const world::world& world,
   const world::interaction_targets& interaction_targets,
   const world::active_entity_types active_entity_types,
   const world::active_layers active_layers,
   const world::tool_visualizers& tool_visualizers, const settings::graphics& settings)
{
   if (active_entity_types.paths and not world.paths.empty()) {
      constexpr bool draw_connections = true;
      constexpr bool draw_orientation = true;

      const float4 path_node_color = {settings.path_node_color, 1.0f};
      const float4 path_node_outline_color = {settings.path_node_outline_color, 1.0f};

      const auto add_path = [&](const world::path& path) {
         if (not active_layers[path.layer] or path.hidden) return;

         const float path_node_size = settings.path_node_size;

         for (auto& node : path.nodes) {
            if (not intersects(view_frustum, node.position, path_node_size)) {
               continue;
            }

            const float4x4 rotation = to_matrix(node.rotation);
            float4x4 transform =
               rotation * float4x4{{path_node_size, 0.0f, 0.0f, 0.0f},
                                   {0.0f, path_node_size, 0.0f, 0.0f},
                                   {0.0f, 0.0f, path_node_size, 0.0f},
                                   {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {node.position, 1.0f};

            _meta_draw_batcher.add_octahedron_outlined(transform, path_node_color,
                                                       path_node_outline_color);

            if (draw_orientation) {
               float4x4 orientation_transform =
                  rotation * float4x4{{path_node_size * 0.5f, 0.0f, 0.0f, 0.0f},
                                      {0.0f, path_node_size * 0.5f, 0.0f, 0.0f},
                                      {0.0f, 0.0f, path_node_size * 0.5f, 0.0f},
                                      {0.0f, 0.0f, 0.0f, 1.0f}};
               orientation_transform[3] = {node.position, 1.0f};

               _meta_draw_batcher.add_arrow_outline_solid(orientation_transform, 3.2f,
                                                          utility::pack_srgb_bgra(
                                                             float4{settings.path_node_orientation_color,
                                                                    1.0f}));
            }
         }

         if (not path.nodes.empty() and draw_connections) {
            const uint32 path_node_connection_color = utility::pack_srgb_bgra(
               float4{settings.path_node_connection_color, 1.0f});

            bool draw_linear_spline = true;

            if (path.type == world::path_type::patrol) {
               draw_linear_spline =
                  path.spline_type != world::path_spline_type::catmull_rom;
            }
            else if (path.type == world::path_type::entity_follow) {
               draw_linear_spline = false;
            }

            if (draw_linear_spline) {
               for (std::size_t i = 0; i < (path.nodes.size() - 1); ++i) {
                  const float3 a = path.nodes[i].position;
                  const float3 b = path.nodes[i + 1].position;

                  const math::bounding_box bbox{.min = min(a, b), .max = max(a, b)};

                  if (not intersects(view_frustum, bbox)) continue;

                  _meta_draw_batcher.add_line_solid(a, b, path_node_connection_color);
               }
            }
            else {
               const std::ptrdiff_t min_node = 0;
               const std::ptrdiff_t max_node = std::ssize(path.nodes) - 1;

               for (std::ptrdiff_t i = 0; i < max_node; ++i) {
                  const float3 a =
                     path.nodes[std::clamp(i - 1, min_node, max_node)].position;
                  const float3 b =
                     path.nodes[std::clamp(i, min_node, max_node)].position;
                  const float3 c =
                     path.nodes[std::clamp(i + 1, min_node, max_node)].position;
                  const float3 d =
                     path.nodes[std::clamp(i + 2, min_node, max_node)].position;

                  auto catmull_rom = [](const float3 a, const float3 b, const float3 c,
                                        const float3 d, const float t) -> float3 {
                     const float t2 = t * t;
                     const float t3 = t * t * t;

                     return 0.5f * ((2.0f * b) + (-a + c) * t +
                                    (2.0f * a - 5.0f * b + 4.0f * c - d) * t2 +
                                    (-a + 3.0f * b - 3.0f * c + d) * t3);
                  };

                  const float3 midpoint = catmull_rom(a, b, c, d, 0.5f);

                  const math::bounding_box bbox{.min = min(b, min(c, midpoint)),
                                                .max = max(b, max(c, midpoint))};

                  if (not intersects(view_frustum, bbox)) continue;

                  const float segment_length = distance(b, c);
                  const float camera_scale = distance(camera.position(), midpoint) *
                                             camera.projection_matrix()[0].x;

                  const float steps_target =
                     settings.path_node_cr_spline_target_tessellation;
                  const float max_steps = settings.path_node_cr_spline_max_tessellation;

                  float steps = std::floor(
                     ((1.0f / camera_scale) * steps_target) * segment_length);

                  steps = std::clamp(steps, 1.0f, max_steps);

                  const float inv_steps = 1.0f / steps;

                  for (float v = 0.0f; v < steps; ++v) {
                     _meta_draw_batcher
                        .add_line_solid(catmull_rom(a, b, c, d, v * inv_steps),
                                        catmull_rom(a, b, c, d, (v + 1.0f) * inv_steps),
                                        path_node_connection_color);
                  }
               }
            }
         }
      };

      for (const auto& path : world.paths) add_path(path);

      if (interaction_targets.creation_entity.is<world::path>()) {
         add_path(interaction_targets.creation_entity.get<world::path>());
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

   if (active_entity_types.regions) {
      const float4 region_color = settings.region_color;

      for (auto& region : world.regions) {
         if (not active_layers[region.layer] or region.hidden) continue;

         add_region(region.rotation, region.position, region.size, region.shape,
                    region_color);
      }

      if (interaction_targets.creation_entity.is<world::region>()) {
         auto& region = interaction_targets.creation_entity.get<world::region>();

         add_region(region.rotation, region.position, region.size, region.shape,
                    region_color);
      }
   }

   if (active_entity_types.barriers) {
      const float barrier_height = settings.barrier_height;
      const uint32 barrier_color = utility::pack_srgb_bgra(
         float4{settings.barrier_outline_color.x, settings.barrier_outline_color.y,
                settings.barrier_outline_color.z, 1.0f});

      const auto add_barrier = [&](const world::barrier& barrier) {
         if (barrier.hidden) return;

         const float4x4 rotation =
            make_rotation_matrix_from_euler({0.0f, barrier.rotation_angle, 0.0f});

         float4x4 transform =
            rotation * float4x4{{barrier.size.x, 0.0f, 0.0f, 0.0f},
                                {0.0f, barrier_height, 0.0f, 0.0f},
                                {0.0f, 0.0f, barrier.size.y, 0.0f},
                                {0.0f, 0.0f, 0.0f, 1.0f}};

         transform[3] = {barrier.position, 1.0f};

         // TODO: Batch these better.
         std::array<float4, 4> corners_f4 = {
            transform * float4{-1.0f, 0.0f, 1.0f, 1.0f},
            transform * float4{-1.0f, 0.0f, -1.0f, 1.0f},
            transform * float4{1.0f, 0.0f, -1.0f, 1.0f},
            transform * float4{1.0f, 0.0f, 1.0f, 1.0f},
         };
         std::array<float3, 4> corners = {
            float3{corners_f4[0].x, corners_f4[0].y, corners_f4[0].z},
            float3{corners_f4[1].x, corners_f4[1].y, corners_f4[1].z},
            float3{corners_f4[2].x, corners_f4[2].y, corners_f4[2].z},
            float3{corners_f4[3].x, corners_f4[3].y, corners_f4[3].z},
         };

         math::bounding_box bbox{.min = corners[0], .max = corners[0]};

         for (std::size_t i = 1; i < corners.size(); ++i) {
            bbox.min = min(bbox.min, corners[i]);
            bbox.max = max(bbox.max, corners[i]);
         }

         bbox.min.y -= barrier_height;
         bbox.max.y += barrier_height;

         if (not intersects(view_frustum, bbox)) return;

         _meta_draw_batcher.add_line_overlay(corners[0], corners[1], barrier_color);
         _meta_draw_batcher.add_line_overlay(corners[1], corners[2], barrier_color);
         _meta_draw_batcher.add_line_overlay(corners[2], corners[3], barrier_color);
         _meta_draw_batcher.add_line_overlay(corners[3], corners[0], barrier_color);

         _ai_overlay_batches.barriers.push_back(transform);
      };

      for (auto& barrier : world.barriers) add_barrier(barrier);

      if (interaction_targets.creation_entity.is<world::barrier>()) {
         add_barrier(interaction_targets.creation_entity.get<world::barrier>());
      }
   }

   if (active_entity_types.lights) {
      const float volume_alpha = settings.light_volume_alpha;

      const auto add_light = [&](const world::light& light) {
         if (not active_layers[light.layer] or light.hidden) return;

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
            const float outer_cone_radius =
               light.range * std::tan(light.outer_cone_angle * 0.5f);
            const float inner_cone_radius =
               light.range * std::tan(light.inner_cone_angle * 0.5f);
            const float half_range = light.range * 0.5f;

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

      if (interaction_targets.creation_entity.is<world::light>()) {
         add_light(interaction_targets.creation_entity.get<world::light>());
      }
   }

   if (active_entity_types.sectors) {
      const uint32 sector_color = utility::pack_srgb_bgra(settings.sector_color);

      const auto add_sector = [&](const world::sector& sector) {
         if (sector.hidden) return;

         for (std::size_t i = 0; i < sector.points.size(); ++i) {
            const float2 a = sector.points[i];
            const float2 b = sector.points[(i + 1) % sector.points.size()];

            const std::array quad = {float3{a.x, sector.base, a.y},
                                     float3{b.x, sector.base, b.y},
                                     float3{a.x, sector.base + sector.height, a.y},
                                     float3{b.x, sector.base + sector.height, b.y}};

            math::bounding_box bbox{.min = quad[0], .max = quad[0]};

            for (auto v : quad) bbox = integrate(bbox, v);

            if (not intersects(view_frustum, bbox)) continue;

            _meta_draw_batcher.add_triangle(quad[0], quad[1], quad[2], sector_color);
            _meta_draw_batcher.add_triangle(quad[2], quad[1], quad[3], sector_color);
            _meta_draw_batcher.add_triangle(quad[0], quad[2], quad[1], sector_color);
            _meta_draw_batcher.add_triangle(quad[2], quad[3], quad[1], sector_color);
         }

         for (const auto point : sector.points) {
            const std::array line = {float3{point.x, sector.base, point.y},
                                     float3{point.x, sector.base + sector.height,
                                            point.y}};

            if (not intersects(view_frustum, line[0], 0.001f) and
                not intersects(view_frustum, line[1], 0.001f)) {
               continue;
            }

            _meta_draw_batcher.add_line_solid(line[0], line[1], sector_color);
         }
      };

      for (auto& sector : world.sectors) {
         add_sector(sector);
      }

      if (interaction_targets.creation_entity.is<world::sector>()) {
         add_sector(interaction_targets.creation_entity.get<world::sector>());
      }
   }

   if (active_entity_types.portals) {
      const uint32 portal_color = utility::pack_srgb_bgra(settings.portal_color);

      const auto add_portal = [&](const world::portal& portal) {
         if (portal.hidden) return;

         const float half_width = portal.width * 0.5f;
         const float half_height = portal.height * 0.5f;

         if (not intersects(view_frustum, portal.position,
                            std::max(half_width, half_height))) {
            return;
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
      };

      for (auto& portal : world.portals) add_portal(portal);

      if (interaction_targets.creation_entity.is<world::portal>()) {
         add_portal(interaction_targets.creation_entity.get<world::portal>());
      }
   }

   if (active_entity_types.hintnodes) {
      const float4 hintnode_color = settings.hintnode_color;
      const uint32 packed_hintnode_color = utility::pack_srgb_bgra(hintnode_color);

      const auto add_hintnode = [&](const world::hintnode& hintnode) {
         if (not active_layers[hintnode.layer] or hintnode.hidden) return;

         if (not intersects(view_frustum, hintnode.position, 3.0f)) return;

         float4x4 rotation = to_matrix(hintnode.rotation);

         float4x4 transform = rotation;
         transform[3] = {hintnode.position, 1.0f};

         _meta_draw_batcher.add_hint_hexahedron(transform, hintnode_color);

         float4x4 arrow_transform = rotation * float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                        {0.0f, 1.0f, 0.0f, 0.0f},
                                                        {0.0f, 0.0f, 1.0f, 0.0f},
                                                        {0.0f, 1.0f, 0.0f, 1.0f}};
         arrow_transform[3] += {hintnode.position, 0.0f};

         _meta_draw_batcher.add_arrow_outline_solid(arrow_transform, 2.4f,
                                                    packed_hintnode_color);
      };

      for (auto& hintnode : world.hintnodes) add_hintnode(hintnode);

      if (interaction_targets.creation_entity.is<world::hintnode>()) {
         add_hintnode(interaction_targets.creation_entity.get<world::hintnode>());
      }
   }

   if (active_entity_types.planning_hubs) {
      const float planning_hub_height = settings.planning_hub_height;
      const uint32 packed_color =
         utility::pack_srgb_bgra({settings.planning_hub_outline_color.x,
                                  settings.planning_hub_outline_color.y,
                                  settings.planning_hub_outline_color.z, 1.0f});

      const auto add_hub = [&](const world::planning_hub& hub) {
         if (hub.hidden) return;

         const math::bounding_box bbox{
            .min = float3{-hub.radius, -planning_hub_height, -hub.radius} + hub.position,
            .max = float3{hub.radius, planning_hub_height, hub.radius} + hub.position};

         if (not intersects(view_frustum, bbox)) return;

         const float3 scale = float3{hub.radius, 0.0f, hub.radius};
         const float3 offset = hub.position;

         const std::array circle = {
            float3{0.0f, 0.0f, 1.0f} * scale + offset,
            float3{-0.195090f, 0.0f, 0.980785f} * scale + offset,
            float3{-0.382683f, 0.0f, 0.923880f} * scale + offset,
            float3{-0.555570f, 0.0f, 0.831470f} * scale + offset,
            float3{-0.707107f, 0.0f, 0.707107f} * scale + offset,
            float3{-0.831470f, 0.0f, 0.555570f} * scale + offset,
            float3{-0.923880f, 0.0f, 0.382683f} * scale + offset,
            float3{-0.980785f, 0.0f, 0.195090f} * scale + offset,
            float3{-1.0f, 0.0f, 0.0f} * scale + offset,
            float3{-0.980785f, 0.0f, -0.195090f} * scale + offset,
            float3{-0.923880f, 0.0f, -0.382683f} * scale + offset,
            float3{-0.831470f, 0.0f, -0.555570f} * scale + offset,
            float3{-0.707107f, 0.0f, -0.707107f} * scale + offset,
            float3{-0.555570f, 0.0f, -0.831470f} * scale + offset,
            float3{-0.382683f, 0.0f, -0.923880f} * scale + offset,
            float3{-0.195090f, 0.0f, -0.980785f} * scale + offset,
            float3{0.0f, 0.0f, -1.0f} * scale + offset,
            float3{0.195091f, 0.0f, -0.980785f} * scale + offset,
            float3{0.382684f, 0.0f, -0.923879f} * scale + offset,
            float3{0.555571f, 0.0f, -0.831469f} * scale + offset,
            float3{0.707107f, 0.0f, -0.707106f} * scale + offset,
            float3{0.831470f, 0.0f, -0.555570f} * scale + offset,
            float3{0.923880f, 0.0f, -0.382683f} * scale + offset,
            float3{0.980785f, 0.0f, -0.195089f} * scale + offset,
            float3{1.0f, 0.0f, 0.000001f} * scale + offset,
            float3{0.980785f, 0.0f, 0.195091f} * scale + offset,
            float3{0.923879f, 0.0f, 0.382684f} * scale + offset,
            float3{0.831469f, 0.0f, 0.555571f} * scale + offset,
            float3{0.707106f, 0.0f, 0.707108f} * scale + offset,
            float3{0.555569f, 0.0f, 0.831470f} * scale + offset,
            float3{0.382682f, 0.0f, 0.923880f} * scale + offset,
            float3{0.195089f, 0.0f, 0.980786f} * scale + offset,
         };

         for (std::size_t i = 0; i < circle.size(); ++i) {
            _meta_draw_batcher.add_line_overlay(circle[i],
                                                circle[(i + 1) % circle.size()],
                                                packed_color);
         }

         _ai_overlay_batches.hubs.push_back({{hub.radius, 0.0f, 0.0f, 0.0f},
                                             {0.0f, planning_hub_height, 0.0f, 0.0f},
                                             {0.0f, 0.0f, hub.radius, 0.0f},
                                             {hub.position, 1.0f}});
      };

      for (auto& hub : world.planning_hubs) add_hub(hub);

      if (interaction_targets.creation_entity.is<world::planning_hub>()) {
         add_hub(interaction_targets.creation_entity.get<world::planning_hub>());
      }
   }

   if (active_entity_types.planning_connections) {
      const float planning_connection_height = settings.planning_connection_height;

      const uint32 packed_color = utility::pack_srgb_bgra(
         {settings.planning_connection_outline_color.x,
          settings.planning_connection_outline_color.y,
          settings.planning_connection_outline_color.z, 1.0f});

      const auto add_connection = [&](const world::planning_connection& connection) {
         if (connection.hidden) return;

         const world::planning_hub& start =
            world.planning_hubs[connection.start_hub_index];
         const world::planning_hub& end =
            world.planning_hubs[connection.end_hub_index];

         const math::bounding_box start_bbox{
            .min = float3{-start.radius, -planning_connection_height, -start.radius} +
                   start.position,
            .max = float3{start.radius, planning_connection_height, start.radius} +
                   start.position};
         const math::bounding_box end_bbox{
            .min = float3{-end.radius, -planning_connection_height, -end.radius} +
                   end.position,
            .max = float3{end.radius, planning_connection_height, end.radius} +
                   end.position};

         const math::bounding_box bbox = math::combine(start_bbox, end_bbox);

         if (not intersects(view_frustum, bbox)) return;

         const float3 normal =
            normalize(float3{-(start.position.z - end.position.z), 0.0f,
                             start.position.x - end.position.x});

         std::array<float3, 4> quad{start.position + normal * start.radius,
                                    start.position - normal * start.radius,
                                    end.position + normal * end.radius,
                                    end.position - normal * end.radius};

         const float3 height_offset = {0.0f, planning_connection_height, 0.0f};

         std::array<float3, 8> corners = {quad[0] + height_offset,
                                          quad[1] + height_offset,
                                          quad[2] + height_offset,
                                          quad[3] + height_offset,
                                          quad[0] - height_offset,
                                          quad[1] - height_offset,
                                          quad[2] - height_offset,
                                          quad[3] - height_offset};

         _ai_overlay_batches.connections.push_back({
            // Top
            corners[3], corners[2], corners[0], //
            corners[3], corners[0], corners[1], //

            // Bottom
            corners[4], corners[6], corners[7], //
            corners[5], corners[4], corners[7], //

            // Side 0
            corners[0], corners[6], corners[4], //
            corners[0], corners[2], corners[6], //

            // Side 1
            corners[1], corners[5], corners[7], //
            corners[7], corners[3], corners[1], //

            // Back
            corners[4], corners[1], corners[0], //
            corners[5], corners[1], corners[4], //

            // Front
            corners[2], corners[3], corners[6], //
            corners[6], corners[3], corners[7]  //
         });

         _meta_draw_batcher.add_line_overlay(quad[0], quad[1], packed_color);
         _meta_draw_batcher.add_line_overlay(quad[1], quad[3], packed_color);
         _meta_draw_batcher.add_line_overlay(quad[3], quad[2], packed_color);
         _meta_draw_batcher.add_line_overlay(quad[2], quad[0], packed_color);
      };

      for (auto& connection : world.planning_connections) {
         add_connection(connection);
      }

      if (interaction_targets.creation_entity.is<world::planning_connection>()) {
         add_connection(
            interaction_targets.creation_entity.get<world::planning_connection>());
      }
   }

   if (active_entity_types.boundaries) {
      const float boundary_height = settings.boundary_height;
      const uint32 boundary_color = utility::pack_srgb_bgra(settings.boundary_color);

      const auto add_boundary = [&](const world::boundary& boundary) {
         if (boundary.hidden) return;

         const std::array<float3, 12> nodes = world::get_boundary_nodes(boundary);

         for (std::size_t i = 0; i < nodes.size(); ++i) {
            const float3 a = nodes[i];
            const float3 b = nodes[(i + 1) % nodes.size()];

            const std::array quad = {float3{a.x, a.y - boundary_height, a.z},
                                     float3{b.x, b.y - boundary_height, b.z},
                                     float3{a.x, a.y + boundary_height, a.z},
                                     float3{b.x, b.y + boundary_height, b.z}};

            _meta_draw_batcher.add_triangle(quad[0], quad[1], quad[2], boundary_color);
            _meta_draw_batcher.add_triangle(quad[2], quad[1], quad[3], boundary_color);
            _meta_draw_batcher.add_triangle(quad[0], quad[2], quad[1], boundary_color);
            _meta_draw_batcher.add_triangle(quad[2], quad[3], quad[1], boundary_color);

            _meta_draw_batcher.add_line_solid(quad[0], quad[2], boundary_color);
            _meta_draw_batcher.add_line_solid(quad[1], quad[3], boundary_color);
         }
      };

      for (auto& boundary : world.boundaries) add_boundary(boundary);

      if (interaction_targets.creation_entity.is<world::boundary>()) {
         add_boundary(interaction_targets.creation_entity.get<world::boundary>());
      }
   }

   if (active_entity_types.measurements) {
      const uint32 measurement_color =
         utility::pack_srgb_bgra(float4{1.0f, 1.0f, 1.0f, 0.25f});
      const float2 viewport_size = {static_cast<float>(_swap_chain.width()),
                                    static_cast<float>(_swap_chain.height())};

      const auto add_measurement = [&](const world::measurement& measurement) {
         if (measurement.hidden) return;

         const math::bounding_box bbox{.min = min(measurement.start, measurement.end),
                                       .max = max(measurement.start, measurement.end)};

         if (not intersects(view_frustum, bbox)) return;

         _meta_draw_batcher.add_line_solid(measurement.start, measurement.end,
                                           measurement_color);

         const float3 centreWS = (measurement.start + measurement.end) * 0.5f;
         const float4 centrePS =
            camera.view_projection_matrix() * float4{centreWS, 1.0f};
         const float2 centreNDC = {centrePS.x / centrePS.w,
                                   centrePS.y / centrePS.w};

         if (centreNDC.x > 1.0f or centreNDC.x < -1.0f or centreNDC.y > 1.0f or
             centreNDC.y < -1.0f) {
            return;
         }

         const float2 positionRT = {(centreNDC.x + 1.0f) * viewport_size.x * 0.5f,
                                    (1.0f - centreNDC.y) * viewport_size.y * 0.5f};

         std::array<char, 128> text;

         const char* const text_end =
            fmt::format_to_n(text.data(), text.size(), "{:.2f}m",
                             distance(measurement.start, measurement.end))
               .out;

         const ImVec2 text_size = ImGui::CalcTextSize(text.data(), text_end);
         const float2 text_size_half = {text_size.x * 0.5f, text_size.y * 0.5f};

         ImGui::GetBackgroundDrawList()->AddRectFilled(
            {positionRT.x - text_size_half.x, positionRT.y - text_size_half.y},
            {positionRT.x + text_size_half.x, positionRT.y + text_size_half.y},
            0x60'00'00'00);
         ImGui::GetBackgroundDrawList()->AddText({positionRT.x - text_size_half.x,
                                                  positionRT.y - text_size_half.y},
                                                 0xff'ff'ff'ff, text.data(), text_end);
      };

      for (auto& measurement : world.measurements) add_measurement(measurement);

      if (interaction_targets.creation_entity.is<world::measurement>()) {
         add_measurement(interaction_targets.creation_entity.get<world::measurement>());
      }
   }

   for (const auto& line : tool_visualizers.lines_overlay()) {
      _meta_draw_batcher.add_line_overlay(line.v0, line.v0_color, line.v1, line.v1_color);
   }

   for (const auto& line : tool_visualizers.lines()) {
      _meta_draw_batcher.add_line_solid(line.v0, line.v0_color, line.v1, line.v1_color);
   }

   for (const auto& octahedron : tool_visualizers.octahedrons()) {
      _meta_draw_batcher.add_octahedron(octahedron.transform, octahedron.color);
   }

   for (const auto& octahedron : tool_visualizers.octahedrons_wireframe()) {
      _meta_draw_batcher.add_octahedron_wireframe(octahedron.transform,
                                                  float3{octahedron.color.x,
                                                         octahedron.color.y,
                                                         octahedron.color.z});
   }

   for (const auto& arrow : tool_visualizers.arrows_wireframe()) {
      _meta_draw_batcher.add_arrow_outline_solid(arrow.transform, 0.0f,
                                                 utility::pack_srgb_bgra(arrow.color));
   }
}

void renderer_impl::draw_terrain_cut_visualizers(const frustum& view_frustum,
                                                 const settings::graphics& settings,
                                                 gpu::graphics_command_list& command_list)
{
   command_list.set_graphics_root_signature(_root_signatures.mesh_wireframe.get());
   command_list.set_graphics_cbv(rs::mesh_wireframe::frame_cbv,
                                 _camera_constant_buffer_view);
   command_list.set_graphics_cbv(rs::mesh_wireframe::wireframe_cbv,
                                 _dynamic_buffer_allocator
                                    .allocate_and_copy(wireframe_constant_buffer{
                                       .color = settings.terrain_cutter_color})
                                    .gpu_address);

   command_list.set_pipeline_state(_pipelines.mesh_wireframe.get());

   for (const auto& cut : _terrain_cut_list) {
      if (not intersects(view_frustum, cut.bbox)) continue;

      command_list.set_graphics_cbv(rs::mesh_wireframe::object_cbv, cut.constant_buffer);

      command_list.ia_set_index_buffer(cut.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, cut.position_vertex_buffer_view);

      command_list.draw_indexed_instanced(cut.index_count, 1, cut.start_index,
                                          cut.start_vertex, 0);
   }
}

void renderer_impl::draw_grid_overlay(const float height, const float length,
                                      const float grid_scale, float3 camera_positionWS,
                                      const settings::graphics& settings,
                                      gpu::graphics_command_list& command_list)
{
   struct grid_constants {
      float3 grid_color;
      float length;
      float grid_offsetWS_x;
      float grid_offsetWS_z;
      float height;
      float inv_grid_scale;
      float line_width;
      float inv_grid_major_scale;
      float major_line_width;
   };

   const float line_width = settings.overlay_grid_line_width;
   const float major_grid_scale = grid_scale * settings.overlay_grid_major_grid_spacing;
   const float grid_wrap_scale = major_grid_scale;

   const float3 grid_offsetWS =
      round(camera_positionWS / grid_wrap_scale) * grid_wrap_scale;

   grid_constants grid_constants{
      .grid_color = settings.overlay_grid_color,
      .length = length,
      .grid_offsetWS_x = grid_offsetWS.x,
      .grid_offsetWS_z = grid_offsetWS.z,
      .height = height,
      .inv_grid_scale = 1.0f / grid_scale,
      .line_width = line_width * (4.0f / grid_scale),
      .inv_grid_major_scale = 1.0f / major_grid_scale,
      .major_line_width = line_width * (16.0f / major_grid_scale),
   };

   command_list.set_graphics_root_signature(_root_signatures.grid_overlay.get());
   command_list.set_graphics_32bit_constants(rs::grid_overlay::inputs,
                                             std::as_bytes(std::span{&grid_constants, 1}),
                                             0);
   command_list.set_graphics_cbv(rs::grid_overlay::frame_cbv,
                                 _camera_constant_buffer_view);

   command_list.set_pipeline_state(_pipelines.grid_overlay.get());

   command_list.draw_instanced(6, 1, 0, 0);
}

void renderer_impl::draw_ai_overlay(gpu::rtv_handle back_buffer_rtv,
                                    const settings::graphics& settings,
                                    gpu::graphics_command_list& command_list)
{
   if (_ai_overlay_batches.empty()) return;

   command_list.om_set_render_targets(_depth_stencil_view.get());
   command_list.om_set_stencil_ref(0x0);

   if (not _ai_overlay_batches.connections.empty()) {
      command_list.set_graphics_root_signature(_root_signatures.ai_overlay_shape.get());
      command_list.set_graphics_cbv(rs::ai_overlay_shape::frame_cbv,
                                    _camera_constant_buffer_view);

      command_list.set_pipeline_state(_pipelines.ai_overlay_shape.get());

      const std::size_t connections_bytes =
         sizeof(std::array<float3, 36>) * _ai_overlay_batches.connections.size();

      auto connections_allocation =
         _dynamic_buffer_allocator.allocate(connections_bytes);

      std::memcpy(connections_allocation.cpu_address,
                  _ai_overlay_batches.connections.data(), connections_bytes);

      command_list.set_graphics_srv(
         rs::ai_overlay_shape::instance_data_srv,
         _dynamic_buffer_allocator.allocate_and_copy(float4x4{}).gpu_address);

      command_list.ia_set_vertex_buffers(
         0, gpu::vertex_buffer_view{.buffer_location = connections_allocation.gpu_address,
                                    .size_in_bytes = static_cast<uint32>(connections_bytes),
                                    .stride_in_bytes = sizeof(float3)});

      command_list.draw_instanced(static_cast<uint32>(
                                     _ai_overlay_batches.connections.size() * 36),
                                  1, 0, 0);

      command_list.om_set_render_targets(back_buffer_rtv, _depth_stencil_view.get());

      const float4 color = settings.planning_connection_overlay_color;

      command_list.set_graphics_root_signature(_root_signatures.ai_overlay_apply.get());
      command_list.set_graphics_32bit_constants(rs::ai_overlay_apply::color,
                                                std::as_bytes(std::span{&color, 1}),
                                                0);

      command_list.set_pipeline_state(_pipelines.ai_overlay_apply.get());

      command_list.draw_instanced(3, 1, 0, 0);
   }

   if (not _ai_overlay_batches.hubs.empty()) {
      command_list.set_graphics_root_signature(_root_signatures.ai_overlay_shape.get());
      command_list.set_graphics_cbv(rs::ai_overlay_shape::frame_cbv,
                                    _camera_constant_buffer_view);

      command_list.set_pipeline_state(_pipelines.ai_overlay_shape.get());

      auto hubs_allocation = _dynamic_buffer_allocator.allocate(
         sizeof(float4x4) * _ai_overlay_batches.hubs.size());

      std::memcpy(hubs_allocation.cpu_address, _ai_overlay_batches.hubs.data(),
                  sizeof(float4x4) * _ai_overlay_batches.hubs.size());

      command_list.set_graphics_srv(rs::ai_overlay_shape::instance_data_srv,
                                    hubs_allocation.gpu_address);

      const geometric_shape cylinder = _geometric_shapes.cylinder();

      command_list.ia_set_index_buffer(cylinder.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, cylinder.position_vertex_buffer_view);

      command_list.draw_indexed_instanced(cylinder.index_count,
                                          static_cast<uint32>(
                                             _ai_overlay_batches.hubs.size()),
                                          0, 0, 0);

      command_list.om_set_render_targets(back_buffer_rtv, _depth_stencil_view.get());

      const float4 color = settings.planning_hub_overlay_color;

      command_list.set_graphics_root_signature(_root_signatures.ai_overlay_apply.get());
      command_list.set_graphics_32bit_constants(rs::ai_overlay_apply::color,
                                                std::as_bytes(std::span{&color, 1}),
                                                0);

      command_list.set_pipeline_state(_pipelines.ai_overlay_apply.get());

      command_list.draw_instanced(3, 1, 0, 0);
   }

   if (not _ai_overlay_batches.barriers.empty()) {
      command_list.set_graphics_root_signature(_root_signatures.ai_overlay_shape.get());
      command_list.set_graphics_cbv(rs::ai_overlay_shape::frame_cbv,
                                    _camera_constant_buffer_view);

      command_list.set_pipeline_state(_pipelines.ai_overlay_shape.get());

      auto barriers_allocation = _dynamic_buffer_allocator.allocate(
         sizeof(float4x4) * _ai_overlay_batches.barriers.size());

      std::memcpy(barriers_allocation.cpu_address,
                  _ai_overlay_batches.barriers.data(),
                  sizeof(float4x4) * _ai_overlay_batches.barriers.size());

      command_list.set_graphics_srv(rs::ai_overlay_shape::instance_data_srv,
                                    barriers_allocation.gpu_address);

      const geometric_shape cube = _geometric_shapes.cube();

      command_list.ia_set_index_buffer(cube.index_buffer_view);
      command_list.ia_set_vertex_buffers(0, cube.position_vertex_buffer_view);

      command_list.draw_indexed_instanced(cube.index_count,
                                          static_cast<uint32>(
                                             _ai_overlay_batches.barriers.size()),
                                          0, 0, 0);

      command_list.om_set_render_targets(back_buffer_rtv, _depth_stencil_view.get());

      const float4 color = settings.barrier_overlay_color;

      command_list.set_graphics_root_signature(_root_signatures.ai_overlay_apply.get());
      command_list.set_graphics_32bit_constants(rs::ai_overlay_apply::color,
                                                std::as_bytes(std::span{&color, 1}),
                                                0);

      command_list.set_pipeline_state(_pipelines.ai_overlay_apply.get());

      command_list.draw_instanced(3, 1, 0, 0);
   }
}

void renderer_impl::draw_interaction_targets(
   const frustum& view_frustum, const world::world& world,
   const world::interaction_targets& interaction_targets,
   const world::object_class_library& world_classes,
   const settings::graphics& settings, gpu::graphics_command_list& command_list)
{
   (void)view_frustum; // TODO: frustum Culling (Is it worth it for interaction targets?)

   const auto draw_path_node = [&](const world::path::node& node, const float3 color) {
      const float path_node_size = settings.path_node_size;

      float4x4 transform =
         to_matrix(node.rotation) * float4x4{{path_node_size, 0.0f, 0.0f, 0.0f},
                                             {0.0f, path_node_size, 0.0f, 0.0f},
                                             {0.0f, 0.0f, path_node_size, 0.0f},
                                             {0.0f, 0.0f, 0.0f, 1.0f}};

      transform[3] = {node.position, 1.0f};

      _meta_draw_batcher.add_octahedron_wireframe(transform, color);
   };

   const auto draw_entity = overload{
      [&](const world::object& object, const float3 color) {
         gpu_virtual_address wireframe_constants = [&] {
            auto allocation =
               _dynamic_buffer_allocator.allocate(sizeof(wireframe_constant_buffer));

            wireframe_constant_buffer constants{.color = color};

            std::memcpy(allocation.cpu_address, &constants,
                        sizeof(wireframe_constant_buffer));

            return allocation.gpu_address;
         }();

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

         model& model = _model_manager[world_classes[object.class_name].model_name];

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
      [&](const world::light& light, const float3 color) {
         if (light.light_type == world::light_type::directional) {
            const float4x4 rotation = to_matrix(light.rotation);
            float4x4 transform = rotation * float4x4{{2.0f, 0.0f, 0.0f, 0.0f},
                                                     {0.0f, 2.0f, 0.0f, 0.0f},
                                                     {0.0f, 0.0f, 2.0f, 0.0f},
                                                     {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {light.position, 1.0f};

            _meta_draw_batcher.add_octahedron_wireframe(transform, color);
            _meta_draw_batcher.add_arrow_outline_solid(transform, 2.2f,
                                                       utility::pack_srgb_bgra(
                                                          float4{color, 1.0f}));
         }
         else if (light.light_type == world::light_type::point) {
            _meta_draw_batcher.add_sphere_wireframe(light.position, light.range, color);
         }
         else if (light.light_type == world::light_type::spot) {
            const float outer_cone_radius =
               light.range * std::tan(light.outer_cone_angle * 0.5f);
            const float inner_cone_radius =
               light.range * std::tan(light.inner_cone_angle * 0.5f);

            const float4x4 rotation = to_matrix(
               light.rotation * quaternion{0.707107f, -0.707107f, 0.0f, 0.0f});

            const float half_range = light.range * 0.5f;

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

            _meta_draw_batcher.add_cone_wireframe(outer_transform, color);
            _meta_draw_batcher.add_cone_wireframe(inner_transform, color);
         }
         else if (light.light_type == world::light_type::directional_region_box) {
            const float3 scale = light.region_size;

            float4x4 transform = to_matrix(light.region_rotation) *
                                 float4x4{{scale.x, 0.0f, 0.0f, 0.0f},
                                          {0.0f, scale.y, 0.0f, 0.0f},
                                          {0.0f, 0.0f, scale.z, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};
            transform[3] = float4{light.position, 0.0f};

            float4x4 arrow_transform = to_matrix(light.rotation);
            arrow_transform[3] = {light.position, 1.0f};

            _meta_draw_batcher.add_box_wireframe(transform, color);
            _meta_draw_batcher.add_arrow_outline_solid(arrow_transform, 0.0f,
                                                       utility::pack_srgb_bgra(
                                                          float4{color, 1.0f}));
         }
         else if (light.light_type == world::light_type::directional_region_sphere) {
            const float scale = length(light.region_size);

            float4x4 arrow_transform = to_matrix(light.rotation);
            arrow_transform[3] = {light.position, 1.0f};

            _meta_draw_batcher.add_sphere_wireframe(light.position, scale, color);
            _meta_draw_batcher.add_arrow_outline_solid(arrow_transform, 0.0f,
                                                       utility::pack_srgb_bgra(
                                                          float4{color, 1.0f}));
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
            transform[3] = float4{light.position, 0.0f};

            float4x4 arrow_transform = to_matrix(light.rotation);
            arrow_transform[3] = {light.position, 1.0f};

            _meta_draw_batcher.add_cylinder_wireframe(transform, color);
            _meta_draw_batcher.add_arrow_outline_solid(arrow_transform, 0.0f,
                                                       utility::pack_srgb_bgra(
                                                          float4{color, 1.0f}));
         }
      },
      [&](const world::path& path, const float3 color) {
         for (auto& node : path.nodes) draw_path_node(node, color);

         if (path.nodes.empty()) return;

         const uint32 packed_color = utility::pack_srgb_bgra({color, 1.0f});

         for (std::size_t i = 0; i < (path.nodes.size() - 1); ++i) {
            const float3 a = path.nodes[i].position;
            const float3 b = path.nodes[i + 1].position;

            _meta_draw_batcher.add_line_solid(a, b, packed_color);
         }
      },
      [&](const world::path::node& path_node, const float3 color) {
         draw_path_node(path_node, color);
      },
      [&](const world::region& region, const float3 color) {
         switch (region.shape) {
         default:
         case world::region_shape::box: {
            float4x4 transform = to_matrix(region.rotation) *
                                 float4x4{{region.size.x, 0.0f, 0.0f, 0.0f},
                                          {0.0f, region.size.y, 0.0f, 0.0f},
                                          {0.0f, 0.0f, region.size.z, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};
            transform[3] = {region.position, 1.0f};

            _meta_draw_batcher.add_box_wireframe(transform, color);
         } break;
         case world::region_shape::sphere: {
            const float sphere_radius = length(region.size);

            _meta_draw_batcher.add_sphere_wireframe(region.position,
                                                    sphere_radius, color);
         } break;
         case world::region_shape::cylinder: {
            const float cylinder_length =
               length(float2{region.size.x, region.size.z});

            float4x4 transform = to_matrix(region.rotation) *
                                 float4x4{{cylinder_length, 0.0f, 0.0f, 0.0f},
                                          {0.0f, region.size.y, 0.0f, 0.0f},
                                          {0.0f, 0.0f, cylinder_length, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};
            transform[3] = {region.position, 1.0f};

            _meta_draw_batcher.add_cylinder_wireframe(transform, color);
         } break;
         }
      },
      [&](const world::sector& sector, const float3 color) {
         const uint32 packed_color = utility::pack_srgb_bgra({color, 1.0f});

         for (std::size_t i = 0; i < sector.points.size(); ++i) {
            const float2 a = sector.points[i];
            const float2 b = sector.points[(i + 1) % sector.points.size()];

            const std::array quad = {float3{a.x, sector.base, a.y},
                                     float3{a.x, sector.base + sector.height, a.y},
                                     float3{b.x, sector.base + sector.height, b.y},
                                     float3{b.x, sector.base, b.y}};

            _meta_draw_batcher.add_line_solid(quad[0], quad[1], packed_color);
            _meta_draw_batcher.add_line_solid(quad[1], quad[2], packed_color);
            _meta_draw_batcher.add_line_solid(quad[2], quad[3], packed_color);
            _meta_draw_batcher.add_line_solid(quad[3], quad[0], packed_color);
         }
      },
      [&](const world::portal& portal, const float3 color) {
         const uint32 packed_color = utility::pack_srgb_bgra({color, 1.0f});

         const float half_width = portal.width * 0.5f;
         const float half_height = portal.height * 0.5f;

         std::array quad = {float3{-half_width, -half_height, 0.0f},
                            float3{-half_width, half_height, 0.0f},
                            float3{half_width, half_height, 0.0f},
                            float3{half_width, -half_height, 0.0f}};

         for (auto& v : quad) {
            v = portal.rotation * v;
            v += portal.position;
         }

         _meta_draw_batcher.add_line_solid(quad[0], quad[1], packed_color);
         _meta_draw_batcher.add_line_solid(quad[1], quad[2], packed_color);
         _meta_draw_batcher.add_line_solid(quad[2], quad[3], packed_color);
         _meta_draw_batcher.add_line_solid(quad[3], quad[0], packed_color);
      },
      [&](const world::hintnode& hintnode, const float3 color) {
         float4x4 rotation = to_matrix(hintnode.rotation);

         float4x4 transform = rotation;
         transform[3] = {hintnode.position, 1.0f};

         _meta_draw_batcher.add_hint_hexahedron_wireframe(transform, color);

         float4x4 arrow_transform = rotation * float4x4{{1.0f, 0.0f, 0.0f, 0.0f},
                                                        {0.0f, 1.0f, 0.0f, 0.0f},
                                                        {0.0f, 0.0f, 1.0f, 0.0f},
                                                        {0.0f, 1.0f, 0.0f, 1.0f}};
         arrow_transform[3] += {hintnode.position, 0.0f};

         _meta_draw_batcher.add_arrow_outline_solid(arrow_transform, 2.4f,
                                                    utility::pack_srgb_bgra(
                                                       float4{color, 1.0f}));
      },
      [&](const world::barrier& barrier, const float3 color) {
         const geometric_shape shape = _geometric_shapes.cube();

         const float barrier_height = settings.barrier_height;

         const float4x4 rotation =
            make_rotation_matrix_from_euler({0.0f, barrier.rotation_angle, 0.0f});

         float4x4 transform =
            rotation * float4x4{{barrier.size.x, 0.0f, 0.0f, 0.0f},
                                {0.0f, barrier_height, 0.0f, 0.0f},
                                {0.0f, 0.0f, barrier.size.y, 0.0f},
                                {0.0f, 0.0f, 0.0f, 1.0f}};

         transform[3] = {barrier.position, 1.0f};

         std::array<float4, 4> corners_f4 = {
            transform * float4{-1.0f, 0.0f, 1.0f, 1.0f},
            transform * float4{-1.0f, 0.0f, -1.0f, 1.0f},
            transform * float4{1.0f, 0.0f, -1.0f, 1.0f},
            transform * float4{1.0f, 0.0f, 1.0f, 1.0f},
         };
         std::array<float3, 4> corners = {
            float3{corners_f4[0].x, corners_f4[0].y, corners_f4[0].z},
            float3{corners_f4[1].x, corners_f4[1].y, corners_f4[1].z},
            float3{corners_f4[2].x, corners_f4[2].y, corners_f4[2].z},
            float3{corners_f4[3].x, corners_f4[3].y, corners_f4[3].z},
         };

         const uint32 packed_color = utility::pack_srgb_bgra({color, 1.0f});

         _meta_draw_batcher.add_line_overlay(corners[0], corners[1], packed_color);
         _meta_draw_batcher.add_line_overlay(corners[1], corners[2], packed_color);
         _meta_draw_batcher.add_line_overlay(corners[2], corners[3], packed_color);
         _meta_draw_batcher.add_line_overlay(corners[3], corners[0], packed_color);

         const float3 height_offset = {0.0f, barrier_height, 0.0f};

         _meta_draw_batcher.add_line_solid(corners[0] + height_offset,
                                           corners[0] - height_offset, packed_color);
         _meta_draw_batcher.add_line_solid(corners[1] + height_offset,
                                           corners[1] - height_offset, packed_color);
         _meta_draw_batcher.add_line_solid(corners[2] + height_offset,
                                           corners[2] - height_offset, packed_color);
         _meta_draw_batcher.add_line_solid(corners[3] + height_offset,
                                           corners[3] - height_offset, packed_color);

         _meta_draw_batcher.add_line_solid(corners[0] + height_offset,
                                           corners[1] + height_offset, packed_color);
         _meta_draw_batcher.add_line_solid(corners[1] + height_offset,
                                           corners[2] + height_offset, packed_color);
         _meta_draw_batcher.add_line_solid(corners[2] + height_offset,
                                           corners[3] + height_offset, packed_color);
         _meta_draw_batcher.add_line_solid(corners[3] + height_offset,
                                           corners[0] + height_offset, packed_color);

         _meta_draw_batcher.add_line_solid(corners[0] - height_offset,
                                           corners[1] - height_offset, packed_color);
         _meta_draw_batcher.add_line_solid(corners[1] - height_offset,
                                           corners[2] - height_offset, packed_color);
         _meta_draw_batcher.add_line_solid(corners[2] - height_offset,
                                           corners[3] - height_offset, packed_color);
         _meta_draw_batcher.add_line_solid(corners[3] - height_offset,
                                           corners[0] - height_offset, packed_color);
      },
      [&](const world::planning_hub& hub, const float3 color) {
         const float3 height = {0.0f, settings.planning_hub_height, 0.0f};
         const float3 scale = float3{hub.radius, 0.0f, hub.radius};
         const float3 offset = hub.position;

         const std::array circle = {
            float3{0.0f, 0.0f, 1.0f} * scale + offset,
            float3{-0.195090f, 0.0f, 0.980785f} * scale + offset,
            float3{-0.382683f, 0.0f, 0.923880f} * scale + offset,
            float3{-0.555570f, 0.0f, 0.831470f} * scale + offset,
            float3{-0.707107f, 0.0f, 0.707107f} * scale + offset,
            float3{-0.831470f, 0.0f, 0.555570f} * scale + offset,
            float3{-0.923880f, 0.0f, 0.382683f} * scale + offset,
            float3{-0.980785f, 0.0f, 0.195090f} * scale + offset,
            float3{-1.0f, 0.0f, 0.0f} * scale + offset,
            float3{-0.980785f, 0.0f, -0.195090f} * scale + offset,
            float3{-0.923880f, 0.0f, -0.382683f} * scale + offset,
            float3{-0.831470f, 0.0f, -0.555570f} * scale + offset,
            float3{-0.707107f, 0.0f, -0.707107f} * scale + offset,
            float3{-0.555570f, 0.0f, -0.831470f} * scale + offset,
            float3{-0.382683f, 0.0f, -0.923880f} * scale + offset,
            float3{-0.195090f, 0.0f, -0.980785f} * scale + offset,
            float3{0.0f, 0.0f, -1.0f} * scale + offset,
            float3{0.195091f, 0.0f, -0.980785f} * scale + offset,
            float3{0.382684f, 0.0f, -0.923879f} * scale + offset,
            float3{0.555571f, 0.0f, -0.831469f} * scale + offset,
            float3{0.707107f, 0.0f, -0.707106f} * scale + offset,
            float3{0.831470f, 0.0f, -0.555570f} * scale + offset,
            float3{0.923880f, 0.0f, -0.382683f} * scale + offset,
            float3{0.980785f, 0.0f, -0.195089f} * scale + offset,
            float3{1.0f, 0.0f, 0.000001f} * scale + offset,
            float3{0.980785f, 0.0f, 0.195091f} * scale + offset,
            float3{0.923879f, 0.0f, 0.382684f} * scale + offset,
            float3{0.831469f, 0.0f, 0.555571f} * scale + offset,
            float3{0.707106f, 0.0f, 0.707108f} * scale + offset,
            float3{0.555569f, 0.0f, 0.831470f} * scale + offset,
            float3{0.382682f, 0.0f, 0.923880f} * scale + offset,
            float3{0.195089f, 0.0f, 0.980786f} * scale + offset,
         };

         const uint32 packed_color = utility::pack_srgb_bgra({color, 1.0f});

         for (std::size_t i = 0; i < circle.size(); ++i) {
            _meta_draw_batcher.add_line_overlay(circle[i],
                                                circle[(i + 1) % circle.size()],
                                                packed_color);

            _meta_draw_batcher.add_line_solid(circle[i] - height,
                                              circle[i] + height, packed_color);

            _meta_draw_batcher.add_line_solid(circle[i] - height,
                                              circle[(i + 1) % circle.size()] - height,
                                              packed_color);
            _meta_draw_batcher.add_line_solid(circle[i] + height,
                                              circle[(i + 1) % circle.size()] + height,
                                              packed_color);
         }
      },
      [&](const world::planning_connection& connection, const float3 color) {
         const float height = settings.planning_connection_height;

         const world::planning_hub& start =
            world.planning_hubs[connection.start_hub_index];
         const world::planning_hub& end =
            world.planning_hubs[connection.end_hub_index];

         const float3 normal =
            normalize(float3{-(start.position.z - end.position.z), 0.0f,
                             start.position.x - end.position.x});

         std::array<float3, 4> quad{start.position + normal * start.radius,
                                    start.position - normal * start.radius,
                                    end.position + normal * end.radius,
                                    end.position - normal * end.radius};

         const float3 height_offset = {0.0f, height, 0.0f};

         std::array<float3, 8> corners = {quad[0] + height_offset,
                                          quad[1] + height_offset,
                                          quad[2] + height_offset,
                                          quad[3] + height_offset,
                                          quad[0] - height_offset,
                                          quad[1] - height_offset,
                                          quad[2] - height_offset,
                                          quad[3] - height_offset};

         const uint32 packed_color = utility::pack_srgb_bgra({color, 1.0f});

         _meta_draw_batcher.add_line_solid(corners[0], corners[1], packed_color);
         _meta_draw_batcher.add_line_solid(corners[0], corners[2], packed_color);
         _meta_draw_batcher.add_line_solid(corners[2], corners[3], packed_color);
         _meta_draw_batcher.add_line_solid(corners[3], corners[1], packed_color);

         _meta_draw_batcher.add_line_solid(corners[4], corners[5], packed_color);
         _meta_draw_batcher.add_line_solid(corners[4], corners[6], packed_color);
         _meta_draw_batcher.add_line_solid(corners[6], corners[7], packed_color);
         _meta_draw_batcher.add_line_solid(corners[7], corners[5], packed_color);

         _meta_draw_batcher.add_line_solid(corners[0], corners[4], packed_color);
         _meta_draw_batcher.add_line_solid(corners[1], corners[5], packed_color);
         _meta_draw_batcher.add_line_solid(corners[2], corners[6], packed_color);
         _meta_draw_batcher.add_line_solid(corners[3], corners[7], packed_color);

         _meta_draw_batcher.add_line_overlay(quad[0], quad[1], packed_color);
         _meta_draw_batcher.add_line_overlay(quad[1], quad[3], packed_color);
         _meta_draw_batcher.add_line_overlay(quad[3], quad[2], packed_color);
         _meta_draw_batcher.add_line_overlay(quad[2], quad[0], packed_color);
      },
      [&](const world::boundary& boundary, const float3 color) {
         const float boundary_height = settings.boundary_height;

         const uint32 packed_color = utility::pack_srgb_bgra({color, 1.0f});

         const std::array<float3, 12> nodes = world::get_boundary_nodes(boundary);

         for (std::size_t i = 0; i < nodes.size(); ++i) {
            const float3 a = nodes[i];
            const float3 b = nodes[(i + 1) % nodes.size()];

            const std::array quad = {float3{a.x, a.y - boundary_height, a.z},
                                     float3{a.x, a.y + boundary_height, a.z},
                                     float3{b.x, b.y + boundary_height, b.z},
                                     float3{b.x, b.y - boundary_height, b.z}};

            _meta_draw_batcher.add_line_solid(quad[0], quad[1], packed_color);
            _meta_draw_batcher.add_line_solid(quad[1], quad[2], packed_color);
            _meta_draw_batcher.add_line_solid(quad[2], quad[3], packed_color);
            _meta_draw_batcher.add_line_solid(quad[3], quad[0], packed_color);
         }
      },
      [&](const world::measurement& measurement, const float3 color) {
         const uint32 packed_color = utility::pack_srgb_bgra({color, 1.0f});

         _meta_draw_batcher.add_line_solid(measurement.start, measurement.end,
                                           packed_color);
      },
   };

   const auto draw_target = [&](world::interaction_target target, const float3 color) {
      std::visit(overload{
                    [&](world::object_id id) {
                       const world::object* object = find_entity(world.objects, id);

                       if (object) draw_entity(*object, color);
                    },
                    [&](world::light_id id) {
                       const world::light* light = find_entity(world.lights, id);

                       if (light) draw_entity(*light, color);
                    },
                    [&](world::path_id_node_mask id_node_mask) {
                       const auto& [id, node_mask] = id_node_mask;

                       const world::path* path = find_entity(world.paths, id);

                       if (not path) return;

                       if (path) {
                          const std::size_t node_count =
                             std::min(path->nodes.size(), world::max_path_nodes);

                          for (uint32 i = 0; i < node_count; ++i) {
                             if (not node_mask[i]) continue;

                             draw_entity(path->nodes[i], color);
                          }
                       }
                    },
                    [&](world::region_id id) {
                       const world::region* region = find_entity(world.regions, id);

                       if (region) draw_entity(*region, color);
                    },
                    [&](world::sector_id id) {
                       const world::sector* sector = find_entity(world.sectors, id);

                       if (sector) draw_entity(*sector, color);
                    },
                    [&](world::portal_id id) {
                       const world::portal* portal = find_entity(world.portals, id);

                       if (portal) draw_entity(*portal, color);
                    },
                    [&](world::hintnode_id id) {
                       const world::hintnode* hintnode =
                          find_entity(world.hintnodes, id);

                       if (hintnode) draw_entity(*hintnode, color);
                    },
                    [&](world::barrier_id id) {
                       const world::barrier* barrier =
                          find_entity(world.barriers, id);

                       if (barrier) draw_entity(*barrier, color);
                    },
                    [&](world::planning_hub_id id) {
                       const world::planning_hub* planning_hub =
                          find_entity(world.planning_hubs, id);

                       if (planning_hub) {
                          draw_entity(*planning_hub, color);
                       }
                    },
                    [&](world::planning_connection_id id) {
                       const world::planning_connection* planning_connection =
                          find_entity(world.planning_connections, id);

                       if (planning_connection) {
                          draw_entity(*planning_connection, color);
                       }
                    },
                    [&](world::boundary_id id) {
                       const world::boundary* boundary =
                          find_entity(world.boundaries, id);

                       if (boundary) draw_entity(*boundary, color);
                    },
                    [&](world::measurement_id id) {
                       const world::measurement* measurement =
                          find_entity(world.measurements, id);

                       if (measurement) draw_entity(*measurement, color);
                    },
                 },
                 target);
   };

   if (interaction_targets.hovered_entity) {
      draw_target(*interaction_targets.hovered_entity, settings.hover_color);
   }

   if (not interaction_targets.selection.empty()) {
      for (auto target : interaction_targets.selection) {
         draw_target(target, settings.selected_color);
      }
   }

   if (interaction_targets.creation_entity.holds_entity()) {
      if (interaction_targets.creation_entity.is<world::object>()) {
         draw_entity(interaction_targets.creation_entity.get<world::object>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::light>()) {
         draw_entity(interaction_targets.creation_entity.get<world::light>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::path>()) {
         draw_entity(interaction_targets.creation_entity.get<world::path>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::region>()) {
         draw_entity(interaction_targets.creation_entity.get<world::region>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::sector>()) {
         draw_entity(interaction_targets.creation_entity.get<world::sector>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::portal>()) {
         draw_entity(interaction_targets.creation_entity.get<world::portal>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::barrier>()) {
         draw_entity(interaction_targets.creation_entity.get<world::barrier>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::hintnode>()) {
         draw_entity(interaction_targets.creation_entity.get<world::hintnode>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::planning_hub>()) {
         draw_entity(interaction_targets.creation_entity.get<world::planning_hub>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::planning_connection>()) {
         draw_entity(interaction_targets.creation_entity.get<world::planning_connection>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::boundary>()) {
         draw_entity(interaction_targets.creation_entity.get<world::boundary>(),
                     settings.creation_color);
      }
      else if (interaction_targets.creation_entity.is<world::measurement>()) {
         draw_entity(interaction_targets.creation_entity.get<world::measurement>(),
                     settings.creation_color);
      }
   }
}

void renderer_impl::build_world_mesh_list(
   gpu::copy_command_list& command_list, const world::world& world,
   const world::active_layers active_layers,
   const world::object_class_library& world_classes,
   const world::object* const creation_object,
   std::span<const world::tool_visualizers_ghost> ghost_objects)
{
   _world_mesh_list.clear();
   _world_mesh_list.reserve(1024 * 16);
   _terrain_cut_list.clear();
   _terrain_cut_list.reserve(256);

   auto& upload_buffer = _object_constants_upload_buffers[_device.frame_index()];

   const gpu_virtual_address constants_upload_gpu_address =
      _device.get_gpu_virtual_address(_object_constants_buffer.get());
   std::byte* const constants_upload_data =
      _object_constants_upload_cpu_ptrs[_device.frame_index()];
   std::size_t constants_data_size = 0;

   for (std::size_t i = 0; i < std::min(world.objects.size(), max_drawn_objects); ++i) {
      const auto& object = world.objects[i];
      auto& model = _model_manager[world_classes[object.class_name].model_name];

      if (not active_layers[object.layer] or object.hidden) continue;

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
         _world_mesh_list.push_back(
            object_bbox, object_constants_address, object.position,
            mesh.material.depth_prepass_flags, mesh.material.flags,
            mesh.material.constant_buffer_view,
            world_mesh{.index_buffer_view = model.gpu_buffer.index_buffer_view,
                       .vertex_buffer_views = {model.gpu_buffer.position_vertex_buffer_view,
                                               model.gpu_buffer.attributes_vertex_buffer_view},
                       .index_count = mesh.index_count,
                       .start_index = mesh.start_index,
                       .start_vertex = mesh.start_vertex});
      }

      for (auto& cut : model.terrain_cuts) {
         _terrain_cut_list.push_back(
            {.bbox = object.rotation * cut.bbox + object.position,

             .constant_buffer = object_constants_address,
             .index_buffer_view = model.gpu_buffer.index_buffer_view,
             .position_vertex_buffer_view = model.gpu_buffer.position_vertex_buffer_view,

             .index_count = cut.index_count,
             .start_index = cut.start_index,
             .start_vertex = cut.start_vertex});
      }
   }

   if (creation_object and world.objects.size() < max_drawn_objects) {
      auto& model =
         _model_manager[world_classes[creation_object->class_name].model_name];

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
         _world_mesh_list.push_back(
            object_bbox, object_constants_address, creation_object->position,
            mesh.material.depth_prepass_flags, mesh.material.flags,
            mesh.material.constant_buffer_view,
            world_mesh{.index_buffer_view = model.gpu_buffer.index_buffer_view,
                       .vertex_buffer_views = {model.gpu_buffer.position_vertex_buffer_view,
                                               model.gpu_buffer.attributes_vertex_buffer_view},
                       .index_count = mesh.index_count,
                       .start_index = mesh.start_index,
                       .start_vertex = mesh.start_vertex});
      }

      for (auto& cut : model.terrain_cuts) {
         _terrain_cut_list.push_back(
            {.bbox = creation_object->rotation * cut.bbox + creation_object->position,

             .constant_buffer = object_constants_address,
             .index_buffer_view = model.gpu_buffer.index_buffer_view,
             .position_vertex_buffer_view = model.gpu_buffer.position_vertex_buffer_view,

             .index_count = cut.index_count,
             .start_index = cut.start_index,
             .start_vertex = cut.start_vertex});
      }
   }

   const std::size_t max_drawn_ghost_objects =
      (world.objects.size() + 1) <= max_drawn_objects
         ? max_drawn_objects - (world.objects.size() + 1)
         : 0;

   for (std::size_t i = 0;
        i < std::min(ghost_objects.size(), max_drawn_ghost_objects); ++i) {
      const world::object* object =
         world::find_entity(world.objects, ghost_objects[i].object_id);

      if (not object) continue;

      auto& model = _model_manager[world_classes[object->class_name].model_name];

      const float3 object_position = {ghost_objects[i].transform[3].x,
                                      ghost_objects[i].transform[3].y,
                                      ghost_objects[i].transform[3].z};

      const auto object_bbox =
         make_quat_from_matrix(ghost_objects[i].transform) * model.bbox + object_position;

      const std::size_t object_constants_offset = constants_data_size;
      const gpu_virtual_address object_constants_address =
         constants_upload_gpu_address + object_constants_offset;

      world_mesh_constants constants;

      constants.object_to_world = ghost_objects[i].transform;

      std::memcpy(constants_upload_data + object_constants_offset,
                  &constants.object_to_world, sizeof(world_mesh_constants));

      constants_data_size += sizeof(world_mesh_constants);

      for (auto& mesh : model.parts) {
         _world_mesh_list.push_back(
            object_bbox, object_constants_address, object_position,
            mesh.material.depth_prepass_flags, mesh.material.flags,
            mesh.material.constant_buffer_view,
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
   constexpr std::array minmax = {1.0f, 0.0f};

   [[likely]] if (_device.supports_write_buffer_immediate()) {
      const gpu_virtual_address depth_minmax_buffer =
         _device.get_gpu_virtual_address(_depth_minmax_buffer.get());

      command_list.write_buffer_immediate(depth_minmax_buffer,
                                          std::bit_cast<uint32>(minmax[0]));
      command_list.write_buffer_immediate(depth_minmax_buffer + sizeof(float),
                                          std::bit_cast<uint32>(minmax[1]));
   }
   else {
      auto allocation = _dynamic_buffer_allocator.allocate_and_copy(minmax);

      command_list.copy_buffer_region(_depth_minmax_buffer.get(), 0,
                                      _dynamic_buffer_allocator.resource(),
                                      allocation.offset, sizeof(minmax));
   }
}

void renderer_impl::reduce_depth_minmax(gpu::graphics_command_list& command_list)
{
   profile_section profile{"Reduce Depth Minmax", command_list, _profiler,
                           profiler_queue::direct};

   [[likely]] if (_device.supports_enhanced_barriers()) {
      command_list.deferred_barrier(
         gpu::texture_barrier{.sync_before = gpu::barrier_sync::depth_stencil,
                              .sync_after = gpu::barrier_sync::compute_shading,
                              .access_before = gpu::barrier_access::depth_stencil_write,
                              .access_after = gpu::barrier_access::shader_resource,
                              .layout_before = gpu::barrier_layout::depth_stencil_write,
                              .layout_after = gpu::barrier_layout::direct_queue_shader_resource,
                              .resource = _depth_stencil_texture.get()});
   }
   else {
      command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
         .resource = _depth_stencil_texture.get(),
         .state_before = gpu::legacy_resource_state::depth_write,
         .state_after = gpu::legacy_resource_state::all_shader_resource});
   }
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

   [[likely]] if (_device.supports_enhanced_barriers()) {
      command_list.deferred_barrier(
         gpu::buffer_barrier{.sync_before = gpu::barrier_sync::compute_shading,
                             .sync_after = gpu::barrier_sync::copy,
                             .access_before = gpu::barrier_access::unordered_access,
                             .access_after = gpu::barrier_access::copy_source,
                             .resource = _depth_minmax_buffer.get()});
   }
   else {
      command_list.deferred_barrier(gpu::legacy_resource_transition_barrier{
         .resource = _depth_minmax_buffer.get(),
         .state_before = gpu::legacy_resource_state::unordered_access,
         .state_after = gpu::legacy_resource_state::copy_source});
   }
   command_list.flush_barriers();

   command_list.copy_buffer_region(_depth_minmax_readback_buffer.get(),
                                   sizeof(float4) * _device.frame_index(),
                                   _depth_minmax_buffer.get(), 0, sizeof(float4));
}

void renderer_impl::update_textures(gpu::copy_command_list& command_list)
{
   _texture_manager.eval_updated_textures([&](const updated_textures& updated) noexcept {
      _model_manager.for_each([&](model& model) noexcept {
         for (auto& part : model.parts) {
            [[likely]] if (_device.supports_write_buffer_immediate()) {
               part.material.process_updated_textures(command_list, updated);
            }
            else {
               part.material
                  .process_updated_textures_copy(_device, _dynamic_buffer_allocator,
                                                 model.gpu_buffer.buffer.get(),
                                                 command_list, updated);
            }
         }
      });

      _terrain.process_updated_texture(updated);
      _water.process_updated_texture(updated);
      _ui_texture_manager.process_updated_textures(updated);
   });
}

auto make_renderer(const renderer_init& init) -> std::unique_ptr<renderer>
{
   return std::make_unique<renderer_impl>(init);
}

}
