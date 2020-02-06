
#include "world_edit.hpp"
#include "hresult_error.hpp"

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
   auto& swap_chain = _gpu_device.swap_chain;
   swap_chain.wait_for_ready();

   auto* command_list = _gpu_device.command_list.get();
   auto [back_buffer, back_buffer_rtv] = swap_chain.current_back_buffer();

   {
      auto command_allocator = _gpu_device.aquire_direct_command_allocator();

      throw_if_failed(command_list->Reset(command_allocator.get(), nullptr));

      const auto rt_barrier =
         CD3DX12_RESOURCE_BARRIER::Transition(&back_buffer, D3D12_RESOURCE_STATE_PRESENT,
                                              D3D12_RESOURCE_STATE_RENDER_TARGET);

      command_list->ResourceBarrier(1, &rt_barrier);

      command_list->ClearRenderTargetView(back_buffer_rtv,
                                          std::array{0.15f, 0.05f, 0.5f, 1.0f}.data(),
                                          0, nullptr);

      const auto present_barrier =
         CD3DX12_RESOURCE_BARRIER::Transition(&back_buffer,
                                              D3D12_RESOURCE_STATE_RENDER_TARGET,
                                              D3D12_RESOURCE_STATE_PRESENT);

      command_list->ResourceBarrier(1, &present_barrier);

      throw_if_failed(command_list->Close());

      ID3D12CommandList* exec_command_list = command_list;

      _gpu_device.command_queue->ExecuteCommandLists(1, &exec_command_list);
      _gpu_device.end_frame();

      swap_chain.present();
   }

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