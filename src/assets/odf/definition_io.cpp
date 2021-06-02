
#include "definition_io.hpp"
#include "utility/string_ops.hpp"

#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

using namespace std::literals;

namespace we::assets::odf {

namespace string = utility::string;

namespace {

enum class section {
   none,
   header,
   properties,
   instance_properties,
};

auto read_property(string::line line) -> property
{
   auto [key, value] = string::split_first_of_exclusive(line.string, "="sv);

   if (boost::contains(key, "//"sv)) {
      throw std::runtime_error{fmt::format(
         "Error in .odf on line #{}! '//' can not appear in the middle of a "
         "property '<Key> = <Value>' pair. Move the comment to the end of the line."sv,
         line.number)};
   }

   key = string::trim_trailing_whitespace(key);
   value = string::trim_leading_whitespace(value);

   if (boost::contains(line.string, "="sv) and value.empty()) {
      throw std::runtime_error{
         fmt::format("Error in .odf on line #{}! The value right of '=' can "
                     "not be only whitespace. "
                     "Use '{} = \"\"' to indicate an empty value."sv,
                     line.number, key)};
   }
   else if (value.empty()) {
      throw std::runtime_error{
         fmt::format("Error in .odf on line #{}! Expected '=' "
                     "left of key '{}'!"sv,
                     line.number, key)};
   }

   if (value.front() == '"') {
      value = value.substr(1);
      value = string::split_first_of_exclusive(value, "\""sv).front();
   }
   else {
      value = string::split_first_of_exclusive_if(value, [](char c) {
                 return std::isspace(c);
              }).front();
   }

   return {.key = std::string{key}, .value = std::string{value}};
}

}

auto read_definition(std::string_view str) -> definition
{
   definition definition;

   section current_section = section::none;

   for (auto line : string::lines_iterator{str}) {
      line.string = string::trim_leading_whitespace(line.string);

      if (line.string.starts_with("//"sv)) continue;
      if (line.string.starts_with(R"(\\)"sv)) continue;
      if (line.string.starts_with("--"sv)) continue;
      if (string::is_whitespace(line.string)) continue;

      if (boost::istarts_with(line.string, "[ExplosionClass]"sv)) {
         current_section = section::header;
         definition.type = type::explosion_class;
      }
      else if (boost::istarts_with(line.string, "[OrdnanceClass]"sv)) {
         current_section = section::header;
         definition.type = type::ordnance_class;
      }
      else if (boost::istarts_with(line.string, "[WeaponClass]"sv)) {
         current_section = section::header;
         definition.type = type::weapon_class;
      }
      else if (boost::istarts_with(line.string, "[GameObjectClass]"sv)) {
         current_section = section::header;
         definition.type = type::game_object_class;
      }
      else if (boost::istarts_with(line.string, "[Properties]"sv)) {
         current_section = section::properties;
      }
      else if (boost::istarts_with(line.string, "[InstanceProperties]"sv)) {
         current_section = section::instance_properties;
      }
      else if (line.string.starts_with("["sv)) {
         throw std::runtime_error{
            fmt::format("Error in .odf on line #{}! Unknown or incomplete .odf header '{}'!"sv,
                        line.number,
                        string::split_first_of_inclusive(line.string, "]"sv).front())};
      }
      else {
         switch (current_section) {
         case section::header:
            definition.header_properties.push_back(read_property(line));
            break;
         case section::properties:
            definition.class_properties.push_back(read_property(line));
            break;
         case section::instance_properties:
            definition.instance_properties.push_back(read_property(line));
            break;
         case section::none:
            throw std::runtime_error{
               fmt::format("Error in .odf on line #{}! Non-empty line is before any .odf header.!"sv,
                           line.number)};
         }
      }
   }

   return definition;
}

}
