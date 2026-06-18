#pragma once

#include "types.hpp"

#include <string>

namespace we::world {

struct object;
struct sector;

/// @brief Used to represent optional links. When it doesn't have a link it stores a name instead. This is used for
//  things like animation hierarchy root entitys that may be missing from the world but we still want to be able to
//  load and edit.
template<typename T>
struct entity_optional_link {
   entity_optional_link() = default;

   entity_optional_link(const uint32 entity_index) : _index{entity_index} {}

   explicit entity_optional_link(std::string entity_name)
      : _name{std::move(entity_name)}
   {
   }

   /// @brief If this link has an entity index. Same as calling `not entity_optional_link::has_name`.
   /// @return If the link has an entity index.
   bool has_index() const noexcept
   {
      return _index >= 0;
   }

   /// @brief If this link has an entity name. Same as calling `not entity_optional_link::has_index`.
   /// @return If the link has an entity name.
   bool has_name() const noexcept
   {
      return not has_index();
   }

   /// @brief Get the linked entity index. Only valid if has_index returns true.
   /// @return The index of the entity.
   auto index() const noexcept -> uint32
   {
      if (_index < 0) std::terminate();

      return static_cast<uint32>(_index);
   }

   /// @brief Get the entity name. Only valid if has_name returns true.
   /// @return The entity name.
   auto name() noexcept -> std::string&
   {
      if (_index >= 0) std::terminate();

      return _name;
   }

   /// @brief Get the entity name. Only valid if has_name returns true.
   /// @return The entity name.
   auto name() const noexcept -> const std::string&
   {
      if (_index >= 0) std::terminate();

      return _name;
   }

   /// @brief Gets the name of the linked entity or returns the store entity name if unlinked.
   /// @param world The world the entity_optional_link is part of.
   /// @return The entity name.
   template<typename World>
   auto name_lookup(const World& world) const noexcept -> const std::string&
   {
      if (has_index()) {
         if constexpr (std::is_same_v<T, object>) {
            assert(index() < world.objects.size());

            return world.objects[index()].name;
         }
         else if constexpr (std::is_same_v<T, sector>) {
            assert(index() < world.sector.size());

            return world.sectors[index()].name;
         }
      }
      else {
         return _name;
      }
   }

private:
   int64 _index = -1;
   std::string _name;
};

template<typename T>
inline bool operator==(const entity_optional_link<T>& left,
                       const entity_optional_link<T>& right) noexcept
{
   if (left.has_index() != right.has_index()) return false;

   if (left.has_index()) {
      return left.index() == right.index();
   }
   else {
      return left.name() == right.name();
   }
}

/// @brief Compare an entity_optional_link against an index.
/// @param link The link to compare.
/// @param index The index to compare.
/// @return True if entity_optional_link stores the index, false otherwise.
template<typename T>
inline bool operator==(const entity_optional_link<T>& link, const uint32 index) noexcept
{
   if (not link.has_index()) return false;

   return link.index() == index;
}

using object_optional_link = entity_optional_link<object>;
using sector_optional_link = entity_optional_link<sector>;

}
