
#include "default_object_class_definition.hpp"

using namespace std::literals;

namespace we::assets::odf {

auto default_object_class_definition() noexcept
   -> const std::shared_ptr<const definition>&
{
   const static auto default_definition = std::make_shared<const definition>(
      definition{.header = {.class_label = "prop"sv}});

   return default_definition;
}
}
