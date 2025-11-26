
#include "string_ops.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>

using namespace std::literals;

namespace we::string {

auto count_lines(const std::string_view str) noexcept -> std::size_t
{
   return std::count(str.cbegin(), str.cend(), '\n');
}

auto split_first_of_inclusive(std::string_view str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2>
{
   const auto offset = str.find(delimiter);

   if (offset == str.npos) return {str, ""sv};

   return {str.substr(0, offset + delimiter.size()),
           str.substr(offset + delimiter.size(), str.size() - offset)};
}

auto split_first_of_exclusive(std::string_view str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2>
{
   const auto offset = str.find(delimiter);

   if (offset == str.npos) return {str, ""sv};

   return {str.substr(0, offset),
           str.substr(offset + delimiter.size(), str.size() - offset)};
}

auto split_first_of_exclusive_whitespace(std::string_view str) noexcept
   -> std::array<std::string_view, 2>
{
   using namespace std::literals;

   const auto iter = std::find_if(str.begin(), str.end(), [](const char c) {
      switch (c) {
      case '\t':
      case '\n':
      case '\v':
      case '\f':
      case '\r':
      case ' ':
         return true;
      default:
         return false;
      }
   });

   if (iter == str.end()) return {str, ""sv};

   const auto offset = std::distance(str.begin(), iter);

   return {str.substr(0, offset), str.substr(offset + 1)};
}

auto split_first_of_right_inclusive_any(std::string_view str,
                                        std::initializer_list<std::string_view> delimiters) noexcept
   -> std::array<std::string_view, 2>
{
   std::optional<std::string_view::size_type> min_offset;

   for (const auto delim : delimiters) {
      const auto offset = str.find(delim);

      if (offset == str.npos) continue;

      if (min_offset) {
         *min_offset = std::min(*min_offset, offset);
      }
      else {
         min_offset.emplace(offset);
      }
   }

   if (not min_offset) return {str, ""sv};

   return {str.substr(0, *min_offset), str.substr(*min_offset)};
}

auto trim_leading_whitespace(std::string_view str) noexcept -> std::string_view
{
   return str.substr(
      std::distance(str.begin(),
                    std::find_if_not(str.begin(), str.end(), std::isspace)));
}

auto trim_trailing_whitespace(std::string_view str) noexcept -> std::string_view
{
   return str.substr(0, std::distance(str.begin(),
                                      std::find_if_not(str.rbegin(), str.rend(), std::isspace)
                                         .base()));
}

auto trim_trailing_digits(std::string_view str) noexcept -> std::string_view
{
   return str.substr(0, std::distance(str.begin(),
                                      std::find_if_not(str.rbegin(), str.rend(), std::isdigit)
                                         .base()));
}

auto trim_whitespace(std::string_view str) noexcept -> std::string_view
{
   return trim_trailing_whitespace(trim_leading_whitespace(str));
}

auto quoted_read(std::string_view str) noexcept
   -> std::optional<std::array<std::string_view, 2>>
{
   if (str.empty() or str.front() != '"') return std::nullopt;

   str = str.substr(1);

   auto offset = str.find('"');

   if (offset == str.npos) return std::nullopt;

   return std::array{str.substr(0, offset), str.substr(offset + 1)};
}

auto quoted_read_with_escapes(std::string_view str) noexcept
   -> std::optional<std::array<std::string_view, 2>>
{
   if (str.empty() or str.front() != '"') return std::nullopt;

   for (std::size_t i = 1; i < str.size(); ++i) {
      if (str[i] == '"') {
         return std::array{str.substr(1, i - 1), str.substr(i + 1)};
      }
      else if (str[i] == '\\') {
         if (i + 1 >= str.size()) {
            break;
         }
         else if (str[i + 1] == '"') {
            i += 1;
         }
      }
   }

   return std::nullopt;
}

bool is_whitespace(const std::string_view str) noexcept
{
   return std::all_of(str.cbegin(), str.cend(), std::isspace);
}

auto substr_distance(const std::string_view str, const std::string_view substr) noexcept
   -> std::ptrdiff_t
{
   return std::distance(str.data(), substr.data());
}

auto indent(std::size_t level, std::string_view input,
            std::string_view indention) noexcept -> std::string
{
   if (level == 0) return std::string{input};

   const auto lines = count_lines(input);
   const auto extra_space = lines * level * indention.size();
   const auto result_size = input.size() + extra_space;

   std::string result;
   result.reserve(result_size);

   while (not input.empty()) {
      auto [line, rest] = split_first_of_inclusive(input, "\n"sv);

      for (std::size_t i = 0; i < level; ++i) result += indention;

      result += line;

      input = rest;
   }

   return result;
}

void token_iterator::advance() noexcept
{
   do {
      const std::size_t sep_offset = _str.find_first_of(_separator);

      if (sep_offset == _str.npos) {
         _token = _str;
         _str = "";
      }
      else {
         _token = _str.substr(0, sep_offset);
         _str.remove_prefix(sep_offset + 1);
      }
   } while (_token.empty() and not _str.empty());
}

}
