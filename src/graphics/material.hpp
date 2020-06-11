#pragma once

#include "gpu/device.hpp"

#include <boost/variant2/variant.hpp>

namespace sk::graphics {

struct material {
   gpu::descriptor_allocation descriptors;
};

}
