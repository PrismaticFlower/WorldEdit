
#include "resource_view_set.hpp"
#include "device.hpp"

namespace we::graphics::gpu {

resource_view_set::~resource_view_set()
{
   if (not _parent_device) return;

   _parent_device->deferred_free_descriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                                             _descriptor_range);

   for (auto& resource : _resources) {
      _parent_device->deferred_destroy_resource(resource);
   }
}

}
