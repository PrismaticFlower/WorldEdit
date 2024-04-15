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

static_assert('A' == 65 and 'Z' == 90);
static_assert('a' == 97 and 'z' == 122);

[[msvc::forceinline]] constexpr wchar_t to_lower(const wchar_t c) noexcept
{
   return (c >= L'A' and c <= L'Z') ? c + L'\x20' : c;
}

static_assert(to_lower(L'A') == L'a');
static_assert(to_lower(L'I') == L'i');
static_assert(to_lower(L'Z') == L'z');

static_assert(to_lower(L'@') == L'@');
static_assert(to_lower(L'[') == L'[');
static_assert(to_lower(L'{') == L'{');

static_assert(L'A' == 65 and L'Z' == 90);
static_assert(L'a' == 97 and L'z' == 122);

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

int icompare(const std::string_view left, const std::string_view right) noexcept
{
   const std::size_t compare_length = std::min(left.size(), right.size());

   for (std::size_t i = 0; i < compare_length; ++i) {
      const char l = to_lower(left[i]);
      const char r = to_lower(right[i]);

      if (l != r) {
         return static_cast<unsigned char>(l) - static_cast<unsigned char>(r);
      }
   }

   if (left.size() < right.size()) return -1;
   if (left.size() > right.size()) return 1;

   return 0;
}

bool iless_than(const std::string_view left, const std::string_view right) noexcept
{
   return icompare(left, right) < 0;
}

bool igreater_than(const std::string_view left, const std::string_view right) noexcept
{
   return icompare(left, right) > 0;
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

bool icontains(const std::string_view left, const std::string_view right) noexcept
{
   if (left.size() < right.size()) return false;

   for (std::size_t i = 0; i < left.size(); ++i) {
      if ((left.size() - i) < right.size()) return false;

      if (iequals(left.substr(i, right.size()), right)) {
         return true;
      }
   }

   return false;
}

bool iequals(const std::wstring_view left, const std::wstring_view right) noexcept
{
   if (left.size() != right.size()) return false;

   for (std::size_t i = 0; i < left.size(); ++i) {
      if (to_lower(left[i]) != to_lower(right[i])) return false;
   }

   return true;
}

}