#pragma once

#include <string>

namespace we::settings {

auto detect_default_text_editor() -> std::string;

struct preferences {
   static std::string default_text_editor;

   std::string text_editor = default_text_editor;
};

}