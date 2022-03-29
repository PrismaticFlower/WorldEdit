#pragma once

#include "key_input_manager.hpp"
#include "output_stream.hpp"

#include <charconv>
#include <concepts>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include <absl/container/flat_hash_map.h>

namespace we {

struct unknown_command : std::runtime_error {
   using std::runtime_error::runtime_error;
};

struct invalid_command_argument : std::runtime_error {
   using std::runtime_error::runtime_error;
};

class commands {
public:
   /// @brief Add an integral value as a command.
   /// @param name Name of the command.
   /// @param value A reference to the value. The value should outlive the commands class.
   void add(std::string name, std::integral auto& value) noexcept
   {
      _commands[name] = make_command_ptr([&value](const std::string_view arg_str) {
         if (const std::from_chars_result result =
                std::from_chars(arg_str.data(), arg_str.data() + arg_str.size(), value);
             result.ec == std::errc{}) {
            return;
         }
         else if (result.ec == std::errc::invalid_argument) {
            throw_invalid_argument(arg_str);
         }
         else if (result.ec == std::errc::result_out_of_range) {
            throw_result_out_of_range(arg_str);
         }
      });
   }

   /// @brief Add a floating point value as a command.
   /// @param name Name of the command.
   /// @param value A reference to the value. The value should outlive the commands class.
   void add(std::string name, std::floating_point auto& value) noexcept
   {
      _commands[name] = make_command_ptr([&value](const std::string_view arg_str) {
         if (const std::from_chars_result result =
                std::from_chars(arg_str.data(), arg_str.data() + arg_str.size(), value);
             result.ec == std::errc{}) {
            return;
         }
         else if (result.ec == std::errc::invalid_argument) {
            throw_invalid_argument(arg_str);
         }
         else if (result.ec == std::errc::result_out_of_range) {
            throw_result_out_of_range(arg_str);
         }
      });
   }

   /// @brief Add a boolean value as a command.
   /// @param name Name of the command.
   /// @param value A reference to the value. The value should outlive the commands class.
   void add(std::string name, bool& value) noexcept;

   /// @brief Add a string value as a command.
   /// @param name Name of the command.
   /// @param value A reference to the value. The value should outlive the commands class.
   void add(std::string name, std::string& value) noexcept;

   /// @brief Add a command taking a single integral argument.
   /// @param name The name of the command.
   /// @param callback The callback to invoke for the command.
   template<std::integral T>
   void add(std::string name, std::invocable<T> auto callback) noexcept
   {
      _commands[name] = make_command_ptr([callback = std::move(callback)](
                                            const std::string_view arg_str) {
         T value{};

         if (const std::from_chars_result result =
                std::from_chars(arg_str.data(), arg_str.data() + arg_str.size(), value);
             result.ec == std::errc{}) {
            std::invoke(callback, value);
         }
         else if (result.ec == std::errc::invalid_argument) {
            throw_invalid_argument(arg_str);
         }
         else if (result.ec == std::errc::result_out_of_range) {
            throw_result_out_of_range(arg_str);
         }
      });
   }

   /// @brief Add a command taking a single floating point argument.
   /// @param name The name of the command.
   /// @param callback The callback to invoke for the command.
   template<std::floating_point T>
   void add(std::string name, std::invocable<T> auto callback) noexcept
   {
      _commands[name] = make_command_ptr([callback = std::move(callback)](
                                            const std::string_view arg_str) {
         T value{};

         if (const std::from_chars_result result =
                std::from_chars(arg_str.data(), arg_str.data() + arg_str.size(), value);
             result.ec == std::errc{}) {
            std::invoke(callback, value);
         }
         else if (result.ec == std::errc::invalid_argument) {
            throw_invalid_argument(arg_str);
         }
         else if (result.ec == std::errc::result_out_of_range) {
            throw_result_out_of_range(arg_str);
         }
      });
   }

   /// @brief Add a command taking a single boolean argument.
   /// @param name The name of the command.
   /// @param callback The callback to invoke for the command.
   void add(std::string name, std::invocable<bool> auto callback) noexcept
   {
      _commands[name] = make_command_ptr(
         [callback = std::move(callback)](const std::string_view arg_str) {
            if (arg_str.starts_with("0")) {
               std::invoke(callback, false);
            }
            else if (arg_str.starts_with("1")) {
               std::invoke(callback, true);
            }
            else {
               throw_invalid_argument(arg_str);
            }
         });
   }

   /// @brief Add a command taking a single string (as a string_view) argument.
   /// @param name The name of the command.
   /// @param callback The callback to invoke for the command.
   void add(std::string name, std::invocable<std::string_view> auto callback) noexcept
   {
      _commands[name] = make_command_ptr(
         [callback = std::move(callback)](const std::string_view arg_str) {
            std::invoke(callback, arg_str);
         });
   }

   /// @brief Add a command that takes no arguments.
   /// @param name The name of the command.
   /// @param callback The callback to invoke for the command.
   void add(std::string name, std::invocable auto callback) noexcept
   {
      _commands[name] =
         make_command_ptr([callback = std::move(callback)](
                             [[maybe_unused]] const std::string_view arg_str) {
            std::invoke(callback);
         });
   }

   /// @brief Execute a command string.
   /// @param str The string to execute.
   void execute(const std::string_view str);

   /// @brief Tests if a command exists.
   /// @param name The name of the command to test for.
   /// @return If the command exists or not.
   [[nodiscard]] bool has_command(const std::string_view name) const noexcept;

private:
   struct command_base {
   public:
      virtual ~command_base() = default;

      virtual void invoke(const std::string_view arg_str) = 0;
   };

   template<std::invocable<std::string_view> T>
   struct command : command_base {
   public:
      command(T callback) : callback{std::move(callback)} {}

      void invoke(const std::string_view arg_str)
      {
         std::invoke(callback, arg_str);
      }

      T callback;
   };

   template<std::invocable<std::string_view> T>
   static auto make_command_ptr(T&& callback) noexcept
      -> std::unique_ptr<command<T>>
   {
      return std::make_unique<command<T>>(std::forward<T>(callback));
   }

   [[noreturn]] static void throw_unknown_command(const std::string_view command);

   [[noreturn]] static void throw_invalid_argument(const std::string_view arg_str);

   [[noreturn]] static void throw_result_out_of_range(const std::string_view arg_str);

   absl::flat_hash_map<std::string, std::unique_ptr<command_base>> _commands;
};

struct command_bind_args {
   std::string command;
   bind_key binding;
   bind_config bind_config;
};

class commands_key_binder {
public:
   commands_key_binder(commands& commands, key_input_manager& key_input_manager,
                       output_stream& error_output_stream) noexcept;

   void set_default_bindings(std::initializer_list<command_bind_args> default_bindings);

   void bind(std::string command, const bind_key binding, const bind_config bind_config);

   void unbind(const bind_key binding) noexcept;

private:
   absl::flat_hash_map<bind_key, std::string> _bindings;
   commands& _commands;
   key_input_manager& _key_input_manager;
   output_stream& _error_output_stream;
};

}
