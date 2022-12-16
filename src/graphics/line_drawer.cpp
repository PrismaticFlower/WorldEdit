
#include "line_drawer.hpp"

namespace we::graphics {

namespace {

constexpr auto max_buffered_lines =
   dynamic_buffer_allocator::alignment * 24 / (sizeof(float3) * 2);

}

void line_draw_context::add(const float3 begin, const float3 end)
{
   std::array<float3, 2> line{begin, end};

   std::memcpy(current_allocation.cpu_address + (sizeof(line) * buffered_lines),
               &line, sizeof(line));

   buffered_lines += 1;

   if (buffered_lines == max_buffered_lines) {
      draw_buffered();

      current_allocation =
         buffer_allocator.allocate(max_buffered_lines * sizeof(float3) * 2);
      buffered_lines = 0;
   }
}

void line_draw_context::draw_buffered()
{
   const gpu::vertex_buffer_view vbv{.buffer_location = current_allocation.gpu_address,
                                     .size_in_bytes =
                                        static_cast<uint32>(current_allocation.size),
                                     .stride_in_bytes = sizeof(float3)};

   command_list.ia_set_vertex_buffers(0, vbv);
   command_list.draw_instanced(buffered_lines * 2, 1, 0, 0);
}

void draw_lines(gpu::graphics_command_list& command_list,
                root_signature_library& root_signatures, pipeline_library& pipelines,
                dynamic_buffer_allocator& buffer_allocator,
                const line_draw_state draw_state,
                std::function<void(line_draw_context&)> draw_callback)
{
   command_list.set_pipeline_state(pipelines.meta_line.get());
   command_list.set_graphics_root_signature(root_signatures.meta_line.get());
   command_list.set_graphics_cbv(rs::meta_line::frame_cbv,
                                 draw_state.camera_constant_buffer_view);

   {
      auto allocation = buffer_allocator.allocate(sizeof(float4));

      const float4 color{draw_state.line_color, 1.0f};

      std::memcpy(allocation.cpu_address, &color, sizeof(float4));

      command_list.set_graphics_cbv(rs::meta_line::color_cbv,
                                            allocation.gpu_address);
   }

   command_list.ia_set_primitive_topology(gpu::primitive_topology::linelist);

   line_draw_context context{command_list, buffer_allocator,
                             buffer_allocator.allocate(max_buffered_lines *
                                                       sizeof(float3) * 2)};

   draw_callback(context);

   if (context.buffered_lines > 0) context.draw_buffered();
}

}
