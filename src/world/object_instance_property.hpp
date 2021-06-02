#pragma once

#include <string>

namespace we::world {

struct instance_property {
   std::string key;
   std::string value;

   bool operator==(const instance_property&) const noexcept = default;
};

}
