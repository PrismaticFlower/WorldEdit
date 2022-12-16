
#include "dynamic_buffer_allocator.hpp"

namespace we::graphics {

dynamic_buffer_allocator::dynamic_buffer_allocator(const uint32 max_size,
                                                   gpu::device& device)
{
   _frame_section_size = math::align_up(max_size, alignment);

   const uint64 buffer_size = _frame_section_size * gpu::frame_pipeline_length;

   _buffer = {device.create_buffer({.size = buffer_size, .debug_name = "dynamic buffer allocator backing memory"},
                                   gpu::heap_type::upload),
              device.direct_queue};

   _cpu_base_address = static_cast<std::byte*>(device.map(_buffer.get(), 0, {0, 0}));
   _gpu_base_address = device.get_gpu_virtual_address(_buffer.get());
}
}
