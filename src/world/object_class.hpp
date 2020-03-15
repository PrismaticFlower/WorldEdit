#pragma once

#include "assets/asset_libraries.hpp"
#include "assets/msh/flat_model.hpp"
#include "assets/odf/definition.hpp"
#include "object_instance_property.hpp"

#include <memory>
#include <string>
#include <vector>

namespace sk::world {

struct object_class {
   object_class() = default;

   object_class(const assets::odf::definition& definition,
                assets::libraries_manager& assets_libraries);

   std::shared_ptr<assets::msh::flat_model> model;
   std::string model_name;

   std::vector<instance_property> instance_properties;
};

}
