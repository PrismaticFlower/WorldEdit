#include "find.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

namespace {

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

auto find_block(const blocks& blocks, const block_id id) -> std::optional<uint32>
{
   switch (id.type()) {
   case block_type::box:
      return find_block(blocks.boxes, id.get_box());
   }

   std::unreachable();
}

}