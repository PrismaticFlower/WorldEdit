
#include "io.hpp"
#include "utility/string_ops.hpp"

#include <charconv>
#include <stdexcept>
#include <tuple>

#include <fmt/core.h>

using namespace std::literals;

namespace we::assets::config {

namespace {

auto parse_string_value(const string::line line, std::string_view str,
                        const read_options& options, values& values_out) -> std::string_view
{
   auto quoted_result = options.support_escape_sequences
                           ? string::quoted_read_with_escapes(str)
                           : string::quoted_read(str);

   if (!quoted_result) {
      throw std::runtime_error{fmt::format(
         "Error on line #{} at column #{}! Expected '\"' to close value #{}.",
         line.number, string::substr_distance(line.string, str) + 1,
         values_out.size())};
   }

   auto [value, rest] = *quoted_result;

   if (not options.support_escape_sequences) {
      values_out.emplace_back(std::string{value});
   }
   else {
      std::string unescaped_str;
      unescaped_str.reserve(value.size());

      for (std::size_t i = 0; i < value.size(); ++i) {
         if (value[i] == '\\') {
            if (i + 1 >= value.size()) continue;

            i += 1;

            switch (value[i]) {
            case '\'': {
               unescaped_str.push_back('\'');
            } break;
            case '"': {
               unescaped_str.push_back('\"');
            } break;
            case '?': {
               unescaped_str.push_back('\?');
            } break;
            case '\\': {
               unescaped_str.push_back('\\');
            } break;
            case 'a': {
               unescaped_str.push_back('\a');
            } break;
            case 'b': {
               unescaped_str.push_back('\b');
            } break;
            case 'f': {
               unescaped_str.push_back('\f');
            } break;
            case 'n': {
               unescaped_str.push_back('\n');
            } break;
            case 'r': {
               unescaped_str.push_back('\r');
            } break;
            case 't': {
               unescaped_str.push_back('\t');
            } break;
            case 'v': {
               unescaped_str.push_back('\v');
            } break;
            }
         }
         else {
            unescaped_str.push_back(value[i]);
         }
      }

      values_out.emplace_back(std::move(unescaped_str));
   }

   return rest;
}

auto parse_number_value(const string::line line, std::string_view str,
                        values& values_out) -> std::string_view
{
   auto [value, rest] =
      string::split_first_of_right_inclusive_any(str, {" "sv, ","sv, ")"sv});

   double dbl_val{};

   if (const auto err =
          std::from_chars(value.data(), value.data() + value.size(), dbl_val);
       err.ec != std::errc{}) {
      throw std::runtime_error{
         fmt::format("Error on line #{} at column #{}! Unexpected character "
                     "'{}' inside number. Value is #{}.",
                     line.number,
                     string::substr_distance(line.string, std::string_view{err.ptr, 1}) + 1,
                     *err.ptr, values_out.size())};
   }

   values_out.emplace_back(dbl_val);

   return rest;
}

auto parse_value(const string::line line, std::string_view str,
                 const read_options& options, values& values_out) -> std::string_view
{
   str = string::trim_whitespace(str);

   if (str.starts_with("\""sv)) {
      return parse_string_value(line, str, options, values_out);
   }

   return parse_number_value(line, str, values_out);
}

void parse_values(const string::line line, std::string_view str,
                  const read_options& options, values& values_out)
{
   // Fixup double bracket open to values.
   if (str.starts_with("(")) str = str.substr(1);

   while (not str.empty()) {
      str = string::trim_whitespace(str);

      if (str.starts_with(")"sv)) {
         return;
      }
      else if (str.starts_with(","sv)) {
         str = str.substr(1);
         str = parse_value(line, str, options, values_out);
      }
      else {
         str = parse_value(line, str, options, values_out);
      }
   }

   throw std::runtime_error{fmt::format(
      "Error on line #{} at column #{}! Expected ')' to close values list.",
      line.number, line.string.size())};
}

void parse_key_values(const string::line line, std::string& key_out,
                      const read_options& options, values& values_out)
{
   auto [key, values] = string::split_first_of_exclusive(line.string, "("sv);

   key = string::trim_whitespace(key);

   if (values.empty()) {
      throw std::runtime_error{
         fmt::format("Error on line #{} at column #{}! Expected '(' to open "
                     "values list for key '{}'.",
                     line.number, string::substr_distance(line.string, key) + 1, key)};
   }

   key_out = key;

   parse_values(line, values, options, values_out);
}

auto parse_node_children(const string::lines_iterator line_iter,
                         const read_options& options, key_node& out)
   -> string::lines_iterator;

auto parse_key_node(const string::lines_iterator line_iter,
                    const read_options& options, key_node& out) -> string::lines_iterator
{
   parse_key_values(*line_iter, out.key, options, out.values);

   return parse_node_children(line_iter, options, out);
}

auto parse_node_children(const string::lines_iterator line_iter,
                         const read_options& options, key_node& out) -> string::lines_iterator
{
   auto child_iter = line_iter;
   ++child_iter;

   string::line child_line;

   for (; child_iter != child_iter.end(); ++child_iter) {
      child_line = *child_iter;
      auto str = string::trim_leading_whitespace(child_line.string);

      if (str.starts_with("//"sv) or str.empty()) {
         continue;
      }
      else if (not str.starts_with("{"sv)) {
         return line_iter;
      }
      else {
         ++child_iter;
         break;
      }
   }

   if (child_iter == child_iter.end()) return line_iter;

   for (; child_iter != child_iter.end(); ++child_iter) {
      child_line = *child_iter;
      auto str = string::trim_leading_whitespace(child_line.string);

      if (str.starts_with("//"sv) or str.empty()) {
         continue;
      }
      else if (str.starts_with("}"sv)) {
         return child_iter;
      }
      else if (not std::isalnum(str.front())) {
         throw std::runtime_error{fmt::format(
            "Error on line #{} at column #{}! Unexpected character '{}'.",
            child_line.number,
            string::substr_distance(child_line.string, str) + 1, str.front())};
      }

      child_iter = parse_key_node(child_iter, options, out.emplace_back());
   }

   throw std::runtime_error{
      fmt::format("Error! Expected '}}' to close '{{' on line #{}.", child_line.number)};
}

}

auto read_config(std::string_view str, const read_options options) -> node
{
   node result;

   for (string::lines_iterator line_iter{str}; line_iter != line_iter.end();
        ++line_iter) {
      auto line = *line_iter;

      str = string::trim_leading_whitespace(line.string);

      if (str.starts_with("//"sv) or str.empty()) {
         continue;
      }
      else if (not std::isalnum(str.front())) {
         throw std::runtime_error{fmt::format(
            "Error on line #{} at column #{}! Unexpected character '{}'.", line.number,
            string::substr_distance(line.string, str) + 1, str.front())};
      }

      line_iter = parse_key_node(line_iter, options, result.emplace_back());
   }

   return result;
}
}
