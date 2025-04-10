#include "bounding_box.hpp"

#include "math/vector_funcs.hpp"

namespace we::world {

auto get_bounding_box(const block_description_box& box) noexcept -> math::bounding_box
{
   return box.rotation * math::bounding_box{.min = -box.size, .max = box.size} +
          box.position;
}

}
