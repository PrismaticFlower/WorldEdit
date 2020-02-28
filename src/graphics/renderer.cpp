
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
     _box_vertex_buffer{_device.create_buffer(sizeof(box_vertices), D3D12_HEAP_TYPE_DEFAULT,
                                              D3D12_RESOURCE_STATE_COPY_DEST)},
     _box_index_buffer{_device.create_buffer(sizeof(box_indices), D3D12_HEAP_TYPE_DEFAULT,
                                             D3D12_RESOURCE_STATE_COPY_DEST)}
{
   auto cpu_box_vertex_buffer =
      _device.create_buffer(sizeof(box_vertices), D3D12_HEAP_TYPE_UPLOAD,
                            D3D12_RESOURCE_STATE_GENERIC_READ);

   auto cpu_box_index_buffer =
      _device.create_buffer(sizeof(box_indices), D3D12_HEAP_TYPE_UPLOAD,
                            D3D12_RESOURCE_STATE_GENERIC_READ);

   const D3D12_RANGE read_range{};
   void* data = nullptr;
   throw_if_failed(cpu_box_vertex_buffer.resource->Map(0, &read_range, &data));

   std::memcpy(data, box_vertices.data(), sizeof(box_vertices));

   D3D12_RANGE write_range{0, sizeof(box_vertices)};
   cpu_box_vertex_buffer.resource->Unmap(0, &write_range);

   throw_if_failed(cpu_box_index_buffer.resource->Map(0, &read_range, &data));

   std::memcpy(data, box_indices.data(), sizeof(box_indices));

   write_range = {0, sizeof(box_indices)};
   cpu_box_index_buffer.resource->Unmap(0, &write_range);

   {
      auto command_allocator =
         _device.aquire_command_allocator(D3D12_COMMAND_LIST_TYPE_COPY);
      auto command_list = _device.aquire_command_list(D3D12_COMMAND_LIST_TYPE_COPY);

      throw_if_failed(command_list->Reset(command_allocator.get(), nullptr));

      command_list->CopyResource(_box_vertex_buffer.resource,
                                 cpu_box_vertex_buffer.resource);
      command_list->CopyResource(_box_index_buffer.resource,
                                 cpu_box_index_buffer.resource);

      throw_if_failed(command_list->Close());

      ID3D12CommandList* exec_command_list = command_list.get();

      const UINT64 fence_value = ++_device.copy_fence_value;

      _device.copy_command_queue->ExecuteCommandLists(1, &exec_command_list);
      _device.copy_command_queue->Signal(_device.copy_fence.get(), fence_value);
      _device.command_queue->Wait(_device.copy_fence.get(), fence_value);
   }
}

void renderer::draw_frame(const camera& camera, const world::world& world)
{
   auto& swap_chain = _device.swap_chain;
   swap_chain.wait_for_ready();

   auto& command_allocator = *_command_allocators[_device.frame_index];
   auto command_list = _device.aquire_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT);
   auto [back_buffer, back_buffer_rtv] = swap_chain.current_back_buffer();

   throw_if_failed(command_allocator.Reset());
   throw_if_failed(command_list->Reset(&command_allocator, nullptr));
   _dynamic_buffer_allocator.reset(_device.frame_index);

   const auto rt_barrier =
      CD3DX12_RESOURCE_BARRIER::Transition(&back_buffer, D3D12_RESOURCE_STATE_PRESENT,
                                           D3D12_RESOURCE_STATE_RENDER_TARGET);

   command_list->ResourceBarrier(1, &rt_barrier);

   command_list->ClearRenderTargetView(back_buffer_rtv,
                                       std::array{0.0f, 0.0f, 0.0f, 1.0f}.data(),
                                       0, nullptr);

   const D3D12_VIEWPORT viewport{.Width =
                                    static_cast<float>(_device.swap_chain.width()),
                                 .Height =
                                    static_cast<float>(_device.swap_chain.height()),
                                 .MaxDepth = 1.0f};
   const D3D12_RECT sissor_rect{.right = static_cast<LONG>(_device.swap_chain.width()),
                                .bottom =
                                   static_cast<LONG>(_device.swap_chain.height())};
   command_list->RSSetViewports(1, &viewport);
   command_list->RSSetScissorRects(1, &sissor_rect);
   command_list->OMSetRenderTargets(1, &back_buffer_rtv, true, nullptr);

   command_list->SetGraphicsRootSignature(_device.root_signatures.basic_test.get());
   command_list->SetPipelineState(_device.pipelines.basic_test.get());

   // TEMP Camera Setup
   {
      auto allocation =
         _dynamic_buffer_allocator.allocate(sizeof(sizeof(matrix4x4)));

      std::memcpy(allocation.cpu_address, &camera.view_projection_matrix(),
                  sizeof(matrix4x4));

      command_list->SetGraphicsRootConstantBufferView(0, allocation.gpu_address);
   }

   D3D12_VERTEX_BUFFER_VIEW vbv{_box_vertex_buffer.resource->GetGPUVirtualAddress(),
                                _box_vertex_buffer.size, 12};
   D3D12_INDEX_BUFFER_VIEW ibv{_box_index_buffer.resource->GetGPUVirtualAddress(),
                               _box_index_buffer.size, DXGI_FORMAT_R16_UINT};
   command_list->IASetVertexBuffers(0, 1, &vbv);
   command_list->IASetIndexBuffer(&ibv);
   command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   // TEMP object placeholder rendering
   for (auto& object : world.objects) {
      // TEMP object constants setup
      {
         auto allocation =
            _dynamic_buffer_allocator.allocate(sizeof(sizeof(matrix4x4)));

         matrix4x4 world_transform = matrix4x4{quaternion{object.rotation}};
         world_transform[3] = {object.position, 1.0f};

         std::memcpy(allocation.cpu_address, &world_transform, sizeof(matrix4x4));

         command_list->SetGraphicsRootConstantBufferView(1, allocation.gpu_address);
      }

      command_list->DrawIndexedInstanced(static_cast<UINT>(box_indices.size() * 3),
                                         1, 0, 0, 0);
   }

   const auto present_barrier =
      CD3DX12_RESOURCE_BARRIER::Transition(&back_buffer,
                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                           D3D12_RESOURCE_STATE_PRESENT);

   command_list->ResourceBarrier(1, &present_barrier);

   throw_if_failed(command_list->Close());

   ID3D12CommandList* exec_command_list = command_list.get();

   _device.command_queue->ExecuteCommandLists(1, &exec_command_list);
   _device.end_frame();

   swap_chain.present();

   _device.process_deferred_resource_destructions();
}

void renderer::window_resized(int width, int height)
{
   _device.wait_for_idle();
   _device.swap_chain.resize(width, height);
}

}