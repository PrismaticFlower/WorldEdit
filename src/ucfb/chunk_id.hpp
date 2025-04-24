#pragma once

#include "types.hpp"

#include <string>

namespace we::ucfb {

enum class chunk_id : uint32 {};

inline auto to_string(const chunk_id id) -> std::string
{
   return {reinterpret_cast<const char*>(&id), sizeof(id)};
}

inline namespace literals {

constexpr auto operator""_id(const char* chars, const std::size_t) noexcept -> chunk_id
{
   uint32 result = 0;

   result |= (static_cast<uint32>(chars[0]) << 0);
   result |= (static_cast<uint32>(chars[1]) << 8);
   result |= (static_cast<uint32>(chars[2]) << 16);
   result |= (static_cast<uint32>(chars[3]) << 24);

   return static_cast<chunk_id>(result);
}
}

}