#pragma once

#include <array>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>

namespace we::string {

struct line {
   int number = 0;
   std::string_view string;
};

auto count_lines(const std::string_view str) noexcept -> std::size_t;

auto split_first_of_inclusive(std::string_view str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2>;

auto split_first_of_inclusive(std::string&& str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2> = delete;

auto split_first_of_exclusive(std::string_view str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2>;

auto split_first_of_exclusive(std::string&& str, std::string_view delimiter) noexcept
   -> std::array<std::string_view, 2> = delete;

auto split_first_of_exclusive_whitespace(std::string_view str) noexcept
   -> std::array<std::string_view, 2>;

auto split_first_of_exclusive_whitespace(std::string&& str) noexcept
   -> std::array<std::string_view, 2> = delete;

auto split_first_of_right_inclusive_any(std::string_view str,
                                        std::initializer_list<std::string_view> delimiters) noexcept
   -> std::array<std::string_view, 2>;

auto split_first_of_right_inclusive_any(std::string&& str,
                                        std::initializer_list<std::string_view> delimiters) noexcept
   -> std::array<std::string_view, 2> = delete;

auto trim_leading_whitespace(std::string_view str) noexcept -> std::string_view;

auto trim_leading_whitespace(std::string&& str) noexcept -> std::string_view = delete;

auto trim_trailing_whitespace(std::string_view str) noexcept -> std::string_view;

auto trim_trailing_whitespace(std::string&& str) noexcept -> std::string_view = delete;

auto trim_trailing_digits(std::string_view str) noexcept -> std::string_view;

auto trim_trailing_digits(std::string&& str) noexcept -> std::string_view = delete;

auto trim_whitespace(std::string_view str) noexcept -> std::string_view;

auto trim_whitespace(std::string&& str) noexcept -> std::string_view = delete;

auto quoted_read(std::string_view str) noexcept
   -> std::optional<std::array<std::string_view, 2>>;

auto quoted_read(std::string&& str) noexcept
   -> std::optional<std::array<std::string_view, 2>> = delete;

bool is_whitespace(const std::string_view str) noexcept;

auto substr_distance(const std::string_view str, const std::string_view substr) noexcept
   -> std::ptrdiff_t;

auto indent(std::size_t level, std::string_view input,
            std::string_view indention = "   ") noexcept -> std::string;

class lines_iterator {
public:
   using value_type = line;
   using pointer = line*;
   using reference = line&;
   using iterator_category = std::input_iterator_tag;

   explicit lines_iterator(std::string_view str) noexcept : _str{str}
   {
      advance();
   }

   auto operator++() noexcept -> lines_iterator&
   {
      advance();

      return *this;
   }

   void operator++(int) noexcept
   {
      advance();
   }

   auto operator*() const noexcept -> const line&
   {
      return _line;
   }

   auto operator->() const noexcept -> const line&
   {
      return _line;
   }

   auto begin() noexcept -> lines_iterator
   {
      return *this;
   }

   auto end() noexcept -> std::nullptr_t
   {
      return nullptr;
   }

   bool operator==(std::nullptr_t) const noexcept
   {
      return _is_end;
   }

private:
   void advance() noexcept
   {
      using namespace std::literals;

      if (_next_is_end) {
         _is_end = true;

         return;
      }

      auto [line, rest] = string::split_first_of_exclusive(_str, "\n"sv);
      _str = rest;

      if (not line.empty() and line.back() == '\r') {
         line = line.substr(0, line.size() - 1);
      }

      _line = {.number = _line_number, .string = line};

      _line_number += 1;
      _next_is_end = _str.empty();
   }

   int _line_number = 1;
   std::string_view _str;
   line _line{};
   bool _next_is_end = false;
   bool _is_end = false;
};

struct token_iterator {
   using value_type = std::string_view;
   using pointer = std::string_view*;
   using reference = std::string_view&;
   using iterator_category = std::input_iterator_tag;

   token_iterator(std::string_view str, char separator) noexcept
      : _str{str}, _separator{separator}
   {
      advance();
   }

   auto operator++() noexcept -> token_iterator&
   {
      advance();

      return *this;
   }

   void operator++(int) noexcept
   {
      advance();
   }

   auto operator*() const noexcept -> const std::string_view&
   {
      return _token;
   }

   auto operator->() const noexcept -> const std::string_view&
   {
      return _token;
   }

   auto begin() noexcept -> token_iterator
   {
      return *this;
   }

   auto end() noexcept -> std::nullptr_t
   {
      return nullptr;
   }

   bool operator==(std::nullptr_t) const noexcept
   {
      return _token.empty();
   }

private:
   void advance() noexcept;

   std::string_view _str;
   std::string_view _token;
   char _separator = ' ';
};

}
