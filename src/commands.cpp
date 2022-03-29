
#include "commands.hpp"
#include "utility/string_ops.hpp"

#include <fmt/format.h>

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
   const auto [command, arg_str] =
      utility::string::split_first_of_exclusive(str, " "sv);

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

commands_key_binder::commands_key_binder(commands& commands,
                                         key_input_manager& key_input_manager,
                                         output_stream& error_output_stream) noexcept
   : _commands{commands}, _key_input_manager{key_input_manager}, _error_output_stream{error_output_stream}
{
}

void commands_key_binder::set_default_bindings(std::initializer_list<command_bind_args> default_bindings)
{
   _bindings.reserve(default_bindings.size());

   for (auto& [command, binding, bind_config] : default_bindings) {
      if (not _bindings.contains(binding)) bind(command, binding, bind_config);
   }
}

void commands_key_binder::bind(std::string command, const bind_key binding,
                               const bind_config bind_config)
{
   auto [command_name, command_args] =
      utility::string::split_first_of_exclusive(command, " "sv);

   if (not _commands.has_command(command_name)) {
      throw unknown_command{fmt::format("Unknown command '{}'!", command_name)};
   }

   _key_input_manager.bind(binding, bind_config, [command, this] {
      try {
         _commands.execute(command);
      }
      catch (invalid_command_argument& e) {
         _error_output_stream.write(
            fmt::format("Failed to execute command '{}'\n   Error: {}\n",
                        command, e.what()));
      }
   });

   _bindings.insert_or_assign(binding, std::move(command));
}

void commands_key_binder::unbind(const bind_key binding) noexcept
{
   _key_input_manager.unbind(binding);
   _bindings.erase(binding);
}

}