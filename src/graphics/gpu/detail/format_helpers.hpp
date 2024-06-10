#pragma once

#include <dxgiformat.h>

namespace we::graphics::gpu::detail {

auto to_typeless(const DXGI_FORMAT format) noexcept -> DXGI_FORMAT;

}