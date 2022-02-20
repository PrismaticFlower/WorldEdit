
#include "renderer.hpp"
#include "gpu/barrier_helpers.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "line_drawer.hpp"
#include "world/world_utilities.hpp"

#include <d3dx12.h>

#include <range/v3/view.hpp>

namespace we::graphics {

using namespace gpu::literals;

namespace {

// TODO: Put this somewhere.
constexpr float temp_barrier_height = 64.0f;

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

renderer::renderer(const HWND window, std::shared_ptr<settings::graphics> settings,
                   std::shared_ptr<async::thread_pool> thread_pool,
                   assets::libraries_manager& asset_libraries)
   : _window{window},
     _thread_pool{thread_pool},
     _device{window},
     _texture_manager{_device, thread_pool, asset_libraries.textures},
     _model_manager{_device, _texture_manager, asset_libraries.models, thread_pool},
     _settings{settings}
{
   auto imgui_font_descriptor =
      _device.descriptor_heap_srv_cbv_uav.allocate_static(1);

   ImGui_ImplDX12_Init(_device.device_d3d.get(), gpu::render_latency,
                       gpu::swap_chain::format_rtv,
                       &_device.descriptor_heap_srv_cbv_uav.get(),
                       imgui_font_descriptor.start().cpu,
                       imgui_font_descriptor.start().gpu);

   _device.create_constant_buffer_view({.buffer_location =
                                           _camera_constant_buffer.gpu_virtual_address(),
                                        .size = _camera_constant_buffer.size()},
                                       _camera_constant_buffer_view[0]);

   // create object constants upload buffers
   for (auto [buffer, cpu_ptr] :
        ranges::views::zip(_object_constants_upload_buffers,
                           _object_constants_upload_cpu_ptrs)) {
      buffer = _device.create_buffer({.size = objects_constants_buffer_size},
                                     D3D12_HEAP_TYPE_UPLOAD,
                                     D3D12_RESOURCE_STATE_GENERIC_READ);

      const D3D12_RANGE read_range{0, 0};
      void* mapped_ptr = nullptr;
      buffer.view_resource()->Map(0, &read_range, &mapped_ptr);

      cpu_ptr = static_cast<std::byte*>(mapped_ptr);
   }
}

void renderer::draw_frame(
   const camera& camera, const world::world& world,
   const absl::flat_hash_map<lowercase_string, std::shared_ptr<world::object_class>>& world_classes)
{
   auto& swap_chain = _device.swap_chain;
   swap_chain.wait_for_ready();

   _device.copy_manager.enqueue_fence_wait_if_needed(_device.command_queue);

   const frustrum view_frustrum{camera};

   gpu::command_allocator_scoped command_allocator{_device.command_allocator_factory,
                                                   D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                   "World Command Allocator"_id,
                                                   _device.fence,
                                                   _device.fence_value + 1};

   auto& command_list = _world_command_list;
   auto [back_buffer, back_buffer_rtv] = swap_chain.current_back_buffer();
   auto depth_stencil_view = _depth_stencil_texture.depth_stencil_view[0].cpu;

   command_list.reset(command_allocator, _dynamic_buffer_allocator, nullptr);
   _dynamic_buffer_allocator.reset(_device.frame_index);

   command_list.set_descriptor_heaps(_device.descriptor_heap_srv_cbv_uav.get());

   if (std::exchange(_terrain_dirty, false)) {
      _terrain.init(world.terrain, command_list, _dynamic_buffer_allocator);
   }

   _model_manager.update_models();

   update_textures(command_list);
   build_world_mesh_list(command_list, world, world_classes);
   build_object_render_list(view_frustrum);

   update_camera_constant_buffer(camera, command_list);

   _light_clusters.TEMP_render_shadow_maps(camera, view_frustrum, _world_mesh_list,
                                           world, _root_signatures, _pipelines,
                                           command_list, _dynamic_buffer_allocator);
   _light_clusters.update_lights(camera, view_frustrum, world, _root_signatures,
                                 _pipelines, command_list, _dynamic_buffer_allocator);

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(back_buffer, D3D12_RESOURCE_STATE_PRESENT,
                              D3D12_RESOURCE_STATE_RENDER_TARGET));
   command_list.flush_deferred_resource_barriers();

