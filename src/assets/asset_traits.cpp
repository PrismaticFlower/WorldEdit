
#include "asset_traits.hpp"
#include "msh/scene_io.hpp"
#include "odf/definition_io.hpp"
#include "utility/read_file.hpp"

namespace sk::assets {

auto asset_traits<odf::definition>::load(const std::filesystem::path& path)
   -> odf::definition
{
   auto file = utility::read_file_to_string(path);

   return odf::read_definition(file);
}

auto asset_traits<msh::flat_model>::load(const std::filesystem::path& path)
   -> msh::flat_model
{
   auto file = utility::read_file_to_bytes(path);

   return msh::flat_model{msh::read_scene_from_bytes(file)};
}

}