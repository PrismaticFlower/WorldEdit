#include "is_similar.hpp"

namespace we::world {

bool is_similar(const object& a, const object& b) noexcept
{
   return a.layer == b.layer and a.class_handle == b.class_handle;
}

bool is_similar(const light& a, const light& b) noexcept
{
   return a.layer == b.layer and a.color == b.color and
          a.light_type == b.light_type and a.texture == b.texture;
}

bool is_similar(const region& a, const region& b) noexcept
{
   return a.layer == b.layer and a.description == b.description;
}

bool is_similar(const portal& a, const portal& b) noexcept
{
   return (a.sector1 == b.sector1 and a.sector2 == b.sector2) or
          (a.sector1 == b.sector2 and a.sector2 == b.sector1);
}

bool is_similar(const barrier& a, const barrier& b) noexcept
{
   return a.flags == b.flags;
}

bool is_similar(const hintnode& a, const hintnode& b) noexcept
{
   return a.layer == b.layer and a.type == b.type and a.mode == b.mode;
}

bool is_similar(const planning_connection& a, const planning_connection& b) noexcept
{
   return a.flags == b.flags and a.dynamic_group == b.dynamic_group;
}

}