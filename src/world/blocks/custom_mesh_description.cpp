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

block_custom_mesh_description::block_custom_mesh_description(
   const block_custom_mesh_description_stairway_floating& stairway) noexcept
   : type{block_custom_mesh_type::stairway_floating}, stairway_floating{stairway}
{
}

block_custom_mesh_description::block_custom_mesh_description(
   const block_custom_mesh_description_ring& ring) noexcept
   : type{block_custom_mesh_type::ring}, ring{ring}
{
}

block_custom_mesh_description::block_custom_mesh_description(
   const block_custom_mesh_description_beveled_box& beveled_box) noexcept
   : type{block_custom_mesh_type::beveled_box}, beveled_box{beveled_box}
{
}

block_custom_mesh_description::block_custom_mesh_description(
   const block_custom_mesh_description_curve& curve) noexcept
   : type{block_custom_mesh_type::curve}, curve{curve}
{
}

block_custom_mesh_description::block_custom_mesh_description(
   const block_custom_mesh_description_cylinder& cylinder) noexcept
   : type{block_custom_mesh_type::cylinder}, cylinder{cylinder}
{
}

block_custom_mesh_description::block_custom_mesh_description(
   const block_custom_mesh_description_cone& cone) noexcept
   : type{block_custom_mesh_type::cone}, cone{cone}
{
}

block_custom_mesh_description::block_custom_mesh_description(
   const block_custom_mesh_description_arch& arch) noexcept
   : type{block_custom_mesh_type::arch}, arch{arch}
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
   case block_custom_mesh_type::stairway_floating: {
      this->stairway_floating = other.stairway_floating;
   } break;
   case block_custom_mesh_type::ring: {
      this->ring = other.ring;
   } break;
   case block_custom_mesh_type::beveled_box: {
      this->beveled_box = other.beveled_box;
   } break;
   case block_custom_mesh_type::curve: {
      this->curve = other.curve;
   } break;
   case block_custom_mesh_type::cylinder: {
      this->cylinder = other.cylinder;
   } break;
   case block_custom_mesh_type::cone: {
      this->cone = other.cone;
   } break;
   case block_custom_mesh_type::arch: {
      this->arch = other.arch;
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
   case block_custom_mesh_type::stairway_floating:
      return left.stairway_floating == right.stairway_floating;
   case block_custom_mesh_type::ring:
      return left.ring == right.ring;
   case block_custom_mesh_type::beveled_box:
      return left.beveled_box == right.beveled_box;
   case block_custom_mesh_type::curve:
      return left.curve == right.curve;
   case block_custom_mesh_type::cylinder:
      return left.cylinder == right.cylinder;
   case block_custom_mesh_type::cone:
      return left.cone == right.cone;
   case block_custom_mesh_type::arch:
      return left.arch == right.arch;
   }

   std::unreachable();
}

}