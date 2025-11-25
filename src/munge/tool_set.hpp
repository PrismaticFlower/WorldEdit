#pragma once

#include "tool.hpp"

#include <vector>

namespace we::munge {

struct tool_set {
   // Common file manifests to pass to side and world munge. Relative to the common output directory.
   std::vector<io::path> common_files = {
      "core.files",
      "common.files",
      "ingame.files",
   };

   std::vector<tool> addme = {
      {.type = tool_type::script_munge},
   };

   std::vector<tool> common = {
      {.type = tool_type::copy_premunged},
      {.type = tool_type::odf_munge},
      {.type = tool_type::config_effects_munge,
       .config_effects_munge = {.effects_directory_only = false}},
      {.type = tool_type::config_combo_munge},
      {.type = tool_type::script_munge},
      {.type = tool_type::config_movie_munge},
      {.type = tool_type::config_soldier_animation_munge},
      {.type = tool_type::config_hud_munge},
      {.type = tool_type::font_munge},
      {.type = tool_type::texture_munge},
      {.type = tool_type::model_munge}, // Should this be restricted to Effects and MSH folders?
      {.type = tool_type::shader_munge},
      {.type = tool_type::config_sound_common_munge},
      {.type = tool_type::sound_munge, .sound_munge = {.sound_child_directory = true}},
      {.type = tool_type::sound_munge,
       .sound_munge = {.stream = true, .sound_child_directory = true}},
      {.type = tool_type::localize_munge},
   };

   std::vector<tool> common_pack = {
      {.type = tool_type::level_pack,
       .level_pack =
          {
             .input_file = "core.req",
             .write_files = "core.files",
          }},
      {.type = tool_type::level_pack,
       .level_pack =
          {
             .input_file = "common.req",
             .write_files = "common.files",
          }},
      {.type = tool_type::level_pack,
       .level_pack =
          {
             .input_file = "ingame.req",
             .write_files = "ingame.files",
          }},
      {.type = tool_type::level_pack,
       .level_pack =
          {
             .input_file = "inshell.req",
             .write_files = "inshell.files",
          }},
      {.type = tool_type::level_pack, .level_pack = {.input_file = "MISSION.req"}},
   };

   std::vector<tool> common_mission_child_pack = {
      {.type = tool_type::level_pack,
       .level_pack = {.child_levels = true, .source_directory = "MISSION"}},
   };

   std::vector<tool> common_mission_pack = {
      {.type = tool_type::level_pack, .level_pack = {.input_file = "MISSION.req"}},
   };

   std::vector<tool> common_fpm_pack = {
      {.type = tool_type::level_pack, .level_pack = {.source_directory = R"(REQ\FPM)"}},
   };

   std::vector<tool> load = {
      {.type = tool_type::config_load_munge},
      {.type = tool_type::texture_munge},
      {.type = tool_type::model_munge},
   };

   std::vector<tool> load_pack = {
      {.type = tool_type::load_pack},
      {.type = tool_type::level_pack, .level_pack = {.input_file = R"(req\load.req)"}},
   };

   std::vector<tool> shell = {
      {.type = tool_type::config_movie_munge},
      {.type = tool_type::movie_munge},
      {.type = tool_type::config_effects_munge,
       .config_effects_munge = {.effects_directory_only = true}},
      {.type = tool_type::script_munge, .script_munge = {.scripts_directory_only = true}},
      {.type = tool_type::font_munge, .font_munge = {.fonts_directory_only = true}},
      {.type = tool_type::texture_munge},
      {.type = tool_type::model_munge},
      {.type = tool_type::bin_ps2_munge},
   };

   std::vector<tool> shell_pack = {
      {.type = tool_type::level_pack, .level_pack = {.input_file = "shell.req"}},
   };

   std::vector<tool> shell_ps2_pack = {
      {.type = tool_type::level_pack, .level_pack = {.input_file = "shellps2.req"}},
   };

   std::vector<tool> side = {
      {.type = tool_type::copy_premunged},
      {.type = tool_type::odf_munge},
      {.type = tool_type::config_effects_munge,
       .config_effects_munge = {.effects_directory_only = true}},
      {.type = tool_type::config_combo_munge},
      {.type = tool_type::config_hud_munge},
      {.type = tool_type::model_munge},
      {.type = tool_type::texture_munge},
      {.type = tool_type::config_sound_common_munge},
   };

   std::vector<tool> side_child_pack = {
      {.type = tool_type::level_pack,
       .level_pack =
          {
             .child_levels = true,
             .source_directory = R"(REQ)",
             .extra_input_directories = {R"(_BUILD\Sides\Common\MUNGED)"},
          }},
   };

   std::vector<tool> side_pack = {
      {.type = tool_type::level_pack,
       .level_pack =
          {
             .extra_input_directories = {R"(_BUILD\Sides\Common\MUNGED)"},
          }},
   };

   std::vector<tool> side_fpm_pack = {
      {.type = tool_type::level_pack, .level_pack = {.source_directory = R"(REQ\FPM)"}},
   };

   std::vector<tool> world = {
      {.type = tool_type::copy_premunged},
      {.type = tool_type::odf_munge},
      {.type = tool_type::model_munge},
      {.type = tool_type::texture_munge},
      {.type = tool_type::terrain_munge},
      {.type = tool_type::world_munge},
      {.type = tool_type::world_munge, .world_munge = {.layers = true}},
      {.type = tool_type::we_blocks_munge},
      {.type = tool_type::path_munge},
      {.type = tool_type::path_planning_munge},
      {.type = tool_type::config_effects_munge,
       .config_effects_munge = {.effects_directory_only = true}},
      {.type = tool_type::config_combo_munge},
      {.type = tool_type::config_sky_munge},
      {.type = tool_type::config_env_effects_munge},
      {.type = tool_type::config_prop_munge},
      {.type = tool_type::config_boundary_munge},
      {.type = tool_type::config_sound_world_munge},
      {.type = tool_type::config_light_munge},
      {.type = tool_type::config_pvs_munge},
   };

   std::vector<tool> world_pack = {
      // Create manifest of assets included in the root .lvl for game modes.
      {.type = tool_type::level_pack,
       .level_pack =
          {
             .only_files = true,
             .write_files = "MZ.files",
             .extra_input_directories = {R"(_BUILD\Worlds\Common\MUNGED)"},
          }},
      // Pack game modes, using the manifest created above to skip assets included in the common game mode.
      {.type = tool_type::level_pack,
       .level_pack =
          {
             .child_levels = true,
             .mrq_input = true,
             .extra_input_directories = {R"(_BUILD\Worlds\Common\MUNGED)"},
             .extra_common_files = {"MZ.files"},
          }},
      // Pack the world.
      {.type = tool_type::level_pack,
       .level_pack =
          {
             .extra_input_directories = {R"(_BUILD\Worlds\Common\MUNGED)"},
          }},
   };
};

}