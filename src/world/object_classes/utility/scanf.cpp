#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "scanf.hpp"

#include <cstdio>
#include <string>
#include <string_view>

namespace we::world {

auto scan(std::string_view str, float& value) -> int
{
   std::string buffer{str};

   return std::sscanf(buffer.c_str(), "%f", &value);
}

auto scan(std::string_view str, float& x, float& y) -> int
{
   std::string buffer{str};

   return std::sscanf(buffer.c_str(), "%f %f", &x, &y);
}

auto scan(std::string_view str, float3& value) -> int
{
   std::string buffer{str};

   return std::sscanf(buffer.c_str(), "%f %f %f", &value.x, &value.y, &value.z);
}

auto scan(std::string_view str, float4& value) -> int
{
   std::string buffer{str};

   return std::sscanf(buffer.c_str(), "%f %f %f %f", &value.x, &value.y,
                      &value.z, &value.w);
}

auto scan(std::string_view str, double& value) -> int
{
   float intermediate = static_cast<float>(value);

   std::string buffer{str};

   const int scanned = std::sscanf(buffer.c_str(), "%f", &intermediate);

   value = intermediate;

   return scanned;
}

auto scan(std::string_view str, int& value) -> int
{
   std::string buffer{str};

   return std::sscanf(buffer.c_str(), "%i", &value);
}

auto scan(std::string_view str, int& x, int& y) -> int
{
   std::string buffer{str};

   return std::sscanf(buffer.c_str(), "%i %i", &x, &y);
}

auto scan(std::string_view str, uint8& value) -> int
{
   int intermediate = value;

   std::string buffer{str};

   const int scanned = std::sscanf(buffer.c_str(), "%i", &intermediate);

   value = static_cast<uint8>(intermediate);

   return scanned;
}

auto scan(std::string_view str, uint32& r, uint32& g, uint32& b, uint32& a) -> int
{
   std::string buffer{str};

   return std::sscanf(buffer.c_str(), "%u %u %u %u", &r, &g, &b, &a);
}

}