
#include "resource_owner_base.hpp"
#include "device.hpp"

namespace sk::graphics::gpu {

void resource_destroyer_helper::destroy_resource(device& device, ID3D12Resource& resource)
{
   device.deferred_destroy_resource(resource);
}

}