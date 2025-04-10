#include "bounding_box.hpp"

#include "math/vector_funcs.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

auto get_bounding_box(const block_description_box& box) noexcept -> math::bounding_box
{
   return box.rotation * math::bounding_box{.min = -box.size, .max = box.size} +
          box.position;
}

auto get_bounding_box(const blocks& blocks, const block_type type,
                      const uint32 block_index) noexcept -> math::bounding_box
{
   switch (type) {
   case world::block_type::box: {
      return get_bounding_box(blocks.boxes.description[block_index]);
   } break;
   }

   std::unreachable();
}

}
