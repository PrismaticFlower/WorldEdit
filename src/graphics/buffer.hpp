#pragma once

#include "resource_owner_base.hpp"
#include "types.hpp"

#include <cstddef>

#include <d3d12.h>
#include <gsl/gsl>
#include <object_ptr.hpp>

namespace sk::graphics {

struct gpu_device;

struct buffer : resource_owner_base<buffer> {
   jss::object_ptr<gpu_device> parent_gpu_device;

   std::size_t size = 0;
};

}
