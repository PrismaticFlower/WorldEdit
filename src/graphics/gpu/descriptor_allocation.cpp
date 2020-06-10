
#include "descriptor_allocation.hpp"
#include "device.hpp"

namespace sk::graphics::gpu {

descriptor_allocation::~descriptor_allocation()
{
   if (_parent_device) _parent_device->deferred_free_descriptors(*this);
}

}
