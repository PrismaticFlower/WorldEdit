#pragma once

#include "types.hpp"
#include "utility/enum_bitflags.hpp"

#include <array>
#include <string>

namespace sk::assets::msh {

enum class material_flags : uint8 {
   none = 0,
   unlit = 1,
   glow = 2,
   transparent = 4,
   doublesided = 8,
   hardedged = 16,
   perpixel = 32,
   additive = 64,
   specular = 128
};

constexpr bool marked_as_enum_bitflag(material_flags) noexcept
{
   return true;
}

enum class rendertype : uint8 {
   normal = 0,
   scrolling = 3,
   specular = 4,
   envmapped = 6,
   reflection = envmapped,
   animated = 7,
   glow = 11,
   detail = glow, // SWBF1
   refraction = 22,
   normalmap_tiled = 24,
   normalmapped_envmapped = 26,
   blinking = 25,
   normalmap = 27,
   normalmap_specular = 28,
   normalmap_tiled_envmapped = 29
};

struct material {
   std::string name;
   float3 specular_color = {1.0f, 1.0f, 1.0f};
   material_flags flags = material_flags::none;
   rendertype rendertype = rendertype::normal;
   uint8 data0 = 0;
   uint8 data1 = 0;

   std::array<std::string, 4> textures{};

   bool operator==(const material&) const noexcept = default;
};

}