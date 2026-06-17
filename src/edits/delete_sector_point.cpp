#include "delete_vector_entry.hpp"
#include "types.hpp"

#include <vector>

namespace we::edits {

auto make_delete_sector_point(std::vector<float2>* points, const uint32 point_index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return make_delete_vector_entry(points, point_index);
}

}
