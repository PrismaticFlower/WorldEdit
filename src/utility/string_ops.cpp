
#include <utility/string_ops.hpp>

#include <algorithm>

using namespace std::literals;

namespace sk::utility::string {

auto count_lines(std::string_view str) noexcept -> std::size_t
{
   return std::count(str.cbegin(), str.cend(), '\n');
}

auto split_first_of_inclusive(std::string_view str, std::string_view delimiter)
   -> std::array<std::string_view, 2>
{
   const auto offset = str.find(delimiter);

   if (offset == str.npos) return {str, ""sv};

   return {str.substr(0, offset + delimiter.size()),
           str.substr(offset + delimiter.size(), str.size() - offset)};
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
