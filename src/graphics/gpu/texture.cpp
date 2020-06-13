#include "texture.hpp"
#include "device.hpp"

namespace sk::graphics::gpu {

texture::~texture()
{
   if (_resource) _parent_device->deferred_destroy_resource(_resource);
}

}
