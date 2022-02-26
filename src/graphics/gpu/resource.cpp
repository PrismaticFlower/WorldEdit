
#include "resource.hpp"
#include "device.hpp"

#include <stdexcept>

namespace we::graphics::gpu {

resource::~resource()
{
   if (_parent_device) {
      _parent_device->deferred_destroy_resource(std::move(_resource),
                                                std::move(_allocation));
   }
   else if (not _parent_device and _resource and _allocation) {
      std::terminate();
   }
}

}
