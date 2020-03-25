
#include "default_object_class_definition.hpp"

using namespace std::literals;

namespace sk::assets::odf {

auto default_object_class_definition() noexcept -> const std::shared_ptr<definition>&
{
   const static auto default_definition = std::make_shared<definition>(
      definition{.header_properties = {{"ClassLabel"sv, "prop"sv}}});

   return default_definition;
}

}