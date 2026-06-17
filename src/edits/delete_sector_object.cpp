#include "delete_vector_entry.hpp"
#include "types.hpp"

namespace we::edits {

auto make_delete_sector_object(std::vector<uint32>* objects, const uint32 object_index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return make_delete_vector_entry(objects, object_index);
}

auto make_delete_sector_object(std::vector<std::string>* objects, const uint32 object_index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return make_delete_vector_entry(objects, object_index);
}

}
