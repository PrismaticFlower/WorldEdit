
#include "renderer.hpp"
#include "gpu/barrier_helpers.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "line_drawer.hpp"
#include "world/world_utilities.hpp"

#include <d3dx12.h>

#include <range/v3/view.hpp>

namespace sk::graphics {

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

// TODO: Put this somewhere.

struct object_constants {
   float4x4 object_to_world;
};

static_assert(sizeof(object_constants) == 64);

}

renderer::renderer(const HWND window) : _window{window}, _device{window}
{
   auto imgui_font_descriptor =
      _device.descriptor_heap_srv_cbv_uav.allocate_static(1);

   ImGui_ImplDX12_Init(_device.device_d3d.get(), gpu::render_latency,
                       gpu::swap_chain::format_rtv,
                       &_device.descriptor_heap_srv_cbv_uav.get(),
                       imgui_font_descriptor.start().cpu,
                       imgui_font_descriptor.start().gpu);

   const D3D12_CONSTANT_BUFFER_VIEW_DESC camera_cbv_desc{
      .BufferLocation = _camera_constant_buffer.resource()->GetGPUVirtualAddress(),
      .SizeInBytes = _camera_constant_buffer.size()};

   _device.device_d3d->CreateConstantBufferView(&camera_cbv_desc,
                                                _camera_constant_buffer_view[0].cpu);
}

void renderer::draw_frame(const camera& camera, const world::world& world,
                          const std::unordered_map<std::string, world::object_class>& world_classes)
{
   auto& swap_chain = _device.swap_chain;
   swap_chain.wait_for_ready();

   _device.copy_manager.enqueue_fence_wait_if_needed(*_device.command_queue);

   auto& command_allocator = *_world_command_allocators[_device.frame_index];
   auto& command_list = _world_command_list;
   auto [back_buffer, back_buffer_rtv] = swap_chain.current_back_buffer();
   auto depth_stencil_view = _depth_stencil_texture.depth_stencil_view[0].cpu;

   throw_if_failed(command_allocator.Reset());
   command_list.reset(command_allocator, _dynamic_buffer_allocator, nullptr);
   _dynamic_buffer_allocator.reset(_device.frame_index);

   command_list.set_descriptor_heaps(_device.descriptor_heap_srv_cbv_uav.get());

   const frustrum view_frustrum{camera};

   update_camera_constant_buffer(camera, command_list);

   _light_clusters.update_lights(view_frustrum, world, command_list,
                                 _dynamic_buffer_allocator);

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(back_buffer, D3D12_RESOURCE_STATE_PRESENT,
                              D3D12_RESOURCE_STATE_RENDER_TARGET));
   command_list.flush_deferred_resource_barriers();

   command_list.clear_render_target_view(back_buffer_rtv,
                                         float4{0.0f, 0.0f, 0.0f, 1.0f});
   command_list.clear_depth_stencil_view(depth_stencil_view,
                                         D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0x0);

   command_list.rs_set_viewports(
      {.Width = static_cast<float>(_device.swap_chain.width()),
       .Height = static_cast<float>(_device.swap_chain.height()),
       .MaxDepth = 1.0f});
   command_list.rs_set_scissor_rects(
      {.right = static_cast<LONG>(_device.swap_chain.width()),
       .bottom = static_cast<LONG>(_device.swap_chain.height())});
   command_list.om_set_render_targets(back_buffer_rtv, depth_stencil_view);

   // Render World
   draw_world(view_frustrum, world, world_classes, command_list);

   // Render World Meta Objects
   draw_world_meta_objects(view_frustrum, world, world_classes, command_list);

   // Render ImGui
   ImGui::Render();
   ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.get_underlying());

   command_list.resource_barrier(
      gpu::transition_barrier(back_buffer, D3D12_RESOURCE_STATE_RENDER_TARGET,
                              D3D12_RESOURCE_STATE_PRESENT));

   command_list.close();

   ID3D12CommandList* exec_command_list = command_list.get_underlying();

   _device.command_queue->ExecuteCommandLists(1, &exec_command_list);

   swap_chain.present();

   _device.end_frame();
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
                                  .DepthStencil = {.Depth = 0.0f, .Stencil = 0x0}}},
       D3D12_RESOURCE_STATE_DEPTH_WRITE};
}

