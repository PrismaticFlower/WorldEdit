#pragma once

#include <array>
#include <cassert>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace we::container {

// clang-format off

template<typename T>
concept enum_array_index = std::is_enum_v<T> and requires {
   { T::COUNT };
};

// clang-format on

template<typename T, enum_array_index Enum>
using enum_array_base = std::array<T, static_cast<std::size_t>(Enum::COUNT)>;

template<typename T, enum_array_index Enum>
struct enum_array : enum_array_base<T, Enum> {
   using reference = typename enum_array_base<T, Enum>::reference;
   using const_reference = typename enum_array_base<T, Enum>::const_reference;

   using enum_array_base<T, Enum>::operator[];
   using enum_array_base<T, Enum>::at;

   constexpr auto operator[](const Enum index) noexcept -> reference
   {
      assert(index < Enum::COUNT);

      return static_cast<enum_array_base<T, Enum>&>(
         *this)[static_cast<std::ptrdiff_t>(index)];
   }

   constexpr auto operator[](const Enum index) const noexcept -> const_reference
   {
      assert(index < Enum::COUNT);

      return static_cast<const enum_array_base<T, Enum>&>(
         *this)[static_cast<std::ptrdiff_t>(index)];
   }

   constexpr auto at(const Enum index) noexcept -> reference
   {
      assert(index < Enum::COUNT);

      return static_cast<enum_array_base<T, Enum>&>(*this).at(
         static_cast<std::ptrdiff_t>(index));
   }

   constexpr auto at(const Enum index) const noexcept -> const_reference
   {
      assert(index < Enum::COUNT);

      return static_cast<const enum_array_base<T, Enum>&>(*this).at(
         static_cast<std::ptrdiff_t>(index));
   }
};

template<typename T, enum_array_index Enum>
inline constexpr auto make_enum_array(std::initializer_list<std::pair<Enum, T>> init)
   -> enum_array<T, Enum>
{
   enum_array<T, Enum> array{};

   for (auto& entry : init) {
      array[entry.first] = std::move(entry.second);
   }

   return array;
}

}
