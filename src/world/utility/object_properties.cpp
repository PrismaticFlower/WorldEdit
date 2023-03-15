#include "object_properties.hpp"

namespace we::world {

auto make_object_instance_properties(const assets::odf::definition& definition,
                                     std::span<const instance_property> existing_properties)
   -> std::vector<instance_property>
{
   if (definition.instance_properties.empty()) return {};

   std::vector<instance_property> properties;
   properties.reserve(definition.instance_properties.size());

   for (const auto& prop : definition.instance_properties) {
      std::string_view existing_value = "";

      for (const auto& existing_prop : existing_properties) {
         if (existing_prop.key == prop.key) {
            existing_value = existing_prop.value;
         }
      }

      properties.emplace_back(std::string{prop.key},
                              std::string{existing_value.empty() ? prop.value
                                                                 : existing_value});
   }

   return properties;
}

}