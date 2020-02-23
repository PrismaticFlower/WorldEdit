#pragma once

#include "resource_owner_base.hpp"
#include "types.hpp"

#include <cstddef>

#include <d3d12.h>
#include <gsl/gsl>
#include <object_ptr.hpp>

namespace sk::graphics::gpu {

struct device;

struct buffer : resource_owner_base<buffer> {
   jss::object_ptr<device> parent_device;

   UINT size = 0;
};

}
