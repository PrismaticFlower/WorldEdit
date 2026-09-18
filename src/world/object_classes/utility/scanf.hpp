#pragma once

#include "types.hpp"

#include <string_view>

namespace we::world {

// Wrappers around sscanf for reading ODF parameters the same way as the game. Ideally the dependency on sscanf would be
// removed but for compatibility with the game it's simpler to keep it. No strings are ever (or must ever be) read using sscanf!

auto scan(std::string_view str, float& value) -> int;

auto scan(std::string_view str, float& x, float& y) -> int;

auto scan(std::string_view str, float3& value) -> int;

auto scan(std::string_view str, float4& value) -> int;

auto scan(std::string_view str, double& value) -> int;

auto scan(std::string_view str, int& value) -> int;

auto scan(std::string_view str, int& x, int& y) -> int;

auto scan(std::string_view str, uint8& value) -> int;

auto scan(std::string_view str, uint32& r, uint32& g, uint32& b, uint32& a) -> int;

}