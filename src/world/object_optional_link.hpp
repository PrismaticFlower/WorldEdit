#pragma once

#include "types.hpp"

#include <string>

namespace we::world {

struct world;

/// @brief Used to represent optional links. When it doesn't have a link it stores a name instead. This is used for
//  things like animation hierarchy root objects that may be missing from the world but we still want to be able to
//  load and edit.
struct object_optional_link {
   object_optional_link() = default;

   object_optional_link(const uint32 object_index);

   explicit object_optional_link(std::string object_name);

   /// @brief If this link has an object index. Same as calling `not object_optional_link::has_name`.
   /// @return If the link has an object index.
   bool has_index() const noexcept;

   /// @brief If this link has an object name. Same as calling `not object_optional_link::has_index`.
   /// @return If the link has an object name.
   bool has_name() const noexcept;

   /// @brief Get the linked object index. Only valid if has_index returns true.
   /// @return The index of the object.
   auto index() const noexcept -> uint32;

   /// @brief Get the object name. Only valid if has_name returns true.
   /// @return The object name.
   auto name() noexcept -> std::string&;

   /// @brief Get the object name. Only valid if has_name returns true.
   /// @return The object name.
   auto name() const noexcept -> const std::string&;

   /// @brief Gets the name of the linked object or returns the store object name if unlinked.
   /// @param world The world the object_optional_link is part of.
   /// @return The object name.
   auto name_lookup(const world& world) const noexcept -> const std::string&;

private:
   int64 _index = -1;
   std::string _name;
};

bool operator==(const object_optional_link& left,
                const object_optional_link& right) noexcept;

/// @brief Compare an object_optional_link against an index.
/// @param link The link to compare.
/// @param index The index to compare.
/// @return True if object_optional_link stores the index, false otherwise.
bool operator==(const object_optional_link& link, const uint32 index) noexcept;

}
