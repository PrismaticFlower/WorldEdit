#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <exception>
#include <span>
#include <type_traits>

namespace we::utility {

template<typename Type>
inline auto make_from_bytes(const std::span<const std::byte> bytes) noexcept -> Type
{
   if (sizeof(Type) != bytes.size()) std::terminate();

   static_assert(std::is_default_constructible_v<Type> &&
                 std::is_trivially_copyable_v<Type>);

   Type val;

   std::memcpy(&val, bytes.data(), bytes.size());

   return val;
}

}
