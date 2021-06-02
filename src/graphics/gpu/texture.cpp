#include "texture.hpp"
#include "device.hpp"

namespace we::graphics::gpu {

texture::~texture()
{
   if (_resource) _parent_device->deferred_destroy_resource(_resource);
}

}
