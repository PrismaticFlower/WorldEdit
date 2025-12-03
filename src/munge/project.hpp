#include "io/path.hpp"

#include <string>
#include <vector>

namespace we::munge {

enum class project_platform { pc, ps2, xbox };

enum class project_platform_filter { all, pc, ps2, xbox };

struct project_custom_command {
   std::string command_line;
   project_platform_filter platform_filter = project_platform_filter::all;

   bool detach = false;

   bool operator==(const project_custom_command&) const noexcept = default;
};

struct project_custom_commands {
   std::vector<project_custom_command> common;
   std::vector<project_custom_command> common_pack;
   std::vector<project_custom_command> common_mission_child_pack;
   std::vector<project_custom_command> common_mission_pack;
   std::vector<project_custom_command> common_fpm_pack;

   std::vector<project_custom_command> load;
   std::vector<project_custom_command> load_pack;

   std::vector<project_custom_command> shell;
   std::vector<project_custom_command> shell_pack;
   std::vector<project_custom_command> shell_ps2_pack;

   std::vector<project_custom_command> side;
   std::vector<project_custom_command> side_child_pack;
   std::vector<project_custom_command> side_pack;
   std::vector<project_custom_command> side_fpm_pack;

   std::vector<project_custom_command> world;
   std::vector<project_custom_command> world_pack;

   bool operator==(const project_custom_commands&) const noexcept = default;
};

struct project_custom_clean_directories {
   std::vector<std::string> common;
   std::vector<std::string> load;
   std::vector<std::string> shell;
   std::vector<std::string> side;
   std::vector<std::string> world;

   bool operator==(const project_custom_clean_directories&) const noexcept = default;
};

struct project_config {
   io::path toolsfl_bin_path;
   project_platform platform = project_platform::pc;

   project_custom_commands custom_commands;
   project_custom_clean_directories custom_clean_directories;

   bool operator==(const project_config&) const noexcept = default;
};

/// @brief Represents a world or side in a project.
struct project_child {
   /// @brief Name of the world or side.
   std::string name;

   /// @brief If the child should be munged/cleaned.
   bool active = true;

   bool operator==(const project_child&) const noexcept = default;
};

/// @brief Represents a sound directory in a project.
struct project_child_sound_shared {
   /// @brief Name of the sound.
   std::string name;

   /// @brief If the child produces localized sound lvls.
   bool localized = false;

   bool operator==(const project_child_sound_shared&) const noexcept = default;
};

/// @brief Represents a sound directory in a project.
struct project_child_sound_world {
   /// @brief Name of the sound.
   std::string name;

   /// @brief If the child should be munged/cleaned.
   bool active = true;

   /// @brief If the child produces localized sound lvls.
   bool localized = false;

   bool operator==(const project_child_sound_world&) const noexcept = default;
};

/// @brief Represents a localization for sound files.
struct project_sound_localization {
   /// @brief Language of the localization. Will cause files with the
   // extensions .stm_{language} and .st4_{language} to be munged.
   std::string language;

   /// @brief Output directory for localized sound lvls.
   std::string output_directory;

   bool operator==(const project_sound_localization&) const noexcept = default;
};

struct project {
   /// @brief Root directory of the project i.e. D:\BF2_ModTools\data_TST
   io::path directory;

   bool deploy = true;

   bool addme_active = true;
   bool common_active = true;
   bool load_active = false;
   bool shell_active = false;
   bool sound_active = false;
   bool sound_common_bank = false;

   std::vector<project_child> sides;
   std::vector<project_child> worlds;

   std::vector<project_child_sound_shared> sound_shared;
   std::vector<project_child_sound_world> sound_worlds;
   std::vector<project_sound_localization> sound_localizations;

   project_config config;

   bool operator==(const project&) const noexcept = default;
};

void save_project(const project& project, const io::path& path) noexcept;

auto load_project(const io::path& path) noexcept -> project;

void merge_loaded_project(project& current_project, const project& loaded_project) noexcept;

auto to_ui_string(const project_platform platform) noexcept -> const char*;

}