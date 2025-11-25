#include "io/path.hpp"

#include <string>
#include <vector>

namespace we::munge {

struct project_config {
   io::path toolsfl_bin_path;

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

}