#pragma once

#include "dynamic_buffer_allocator.hpp"
#include "gpu/rhi.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "types.hpp"

#include <functional>

namespace we::graphics {

enum class line_connect_mode { linear, smooth };

struct line_draw_state {
   float3 line_color = {1.0f, 1.0f, 1.0f};

   gpu_virtual_address camera_constant_buffer_view{};

   line_connect_mode connect_mode = line_connect_mode::linear;
};

class line_draw_context {
public:
   void add(const float3 begin, const float3 end);

private:
   friend void draw_lines(gpu::graphics_command_list& command_list,
                          root_signature_library& root_signatures,
                          pipeline_library& pipelines,
                          dynamic_buffer_allocator& buffer_allocator,
                          const line_draw_state draw_state,
                          std::function<void(line_draw_context&)> draw_callback);

   line_draw_context(gpu::graphics_command_list& command_list,
                     dynamic_buffer_allocator& buffer_allocator,
                     dynamic_buffer_allocator::allocation current_allocation)
      : command_list{command_list}, buffer_allocator{buffer_allocator}, current_allocation{current_allocation}
   {
   }

   void draw_buffered();

   gpu::graphics_command_list& command_list;
   dynamic_buffer_allocator& buffer_allocator;

   dynamic_buffer_allocator::allocation current_allocation;
   uint32 buffered_lines = 0;
};

void draw_lines(gpu::graphics_command_list& command_list,
                root_signature_library& root_signatures, pipeline_library& pipelines,
                dynamic_buffer_allocator& buffer_allocator,
                const line_draw_state draw_state,
                std::function<void(line_draw_context&)> draw_callback);

}
