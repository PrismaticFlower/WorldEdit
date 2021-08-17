#pragma once

#include "types.hpp"

#include <string_view>

#include <d3d12.h>

namespace we::graphics::gpu {

inline void set_debug_name(ID3D12Object& child, std::string_view name) noexcept
{
   child.SetPrivateData(WKPDID_D3DDebugObjectName, to_uint32(name.size()),
                        name.data());
}

}
