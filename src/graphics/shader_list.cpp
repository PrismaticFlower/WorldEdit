
#include "shader_list.hpp"

#include <fmt/format.h>

using namespace std::literals;

namespace we::graphics {

namespace {

auto type_from_name(const std::string_view name) -> shader_type
{
   if (name.ends_with("VS")) return shader_type::vertex;
   if (name.ends_with("HS")) return shader_type::hull;
   if (name.ends_with("DS")) return shader_type::domain;
   if (name.ends_with("GS")) return shader_type::geometry;
   if (name.ends_with("PS")) return shader_type::pixel;
   if (name.ends_with("CS")) return shader_type::compute;

   return shader_type::library;
}

auto shader(const std::string_view name) -> shader_description
{
   return {.name = std::string{name},
           .entrypoint = L"main",
           .type = type_from_name(name),
           .model = shader_model_6_0,
           .file = fmt::format("worldedit/shaders/{}.hlsl", name)};
}

auto material_shader(const std::string_view name, const std::string_view file_name,
                     std::initializer_list<shader_define> defines) -> shader_description
{
   return {.name = std::string{name},
           .entrypoint = L"main",
           .type = type_from_name(name),
           .model = shader_model_6_0,
           .file = fmt::format("worldedit/shaders/{}.hlsl", file_name),
           .defines = defines};
}

}

std::initializer_list<shader_description> shader_list =
   {shader("mesh_shadowVS"),

    shader("meshVS"),
    shader("mesh_basicPS"),
    shader("mesh_basic_lightingPS"),
    shader("mesh_depth_prepassVS"),
    shader("mesh_depth_cutoutPS"),

    material_shader("mesh_normalPS", "mesh_normalPS", {{L"MATERIAL_TRANSPARENT", 0}}),
    material_shader("mesh_normal_transparentPS", "mesh_normalPS",
                    {{L"MATERIAL_TRANSPARENT", 1}}),

    shader("terrain_patchVS"),
    shader("terrain_basicPS"),
    shader("terrain_lightingPS"),
    shader("terrain_normalPS"),

    shader("meta_lineVS"),

    shader("meta_meshVS"),
    shader("meta_meshPS"),

    shader("meta_mesh_outlinedGS"),
    shader("meta_mesh_outlinedPS")};
}
