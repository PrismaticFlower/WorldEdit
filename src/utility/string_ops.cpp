
#include "string_ops.hpp"

#include <cctype>

using namespace std::literals;

namespace sk::utility::string {

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

}
