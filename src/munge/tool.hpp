#pragma once

#include "io/path.hpp"

#include <vector>

namespace we::munge {

enum class tool_type {
   animation_munge,

   bin_ps2_munge,

   // Munges world boundaries.
   //
   // input: *.bnd
   // output: *.boundary
   // chunk: bnd_
   // hashed strings: true
   config_boundary_munge,

   // Munges melee combo configs.
   //
   // input: *.combo
   // output: *.config
   // chunk: comb
   config_combo_munge,

   // Munges particle effects and some obsecure files (lights in config format, decals, etc)
   //
   // input: *.fx
   // output: *.config
   // chunk: fx__
   config_effects_munge,

   // Munges world enviroment effects that frustratingly use the same extension as regular effects.
   //
   // input: *.fx
   // output: *.envfx
   // chunk: fx__
   config_env_effects_munge,

   // Munges player HUD configs.
   //
   // input: *.hud
   // output: *.config
   // chunk: hud_
   config_hud_munge,

   // Munges world lights.
   //
   // input: *.lgt
   // output: *.light
   // chunk: lght
   config_light_munge,

   // Munges load screen configs.
   //
   // input: *.cfg
   // output: *.config
   // chunk: load
   config_load_munge,

   // Munges movie configs.
   //
   // input: *.mcfg
   // output: *.config
   // chunk: mcfg
   // hashed strings: true
   config_movie_munge,

   // Munges world path configs.
   //
   // input: *.pth
   // output: *.path
   // chunk: path
   config_path_munge,

   // Munges world foliage configs.
   //
   // input: *.prp
   // output: *.prop
   // chunk: prp
   // hashed strings: true
   config_prop_munge,

   // Munges world portals and sectors.
   //
   // input: *.pvs
   // output: *.povs
   // chunk: PORT
   config_pvs_munge,

   // Munges world sky config.
   //
   // input: *.sky
   // output: *.config
   // chunk: sky_
   config_sky_munge,

   // Munges solider animation configs.
   //
   // input: *.sanm
   // output: *.config
   // chunk: sanm
   config_soldier_animation_munge,

   // Munges sound configs.
   //
   // input: *.snd *.mus *.ffx *.tsr
   // output: *.config
   // chunk: snd_ mus_ ffx_ tsr_
   // hashed strings: true
   config_sound_munge,

   // Munges sound configs.
   //
   // input: *.snd *.mus
   // output: *.config
   // chunk: snd_ mus_
   // hashed strings: true
   config_sound_common_munge,

   // Munges sound configs from the Sounds/ directory of the input directory.
   //
   // input: *.snd *.mus *.tsr
   // output: *.config
   // chunk: snd_ mus_ tsr_
   // hashed strings: true
   config_sound_world_munge,

   font_munge,
   level_pack,
   load_pack,
   localize_munge,
   model_munge,
   movie_munge,
   odf_munge,
   path_munge,
   path_planning_munge,
   script_munge,
   shader_munge,
   sound_directory_munge,
   sound_munge,
   terrain_munge,
   texture_munge,
   world_munge,

   copy_premunged,
};

struct config_effects_munge_inputs {
   // Only munge .fx files in the Effects/ directory of the input directory.
   bool effects_directory_only = false;
};

struct level_pack_inputs {
   // Output packed .lvl files back to the input directory.
   bool child_levels = false;

   // Process .mrq files instead of .req files.
   bool mrq_input = false;

   // Only write the files manifest and do not pack the .lvl
   bool only_files = false;

   // If not empty only process this file. Relative to source directory.
   io::path input_file;

   // If not empty write packed files manifest to this file. Relative to output directory.
   io::path write_files;

   // Source directory for .req files, relative to source directory for the munge.
   io::path source_directory;

   // Extra input file directories, without the platform folder. Relative to project directory.
   std::vector<io::path> extra_input_directories;

   // Extra input file manifests for common files. Relative to output directory.
   std::vector<io::path> extra_common_files;
};

struct world_munge_inputs {
   /// @brief Munge .lyr files instead of .wld files.
   bool layers = false;
};

struct sound_munge_inputs {
   /// @brief Munge .stm files instead of .sfx files. Also pass -stream to soundflmunge.
   bool stream = false;

   /// @brief Munge .asfx files instead of .sfx files.
   bool additional_bank = false;

   /// @brief Munge from the Sound/ directory (relative to source directory for the munge) only.
   bool sound_child_directory = false;
};

struct script_munge_inputs {
   // Only munge .lua files in the Scripts/ directory of the input directory.
   bool scripts_directory_only = false;
};

struct font_munge_inputs {
   // Only munge .fff files in the Fonts/ directory of the input directory.
   bool fonts_directory_only = false;
};

struct tool {
   tool_type type;

   config_effects_munge_inputs config_effects_munge;
   level_pack_inputs level_pack;
   world_munge_inputs world_munge;
   sound_munge_inputs sound_munge;
   script_munge_inputs script_munge;
   font_munge_inputs font_munge;
};

}