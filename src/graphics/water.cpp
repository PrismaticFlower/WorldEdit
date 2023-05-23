#include "water.hpp"

namespace we::graphics {

water::water(gpu::device& device, texture_manager& texture_manager)
   : _device{device}, _texture_manager{texture_manager}
{
}

void water::init(const world::terrain& terrain, gpu::copy_command_list& command_list,
                 dynamic_buffer_allocator& dynamic_buffer_allocator)
{
}

}