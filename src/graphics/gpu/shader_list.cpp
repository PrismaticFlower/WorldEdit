
#include "shader_list.hpp"

#include <fmt/format.h>

using namespace std::literals;

namespace sk::graphics::gpu {

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
           .entrypoint = "main",
           .type = type_from_name(name),
           .file = fmt::format("worldedit/shaders/{}.hlsl", name)};
}

auto material_shader(const std::string_view name, const std::string_view file_name,
                     std::initializer_list<shader_define> defines) -> shader_description
{
   return {.name = std::string{name},
           .entrypoint = "main",
           .type = type_from_name(name),
           .file = fmt::format("worldedit/shaders/{}.hlsl", file_name),
           .defines = defines};
}

}

std::initializer_list<shader_description> shader_list =
   {shader("basic_object_meshVS"),
    shader("basic_object_meshPS"),
    shader("basic_mesh_lightingPS"),

    material_shader("normal_meshPS", "normal_meshPS",
                    {{"MATERIAL_ALPHA_CUTOUT", 0}, {"MATERIAL_TRANSPARENT", 0}}),
    material_shader("normal_cutout_meshPS", "normal_meshPS",
                    {{"MATERIAL_ALPHA_CUTOUT", 1}, {"MATERIAL_TRANSPARENT", 0}}),
    material_shader("normal_transparent_meshPS", "normal_meshPS",
                    {{"MATERIAL_ALPHA_CUTOUT", 0}, {"MATERIAL_TRANSPARENT", 1}}),

    shader("terrain_patchVS"),
    shader("terrain_basicPS"),
    shader("terrain_lightingPS"),
    shader("terrain_normalPS"),

    shader("meta_lineVS"),

    shader("meta_object_meshVS"),
    shader("meta_object_meshPS"),

    shader("meta_object_mesh_outlinedGS"),
    shader("meta_object_mesh_outlinedPS")};
}
