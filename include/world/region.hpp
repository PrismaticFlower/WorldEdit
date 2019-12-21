#pragma once

#include <types.hpp>

#include <string>

namespace sk::world {

struct region {
   std::string name;
   int layer = 0;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   vec3 position{};
   vec3 size{};

   std::string description;

   bool operator==(const region&) const noexcept = default;
};

}