#include "ai_overlay_batches.hpp"

namespace we::graphics {

ai_overlay_batches::ai_overlay_batches() noexcept
{
   barriers.reserve(1024);
}

void ai_overlay_batches::clear() noexcept
{
   barriers.clear();
}

bool ai_overlay_batches::empty() const noexcept
{
   return barriers.empty();
}

}
