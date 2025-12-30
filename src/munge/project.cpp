
#include "project.hpp"

#include "assets/config/io.hpp"

#include "io/error.hpp"
#include "io/output_file.hpp"
#include "io/read_file.hpp"

#include "utility/string_icompare.hpp"

#include <absl/container/btree_map.h>

template<>
struct fmt::formatter<we::munge::project_platform>
   : fmt::formatter<std::string_view> {
   auto format(we::munge::project_platform v, format_context& ctx) const -> fmt::appender
   {
      switch (v) {
      default:
      case we::munge::project_platform::pc: {
         return fmt::formatter<std::string_view>::format("PC", ctx);
      };
      case we::munge::project_platform::ps2: {
         return fmt::formatter<std::string_view>::format("PS2", ctx);
      };
      case we::munge::project_platform::xbox: {
         return fmt::formatter<std::string_view>::format("Xbox", ctx);
      };
      }
   }
};

template<>
struct fmt::formatter<we::munge::project_platform_filter>
   : fmt::formatter<std::string_view> {
   auto format(we::munge::project_platform_filter v, format_context& ctx) const
      -> fmt::appender
   {
      switch (v) {
      case we::munge::project_platform_filter::all: {
         return fmt::formatter<std::string_view>::format("all", ctx);
      };
      case we::munge::project_platform_filter::pc: {
         return fmt::formatter<std::string_view>::format("pc", ctx);
      };
      case we::munge::project_platform_filter::ps2: {
         return fmt::formatter<std::string_view>::format("ps2", ctx);
      };
      case we::munge::project_platform_filter::xbox: {
         return fmt::formatter<std::string_view>::format("xbox", ctx);
      };
      }

      return fmt::formatter<std::string_view>::format("<unknown>", ctx);
   }
};

