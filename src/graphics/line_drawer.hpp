#pragma once

#include "gpu/command_list.hpp"
#include "gpu/descriptor_range.hpp"
#include "gpu/device.hpp"
#include "gpu/dynamic_buffer_allocator.hpp"
#include "pipeline_library.hpp"
#include "root_signature_library.hpp"
#include "types.hpp"

#include <functional>

namespace we::graphics {

enum class line_connect_mode { linear, smooth };

struct line_draw_state {
   float3 line_color = {1.0f, 1.0f, 1.0f};

   gpu::descriptor_range camera_constant_buffer_view{};

   line_connect_mode connect_mode = line_connect_mode::linear;
};

class line_draw_context {
public:
   void add(const float3 begin, const float3 end);

private:
   friend void draw_lines(gpu::command_list& command_list,
                          root_signature_library& root_signatures,
                          pipeline_library& pipelines,
                          gpu::dynamic_buffer_allocator& buffer_allocator,
                          const line_draw_state draw_state,
                          std::function<void(line_draw_context&)> draw_callback);

   line_draw_context(gpu::command_list& command_list,
                     gpu::dynamic_buffer_allocator& buffer_allocator,
                     gpu::dynamic_buffer_allocator::allocation current_allocation)
      : command_list{command_list}, buffer_allocator{buffer_allocator}, current_allocation{current_allocation}
   {
   }

   void draw_buffered();

   gpu::command_list& command_list;
   gpu::dynamic_buffer_allocator& buffer_allocator;

   gpu::dynamic_buffer_allocator::allocation current_allocation;
   uint32 buffered_lines = 0;
};

void draw_lines(gpu::command_list& command_list,
                root_signature_library& root_signatures, pipeline_library& pipelines,
                gpu::dynamic_buffer_allocator& buffer_allocator,
                const line_draw_state draw_state,
                std::function<void(line_draw_context&)> draw_callback);

}
