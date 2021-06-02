
#include "buffer.hpp"
#include "device.hpp"

namespace we::graphics::gpu {

buffer::~buffer()
{
   if (_resource) _parent_device->deferred_destroy_resource(_resource);
}

}
