#include "object_properties.hpp"

namespace we::world {

auto make_object_instance_properties(const assets::odf::definition& definition,
                                     std::span<const instance_property> existing_properties)
   -> std::vector<instance_property>
{
   std::vector<instance_property> properties;
   properties.reserve(definition.instance_properties.size());

   for (const auto& prop : definition.instance_properties) {
      (void)prop, existing_properties;
   }

   return properties;
}

}