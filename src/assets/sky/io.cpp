#include "io.hpp"
#include "assets/config/io.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

namespace we::assets::sky {

namespace {

using assets::config::node;

struct platform_flags {
   const bool pc : 1 = false;
   const bool ps2 : 1 = false;
   const bool psp : 1 = false;
   const bool xbox : 1 = false;
};

void read_dome_model(const node& node, const platform_flags platform, dome_model& model)
{
   for (const auto& child : node) {
      if (platform.pc and string::iequals(child.key, "PC")) {
         read_dome_model(child, platform, model);
      }
      else if (platform.ps2 and string::iequals(child.key, "PS2")) {
         read_dome_model(child, platform, model);
      }
      else if (platform.psp and string::iequals(child.key, "PSP")) {
         read_dome_model(child, platform, model);
      }
      else if (platform.xbox and string::iequals(child.key, "XBOX")) {
         read_dome_model(child, platform, model);
      }
      else if (string::iequals(child.key, "Geometry")) {
         model.geometry = child.values.get<std::string>(0);
      }
      else if (string::iequals(child.key, "MovementScale")) {
         model.movement_scale = child.values.get<float>(0);
      }
      else if (string::iequals(child.key, "Offset")) {
         model.offset = child.values.get<float>(0);
      }
      else if (string::iequals(child.key, "RotationSpeed")) {
         model.rotation.speed = child.values.get<float>(0);
         model.rotation.direction.x = child.values.get<float>(1);
         model.rotation.direction.y = child.values.get<float>(2);
         model.rotation.direction.z = child.values.get<float>(3);
      }
   }
}

void read_dome_info(const node& node, const platform_flags platform, config& sky)
{
   for (const auto& child : node) {
      if (platform.pc and string::iequals(child.key, "PC")) {
         read_dome_info(child, platform, sky);
      }
      else if (platform.ps2 and string::iequals(child.key, "PS2")) {
         read_dome_info(child, platform, sky);
      }
      else if (platform.psp and string::iequals(child.key, "PSP")) {
         read_dome_info(child, platform, sky);
      }
      else if (platform.xbox and string::iequals(child.key, "XBOX")) {
         read_dome_info(child, platform, sky);
      }
      else if (string::iequals(child.key, "TerrainBumpTexture")) {
         sky.terrain_normal_map = child.values.get<std::string>(0);
         sky.terrain_normal_map_tiling = child.values.get<float>(1);
      }
      else if (string::iequals(child.key, "DomeModel")) {
         read_dome_model(child, platform, sky.dome_models.emplace_back());
      }
   }
}

void read_root(const node& node, const platform_flags platform, config& sky)
{
   for (const auto& child : node) {
      if (platform.pc and string::iequals(child.key, "PC")) {
         read_root(child, platform, sky);
      }
      else if (platform.ps2 and string::iequals(child.key, "PS2")) {
         read_root(child, platform, sky);
      }
      else if (platform.psp and string::iequals(child.key, "PSP")) {
         read_root(child, platform, sky);
      }
      else if (platform.xbox and string::iequals(child.key, "XBOX")) {
         read_root(child, platform, sky);
      }
      else if (string::iequals(child.key, "DomeInfo")) {
         read_dome_info(child, platform, sky);
      }
   }
}

}

auto read(const std::string_view str, const std::string_view platform) -> config
{
   config sky;

   read_root(assets::config::read_config(str),
             {.pc = string::iequals(platform, "PC"),
              .ps2 = string::iequals(platform, "PS2"),
              .psp = string::iequals(platform, "PSP"),
              .xbox = string::iequals(platform, "XBOX")},
             sky);

   return sky;
}

}
