
#include "world_edit.hpp"

#include <hresult_error.hpp>

#include <stdexcept>
#include <type_traits>

#include <fmt/format.h>

#include <d3dx12.h>

namespace sk {

world_edit::world_edit(const HWND window) : _window{window}, _gpu_device{window}
{
}

bool world_edit::update()
{
   const auto frame_index = _gpu_device.frame_index;
   auto* command_list = _gpu_device.command_list.get();

   throw_if_failed(
      command_list->Reset(_gpu_device.command_allocators[frame_index].get(), nullptr));

   const auto rt_barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      _gpu_device.swap_chain_render_targets[frame_index].get(),
      D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

   command_list->ResourceBarrier(1, &rt_barrier);

   CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_descriptor(
      _gpu_device.swap_chain_descriptors->GetCPUDescriptorHandleForHeapStart(),
      frame_index, _gpu_device.rtv_descriptor_size);

   command_list->ClearRenderTargetView(rtv_descriptor,
                                       std::array{0.15f, 0.05f, 0.5f, 1.0f}.data(),
                                       0, nullptr);

   const auto present_barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      _gpu_device.swap_chain_render_targets[frame_index].get(),
      D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

   command_list->ResourceBarrier(1, &present_barrier);

   throw_if_failed(command_list->Close());

   ID3D12CommandList* exec_command_list = command_list;

   _gpu_device.command_queue->ExecuteCommandLists(1, &exec_command_list);
   _gpu_device.swap_chain->Present(0, 0);

   _gpu_device.end_frame();

   return true;
}

void world_edit::resized(int width, int height)
{
   (void)(width, height);
}

void world_edit::focused() {}

void world_edit::unfocused() {}

}