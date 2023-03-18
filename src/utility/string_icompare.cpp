#include "string_icompare.hpp"

namespace we::string {

namespace {

static_assert('A' == 65 and 'Z' == 90);
static_assert('a' == 97 and 'z' == 122);

[[msvc::forceinline]] constexpr char to_lower(const char c) noexcept
{
   return (c >= 'A' and c <= 'Z') ? c + '\x20' : c;
}

static_assert(to_lower('A') == 'a');
static_assert(to_lower('I') == 'i');
static_assert(to_lower('Z') == 'z');

static_assert(to_lower('@') == '@');
static_assert(to_lower('[') == '[');
static_assert(to_lower('{') == '{');

}

// These functions ignore (A-Z and a-z but nothing else, this is good enough for our use case).

bool iequals(const std::string_view left, const std::string_view right) noexcept
{
   if (left.size() != right.size()) return false;

   for (std::size_t i = 0; i < left.size(); ++i) {
      if (to_lower(left[i]) != to_lower(right[i])) return false;
   }

   return true;
}

bool istarts_with(const std::string_view left, const std::string_view right) noexcept
{
   if (left.size() < right.size()) return false;

   return iequals(left.substr(0, right.size()), right);
}

bool iends_with(const std::string_view left, const std::string_view right) noexcept
{
   if (left.size() < right.size()) return false;

   return iequals(left.substr(left.size() - right.size(), right.size()), right);
}

}