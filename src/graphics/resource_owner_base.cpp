
#include "resource_owner_base.hpp"
#include "gpu_device.hpp"

namespace sk::graphics {

void resource_destroyer_helper::destroy_resource(gpu_device& device,
                                                 ID3D12Resource& resource)
{
   device.deferred_destroy_resource(resource);
}

}