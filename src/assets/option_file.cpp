
#include "option_file.hpp"
#include "utility/string_ops.hpp"

#include <tuple>
#include <utility>

namespace we::assets {

using namespace string;

namespace {

auto parse_arguments(std::string_view str)
   -> std::pair<std::string_view, boost::container::small_vector<std::string, 8>>
{
   boost::container::small_vector<std::string, 8> arguments;

   while (not str.empty() and str.front() != '-') {
      str = trim_leading_whitespace(str);

      if (str.empty()) break;

      if (auto quoted_arg = quoted_read(str); quoted_arg) {
         arguments.emplace_back((*quoted_arg)[0]);
         str = (*quoted_arg)[1];
      }
      else {
         auto [argument, rest] = split_first_of_exclusive_if(str, std::isspace);
         arguments.emplace_back(argument);
         str = rest;
      }
   }

   return {str, arguments};
}

auto parse_option(std::string_view str) -> std::pair<std::string_view, option>
{
   str = trim_leading_whitespace(str);

   auto [name, arguments_rest] = split_first_of_exclusive_if(str, std::isspace);
   auto [rest, arguments] = parse_arguments(arguments_rest);

   return {rest, option{.name = std::string{name}, .arguments = std::move(arguments)}};
}

}

auto parse_options(std::string_view str) -> options
{
   options opts;

   while (not str.empty()) {
      auto [rest, opt] = parse_option(str);
      opts.emplace_back(std::move(opt));
      str = rest;
   }

   return opts;
}

}
