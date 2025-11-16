#include "to_ui_string.hpp"

#pragma warning(default : 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label

namespace we::world {

auto to_ui_string(const light_type type) noexcept -> const char*
{
   switch (type) {
      // clang-format off
   case light_type::directional:                 return "Directional";
   case light_type::point:                       return "Point";
   case light_type::spot:                        return "Spot";
   case light_type::directional_region_box:      return "Directional Region Box";
   case light_type::directional_region_sphere:   return "Directional Region Sphere";
   case light_type::directional_region_cylinder: return "Directional Region Cylinder";
   default:                                      return "<unknown>";
      // clang-format on
   }
}

auto to_ui_string(const texture_addressing addressing) noexcept -> const char*
{
   switch (addressing) {
      // clang-format off
   case texture_addressing::wrap:  return "Wrap";
   case texture_addressing::clamp: return "Clamp";
   default:                        return "<unknown>";
      // clang-format on
   }
}

auto to_ui_string(const ps2_blend_mode mode) noexcept -> const char*
{
   switch (mode) {
      // clang-format off
   case ps2_blend_mode::add:      return "Add";
   case ps2_blend_mode::multiply: return "Multiply";
   case ps2_blend_mode::blend:    return "Blend";
   default:                       return "<unknown>";
      // clang-format on
   }
}

auto to_ui_string(const path_type type) noexcept -> const char*
{
   switch (type) {
      // clang-format off
   case path_type::none:          return "None";
   case path_type::entity_follow: return "Entity Follow";
   case path_type::formation:     return "Formation";
   case path_type::patrol:        return "Patrol";
   default:                       return "<unknown>";
      // clang-format on
   }
}

auto to_ui_string(const path_spline_type type) noexcept -> const char*
{
   switch (type) {
      // clang-format off
   case path_spline_type::none:        return "None";
   case path_spline_type::linear:      return "Linear";
   case path_spline_type::hermite:     return "Hermite";
   case path_spline_type::catmull_rom: return "Catmull-Rom";
   default:                            return "<unknown>";
      // clang-format on
   }
}

auto to_ui_string(const region_type type) noexcept -> const char*
{
   switch (type) {
      // clang-format off
   case region_type::typeless:      return "Typeless";
   case region_type::soundstream:   return "Sound Stream";
   case region_type::soundstatic:   return "Sound Static";
   case region_type::soundspace:    return "Sound Space";
   case region_type::soundtrigger:  return "Sound Trigger";
   case region_type::foleyfx:       return "Foley FX";
   case region_type::shadow:        return "Shadow";
   case region_type::mapbounds:     return "Map Bounds";
   case region_type::rumble:        return "Rumble";
   case region_type::reflection:    return "Reflection";
   case region_type::rainshadow:    return "Rain Shadow";
   case region_type::danger:        return "Danger";
   case region_type::damage_region: return "Damage Region";
   case region_type::ai_vis:        return "AI Vis";
   case region_type::colorgrading:  return "Color Grading (Shader Patch)";
   default:                         return "<unknown>";
      // clang-format on
   }
}

auto to_ui_string(const region_shape shape) noexcept -> const char*
{
   switch (shape) {
      // clang-format off
   case region_shape::box:      return "Box";
   case region_shape::sphere:   return "Sphere";
   case region_shape::cylinder: return "Cylinder";
   default:                     return "<unknown>";
      // clang-format on
   }
}

auto to_ui_string(const hintnode_type type) noexcept -> const char*
{
   switch (type) {
      // clang-format off
   case hintnode_type::snipe:               return "Snipe";
   case hintnode_type::patrol:              return "Patrol";
   case hintnode_type::cover:               return "Cover";
   case hintnode_type::access:              return "Access";
   case hintnode_type::jet_jump:            return "Jet Jump";
   case hintnode_type::mine:                return "Mine";
   case hintnode_type::land:                return "Land";
   case hintnode_type::fortification:       return "Fortification";
   case hintnode_type::vehicle_cover:       return "Vehicle Cover";
   case hintnode_type::unknown_types_start: return "<unknown>";
   default:                                 return "<unknown>";
      // clang-format on
   }
}

auto to_ui_string(const hintnode_mode mode) noexcept -> const char*
{
   switch (mode) {
      // clang-format off
   case hintnode_mode::none:                return "None";
   case hintnode_mode::attack:              return "Attack";
   case hintnode_mode::defend:              return "Defend";
   case hintnode_mode::both:                return "Both";
   case hintnode_mode::unknown_modes_start: return "<unknown>";
   default:                                 return "<unknown>";
      // clang-format on
   }
}

}