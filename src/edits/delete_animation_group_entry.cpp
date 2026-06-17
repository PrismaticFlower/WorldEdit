#include "delete_animation_group_entry.hpp"
#include "delete_vector_entry.hpp"

namespace we::edits {

auto make_delete_animation_group_entry(std::vector<world::animation_group::entry>* entries,
                                       uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return make_delete_vector_entry(entries, index);
}

auto make_delete_animation_group_entry(
   std::vector<world::animation_group::entry_broken>* entries, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return make_delete_vector_entry(entries, index);
}

}