namespace we::munge {

namespace {

void write_commands(io::output_file& out, std::string_view name,
                    std::span<const project_custom_command> commands)
{
   out.write_ln("      {}()", name);
   out.write_ln("      {");

   for (const project_custom_command& command : commands) {
      out.write_ln("         Command()");
      out.write_ln("         {");
      out.write_ln("            CommandLine({:?});", command.command_line);
      out.write_ln("            PlatformFilter(\"{}\");", command.platform_filter);
      out.write_ln("            Detach({:d});", command.detach);
      out.write_ln("         }");
   }

   out.write_ln("      }");
}

void read_commands(const assets::config::key_node& key_node,
                   std::vector<project_custom_command>& commands)
{
   using namespace assets;
   using string::iequals;

   for (const config::key_node& command_key_node : key_node) {
      if (not iequals(command_key_node.key, "Command")) continue;

      project_custom_command command;

      for (const config::key_node& property_key_node : command_key_node) {
         if (iequals(property_key_node.key, "CommandLine")) {
            command.command_line = property_key_node.values.get<std::string>(0);
         }
         else if (iequals(property_key_node.key, "PlatformFilter")) {
            if (iequals("all", property_key_node.values.get<std::string_view>(0))) {
               command.platform_filter = project_platform_filter::all;
            }
            else if (iequals("pc", property_key_node.values.get<std::string_view>(0))) {
               command.platform_filter = project_platform_filter::pc;
            }
            else if (iequals("ps2", property_key_node.values.get<std::string_view>(0))) {
               command.platform_filter = project_platform_filter::ps2;
            }
            else if (iequals("xbox",
                             property_key_node.values.get<std::string_view>(0))) {
               command.platform_filter = project_platform_filter::xbox;
            }
         }
         else if (iequals(property_key_node.key, "Detach")) {
            command.detach = property_key_node.values.get<int>(0) != 0;
         }
      }

      commands.push_back(std::move(command));
   }
}

void write_clean_directories(io::output_file& out, std::string_view name,
                             std::span<const std::string> directories)
{
   out.write_ln("      {}()", name);
   out.write_ln("      {");

   for (const std::string& directory : directories) {
      out.write_ln("         Directory({:?});", directory);
   }

   out.write_ln("      }");
}

void read_clean_directories(const assets::config::key_node& key_node,
                            std::vector<std::string>& directories)
{
   using namespace assets;
   using string::iequals;

   for (const config::key_node& directory_key_node : key_node) {
      if (not iequals(directory_key_node.key, "Directory")) continue;

      directories.push_back(directory_key_node.values.get<std::string>(0));
   }
}

}

void save_project(const project& project, const io::path& path) noexcept
{
   try {
      io::output_file out{path};

      out.write_ln("Version(1);");

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
         out.write_ln("      OutputDirectory({:?});", localization.output_directory);
         out.write_ln("   }");
      }

      out.write_ln("}");

      out.write_ln("Config()");
      out.write_ln("{");
      out.write_ln("   ToolsFLBin({:?});",
                   project.config.toolsfl_bin_path.string_view());
      out.write_ln("   UseBuiltinTools({:d});", project.config.use_builtin_tools);
      out.write_ln("   Platform(\"{}\");", project.config.platform);

      out.write_ln("   CustomMungeCommands()");
      out.write_ln("   {");

      const project_custom_commands& custom_commands = project.config.custom_commands;

      write_commands(out, "Common", custom_commands.common);
      write_commands(out, "CommonPack", custom_commands.common_pack);
      write_commands(out, "CommonMissionChildPack",
                     custom_commands.common_mission_child_pack);
      write_commands(out, "CommonMissionPack", custom_commands.common_mission_pack);
      write_commands(out, "CommonFPMPack", custom_commands.common_fpm_pack);

      write_commands(out, "Load", custom_commands.load);
      write_commands(out, "LoadPack", custom_commands.load_pack);

      write_commands(out, "Shell", custom_commands.shell);
      write_commands(out, "ShellPack", custom_commands.shell_pack);
      write_commands(out, "ShellPS2Pack", custom_commands.shell_ps2_pack);

      write_commands(out, "Side", custom_commands.side);
      write_commands(out, "SideChildPack", custom_commands.side_child_pack);
      write_commands(out, "SidePack", custom_commands.side_pack);
      write_commands(out, "SideFPMPack", custom_commands.side_fpm_pack);

      write_commands(out, "World", custom_commands.world);
      write_commands(out, "WorldPack", custom_commands.world_pack);

      out.write_ln("   }");

      const project_custom_clean_directories& clean_directories =
         project.config.custom_clean_directories;

      out.write_ln("   CustomCleanDirectories()");
      out.write_ln("   {");

      write_clean_directories(out, "Common", clean_directories.common);
      write_clean_directories(out, "Load", clean_directories.load);
      write_clean_directories(out, "Shell", clean_directories.shell);
      write_clean_directories(out, "Side", clean_directories.side);
      write_clean_directories(out, "World", clean_directories.world);

      out.write_ln("   }");

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

      const std::string project_file_contents = io::read_file_to_string(path);
      const bool no_escape_seqeunces =
         string::istarts_with(project_file_contents, "Version(0);");

      project project;

      for (const config::key_node& key_node :
           config::read_config(project_file_contents,
                               {.support_escape_sequences = not no_escape_seqeunces})) {
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
               else if (iequals("Project", config_key_node.key)) {
                  const std::string_view platform =
                     config_key_node.values.get<std::string_view>(0);

                  if (iequals(platform, "PC")) {
                     project.config.platform = project_platform::pc;
                  }
                  else if (iequals(platform, "PS2")) {
                     project.config.platform = project_platform::ps2;
                  }
                  else if (iequals(platform, "Xbox")) {
                     project.config.platform = project_platform::xbox;
                  }
               }
               else if (iequals("UseBuiltinTools", config_key_node.key)) {
                  project.config.use_builtin_tools =
                     config_key_node.values.get<int>(0) != 0;
               }
               else if (iequals("CustomMungeCommands", config_key_node.key)) {
                  project_custom_commands& custom_commands =
                     project.config.custom_commands;

                  for (const config::key_node& command_key_node : config_key_node) {
                     if (iequals("Common", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.common);
                     }
                     else if (iequals("CommonPack", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.common_pack);
                     }
                     else if (iequals("CommonMissionChildPack", command_key_node.key)) {
                        read_commands(command_key_node,
                                      custom_commands.common_mission_child_pack);
                     }
                     else if (iequals("CommonMissionPack", command_key_node.key)) {
                        read_commands(command_key_node,
                                      custom_commands.common_mission_pack);
                     }
                     else if (iequals("CommonFPMPack", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.common_fpm_pack);
                     }
                     else if (iequals("Load", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.load);
                     }
                     else if (iequals("LoadPack", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.load_pack);
                     }
                     else if (iequals("Shell", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.shell);
                     }
                     else if (iequals("ShellPack", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.shell_pack);
                     }
                     else if (iequals("ShellPS2Pack", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.shell_ps2_pack);
                     }
                     else if (iequals("Side", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.side);
                     }
                     else if (iequals("SideChildPack", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.side_child_pack);
                     }
                     else if (iequals("SidePack", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.side_pack);
                     }
                     else if (iequals("SideFPMPack", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.side_fpm_pack);
                     }
                     else if (iequals("World", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.world);
                     }
                     else if (iequals("WorldPack", command_key_node.key)) {
                        read_commands(command_key_node, custom_commands.world_pack);
                     }
                  }
               }
               else if (iequals("CustomCleanDirectories", config_key_node.key)) {
                  project_custom_clean_directories& clean_directories =
                     project.config.custom_clean_directories;

                  for (const config::key_node& set_key_node : config_key_node) {
                     if (iequals("Common", set_key_node.key)) {
                        read_clean_directories(set_key_node, clean_directories.common);
                     }
                     else if (iequals("Load", set_key_node.key)) {
                        read_clean_directories(set_key_node, clean_directories.load);
                     }
                     else if (iequals("Shell", set_key_node.key)) {
                        read_clean_directories(set_key_node, clean_directories.shell);
                     }
                     else if (iequals("Side", set_key_node.key)) {
                        read_clean_directories(set_key_node, clean_directories.side);
                     }
                     else if (iequals("World", set_key_node.key)) {
                        read_clean_directories(set_key_node, clean_directories.world);
                     }
                  }
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

   if (current_project.config.toolsfl_bin_path.empty()) {
      current_project.config.toolsfl_bin_path = loaded_project.config.toolsfl_bin_path;
   }

   current_project.config.platform = loaded_project.config.platform;
   current_project.config.use_builtin_tools = loaded_project.config.use_builtin_tools;

   current_project.config.custom_commands = loaded_project.config.custom_commands;
   current_project.config.custom_clean_directories =
      loaded_project.config.custom_clean_directories;
}

auto to_ui_string(const project_platform platform) noexcept -> const char*
{
   switch (platform) {
   case project_platform::pc:
      return "PC";
   case project_platform::ps2:
      return "PS2";
   case project_platform::xbox:
      return "Xbox";
   default:
      return "<unknown>";
   }
}

auto to_ui_string(const project_platform_filter filter) noexcept -> const char*
{
   switch (filter) {
   case munge::project_platform_filter::all:
      return "All";
   case munge::project_platform_filter::pc:
      return "PC";
   case munge::project_platform_filter::ps2:
      return "PS2";
   case munge::project_platform_filter::xbox:
      return "Xbox";
   default:
      return "<unknown>";
   }
}

}