#pragma once

#include "assets/msh/flat_model.hpp"
#include "assets/odf/definition.hpp"
#include "object_instance_property.hpp"

#include <memory>
#include <string>
#include <vector>

namespace sk::world {

struct object_class {
   object_class() = default;

   explicit object_class(const assets::odf::definition& definition);

   std::shared_ptr<assets::msh::flat_model> model;
   std::string model_name;

   std::vector<instance_property> instance_properties;
};

}