   command_list.clear_render_target_view(back_buffer_rtv,
                                         float4{0.0f, 0.0f, 0.0f, 1.0f});
   command_list.clear_depth_stencil_view(depth_stencil_view,
                                         D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0x0);

   command_list.rs_set_viewports(
      {.Width = static_cast<float>(_device.swap_chain.width()),
       .Height = static_cast<float>(_device.swap_chain.height()),
       .MaxDepth = 1.0f});
   command_list.rs_set_scissor_rects(
      {.right = static_cast<LONG>(_device.swap_chain.width()),
       .bottom = static_cast<LONG>(_device.swap_chain.height())});
   command_list.om_set_render_targets(back_buffer_rtv, depth_stencil_view);

   // Render World
   draw_world(view_frustrum, command_list);

   // Render World Meta Objects
   draw_world_meta_objects(view_frustrum, world, world_classes, command_list);

   // Render ImGui
   ImGui::Render();
   ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.get_underlying());

   command_list.resource_barrier(
      gpu::transition_barrier(back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET,
                              D3D12_RESOURCE_STATE_PRESENT));

   command_list.close();

   _device.command_queue.execute_command_lists(command_list);

   swap_chain.present();

   _device.end_frame();
   _model_manager.trim_models();
}

void renderer::window_resized(uint16 width, uint16 height)
{
   if (width == _device.swap_chain.width() and height == _device.swap_chain.height()) {
      return;
   }

   _device.wait_for_idle();
   _device.swap_chain.resize(width, height);
   _depth_stencil_texture =
      {_device,
       {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .width = _device.swap_chain.width(),
        .height = _device.swap_chain.height(),
        .optimized_clear_value = {.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                  .DepthStencil = {.Depth = 1.0f, .Stencil = 0x0}}},
       D3D12_RESOURCE_STATE_DEPTH_WRITE};

   _light_clusters.update_render_resolution(width, height);
}

void renderer::mark_dirty_terrain() noexcept
{
   _terrain_dirty = true;
}

