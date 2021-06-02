#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <span>
#include <type_traits>

#include <gsl/gsl>

namespace we::utility {

template<typename Type>
inline auto make_from_bytes(const std::span<const std::byte> bytes) noexcept -> Type
{
   Expects(sizeof(Type) == bytes.size());

   static_assert(std::is_default_constructible_v<Type> &&
                 std::is_trivially_copyable_v<Type>);

   Type val;

   std::memcpy(&val, bytes.data(), bytes.size());

   return val;
}

}
