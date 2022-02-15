#pragma once

#include "id.hpp"
#include "types.hpp"
#include "utility/enum_bitflags.hpp"

#include <string>

namespace we::world {

enum class hintnode_type {
   snipe = 0,
   patrol = 1,
   cover = 2,
   access = 3,
   jet_jump = 4,
   mine = 5,
   land = 6,
   fortification = 7,
   vehicle_cover = 8,

   unknown_types_start
};

enum class stance_flags {
   none = 0,
   stand = 1,
   crouch = 2,
   prone = 4,
   left = 8,
   right = 16,

   primary_stance_mask = 0b111,
   secondary_stance_mask = 0b11111
};

constexpr bool marked_as_enum_bitflag(stance_flags) noexcept
{
   return true;
}

enum class hintnode_mode {
   none = 0,
   attack = 1,
   defend = 2,
   both = 3,

   unknown_modes_start
};

struct hintnode {
   std::string name;
   int layer = 0;

   quaternion rotation = {1.0f, 0.0f, 0.0f, 0.0f};
   float3 position{};

   hintnode_type type = hintnode_type::snipe;
   hintnode_mode mode = hintnode_mode::none;
   float radius = 0.0f;

   stance_flags primary_stance = stance_flags::none;
   stance_flags secondary_stance = stance_flags::none;

   std::string command_post;

   id<hintnode> id{};

   bool operator==(const hintnode&) const noexcept = default;
};

using hintnode_id = id<hintnode>;

}
