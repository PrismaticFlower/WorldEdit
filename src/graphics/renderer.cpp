
#include "renderer.hpp"

#include <d3dx12.h>

namespace sk::graphics {

constexpr std::array<float3, 8> box_vertices{{{1.0f, 1.0f, 1.0f},
                                              {-1.0f, 1.0f, -1.0f},
                                              {-1.0f, 1.0f, 1.0f},
                                              {1.0f, -1.0f, -1.0f},
                                              {-1.0f, -1.0f, -1.0f},
                                              {1.0f, 1.0f, -1.0f},
                                              {1.0f, -1.0f, 1.0f},
                                              {-1.0f, -1.0f, 1.0f}}};

constexpr std::array<std::array<uint16, 3>, 12> box_indices{{{0, 1, 2},
                                                             {1, 3, 4},
                                                             {5, 6, 3},
                                                             {7, 3, 6},
                                                             {2, 4, 7},
                                                             {0, 7, 6},
                                                             {0, 5, 1},
                                                             {1, 5, 3},
                                                             {5, 0, 6},
                                                             {7, 4, 3},
                                                             {2, 1, 4},
                                                             {0, 2, 7}}};

renderer::renderer(const HWND window)
   : _window{window},
     _device{window},
     _box_vertex_buffer{_device, sizeof(box_vertices), D3D12_HEAP_TYPE_DEFAULT,
                        D3D12_RESOURCE_STATE_COPY_DEST},
     _box_index_buffer{_device, sizeof(box_indices), D3D12_HEAP_TYPE_DEFAULT,
                       D3D12_RESOURCE_STATE_COPY_DEST}
{
   gpu::buffer cpu_box_vertex_buffer{_device, sizeof(box_vertices),
                                     D3D12_HEAP_TYPE_UPLOAD,
                                     D3D12_RESOURCE_STATE_GENERIC_READ};

   gpu::buffer cpu_box_index_buffer = {_device, sizeof(box_indices),
                                       D3D12_HEAP_TYPE_UPLOAD,
                                       D3D12_RESOURCE_STATE_GENERIC_READ};

   const D3D12_RANGE read_range{};
   void* data = nullptr;
   throw_if_failed(cpu_box_vertex_buffer.resource()->Map(0, &read_range, &data));

   std::memcpy(data, box_vertices.data(), sizeof(box_vertices));

   D3D12_RANGE write_range{0, sizeof(box_vertices)};
   cpu_box_vertex_buffer.resource()->Unmap(0, &write_range);

   throw_if_failed(cpu_box_index_buffer.resource()->Map(0, &read_range, &data));

   std::memcpy(data, box_indices.data(), sizeof(box_indices));

   write_range = {0, sizeof(box_indices)};
   cpu_box_index_buffer.resource()->Unmap(0, &write_range);

   {
      auto& copy_manager = _device.copy_manager;
      auto copy_context = copy_manager.aquire_context();
      auto& command_list = copy_context.command_list;

      command_list.CopyResource(_box_vertex_buffer.resource(),
                                cpu_box_vertex_buffer.resource());
      command_list.CopyResource(_box_index_buffer.resource(),
                                cpu_box_index_buffer.resource());

      const UINT64 fence_value = copy_manager.close_and_execute(copy_context);

      _device.command_queue->Wait(&copy_manager.fence(), fence_value);
   }
}

void renderer::draw_frame(const camera& camera, const world::world& world,
                          const std::unordered_map<std::string, world::object_class>& world_classes)
{
   build_object_render_list(world, world_classes);

   auto& swap_chain = _device.swap_chain;
   swap_chain.wait_for_ready();

   _device.command_queue->Wait(&_device.copy_manager.fence(),
                               _model_manager.copy_fence_wait_value());

   auto& command_allocator = *_world_command_allocators[_device.frame_index];
   auto& command_list = *_world_command_list;
   auto [back_buffer, back_buffer_rtv] = swap_chain.current_back_buffer();

   throw_if_failed(command_allocator.Reset());
   throw_if_failed(command_list.Reset(&command_allocator, nullptr));
   _dynamic_buffer_allocator.reset(_device.frame_index);

   const auto rt_barrier =
      CD3DX12_RESOURCE_BARRIER::Transition(&back_buffer, D3D12_RESOURCE_STATE_PRESENT,
                                           D3D12_RESOURCE_STATE_RENDER_TARGET);

   command_list.ResourceBarrier(1, &rt_barrier);

   command_list.ClearRenderTargetView(back_buffer_rtv,
                                      std::array{0.0f, 0.0f, 0.0f, 1.0f}.data(),
                                      0, nullptr);
   command_list.ClearDepthStencilView(_depth_stencil_texture.depth_stencil_view,
                                      D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0x0, 0, nullptr);

   const D3D12_VIEWPORT viewport{.Width =
                                    static_cast<float>(_device.swap_chain.width()),
                                 .Height =
                                    static_cast<float>(_device.swap_chain.height()),
                                 .MaxDepth = 1.0f};
   const D3D12_RECT sissor_rect{.right = static_cast<LONG>(_device.swap_chain.width()),
                                .bottom =
                                   static_cast<LONG>(_device.swap_chain.height())};
   command_list.RSSetViewports(1, &viewport);
   command_list.RSSetScissorRects(1, &sissor_rect);
   command_list.OMSetRenderTargets(1, &back_buffer_rtv, true,
                                   &_depth_stencil_texture.depth_stencil_view);

   command_list.SetGraphicsRootSignature(_device.root_signatures.basic_test.get());
   command_list.SetPipelineState(_device.pipelines.basic_test.get());

   // TEMP Camera Setup
   {
      auto allocation = _dynamic_buffer_allocator.allocate(sizeof(sizeof(float4x4)));

      std::memcpy(allocation.cpu_address, &camera.view_projection_matrix(),
                  sizeof(float4x4));

      command_list.SetGraphicsRootConstantBufferView(0, allocation.gpu_address);
   }

   command_list.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   // TEMP object placeholder rendering
   for (auto& object : _object_render_list) {
      // TEMP object constants setup
      {
         auto allocation =
            _dynamic_buffer_allocator.allocate(sizeof(sizeof(float4x4)));

         std::memcpy(allocation.cpu_address, &object.transform, sizeof(float4x4));

         command_list.SetGraphicsRootConstantBufferView(1, allocation.gpu_address);
      }

      command_list.IASetVertexBuffers(0,
                                      static_cast<UINT>(
                                         object.vertex_buffer_views.size()),
                                      object.vertex_buffer_views.data());
      command_list.IASetIndexBuffer(&object.index_buffer_view);
      command_list.DrawIndexedInstanced(object.index_count, 1, object.start_index,
                                        object.start_vertex, 0);
   }

   const auto present_barrier =
      CD3DX12_RESOURCE_BARRIER::Transition(&back_buffer,
                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                           D3D12_RESOURCE_STATE_PRESENT);

   command_list.ResourceBarrier(1, &present_barrier);

   throw_if_failed(command_list.Close());

   ID3D12CommandList* exec_command_list = &command_list;

   _device.command_queue->ExecuteCommandLists(1, &exec_command_list);

   swap_chain.present();

   _device.end_frame();
}

void renderer::window_resized(uint16 width, uint16 height)
{
   _device.wait_for_idle();
   _device.swap_chain.resize(width, height);
   _depth_stencil_texture =
      {_device,
       {.format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .width = _device.swap_chain.width(),
        .height = _device.swap_chain.height(),
        .optimized_clear_value = {.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                                  .DepthStencil = {.Depth = 0.0f, .Stencil = 0x0}}},
       D3D12_HEAP_TYPE_DEFAULT,
       D3D12_RESOURCE_STATE_DEPTH_WRITE};
}

void renderer::build_object_render_list(
   const world::world& world,
   const std::unordered_map<std::string, world::object_class>& world_classes)
{
   _object_render_list.clear();
   _object_render_list.reserve(world.objects.size());

   for (auto& object : world.objects) {
      auto& model = _model_manager.get(world_classes.at(object.class_name).model);

      float4x4 transform = static_cast<float4x4>(object.rotation);
      transform[3] = float4{object.position, 1.0f};

      for (const auto& mesh : model.parts) {
         _object_render_list.push_back(
            {.distance = 0.0f, // TODO: Distance, frustum culling...
             .index_count = mesh.index_count,
             .start_index = mesh.start_index,
             .start_vertex = mesh.start_vertex,
             .index_buffer_view = model.gpu_buffer.index_buffer_view,
             .vertex_buffer_views = {model.gpu_buffer.position_vertex_buffer_view,
                                     model.gpu_buffer.normal_vertex_buffer_view,
                                     model.gpu_buffer.texcoord_vertex_buffer_view},
             .transform = transform});
      }
   }
}

}