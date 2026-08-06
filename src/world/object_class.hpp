#pragma once

#include "assets/asset_ref.hpp"
#include "assets/msh/flat_model.hpp"
#include "assets/odf/definition.hpp"
#include "lowercase_string.hpp"
#include "object_instance_property.hpp"

#include <string>

namespace we::assets {

struct libraries_manager;

}

namespace we::world {

struct object_class_flags {
   uint32 is_billboard_patch : 1 = false;
   uint32 hidden_ingame : 1 = false;
};

struct object_class {
   asset_ref<assets::odf::definition> definition_asset;
   asset_data<assets::odf::definition> definition;

   asset_ref<assets::msh::flat_model> model_asset;
   asset_data<assets::msh::flat_model> model;

   object_class_flags flags;
   lowercase_string model_name;
};

}
