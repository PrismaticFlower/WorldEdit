
#include "object_class.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/msh/default_missing_scene.hpp"

using namespace std::literals;

namespace sk::world {

object_class::object_class(const assets::odf::definition& definition,
                           assets::libraries_manager& assets_libraries)
{
   if (definition.class_properties.contains("GeometryName"sv)) {
      model_name = definition.class_properties.at("GeometryName"sv);
   }
   else {
      // TODO: Handle the case of .msh being absent in a more user friendly manner.
      model_name =
         definition.header_properties.at("GeometryName"sv)
            .substr(0, definition.header_properties.at("GeometryName"sv).size() -
                          ".msh"sv.size());
   }

   model = assets_libraries.models.aquire_if(model_name);

   if (not model) model = assets::msh::default_missing_scene();

   instance_properties.reserve(definition.instance_properties.size());

   for (const auto& prop : instance_properties) {
      instance_properties.push_back({.key = prop.key, .value = prop.value});
   }
}

}