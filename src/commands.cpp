
#include "commands.hpp"
#include "utility/string_ops.hpp"

#include <fmt/core.h>

using namespace std::literals;

namespace we {

void commands::add(std::string name, bool& value) noexcept
{
   _commands[name] = make_command_ptr([&value](const std::string_view arg_str) {
      if (arg_str.starts_with("0")) {
         value = false;
      }
      else if (arg_str.starts_with("1")) {
         value = true;
      }
      else {
         value = not value;
      }
   });
}

void commands::add(std::string name, std::string& value) noexcept
{
   _commands[name] = make_command_ptr([&value](const std::string_view arg_str) {
      value = std::string{arg_str};
   });
}

void commands::execute(const std::string_view str)
{
   const auto [command, arg_str] = string::split_first_of_exclusive(str, " "sv);

   if (_commands.contains(command)) {
      _commands[command]->invoke(arg_str);
   }
   else {
      throw_unknown_command(command);
   }
}

bool commands::has_command(const std::string_view name) const noexcept
{
   return _commands.contains(name);
}

[[noreturn]] void commands::throw_unknown_command(const std::string_view command)
{
   throw unknown_command{fmt::format("Uknown command '{}'!", command)};
}

[[noreturn]] void commands::throw_invalid_argument(const std::string_view arg_str)
{
   throw invalid_command_argument{fmt::format("Invalid command argument '{}'!", arg_str)};
}

[[noreturn]] void commands::throw_result_out_of_range(const std::string_view arg_str)
{
   throw invalid_command_argument{
      fmt::format("Command argument is out of range for the value '{}'!", arg_str)};
}

}