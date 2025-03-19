#include "find.hpp"

namespace we::world {

auto find_block(const blocks_boxes& boxes, const block_box_id id)
   -> std::optional<uint32>
{
   if (auto it = std::lower_bound(boxes.ids.begin(), boxes.ids.end(), id,
                                  [](const block_box_id left, const block_box_id right) {
                                     return left < right;
                                  });
       it != boxes.ids.end()) {
      if (*it == id) return static_cast<uint32>(it - boxes.ids.begin());
   }

   return std::nullopt;
}

}