void renderer::update_camera_constant_buffer(const camera& camera,
                                             gpu::graphics_command_list& command_list)
{
   auto allocation = _dynamic_buffer_allocator.allocate(sizeof(float4x4));

   std::memcpy(allocation.cpu_address, &camera.view_projection_matrix(),
               sizeof(float4x4));

   command_list.copy_buffer_region(*_camera_constant_buffer.resource(), 0,
                                   *_dynamic_buffer_allocator.view_resource(),
                                   allocation.gpu_address -
                                      _dynamic_buffer_allocator.gpu_base_address(),
                                   sizeof(float4x4));

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_camera_constant_buffer.resource(),
                              D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

void renderer::draw_world(const frustrum& view_frustrum,
                          gpu::graphics_command_list& command_list)
{
   draw_world_render_list_depth_prepass(_opaque_object_render_list, command_list);

   _terrain.draw(terrain_draw::depth_prepass, view_frustrum, _camera_constant_buffer_view,
                 _light_clusters.light_descriptors(), command_list,
                 _root_signatures, _pipelines, _dynamic_buffer_allocator);

   draw_world_render_list(_opaque_object_render_list, command_list);

   _terrain.draw(terrain_draw::main, view_frustrum, _camera_constant_buffer_view,
                 _light_clusters.light_descriptors(), command_list,
                 _root_signatures, _pipelines, _dynamic_buffer_allocator);

   draw_world_render_list(_transparent_object_render_list, command_list);
}

void renderer::draw_world_render_list(const std::vector<render_list_item>& list,
                                      gpu::graphics_command_list& command_list)
{
   command_list.set_graphics_root_signature(*_root_signatures.mesh);
   command_list.set_graphics_root_descriptor_table(rs::mesh::camera_descriptor_table,
                                                   _camera_constant_buffer_view);
   command_list.set_graphics_root_descriptor_table(rs::mesh::lights_descriptor_table,
                                                   _light_clusters.light_descriptors());
   command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   ID3D12PipelineState* pipeline_state = nullptr;

   for (auto& object : list) {
      if (pipeline_state != object.pipeline) {
         command_list.set_pipeline_state(*object.pipeline);
      }

      command_list.set_graphics_root_constant_buffer_view(rs::mesh::object_cbv,
                                                          object.object_constants_address);
      command_list.set_graphics_root_descriptor_table(rs::mesh::material_descriptor_table,
                                                      object.material_descriptor_range);
      command_list.ia_set_vertex_buffers(0, object.vertex_buffer_views);
      command_list.ia_set_index_buffer(object.index_buffer_view);
      command_list.draw_indexed_instanced(object.index_count, 1, object.start_index,
                                          object.start_vertex, 0);
   }
}

void renderer::draw_world_render_list_depth_prepass(
   const std::vector<render_list_item>& list, gpu::graphics_command_list& command_list)
{
   command_list.set_graphics_root_signature(*_root_signatures.mesh_depth_prepass);
   command_list.set_graphics_root_descriptor_table(rs::mesh_depth_prepass::camera_descriptor_table,
                                                   _camera_constant_buffer_view);
   command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   ID3D12PipelineState* pipeline_state = nullptr;

   for (auto& object : list) {
      if (pipeline_state != object.depth_prepass_pipeline) {
         command_list.set_pipeline_state(*object.depth_prepass_pipeline);
      }

      command_list.set_graphics_root_constant_buffer_view(rs::mesh_depth_prepass::object_cbv,
                                                          object.object_constants_address);
      command_list.set_graphics_root_descriptor_table(rs::mesh_depth_prepass::material_descriptor_table,
                                                      object.material_descriptor_range);
      command_list.ia_set_vertex_buffers(0, object.vertex_buffer_views);
      command_list.ia_set_index_buffer(object.index_buffer_view);
      command_list.draw_indexed_instanced(object.index_count, 1, object.start_index,
                                          object.start_vertex, 0);
   }
}

void renderer::draw_world_meta_objects(
   const frustrum& view_frustrum, const world::world& world,
   const absl::flat_hash_map<lowercase_string, std::shared_ptr<world::object_class>>& world_classes,
   gpu::graphics_command_list& command_list)
{
   (void)view_frustrum; // TODO: Frustrum Culling (Is it worth it for meta objects?)

   command_list.set_graphics_root_signature(*_root_signatures.meta_mesh);
   command_list.set_graphics_root_descriptor_table(rs::meta_mesh::camera_descriptor_table,
                                                   _camera_constant_buffer_view);

   static bool draw_paths = false;

   ImGui::Checkbox("Draw Paths", &draw_paths);

   if (draw_paths) {
      static bool draw_nodes = true;
      static bool draw_connections = true;
      static bool draw_orientation = false;

      ImGui::Indent();
      ImGui::Checkbox("Draw Nodes", &draw_nodes);
      ImGui::Checkbox("Draw Connections", &draw_connections);
      ImGui::Checkbox("Draw Orientation", &draw_orientation);
      ImGui::Unindent();

      if (draw_nodes) {
         // Set Path Nodes Constants
         {
            struct {
               float4 color;
               float4 outline_color;
               float2 viewport_size;
               float2 viewport_topleft = {0.0f, 0.0f};
            } temp_constants;

            temp_constants.color = float4{_settings->path_node_color(), 1.0f};
            temp_constants.outline_color =
               float4{_settings->path_node_outline_color(), 1.0f};

            temp_constants.viewport_size = {static_cast<float>(
                                               _device.swap_chain.width()),
                                            static_cast<float>(
                                               _device.swap_chain.height())};

            command_list.set_graphics_root_constant_buffer(rs::meta_mesh::color_cbv,
                                                           temp_constants);
         }

         command_list.set_pipeline_state(*_pipelines.meta_mesh_outlined);

         for (auto& path : world.paths) {
            for (auto& node : path.nodes) {
               // TEMP constants setup
               {
                  float4x4 transform = static_cast<float4x4>(node.rotation) *
                                       float4x4{{0.5f, 0.0f, 0.0f, 0.0f},
                                                {0.0f, 0.5f, 0.0f, 0.0f},
                                                {0.0f, 0.0f, 0.5f, 0.0f},
                                                {0.0f, 0.0f, 0.0f, 1.0f}};

                  transform[3] = {node.position, 1.0f};

                  command_list.set_graphics_root_constant_buffer(rs::meta_mesh::object_cbv,
                                                                 transform);
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
         draw_lines(
            command_list, _root_signatures, _pipelines, _dynamic_buffer_allocator,
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
                     float4x4 transform = static_cast<float4x4>(node.rotation);

                     transform[3] = {node.position, 1.0f};

                     for (const auto line : path_node_arrow_wireframe) {
                        const float3 a = transform * float4{line[0], 1.0f};
                        const float3 b = transform * float4{line[1], 1.0f};

                        draw_context.add(a, b);
                     }
                  }
               }
            });
      }
   }

   command_list.set_pipeline_state(*_pipelines.meta_mesh);
   command_list.set_graphics_root_signature(*_root_signatures.meta_mesh);

   command_list.set_graphics_root_descriptor_table(rs::meta_mesh::camera_descriptor_table,
                                                   _camera_constant_buffer_view);
   command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
               return float3{glm::length(region.size)};
            }
            case world::region_shape::cylinder: {
               const float cylinder_length =
                  glm::length(float2{region.size.x, region.size.z});
               return float3{cylinder_length, region.size.y, cylinder_length};
            }
            }
         }();

         float4x4 transform = static_cast<float4x4>(region.rotation) *
                              float4x4{{scale.x, 0.0f, 0.0f, 0.0f},
                                       {0.0f, scale.y, 0.0f, 0.0f},
                                       {0.0f, 0.0f, scale.z, 0.0f},
                                       {0.0f, 0.0f, 0.0f, 1.0f}};

         transform[3] = {region.position, 1.0f};

         command_list.set_graphics_root_constant_buffer(rs::meta_mesh::object_cbv,
                                                        transform);
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

   static bool draw_regions = false;

   ImGui::Checkbox("Draw Regions", &draw_regions);

   if (draw_regions) {
      // Set Regions Color
      {
         command_list.set_graphics_root_constant_buffer(rs::meta_mesh::color_cbv,
                                                        _settings->region_color());
      }

      for (auto& region : world.regions) {
         draw_region(region);
      }
   }

   static bool draw_barriers = false;

   ImGui::Checkbox("Draw Barriers", &draw_barriers);

   if (draw_barriers) {
      // Set Barriers Color
      {
         command_list.set_graphics_root_constant_buffer(rs::meta_mesh::color_cbv,
                                                        _settings->barrier_color());
      }

      // Set Barriers IA State
      {
         const geometric_shape shape = _geometric_shapes.cube();

         command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
         command_list.ia_set_index_buffer(shape.index_buffer_view);
      }

      for (auto& barrier : world.barriers) {
         // TEMP constants setup
         {
            const float2 position = (barrier.corners[0] + barrier.corners[2]) / 2.0f;
            const float2 size{glm::distance(barrier.corners[0], barrier.corners[3]),
                              glm::distance(barrier.corners[0], barrier.corners[1])};
            const float angle =
               std::atan2(barrier.corners[1].x - barrier.corners[0].x,
                          barrier.corners[1].y - barrier.corners[0].y);

            const quaternion rotation{float3{0.0f, angle, 0.0f}};

            float4x4 transform = static_cast<float4x4>(rotation) *
                                 float4x4{{size.x / 2.0f, 0.0f, 0.0f, 0.0f},
                                          {0.0f, temp_barrier_height, 0.0f, 0.0f},
                                          {0.0f, 0.0f, size.y / 2.0f, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {position.x, 0.0f, position.y, 1.0f};

            command_list.set_graphics_root_constant_buffer(rs::meta_mesh::object_cbv,
                                                           transform);
         }

         command_list.draw_indexed_instanced(_geometric_shapes.cube().index_count,
                                             1, 0, 0, 0);
      }
   }

   static bool draw_aabbs = false;

   ImGui::Checkbox("Draw AABBs", &draw_aabbs);

   if (draw_aabbs) {
      // Set AABBs Color
      {
         command_list.set_graphics_root_constant_buffer(rs::meta_mesh::color_cbv,
                                                        _settings->aabb_color());
      }

      // Set Barriers IA State
      {
         const geometric_shape shape = _geometric_shapes.cube();

         command_list.ia_set_vertex_buffers(rs::meta_mesh::object_cbv,
                                            shape.position_vertex_buffer_view);
         command_list.ia_set_index_buffer(shape.index_buffer_view);
      }

      for (auto& object : world.objects) {
         // TEMP constants setup
         {
            const auto& model = world_classes.at(object.class_name)->model;
            const auto object_bbox =
               object.rotation * model->bounding_box + object.position;
            const auto bbox_centre = (object_bbox.min + object_bbox.max) / 2.0f;
            const auto bbox_size = (object_bbox.max - object_bbox.min) / 2.0f;

            float4x4 transform = float4x4{{bbox_size.x, 0.0f, 0.0f, 0.0f},
                                          {0.0f, bbox_size.y, 0.0f, 0.0f},
                                          {0.0f, 0.0f, bbox_size.z, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {bbox_centre, 1.0f};

            command_list.set_graphics_root_constant_buffer(rs::meta_mesh::object_cbv,
                                                           transform);
         }

         command_list.draw_indexed_instanced(_geometric_shapes.cube().index_count,
                                             1, 0, 0, 0);
      }
   }

   static bool draw_draw_light_volumes = false;

   ImGui::Checkbox("Draw Light Volumes", &draw_draw_light_volumes);

   if (draw_draw_light_volumes) {
      const float volume_alpha = _settings->light_volume_alpha();

      for (auto& light : world.lights) {
         // Set Color
         {
            const float4 color{light.color, light.light_type == world::light_type::spot
                                               ? volume_alpha * 0.5f
                                               : volume_alpha};

            command_list.set_graphics_root_constant_buffer(rs::meta_mesh::color_cbv,
                                                           color);
         }

         switch (light.light_type) {
         case world::light_type::directional: {
            if (not light.directional_region) break;

            if (const world::region* const region =
                   world::find_region_by_description(world, *light.directional_region);
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

               command_list.set_graphics_root_constant_buffer(rs::meta_mesh::object_cbv,
                                                              transform);
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
                  static_cast<float4x4>(light.rotation) *
                  static_cast<float4x4>(glm::quat{0.707107f, -0.707107f, 0.0f, 0.0f}) *
                  float4x4{{cone_radius, 0.0f, 0.0f, 0.0f},
                           {0.0f, half_range, 0.0f, 0.0f},
                           {0.0f, 0.0f, cone_radius, 0.0f},
                           {0.0f, -half_range, 0.0f, 1.0f}};

               transform[3] += float4{light.position, 0.0f};

               command_list.set_graphics_root_constant_buffer(rs::meta_mesh::object_cbv,
                                                              transform);
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
}

void renderer::build_world_mesh_list(
   gpu::graphics_command_list& command_list, const world::world& world,
   const absl::flat_hash_map<lowercase_string, std::shared_ptr<world::object_class>>& world_classes)
{
   _world_mesh_list.clear();
   _world_mesh_list.reserve(1024 * 16);

   auto& upload_buffer = _object_constants_upload_buffers[_device.frame_index];

   const gpu::virtual_address constants_upload_gpu_address =
      _object_constants_buffer.gpu_virtual_address();
   std::byte* const constants_upload_data =
      _object_constants_upload_cpu_ptrs[_device.frame_index];
   std::size_t constants_data_size = 0;

   for (std::size_t i = 0; i < std::min(world.objects.size(), max_drawn_objects); ++i) {
      const auto& object = world.objects[i];
      auto& model = _model_manager[world_classes.at(object.class_name)->model_name];

      const auto object_bbox = object.rotation * model.bbox + object.position;

      const std::size_t object_constants_offset = constants_data_size;
      const gpu::virtual_address object_constants_address =
         constants_upload_gpu_address + object_constants_offset;

      world_mesh_constants constants;

      constants.object_to_world = static_cast<float4x4>(object.rotation);
      constants.object_to_world[3] = float4{object.position, 1.0f};

      std::memcpy(constants_upload_data + object_constants_offset,
                  &constants.object_to_world, sizeof(world_mesh_constants));

      constants_data_size += sizeof(world_mesh_constants);

      for (auto& mesh : model.parts) {
         ID3D12PipelineState* const pipeline =
            _pipelines.mesh_normal[mesh.material.flags].get();

         _world_mesh_list.push_back(
            object_bbox, object_constants_address, object.position, pipeline,
            mesh.material.flags, mesh.material.constant_buffer_view.descriptors(),
            world_mesh{.index_buffer_view = model.gpu_buffer.index_buffer_view,
                       .vertex_buffer_views = {model.gpu_buffer.position_vertex_buffer_view,
                                               model.gpu_buffer.attributes_vertex_buffer_view},
                       .index_count = mesh.index_count,
                       .start_index = mesh.start_index,
                       .start_vertex = mesh.start_vertex});

         if (_config.use_raytracing && not mesh.raytracing_blas) {
            mesh.raytracing_blas = create_raytacing_blas(command_list, model, mesh);
         }
      }
   }

   command_list.copy_buffer_region(*_object_constants_buffer.view_resource(), 0,
                                   *upload_buffer.view_resource(), 0,
                                   constants_data_size);
   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_object_constants_buffer.view_resource(),
                              D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER |
                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                                 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

void renderer::build_object_render_list(const frustrum& view_frustrum)
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

      ID3D12PipelineState* const depth_prepass_pipeline = [&]() {
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
         .distance = glm::dot(view_frustrum.planes[frustrum_planes::near_],
                              float4{meshes.position[i], 1.0f}),

         .pipeline = meshes.pipeline[i],
         .depth_prepass_pipeline = depth_prepass_pipeline,

         .index_buffer_view = meshes.mesh[i].index_buffer_view,
         .vertex_buffer_views = {meshes.mesh[i].vertex_buffer_views},

         .object_constants_address = meshes.gpu_constants[i],
         .material_descriptor_range = meshes.material_descriptor_range[i],

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

void renderer::update_textures(gpu::graphics_command_list& command_list)
{
   _texture_manager.eval_updated_textures(
      [&](const absl::flat_hash_map<lowercase_string, std::shared_ptr<const world_texture>>& updated) {
         _model_manager.for_each([&](model& model) {
            for (auto& part : model.parts) {
               part.material.process_updated_textures(command_list,
                                                      _dynamic_buffer_allocator,
                                                      updated);
            }
         });

         _terrain.process_updated_texture(updated);
      });
}

auto renderer::create_raytacing_blas(gpu::graphics_command_list& command_list,
                                     const model& model, const mesh_part& part)
   -> gpu::buffer
{
   D3D12_RAYTRACING_GEOMETRY_DESC geometry_desc{
      .Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
      .Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
      .Triangles =
         {.IndexFormat = DXGI_FORMAT_R16_UINT,
          .VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
          .IndexCount = part.index_count,
          .VertexCount = part.vertex_count,
          .IndexBuffer = model.gpu_buffer.index_buffer_view.BufferLocation +
                         part.start_index * sizeof(uint16),
          .VertexBuffer = {.StartAddress =
                              model.gpu_buffer.position_vertex_buffer_view.BufferLocation +
                              part.start_vertex * sizeof(float3),
                           .StrideInBytes = sizeof(float3)}}};

   D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{
      .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
      .Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
      .NumDescs = 1,
      .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
      .pGeometryDescs = &geometry_desc};

   ID3D12Device8& device = *_device.device_d3d;

   D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild;

   device.GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

   gpu::buffer scratch_buffer =
      _device.create_buffer({.size = to_uint32(prebuild.ScratchDataSizeInBytes),
                             .flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS},
                            D3D12_HEAP_TYPE_DEFAULT,
                            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

   gpu::buffer result_buffer =
      _device.create_buffer({.size = to_uint32(prebuild.ResultDataMaxSizeInBytes),
                             .flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS},
                            D3D12_HEAP_TYPE_DEFAULT,
                            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

   D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc{
      .DestAccelerationStructureData = result_buffer.gpu_virtual_address(),
      .Inputs = inputs,
      .ScratchAccelerationStructureData = scratch_buffer.gpu_virtual_address()};

   command_list.build_raytracing_acceleration_structure(build_desc);
   command_list.deferred_resource_barrier(
      gpu::uav_barrier(*scratch_buffer.view_resource()));
   command_list.deferred_resource_barrier(
      gpu::uav_barrier(*result_buffer.view_resource()));

   // TODO: Compaction solution!

   return result_buffer;
}

}
