
#include "resource.hpp"
#include "device.hpp"

namespace we::graphics::gpu {

resource::~resource()
{
   if (_parent_device) {
      _parent_device->deferred_destroy_resource(_resource, _allocation);
   }
}

}
