
#include "world_edit.hpp"
#include "hresult_error.hpp"

#include <stdexcept>
#include <type_traits>

#include <fmt/format.h>

#include <d3dx12.h>

namespace sk {

constexpr std::array triangle_vertices{0.0f, 0.25f,  0.0f,   0.25f, -0.25f,
                                       0.0f, -0.25f, -0.25f, 0.0f};

world_edit::world_edit(const HWND window)
   : _window{window},
     _gpu_device{window},
     _triangle_buffer{_gpu_device.create_buffer(sizeof(triangle_vertices),
                                                D3D12_HEAP_TYPE_DEFAULT,
                                                D3D12_RESOURCE_STATE_COPY_DEST)}
{
   auto cpu_buffer =
      _gpu_device.create_buffer(sizeof(triangle_vertices), D3D12_HEAP_TYPE_UPLOAD,
                                D3D12_RESOURCE_STATE_GENERIC_READ);

   const D3D12_RANGE read_range{};
   void* data = nullptr;
   throw_if_failed(cpu_buffer.resource->Map(0, &read_range, &data));

   std::memcpy(data, triangle_vertices.data(), sizeof(triangle_vertices));

   const D3D12_RANGE write_range{0, sizeof(triangle_vertices)};
   cpu_buffer.resource->Unmap(0, &write_range);

   {
      auto command_allocator =
         _gpu_device.aquire_command_allocator(D3D12_COMMAND_LIST_TYPE_COPY);
      auto command_list =
         _gpu_device.aquire_command_list(D3D12_COMMAND_LIST_TYPE_COPY);

      throw_if_failed(command_list->Reset(command_allocator.get(), nullptr));

      command_list->CopyResource(_triangle_buffer.resource, cpu_buffer.resource);

      throw_if_failed(command_list->Close());

      ID3D12CommandList* exec_command_list = command_list.get();

      const UINT64 fence_value = ++_gpu_device.fence_value;

      _gpu_device.copy_command_queue->ExecuteCommandLists(1, &exec_command_list);
      _gpu_device.copy_command_queue->Signal(_gpu_device.copy_fence.get(), fence_value);
      _gpu_device.command_queue->Wait(_gpu_device.copy_fence.get(), fence_value);

      _triangle_buffer.resource_state = D3D12_RESOURCE_STATE_COMMON;
   }
}

bool world_edit::update()
{
   auto& swap_chain = _gpu_device.swap_chain;
   swap_chain.wait_for_ready();

   auto command_allocator =
      _gpu_device.aquire_command_allocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
   auto command_list = _gpu_device.aquire_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT);
   auto [back_buffer, back_buffer_rtv] = swap_chain.current_back_buffer();

   throw_if_failed(command_list->Reset(command_allocator.get(), nullptr));

   const auto rt_barrier =
      CD3DX12_RESOURCE_BARRIER::Transition(&back_buffer, D3D12_RESOURCE_STATE_PRESENT,
                                           D3D12_RESOURCE_STATE_RENDER_TARGET);

   command_list->ResourceBarrier(1, &rt_barrier);

   command_list->ClearRenderTargetView(back_buffer_rtv,
                                       std::array{0.0f, 0.0f, 0.0f, 1.0f}.data(),
                                       0, nullptr);

   const D3D12_VIEWPORT viewport{.Width = static_cast<float>(
                                    _gpu_device.swap_chain.width()),
                                 .Height = static_cast<float>(
                                    _gpu_device.swap_chain.height()),
                                 .MaxDepth = 1.0f};
   const D3D12_RECT sissor_rect{.right =
                                   static_cast<LONG>(_gpu_device.swap_chain.width()),
                                .bottom = static_cast<LONG>(
                                   _gpu_device.swap_chain.height())};
   command_list->RSSetViewports(1, &viewport);
   command_list->RSSetScissorRects(1, &sissor_rect);
   command_list->OMSetRenderTargets(1, &back_buffer_rtv, true, nullptr);

   command_list->SetGraphicsRootSignature(_gpu_device.root_signatures.basic_test.get());
   command_list->SetPipelineState(_gpu_device.pipelines.basic_test.get());

   D3D12_VERTEX_BUFFER_VIEW vbv{_triangle_buffer.resource->GetGPUVirtualAddress(),
                                36, 12};
   command_list->IASetVertexBuffers(0, 1, &vbv);
   command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
   command_list->DrawInstanced(3, 1, 0, 0);

   const auto present_barrier =
      CD3DX12_RESOURCE_BARRIER::Transition(&back_buffer,
                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                           D3D12_RESOURCE_STATE_PRESENT);

   command_list->ResourceBarrier(1, &present_barrier);

   throw_if_failed(command_list->Close());

   ID3D12CommandList* exec_command_list = command_list.get();

   _gpu_device.command_queue->ExecuteCommandLists(1, &exec_command_list);
   _gpu_device.end_frame();

   swap_chain.present();

   _gpu_device.process_deferred_resource_destructions();

   return true;
}

void world_edit::resized(int width, int height)
{
   _gpu_device.wait_for_idle();
   _gpu_device.swap_chain.resize(width, height);
}

void world_edit::focused() {}

void world_edit::unfocused() {}

}