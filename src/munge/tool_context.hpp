#pragma once

#include "feedback.hpp"

#include "io/path.hpp"

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
};

}