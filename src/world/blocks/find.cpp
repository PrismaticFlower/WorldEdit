#include "find.hpp"

#include "utility/string_icompare.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(default : 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled

namespace we::world {

namespace {

template<typename T>
auto find_block_impl(const pinned_vector<T>& ids, const std::type_identity_t<T> id)
   -> std::optional<uint32>
{
   if (auto it = std::lower_bound(ids.begin(), ids.end(), id,
                                  [](const T left, const T right) {
                                     return left < right;
                                  });
       it != ids.end()) {
      if (*it == id) return static_cast<uint32>(it - ids.begin());
   }

   return std::nullopt;
}

}

auto find_block(const blocks& blocks, const block_id id) -> std::optional<uint32>
{
   switch (id.type()) {
   case block_type::box:
      return find_block_impl(blocks.boxes.ids, id.get_box());
   case block_type::ramp:
      return find_block_impl(blocks.ramps.ids, id.get_ramp());
   }

   std::unreachable();
}

auto find_block_equivalent_material(const blocks& blocks, const block_material& material)
   -> std::optional<uint8>
{
   using string::iequals;

   for (uint32 material_index = 0; material_index < world::max_block_materials;
        ++material_index) {
      const block_material& other = blocks.materials[material_index];

      if (not iequals(other.diffuse_map, material.diffuse_map)) continue;
      if (not iequals(other.normal_map, material.normal_map)) continue;
      if (not iequals(other.detail_map, material.detail_map)) continue;
      if (not iequals(other.env_map, material.env_map)) continue;
      if (other.detail_tiling != material.detail_tiling) continue;
      if (other.tile_normal_map != material.tile_normal_map) continue;
      if (other.specular_lighting != material.specular_lighting) continue;
      if (other.foley_group != material.foley_group) continue;

      return static_cast<uint8>(material_index);
   }

   return std::nullopt;
}

auto find_block_empty_material(const blocks& blocks) -> std::optional<uint8>
{
   using string::iequals;

   const block_material empty_material;

   for (uint32 material_index = 0; material_index < world::max_block_materials;
        ++material_index) {
      const block_material& other = blocks.materials[material_index];

      if (other != empty_material) continue;

      return static_cast<uint8>(material_index);
   }

   return std::nullopt;
}

}