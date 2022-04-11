
#include "object_class.hpp"
#include "assets/asset_libraries.hpp"
#include "assets/msh/default_missing_scene.hpp"
#include "assets/odf/default_object_class_definition.hpp"

using namespace std::literals;

namespace we::world {

object_class::object_class(assets::libraries_manager& assets_libraries,
                           asset_ref<assets::odf::definition> definition_asset)
{
   update_definition(assets_libraries, definition_asset);
}

void object_class::update_definition(assets::libraries_manager& assets_libraries,
                                     asset_ref<assets::odf::definition> new_definition_asset)
{
   definition_asset = new_definition_asset;
   definition = definition_asset.get_if();

   if (not definition) {
      definition = assets::odf::default_object_class_definition();
   }

   update_from_definition(assets_libraries);
}

void object_class::update_from_definition(assets::libraries_manager& assets_libraries)
{
   instance_properties.clear();
   instance_properties.reserve(definition->instance_properties.size());

   for (const auto& prop : instance_properties) {
      instance_properties.push_back({.key = prop.key, .value = prop.value});
   }

   if (definition->class_properties.contains("GeometryName"sv)) {
      model_name =
         lowercase_string{definition->class_properties.at("GeometryName"sv)};
   }
   else if (definition->header_properties.contains("GeometryName"sv)) {
      model_name = lowercase_string{
         definition->header_properties.at("GeometryName"sv)
            .substr(0, definition->header_properties.at("GeometryName"sv).size() -
                          ".msh"sv.size())};
   }
   else {
      model_name = {};
   }

   if (not model_name.empty()) {
      model_asset = assets_libraries.models[model_name];
      model = model_asset.get_if();
   }

   if (not model) model = assets::msh::default_missing_scene();
}

}
