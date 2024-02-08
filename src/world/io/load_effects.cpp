#include "load_effects.hpp"

#include "assets/config/io.hpp"
#include "load_failure.hpp"
#include "utility/string_icompare.hpp"

#include <fmt/core.h>

using namespace std::literals;

namespace we::world {

namespace {

void read_bool(const assets::config::values& values, void* pc_value,
               void* ps2_value, void* xbox_value)
{
   const bool value = values.get<int>(0) != 0;

   if (pc_value) *static_cast<bool*>(pc_value) = value;
   if (ps2_value) *static_cast<bool*>(ps2_value) = value;
   if (xbox_value) *static_cast<bool*>(xbox_value) = value;
}

void read_float(const assets::config::values& values, void* pc_value,
                void* ps2_value, void* xbox_value)
{
   const float value = values.get<float>(0);

   if (pc_value) *static_cast<float*>(pc_value) = value;
   if (ps2_value) *static_cast<float*>(ps2_value) = value;
   if (xbox_value) *static_cast<float*>(xbox_value) = value;
}

struct property {
   property(std::string_view name, bool* per_platform_value, bool* pc_value,
            bool* ps2_value, bool* xbox_value)
      : name{name},
        per_platform_value{per_platform_value},
        read_value{read_bool},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   property(std::string_view name, bool* per_platform_value, float* pc_value,
            float* ps2_value, float* xbox_value)
      : name{name},
        read_value{read_float},
        per_platform_value{per_platform_value},
        pc_value{pc_value},
        ps2_value{ps2_value},
        xbox_value{xbox_value}
   {
   }

   void read(const assets::config::values& values) const
   {
      read_value(values, pc_value, ps2_value, xbox_value);
   }

   void read_pc(const assets::config::values& values) const
   {
      read_value(values, pc_value, nullptr, nullptr);

      if (per_platform_value) *per_platform_value = true;
   }

   void read_ps2(const assets::config::values& values) const
   {
      read_value(values, nullptr, ps2_value, nullptr);

      if (per_platform_value) *per_platform_value = true;
   }

   void read_xbox(const assets::config::values& values) const
   {
      read_value(values, nullptr, nullptr, xbox_value);

      if (per_platform_value) *per_platform_value = true;
   }

   std::string_view name = "";

private:
   void (*read_value)(const assets::config::values& values, void* pc, void* ps2,
                      void* xbox) = nullptr;
   bool* per_platform_value = nullptr;
   void* pc_value = nullptr;
   void* ps2_value = nullptr;
   void* xbox_value = nullptr;
};

#define UNPACK_VAR(var, member)                                                \
   &var.member##_per_platform, &var.member##_pc, &var.member##_ps2, &var.member##_xbox

#define UNPACK_PC_XB_VAR(var, member)                                          \
   &var.member##_per_platform, &var.member##_pc, nullptr, &var.member##_xbox

auto read_color_control(assets::config::node& node) -> color_control
{
   static color_control control{};

   const property properties[] = {
      {"Enable"sv, UNPACK_VAR(control, enable)},
      {"GammaBrightness"sv, UNPACK_PC_XB_VAR(control, gamma_brightness)},
      {"GammaColorBalance"sv, UNPACK_PC_XB_VAR(control, gamma_color_balance)},
      {"GammaContrast"sv, UNPACK_PC_XB_VAR(control, gamma_contrast)},
      {"GammaCorrection"sv, UNPACK_PC_XB_VAR(control, gamma_correction)},
      {"GammaHue"sv, UNPACK_PC_XB_VAR(control, gamma_hue)},
      {"WorldBrightness"sv, UNPACK_VAR(control, world_brightness)},
      {"WorldContrast"sv, UNPACK_VAR(control, world_contrast)},
      {"WorldSaturation"sv, UNPACK_VAR(control, world_saturation)},
   };

   for (auto& key_node : node) {
      for (const property& prop : properties) {
         if (not string::iequals(key_node.key, prop.name)) continue;

         prop.read(key_node.values);

         break;
      }
   }

   for (auto& platform_node : node) {
      if (string::iequals(platform_node.key, "PC")) {
         for (auto& key_node : platform_node) {
            for (const property& prop : properties) {
               if (not string::iequals(key_node.key, prop.name)) continue;

               prop.read_pc(key_node.values);

               break;
            }
         }
      }
      else if (string::iequals(platform_node.key, "PS2")) {
         for (auto& key_node : platform_node) {
            for (const property& prop : properties) {
               if (not string::iequals(key_node.key, prop.name)) continue;

               prop.read_ps2(key_node.values);

               break;
            }
         }
      }
      else if (string::iequals(platform_node.key, "XBOX")) {
         for (auto& key_node : platform_node) {
            for (const property& prop : properties) {
               if (not string::iequals(key_node.key, prop.name)) continue;

               prop.read_xbox(key_node.values);

               break;
            }
         }
      }
   }

   return control;
}

}

auto load_effects(const std::string_view str, [[maybe_unused]] output_stream& output)
   -> effects
{
   effects effects{};

   try {
      for (auto& key_node : assets::config::read_config(str)) {
         if (string::iequals(key_node.key, "Effect"sv)) {
            const std::string_view effect = key_node.values.get<std::string_view>(0);

            if (string::iequals(effect, "ColorControl"sv)) {
               effects.color_control = read_color_control(key_node);
            }
            else {
               throw load_failure{
                  fmt::format("Unknown Effect ('{}') in world effects file.", effect)};
            }
         }
         else {
            throw load_failure{
               fmt::format("Unknown entry ('{}') in world effects file.",
                           key_node.key)};
         }
      }
   }
   catch (std::exception& e) {
      throw load_failure{e.what()};
   }

   return effects;
}

}