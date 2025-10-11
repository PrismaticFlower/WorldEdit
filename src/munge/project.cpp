
#include "project.hpp"

#include "assets/config/io.hpp"

#include "io/error.hpp"
#include "io/output_file.hpp"
#include "io/read_file.hpp"

#include "utility/string_icompare.hpp"

#include <absl/container/btree_map.h>

namespace we::munge {

void save_project(const project& project, const io::path& path) noexcept
{
   try {
      io::output_file out{path};

      out.write_ln("Version(0);"); // Just incase

      out.write_ln("Deploy({:d});", project.deploy);
      out.write_ln("AddmeActive({:d});", project.addme_active);
      out.write_ln("CommonActive({:d});", project.common_active);
      out.write_ln("LoadActive({:d});", project.load_active);
      out.write_ln("ShellActive({:d});", project.shell_active);
      out.write_ln("SoundActive({:d});", project.sound_active);
      out.write_ln("SoundCommonBank({:d});", project.sound_common_bank);

      out.write_ln("Sides()");
      out.write_ln("{");

      for (const project_child& side : project.sides) {
         out.write_ln("   {}({:d});", side.name, side.active);
      }

      out.write_ln("}");

      out.write_ln("Worlds()");
      out.write_ln("{");

      for (const project_child& world : project.worlds) {
         out.write_ln("   {}({:d});", world.name, world.active);
      }

      out.write_ln("}");

      out.write_ln("SoundShared()");
      out.write_ln("{");

      for (const project_child_sound_shared& sound : project.sound_shared) {
         out.write_ln("   {}()", sound.name);
         out.write_ln("   {");
         out.write_ln("      Localized({:d});", sound.localized);
         out.write_ln("   }");
      }

      out.write_ln("}");

      out.write_ln("SoundWorlds()");
      out.write_ln("{");

      for (const project_child_sound_world& sound : project.sound_worlds) {
         out.write_ln("   {}()", sound.name);
         out.write_ln("   {");
         out.write_ln("      Active({:d});", sound.active);
         out.write_ln("      Localized({:d});", sound.localized);
         out.write_ln("   }");
      }

      out.write_ln("}");

      out.write_ln("SoundLocalizations()");
      out.write_ln("{");

      for (const project_sound_localization& localization : project.sound_localizations) {
         out.write_ln("   {}()", localization.language);
         out.write_ln("   {");
         out.write_ln("      OutputDirectory(\"{}\");", localization.output_directory);
         out.write_ln("   }");
      }

      out.write_ln("}");

      out.write_ln("Config()");
      out.write_ln("{");
      out.write_ln("   ToolsFLBin(\"{}\");",
                   project.config.toolsfl_bin_path.string_view());
      out.write_ln("}");
   }
   catch (io::open_error&) {
      // TODO: Error handling strategy for failed saves.
   }
}

auto load_project(const io::path& path) noexcept -> project
{
   try {
      using namespace assets;
      using string::iequals;

      project project;

      for (const config::key_node& key_node :
           config::read_config(io::read_file_to_string(path))) {
         if (iequals("Deploy", key_node.key)) {
            project.deploy = key_node.values.get<int>(0) != 0;
         }
         else if (iequals("AddmeActive", key_node.key)) {
            project.addme_active = key_node.values.get<int>(0) != 0;
         }
         else if (iequals("CommonActive", key_node.key)) {
            project.common_active = key_node.values.get<int>(0) != 0;
         }
         else if (iequals("LoadActive", key_node.key)) {
            project.load_active = key_node.values.get<int>(0) != 0;
         }
         else if (iequals("ShellActive", key_node.key)) {
            project.shell_active = key_node.values.get<int>(0) != 0;
         }
         else if (iequals("SoundActive", key_node.key)) {
            project.sound_active = key_node.values.get<int>(0) != 0;
         }
         else if (iequals("SoundCommonBank", key_node.key)) {
            project.sound_common_bank = key_node.values.get<int>(0) != 0;
         }
         else if (iequals("Sides", key_node.key)) {
            for (const config::key_node& side_key_node : key_node) {
               project.sides.emplace_back(side_key_node.key,
                                          side_key_node.values.get<int>(0) != 0);
            }
         }
         else if (iequals("Worlds", key_node.key)) {
            for (const config::key_node& world_key_node : key_node) {
               project.worlds.emplace_back(world_key_node.key,
                                           world_key_node.values.get<int>(0) != 0);
            }
         }
         else if (iequals("SoundShared", key_node.key)) {
            for (const config::key_node& sound_key_node : key_node) {
               project_child_sound_shared& sound =
                  project.sound_shared.emplace_back();

               sound.name = sound_key_node.key;

               for (const config::key_node& prop : sound_key_node) {
                  if (iequals("Localized", prop.key)) {
                     sound.localized = prop.values.get<int>(0) != 0;
                  }
               }
            }
         }
         else if (iequals("SoundWorlds", key_node.key)) {
            for (const config::key_node& sound_key_node : key_node) {
               project_child_sound_world& sound = project.sound_worlds.emplace_back();

               sound.name = sound_key_node.key;

               for (const config::key_node& prop : sound_key_node) {
                  if (iequals("Active", prop.key)) {
                     sound.active = prop.values.get<int>(0) != 0;
                  }
                  else if (iequals("Localized", prop.key)) {
                     sound.localized = prop.values.get<int>(0) != 0;
                  }
               }
            }
         }
         else if (iequals("SoundLocalizations", key_node.key)) {
            for (const config::key_node& localization_key_node : key_node) {
               project_sound_localization& localization =
                  project.sound_localizations.emplace_back();

               localization.language = localization_key_node.key;

               for (const config::key_node& prop : localization_key_node) {
                  if (iequals("OutputDirectory", prop.key)) {
                     localization.output_directory = prop.values.get<std::string>(0);
                  }
               }
            }
         }
         else if (iequals("Config", key_node.key)) {
            for (const config::key_node& config_key_node : key_node) {
               if (iequals("ToolsFLBin", config_key_node.key)) {
                  project.config.toolsfl_bin_path =
                     io::path{config_key_node.values.get<std::string>(0)};
               }
            }
         }
      }

      return project;
   }
   catch (io::error&) {
      return {};
   }
   catch (std::runtime_error&) {
      return {};
   }
}

void merge_loaded_project(project& current_project, const project& loaded_project) noexcept
{
   current_project.deploy = loaded_project.deploy;

   current_project.addme_active = loaded_project.addme_active;
   current_project.common_active = loaded_project.common_active;
   current_project.load_active = loaded_project.load_active;
   current_project.shell_active = loaded_project.shell_active;
   current_project.sound_active = loaded_project.sound_active;
   current_project.sound_common_bank = loaded_project.sound_common_bank;

   current_project.sound_localizations = loaded_project.sound_localizations;

   constexpr static auto comparator = [](std::string_view left, std::string_view right) {
      return string::iless_than(left, right);
   };

   // Merge Worlds
   {
      absl::btree_map<std::string_view, std::size_t, decltype(comparator)> world_map;

      for (std::size_t i = 0; i < loaded_project.worlds.size(); ++i) {
         world_map.emplace(loaded_project.worlds[i].name, i);
      }

      for (project_child& world : current_project.worlds) {
         const auto it = world_map.find(world.name);
         if (it == world_map.end()) continue;

         world = loaded_project.worlds[it->second];
      }
   }

   // Merge Sides
   {
      absl::btree_map<std::string_view, std::size_t, decltype(comparator)> side_map;

      for (std::size_t i = 0; i < loaded_project.sides.size(); ++i) {
         side_map.emplace(loaded_project.sides[i].name, i);
      }

      for (project_child& side : current_project.sides) {
         const auto it = side_map.find(side.name);
         if (it == side_map.end()) continue;

         side = loaded_project.sides[it->second];
      }
   }

   // Merge Shared Sounds
   {
      absl::btree_map<std::string_view, std::size_t, decltype(comparator)> sound_shared_map;

      for (std::size_t i = 0; i < loaded_project.sound_shared.size(); ++i) {
         sound_shared_map.emplace(loaded_project.sound_shared[i].name, i);
      }

      for (project_child_sound_shared& sound_shared : current_project.sound_shared) {
         const auto it = sound_shared_map.find(sound_shared.name);
         if (it == sound_shared_map.end()) continue;

         sound_shared = loaded_project.sound_shared[it->second];
      }
   }

   // Merge World Sounds
   {
      absl::btree_map<std::string_view, std::size_t, decltype(comparator)> sound_world_map;

      for (std::size_t i = 0; i < loaded_project.sound_worlds.size(); ++i) {
         sound_world_map.emplace(loaded_project.sound_worlds[i].name, i);
      }

      for (project_child_sound_world& sound_world : current_project.sound_worlds) {
         const auto it = sound_world_map.find(sound_world.name);
         if (it == sound_world_map.end()) continue;

         sound_world = loaded_project.sound_worlds[it->second];
      }
   }
}
}