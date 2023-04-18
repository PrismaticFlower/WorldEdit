#include "os_execute.hpp"

#include <Windows.h>

namespace we::utility {

auto expand_environment_strings(const std::string& string) noexcept -> std::string
{
   const DWORD expanded_size = ExpandEnvironmentStringsA(string.c_str(), nullptr, 0);

   if (expanded_size == 0) return "";

   std::string expanded_string;
   expanded_string.resize(expanded_size - 1);

   if (ExpandEnvironmentStringsA(string.c_str(), expanded_string.data(),
                                 expanded_size) == 0) {
      return "";
   }

   return expanded_string;
}

bool os_execute_async(const std::string& command_line) noexcept
{
   std::string expanded_command_line = expand_environment_strings(command_line);

   STARTUPINFOA startup_info{.cb = sizeof(STARTUPINFOA)};
   PROCESS_INFORMATION process_info{};

   if (CreateProcessA(nullptr, expanded_command_line.data(), nullptr, nullptr, false,
                      NORMAL_PRIORITY_CLASS | DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP,
                      nullptr, nullptr, &startup_info, &process_info)) {
      CloseHandle(process_info.hProcess);
      CloseHandle(process_info.hThread);

      return true;
   }

   return false;
}

}