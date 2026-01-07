
#include "asset_traits.hpp"
#include "io/read_file.hpp"
#include "msh/scene_io.hpp"
#include "odf/definition_io.hpp"
#include "sky/io.hpp"
#include "texture/texture_io.hpp"

namespace we::assets {

auto asset_traits<odf::definition>::load(const io::path& path) -> odf::definition
{
   return odf::read_definition(io::read_file_to_chars(path));
}

auto asset_traits<msh::flat_model>::load(const io::path& path) -> msh::flat_model
{
   return msh::flat_model{msh::read_scene(path, {})};
}

auto asset_traits<texture::texture>::load(const io::path& path) -> texture::texture
{
   return texture::load_texture(path);
}

auto asset_traits<sky::config>::load(const io::path& path) -> sky::config
{
   return sky::read(io::read_file_to_string(path), "PC");
}

}
