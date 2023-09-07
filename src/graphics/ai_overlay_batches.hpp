#pragma once

#include "types.hpp"

#include <array>
#include <vector>

namespace we::graphics {

struct ai_overlay_batches {
   ai_overlay_batches() noexcept;

   void clear() noexcept;

   bool empty() const noexcept;

   std::vector<float4x4> barriers;
   std::vector<float4x4> hubs;
   std::vector<std::array<float3, 36>> connections;
};

}