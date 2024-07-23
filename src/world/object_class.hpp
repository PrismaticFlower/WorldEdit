#pragma once

#include "assets/asset_ref.hpp"
#include "assets/msh/flat_model.hpp"
#include "assets/odf/definition.hpp"
#include "lowercase_string.hpp"
#include "object_instance_property.hpp"

#include <string>
#include <vector>

namespace we::assets {

struct libraries_manager;

}

namespace we::world {

struct object_class_flags {
   uint32 hidden_ingame : 1 = false;
};

struct object_class {
   object_class() = default;

   object_class(assets::libraries_manager& assets_libraries,
                asset_ref<assets::odf::definition> definition_asset);

   asset_ref<assets::odf::definition> definition_asset;
   asset_data<assets::odf::definition> definition;

   asset_ref<assets::msh::flat_model> model_asset;
   asset_data<assets::msh::flat_model> model;

   object_class_flags flags;
   lowercase_string model_name;

   std::vector<instance_property> instance_properties;

   int32 world_frame_references = 1;

   /// @brief Update the object class from a definition asset.
   /// @param assets_libraries A reference to the assets::libraries_manager to pull model assets from.
   /// @param new_definition_asset The new definition asset from the object class.
   void update_definition(assets::libraries_manager& assets_libraries,
                          asset_ref<assets::odf::definition> new_definition_asset);

   /// @brief Update the object class from it's current definition asset.
   /// @param assets_libraries A reference to the assets::libraries_manager to pull model assets from.
   void update_from_definition(assets::libraries_manager& assets_libraries);
};

}
