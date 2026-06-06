#pragma once

#include "feedback.hpp"
#include "shared.hpp"

#include "async/thread_pool.hpp"

#include "io/path.hpp"

#include "os/process.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace we::munge {

struct tool_context {
   io::path toolsfl_bin_path;
   io::path project_path;
   io::path source_path;
   io::path output_path;
   io::path lvl_output_path;
   std::string_view platform;
   std::vector<io::path> common_files = {};
   std::vector<std::string> sound_languages;
   munge_feedback& feedback;
   async::thread_pool& thread_pool;

   bool use_builtin_model_munge = true;
   bool use_builtin_odf_munge = true;
   bool use_builtin_texture_munge = true;

   texture_quality texture_quality = texture_quality::default_;

   os::process_priority munge_process_priority = os::process_priority::below_normal;
};

}