#include "save_blocks_meshes.hpp"

#include "../blocks/export/mesh_gather.hpp"

#include "assets/msh/scene_io.hpp"

#include "io/output_file.hpp"

#include <vector>

namespace we::world {

namespace {

auto foley_name(const block_foley_group group) noexcept -> const char*
{
   // clang-format off
   switch (group) {
   case block_foley_group::stone:   return "stone";
   case block_foley_group::dirt:    return "dirt";
   case block_foley_group::grass:   return "grass";
   case block_foley_group::metal:   return "metal";
   case block_foley_group::snow:    return "snow";
   case block_foley_group::terrain: return "terrain";
   case block_foley_group::water:   return "water";
   case block_foley_group::wood:    return "wood";
   }
   // clang-format on

   return "unknown";
}

}

auto save_blocks_meshes(const io::path& output_directory,
                        const std::string_view world_name, const blocks& blocks)
   -> std::size_t
{
   if (not io::exists(output_directory) and not io::create_directory(output_directory)) {
      throw std::runtime_error{"Unable to create directory to save blocks."};
   }

   const std::vector<block_export_scene> scenes = gather_export_meshes(blocks);

   for (std::size_t scene_index = 0; scene_index < scenes.size(); ++scene_index) {
      const block_export_scene& scene = scenes[scene_index];
      const std::string mesh_name =
         fmt::format("WE_{}_blocks{}", world_name, scene_index);

      assets::msh::save_scene(io::compose_path(output_directory, mesh_name, ".msh"),
                              scene.msh_scene);

      // write .msh.option file
      {
         io::output_file option{
            io::compose_path(output_directory, mesh_name, ".msh.option")};

         option.write("-donotmergecollision");

         if (not scene.has_collision) option.write(" -nocollision");
      }

      // write .odf
      {
         io::output_file odf{io::compose_path(output_directory, mesh_name, ".odf")};

         odf.write_ln("[GameObjectClass]");
         odf.write_ln("");
         odf.write_ln("ClassLabel   = \"prop\"");
         odf.write_ln("GeometryName = \"{}.msh\"", mesh_name);
         odf.write_ln("");
         odf.write_ln("[Properties]");
         odf.write_ln("");
         odf.write_ln("GeometryName = \"{}\"", mesh_name);
         odf.write_ln("FoleyFXGroup = \"{}_foley\"", foley_name(scene.foley_group));
      }
   }

   return scenes.size();
}
}