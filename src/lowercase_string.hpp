#pragma once

#include <concepts>
#include <cstring>
#include <string>
#include <string_view>

namespace we {

class lowercase_string : public std::string {
public:
   lowercase_string() = default;

   explicit lowercase_string(const std::string& string) noexcept
      : std::string{string}
   {
      make_lowercase();
   }

   explicit lowercase_string(std::string&& string) noexcept
      : std::string{std::move(string)}
   {
      make_lowercase();
   }

   explicit lowercase_string(std::string_view string) noexcept
      : std::string{string}
   {
      make_lowercase();
   }

private:
   void make_lowercase() noexcept
   {
      for (auto& c : *this) c = static_cast<char>(std::tolower(c));
   }
};

}

namespace std {

template<>
struct hash<we::lowercase_string> {
   auto operator()(const we::lowercase_string& string) const noexcept
   {
      return std::hash<std::string_view>{}(string);
   }
};

}
