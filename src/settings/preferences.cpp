#include "preferences.hpp"
#include "utility/os_execute.hpp"

#include <array>
#include <filesystem>

using namespace std::literals;

namespace we::settings {

std::string preferences::default_text_editor = detect_default_text_editor();

auto detect_default_text_editor() -> std::string
{
   const std::array known_common_editors = {R"(%ProgramFiles%\Sublime Text\sublime_text.exe)"s,
                                            R"(%LocalAppData%\Programs\Microsoft VS Code\code.exe)"s,
                                            R"(%ProgramFiles%\Notepad++\Notepad++.exe)"s};

   for (const auto& known : known_common_editors) {
      try {
         if (std::filesystem::exists(utility::expand_environment_strings(known))) {
            return known;
         }
      }
      catch (std::filesystem::filesystem_error&) {
      }
   }

   return R"(%WinDir%\notepad.exe)"s;
}

}