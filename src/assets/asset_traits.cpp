
#include "asset_traits.hpp"
#include "io/read_file.hpp"
#include "msh/scene_io.hpp"
#include "odf/definition_io.hpp"
#include "texture/texture_io.hpp"

namespace we::assets {

auto asset_traits<odf::definition>::load(const std::filesystem::path& path)
   -> odf::definition
{
   auto file = io::read_file_to_string(path);

   return odf::read_definition(file);
}

auto asset_traits<msh::flat_model>::load(const std::filesystem::path& path)
   -> msh::flat_model
{
   return msh::flat_model{msh::read_scene(path)};
}

auto asset_traits<texture::texture>::load(const std::filesystem::path& path)
   -> texture::texture
{
   return texture::load_texture(path);
}

}