void renderer::update_camera_constant_buffer(const camera& camera,
                                             gpu::command_list& command_list)
{
   auto allocation = _dynamic_buffer_allocator.allocate(sizeof(float4x4));

   std::memcpy(allocation.cpu_address, &camera.view_projection_matrix(),
               sizeof(float4x4));

   command_list.copy_buffer_region(*_camera_constant_buffer.resource(), 0,
                                   *_dynamic_buffer_allocator.resource(),
                                   allocation.gpu_address -
                                      _dynamic_buffer_allocator.gpu_base_address(),
                                   sizeof(float4x4));

   command_list.deferred_resource_barrier(
      gpu::transition_barrier(*_camera_constant_buffer.resource(),
                              D3D12_RESOURCE_STATE_COPY_DEST,
                              D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
}

void renderer::draw_world(const frustrum& view_frustrum, const world::world& world,
                          const std::unordered_map<std::string, world::object_class>& world_classes,
                          gpu::command_list& command_list)
{
   build_object_render_list(view_frustrum, world, world_classes);

   command_list.set_graphics_root_signature(*_device.root_signatures.object_mesh);
   command_list.set_pipeline_state(*_device.pipelines.basic_mesh_lighting.get());

   command_list.set_graphics_root_descriptor_table(1, _camera_constant_buffer_view);
   command_list.set_graphics_root_descriptor_table(2, _light_clusters.light_descriptors());
   command_list.ia_set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   for (auto& object : _object_render_list) {
      command_list.set_graphics_root_constant_buffer_view(0, object.object_constants_address);
      command_list.ia_set_vertex_buffers(0, object.vertex_buffer_views);
      command_list.ia_set_index_buffer(object.index_buffer_view);
      command_list.draw_indexed_instanced(object.index_count, 1, object.start_index,
                                          object.start_vertex, 0);
   }
}

void renderer::draw_world_meta_objects(
   const frustrum& view_frustrum, const world::world& world,
   const std::unordered_map<std::string, world::object_class>& world_classes,
   gpu::command_list& command_list)
{
   (void)view_frustrum; // TODO: Frustrum Culling (Is it worth it for meta objects?)

   command_list.set_graphics_root_signature(*_device.root_signatures.meta_object_mesh);
   command_list.set_graphics_root_descriptor_table(2, _camera_constant_buffer_view);

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
               float4 color{0.15f, 1.0f, 0.3f, 1.0f};
               float4 outline_color{0.1125f, 0.6f, 0.2225f, 1.0f};
               float2 viewport_size;
               float2 viewport_topleft = {0.0f, 0.0f};
            } temp_constants;

            temp_constants.viewport_size = {static_cast<float>(
                                               _device.swap_chain.width()),
                                            static_cast<float>(
                                               _device.swap_chain.height())};

            command_list.set_graphics_root_constant_buffer(1, temp_constants);
         }

         command_list.set_pipeline_state(*_device.pipelines.meta_object_mesh_outlined);

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

                  command_list.set_graphics_root_constant_buffer(0, transform);
               }

               const geometric_shape shape = _geometric_shapes.octahedron();

               command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
               command_list.ia_set_index_buffer(shape.index_buffer_view);
               command_list.draw_indexed_instanced(shape.index_count, 1, 0, 0, 0);
            }
         }
      }

      if (draw_connections) {
         draw_lines(command_list, _device, _dynamic_buffer_allocator,
                    {.line_color = {0.1f, 0.1f, 0.75f},

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
            command_list, _device, _dynamic_buffer_allocator,
            {.line_color = {1.0f, 1.0f, 0.1f},

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

   command_list.set_pipeline_state(*_device.pipelines.meta_object_transparent_mesh);
   command_list.set_graphics_root_signature(*_device.root_signatures.meta_object_mesh);

   command_list.set_graphics_root_descriptor_table(2, _camera_constant_buffer_view);
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

         command_list.set_graphics_root_constant_buffer(0, transform);
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
         const float4 color{0.25f, 0.4f, 1.0f, 0.3f};

         command_list.set_graphics_root_constant_buffer(1, color);
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
         const float4 color{1.0f, 0.1f, 0.5f, 0.3f};

         command_list.set_graphics_root_constant_buffer(1, color);
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

            command_list.set_graphics_root_constant_buffer(0, transform);
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
         const float4 color{0.0f, 1.0f, 0.0f, 0.3f};

         command_list.set_graphics_root_constant_buffer(1, color);
      }

      // Set Barriers IA State
      {
         const geometric_shape shape = _geometric_shapes.cube();

         command_list.ia_set_vertex_buffers(0, shape.position_vertex_buffer_view);
         command_list.ia_set_index_buffer(shape.index_buffer_view);
      }

      for (auto& object : world.objects) {
         // TEMP constants setup
         {
            const auto& model = world_classes.at(object.class_name).model;
            const auto object_bbox =
               object.rotation * model->bounding_box + object.position;
            const auto bbox_centre = (object_bbox.min + object_bbox.max) / 2.0f;
            const auto bbox_size = (object_bbox.max - object_bbox.min) / 2.0f;

            float4x4 transform = float4x4{{bbox_size.x, 0.0f, 0.0f, 0.0f},
                                          {0.0f, bbox_size.y, 0.0f, 0.0f},
                                          {0.0f, 0.0f, bbox_size.z, 0.0f},
                                          {0.0f, 0.0f, 0.0f, 1.0f}};

            transform[3] = {bbox_centre, 1.0f};

            command_list.set_graphics_root_constant_buffer(0, transform);
         }

         command_list.draw_indexed_instanced(_geometric_shapes.cube().index_count,
                                             1, 0, 0, 0);
      }
   }

   static bool draw_draw_light_volumes = false;

   ImGui::Checkbox("Draw Light Volumes", &draw_draw_light_volumes);

   if (draw_draw_light_volumes) {
      for (auto& light : world.lights) {
         // Set Color
         {
            const float4 color{light.color, light.light_type == world::light_type::spot
                                               ? 0.025f
                                               : 0.05f};

            command_list.set_graphics_root_constant_buffer(1, color);
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

               command_list.set_graphics_root_constant_buffer(0, transform);
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

               command_list.set_graphics_root_constant_buffer(0, transform);
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

void renderer::build_object_render_list(
   const frustrum& view_frustrum, const world::world& world,
   const std::unordered_map<std::string, world::object_class>& world_classes)
{
   _object_render_list.clear();
   _object_render_list.reserve(world.objects.size());

   for (auto& object : world.objects) {
      const auto& model =
         _model_manager.get(world_classes.at(object.class_name).model);

      const auto object_bbox = object.rotation * model.bbox + object.position;

      if (!intersects(view_frustrum, object_bbox)) continue;

      const D3D12_GPU_VIRTUAL_ADDRESS object_constants_address = [&] {
         auto allocation =
            _dynamic_buffer_allocator.allocate(sizeof(object_constants));

         object_constants constants;

         constants.object_to_world = static_cast<float4x4>(object.rotation);
         constants.object_to_world[3] = float4{object.position, 1.0f};

         std::memcpy(allocation.cpu_address, &constants.object_to_world,
                     sizeof(object_constants));

         return allocation.gpu_address;
      }();

      for (const auto& mesh : model.parts) {
         _object_render_list.push_back(
            {.index_count = mesh.index_count,
             .start_index = mesh.start_index,
             .start_vertex = mesh.start_vertex,
             .index_buffer_view = model.gpu_buffer.index_buffer_view,
             .vertex_buffer_views = {model.gpu_buffer.position_vertex_buffer_view,
                                     model.gpu_buffer.normal_vertex_buffer_view,
                                     model.gpu_buffer.texcoord_vertex_buffer_view},
             .object_constants_address = object_constants_address});
      }
   }
}
}
