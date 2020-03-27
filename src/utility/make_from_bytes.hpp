#pragma once

#include <cstddef>
#include <cstring>
#include <type_traits>

#include <gsl/gsl>

namespace sk::utility {

template<typename Type>
inline auto make_from_bytes(const gsl::span<const std::byte> bytes) noexcept -> Type
{
   Expects(sizeof(Type) == bytes.size());

   static_assert(std::is_default_constructible_v<Type> &&
                 std::is_trivially_copyable_v<Type>);

   Type val;

   std::memcpy(&val, bytes.data(), bytes.size());

   return val;
}

}