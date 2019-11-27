#pragma once

#include <cstddef>
#include <cstring>
#include <type_traits>

#include <gsl/gsl>

namespace sk::utility {

template<typename type>
inline auto make_from_bytes(const gsl::span<const std::byte> bytes) noexcept -> type
{
   Expects(sizeof(type) == bytes.size());

   static_assert(std::is_trivial_v<type>);

   type val;

   std::memcpy(&val, bytes.data(), bytes.size());

   return val;
}

}
