#pragma once

#include "types.hpp"

#include <vector>

namespace we::graphics {

struct ai_overlay_batches {
   ai_overlay_batches() noexcept;

   void clear() noexcept;

   bool empty() const noexcept;

   std::vector<float4x4> barriers;
};

}