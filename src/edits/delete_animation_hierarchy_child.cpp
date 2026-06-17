#include "delete_animation_hierarchy_child.hpp"
#include "delete_vector_entry.hpp"

namespace we::edits {

auto make_delete_animation_hierarchy_child(std::vector<uint32>* children, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return make_delete_vector_entry(children, index);
}

auto make_delete_animation_hierarchy_child(std::vector<std::string>* children, uint32 index)
   -> std::unique_ptr<edit<world::edit_context>>
{
   return make_delete_vector_entry(children, index);
}

}