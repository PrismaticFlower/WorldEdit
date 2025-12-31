
#include "definition_io.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <stdexcept>

#include <fmt/core.h>

using namespace std::literals;

namespace we::assets::odf {

namespace {

enum class section {
   none,
   header,
   properties,
   instance_properties,
};

struct property_counts {
   std::size_t header = 0;
   std::size_t properties = 0;
   std::size_t instance_properties = 0;
};

auto read_property(string::line line) -> property
{
   auto [key, value] = string::split_first_of_exclusive(line.string, "="sv);

   if (key.contains("//"sv)) {
      throw std::runtime_error{fmt::format(
         "Error in .odf on line #{}! '//' can not appear in the middle of a "
         "property '<Key> = <Value>' pair. Move the comment to the end of the "
         "line.",
         line.number)};
   }

   key = string::trim_trailing_whitespace(key);
   value = string::trim_leading_whitespace(value);

   if (line.string.contains("="sv) and value.empty()) {
      throw std::runtime_error{
         fmt::format("Error in .odf on line #{}! The value right of '=' can "
                     "not be only whitespace. "
                     "Use '{} = \"\"' to indicate an empty value.",
                     line.number, key)};
   }
   else if (value.empty()) {
      throw std::runtime_error{
         fmt::format("Error in .odf on line #{}! Expected '=' "
                     "left of key '{}'!",
                     line.number, key)};
   }

   if (value.front() == '"') {
      value = value.substr(1);
      value = string::split_first_of_exclusive(value, "\""sv).front();
   }
   else {
      value = string::split_first_of_exclusive_whitespace(value).front();
   }

   return {.key = key, .value = value};
}

template<typename T>
void read_definition(std::string_view str, T& result)
{
   type definition_type = type::game_object_class;

   section current_section = section::none;

   for (auto line : string::lines_iterator{str}) {
      line.string = string::trim_leading_whitespace(line.string);

      if (line.string.starts_with("//"sv)) continue;
      if (line.string.starts_with(R"(\\)"sv)) continue;
      if (line.string.starts_with("--"sv)) continue;
      if (string::is_whitespace(line.string)) continue;

      if (string::istarts_with(line.string, "[ExplosionClass]"sv)) {
         current_section = section::header;
         definition_type = type::explosion_class;
      }
      else if (string::istarts_with(line.string, "[OrdnanceClass]"sv)) {
         current_section = section::header;
         definition_type = type::ordnance_class;
      }
      else if (string::istarts_with(line.string, "[WeaponClass]"sv)) {
         current_section = section::header;
         definition_type = type::weapon_class;
      }
      else if (string::istarts_with(line.string, "[GameObjectClass]"sv)) {
         current_section = section::header;
         definition_type = type::game_object_class;
      }
      else if (string::istarts_with(line.string, "[Properties]"sv)) {
         current_section = section::properties;
      }
      else if (string::istarts_with(line.string, "[InstanceProperties]"sv)) {
         current_section = section::instance_properties;
      }
      else if (line.string.starts_with("["sv)) {
         throw std::runtime_error{
            fmt::format("Error in .odf on line #{}! Unknown or incomplete .odf "
                        "header '{}'!",
                        line.number,
                        string::split_first_of_inclusive(line.string, "]"sv).front())};
      }
      else {
         if constexpr (std::is_same_v<T, property_counts>) {
            switch (current_section) {
            case section::header:
               result.header += 1;
               break;
            case section::properties:
               result.properties += 1;
               break;
            case section::instance_properties:
               result.instance_properties += 1;
               break;
            case section::none:
               throw std::runtime_error{
                  fmt::format("Error in .odf on line #{}! Non-empty line is "
                              "before any .odf header!",
                              line.number)};
            }
         }
         else {
            switch (current_section) {
            case section::header: {
               const property property = read_property(line);

               if (string::iequals(property.key, "ClassLabel"sv)) {
                  result.header.base = property.value;
               }
               else if (string::iequals(property.key, "ClassParent"sv)) {
                  result.header.base = property.value;
               }
               else if (string::iequals(property.key, "GeometryName"sv)) {
                  result.header.geometry_name = property.value;
               }
               break;
            }
            case section::properties:
               result.properties.push_back(read_property(line));
               break;
            case section::instance_properties:
               result.instance_properties.push_back(read_property(line));
               break;
            case section::none:
               throw std::runtime_error{
                  fmt::format("Error in .odf on line #{}! Non-empty line is "
                              "before any .odf header!",
                              line.number)};
            }
         }
      }
   }

   if constexpr (std::is_same_v<T, definition>) {
      result.type = definition_type;
   }
}

}

auto read_definition(std::vector<char> string_storage) -> definition
{
   definition definition{.storage = std::move(string_storage)};

   std::string_view str{definition.storage.data(), definition.storage.size()};

   property_counts property_counts;

   read_definition(str, property_counts);

   definition.properties.reserve(property_counts.properties);
   definition.instance_properties.reserve(property_counts.instance_properties);

   read_definition(str, definition);

   return definition;
}

}
