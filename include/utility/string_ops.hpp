#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <string_view>

#include <experimental/generator>

namespace sk::utility::string {

struct line {
   int number = 0;
   std::string_view string;
};

auto count_lines(const std::string_view str) noexcept -> std::size_t;

auto iterate_lines(std::string_view str) noexcept
   -> std::experimental::generator<line>;

auto split_first_of_inclusive(std::string_view str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2>;

auto split_first_of_inclusive(std::string&& str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2> = delete;

auto split_first_of_exclusive(std::string_view str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2>;

auto split_first_of_exclusive(std::string&& str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2> = delete;

template<typename Predicate>
auto split_first_of_exclusive_if(std::string_view str, Predicate&& predicate) noexcept
   -> std::array<std::string_view, 2>;

template<typename Predicate>
auto split_first_of_exclusive_if(std::string&& str, Predicate predicate) noexcept
   -> std::array<std::string_view, 2> = delete;

auto trim_leading_whitespace(std::string_view str) noexcept -> std::string_view;

auto trim_leading_whitespace(std::string&& str) noexcept -> std::string_view = delete;

auto trim_trailing_whitespace(std::string_view str) noexcept -> std::string_view;

auto trim_trailing_whitespace(std::string&& str) noexcept -> std::string_view = delete;

bool is_whitespace(const std::string_view str) noexcept;

auto indent(std::size_t level, std::string_view input,
            std::string_view indention = "   ") noexcept -> std::string;

// Inline Implementations

inline auto iterate_lines(std::string_view str) noexcept
   -> std::experimental::generator<line>
{
   using namespace std::literals;

   for (int line_number = 1; not str.empty(); ++line_number) {
      auto [line, rest] = string::split_first_of_exclusive(str, "\n"sv);
      str = rest;

      if (not line.empty() and line.back() == '\r') {
         line = line.substr(0, line.size() - 1);
      }

      co_yield{.number = line_number, .string = line};
   }
}

template<typename Predicate>
inline auto split_first_of_exclusive_if(std::string_view str, Predicate&& predicate) noexcept
   -> std::array<std::string_view, 2>
{
   using namespace std::literals;

   const auto iter =
      std::find_if(str.begin(), str.end(), std::forward<Predicate>(predicate));

   if (iter == str.end()) return {str, ""sv};

   const auto offset = std::distance(str.begin(), iter);

   return {str.substr(0, offset), str.substr(offset + 1)};
}

}
