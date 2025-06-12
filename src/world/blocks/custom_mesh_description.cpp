#include "custom_mesh_description.hpp"

#include <utility>

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

block_custom_mesh_description::block_custom_mesh_description(
   const block_custom_mesh_description& other) noexcept
{
   *this = other;
}

block_custom_mesh_description::block_custom_mesh_description(
   const block_custom_mesh_description_stairway& stairway) noexcept
   : type{block_custom_mesh_type::stairway}, stairway{stairway}
{
}

auto block_custom_mesh_description::operator=(const block_custom_mesh_description& other) noexcept
   -> block_custom_mesh_description&
{
   this->type = other.type;

   switch (other.type) {
   case block_custom_mesh_type::stairway: {
      this->stairway = other.stairway;
   } break;
   }

   return *this;
}

bool operator==(const block_custom_mesh_description& left,
                const block_custom_mesh_description& right) noexcept
{
   if (left.type != right.type) return false;

   switch (left.type) {
   case block_custom_mesh_type::stairway:
      return left.stairway == right.stairway;
   }

   std::unreachable();
}

}