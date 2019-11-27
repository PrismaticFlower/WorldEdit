#pragma once

#include <array>
#include <string>
#include <string_view>

namespace sk::utility::string {

auto count_lines(std::string_view str) noexcept -> std::size_t;

auto split_first_of_inclusive(std::string_view str, std::string_view delimiter)
   -> std::array<std::string_view, 2>;

auto split_first_of_inclusive(std::string&& str, std::string_view delimiter)
   -> std::array<std::string_view, 2> = delete;

auto indent(std::size_t level, std::string_view input,
            std::string_view indention = "   ") noexcept -> std::string;

}
