
#include "buffer.hpp"
#include "device.hpp"

namespace sk::graphics::gpu {

buffer::~buffer()
{
   if (_resource) _parent_device->deferred_destroy_resource(*this);
}

}
