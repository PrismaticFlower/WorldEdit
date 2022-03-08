#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>

namespace we {

/// @brief Like std::find, except it returns a pointer to the found element instead of an interator.
/// @param range The forward_range to look through.
/// @param cmp_value The value to compare against.
/// @return A pointer to the found value or nullptr if the value was not found.
template<std::ranges::forward_range forward_range>
[[nodiscard]] constexpr auto look_for(
   forward_range& range,
   const std::equality_comparable_with<std::ranges::range_reference_t<forward_range>> auto& cmp_value)
   -> std::remove_reference_t<std::ranges::range_reference_t<forward_range>>*
{
   for (auto& v : range) {
      if (v == cmp_value) return &v;
   }

   return nullptr;
}

/// @brief Like std::find_if, except it returns a pointer to the found element instead of an interator.
/// @param range The forward_range to look through.
/// @param predicate The predicate to call to check for matching values.
/// @return A pointer to the found value or nullptr if the value was not found.
template<std::ranges::forward_range forward_range>
[[nodiscard]] constexpr auto look_for(
   forward_range& range,
   const std::regular_invocable<std::ranges::range_reference_t<forward_range>> auto& predicate)
   -> std::remove_reference_t<std::ranges::range_reference_t<forward_range>>*
{
   for (auto& v : range) {
      if (predicate(v)) return &v;
   }

   return nullptr;
}

}