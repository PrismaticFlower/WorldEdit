#pragma once

#include "ai_path_flags.hpp"
#include "id.hpp"
#include "types.hpp"

#include <array>
#include <string>

namespace we::world {

struct barrier {
   std::string name;

   std::array<float2, 4> corners{};
   ai_path_flags flags = ai_path_flags::soldier | ai_path_flags::hover |
                         ai_path_flags::small | ai_path_flags::medium |
                         ai_path_flags::huge | ai_path_flags::flyer;

   id<barrier> id{};

   bool operator==(const barrier&) const noexcept = default;
};

using barrier_id = id<barrier>;

}
