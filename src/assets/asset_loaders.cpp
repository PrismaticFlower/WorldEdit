
#include "asset_loaders.hpp"
#include "msh/scene_io.hpp"
#include "odf/definition_io.hpp"
#include "utility/read_file.hpp"

namespace sk::assets {

auto loaders<odf::definition>::load(const std::filesystem::path& path) -> odf::definition
{
   auto file = utility::read_file_to_string(path);

   return odf::read_definition(file);
}

auto loaders<msh::flat_model>::load(const std::filesystem::path& path) -> msh::flat_model
{
   auto file = utility::read_file_to_bytes(path);

   return msh::flat_model{msh::read_scene_from_bytes(file)};
}

}