#include "path_properties.hpp"

#include <array>

namespace we::world {

namespace {

constexpr std::array entity_follow_props{
   "MaxEntities",     "MaxSquadronSize", "PathRoll",  "Speed",
   "VarianceX",       "VarianceY",       "VarianceZ", "BranchDifferent",
   "SingleDirection", "Direction",       "Frequency", "EnableObject",
   "Class",           "Class1",          "Class2",    "Class3",
   "Class4",          "Class5",
};

constexpr std::array entity_follow_node_props{
   "BranchProbability", "Range",        "BranchRegion", "MaxAngle",
   "BranchPaths",       "Speed",        "VarianceX",    "VarianceY",
   "VarianceZ",         "LandOnArrival"};

constexpr std::array formation_props{"RootIsSlot"};

constexpr std::array formation_node_props{"Root", "MemberID"};

constexpr std::array patrol_node_props{"WaitMin", "WaitMax"};

}

auto get_path_properties(const path_type type) -> std::span<const char* const>
{
   switch (type) {
   default:
   case path_type::none:
      return {};
   case path_type::entity_follow:
      return entity_follow_props;
   case path_type::formation:
      return formation_props;
   case path_type::patrol:
      return {};
   }
}

auto get_path_node_properties(const path_type type) -> std::span<const char* const>
{
   switch (type) {
   default:
   case path_type::none:
      return {};
   case path_type::entity_follow:
      return entity_follow_node_props;
   case path_type::formation:
      return formation_node_props;
   case path_type::patrol:
      return patrol_node_props;
   }
}

}
