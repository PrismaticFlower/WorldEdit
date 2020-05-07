#pragma once

#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "types.hpp"

#include <functional>

namespace sk::graphics {

enum class line_connect_mode { linear, smooth };

struct line_draw_state {
   float3 line_color = {1.0f, 1.0f, 1.0f};

   D3D12_GPU_VIRTUAL_ADDRESS camera_constants_address{};

   line_connect_mode connect_mode = line_connect_mode::linear;
};

class line_draw_context {
public:
   void add(const float3 begin, const float3 end);

private:
   friend void draw_lines(ID3D12GraphicsCommandList5& command_list, gpu::device& device,
                          gpu::dynamic_buffer_allocator& buffer_allocator,
                          const line_draw_state draw_state,
                          std::function<void(line_draw_context&)> draw_callback);

   line_draw_context(ID3D12GraphicsCommandList5& command_list,
                     gpu::dynamic_buffer_allocator& buffer_allocator,
                     gpu::dynamic_buffer_allocator::allocation current_allocation)
      : command_list{command_list}, buffer_allocator{buffer_allocator}, current_allocation{current_allocation}
   {
   }

   void draw_buffered();

   ID3D12GraphicsCommandList5& command_list;
   gpu::dynamic_buffer_allocator& buffer_allocator;

   gpu::dynamic_buffer_allocator::allocation current_allocation;
   uint32 buffered_lines = 0;
};

void draw_lines(ID3D12GraphicsCommandList5& command_list, gpu::device& device,
                gpu::dynamic_buffer_allocator& buffer_allocator,
                const line_draw_state draw_state,
                std::function<void(line_draw_context&)> draw_callback);

}