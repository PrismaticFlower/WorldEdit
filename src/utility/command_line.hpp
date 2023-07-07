#pragma once

#include <charconv>
#include <concepts>
#include <string_view>

namespace we::utility {

/// @brief Simple class for handling command line arguments.
class command_line {
public:
   command_line(int arg_count, const char* const* args)
      : arg_count{arg_count}, args{args}
   {
   }

   /// @brief Gets a command line argument.
   /// @param name The name of the argument.
   /// @param fallback The fallback value if the argument does not exist.
   /// @return The value of the argument or the fallback.
   auto get_or(std::string_view name, std::integral auto fallback) const noexcept
   {
      return get_or_impl(name, fallback);
   }

   /// @brief Gets a command line argument.
   /// @param name The name of the argument.
   /// @param fallback The fallback value if the argument does not exist.
   /// @return The value of the argument or the fallback.
   auto get_or(std::string_view name, std::floating_point auto fallback) const noexcept
   {
      return get_or_impl(name, fallback);
   }

   /// @brief Gets a command line argument.
   /// @param name The name of the argument.
   /// @param fallback The fallback value if the argument does not exist.
   /// @return The value of the argument or the fallback.
   auto get_or(std::string_view name, std::string_view fallback) const noexcept
   {
      return get_or_impl(name, fallback);
   }

   /// @brief Gets a command line flag.
   /// @param name The name of the flag.
   /// @return True if the flag was passed, false if it wasn't.
   bool get_flag(std::string_view name) const noexcept
   {
      // skip the program name
      for (int i = 1; i < arg_count; ++i) {
         if (name == std::string_view{args[i]}) {
            return true;
         }
      }

      return false;
   }

private:
   auto get_or_impl(std::string_view name, auto fallback) const noexcept
      -> decltype(fallback)
   {
      // skip the program name
      for (int i = 1; i < arg_count; ++i) {
         if (name == std::string_view{args[i]}) {
            if (i + 1 < arg_count) {
               return parse_value(args[i + 1], fallback);
            }
            else {
               return fallback;
            }
         }
      }

      return fallback;
   }

   auto parse_value(std::string_view str_value, auto fallback) const noexcept
      -> decltype(fallback)
   {
      decltype(fallback) value = 0;

      if (std::from_chars(&str_value[0], &str_value[0] + str_value.size(), value)
             .ec != std::errc{}) {
         return fallback;
      }

      return value;
   }

   auto parse_value(std::string_view str_value,
                    [[maybe_unused]] std::string_view fallback) const noexcept
      -> std::string_view
   {
      return str_value.starts_with("-") ? fallback : str_value;
   }

   int arg_count = 0;
   const char* const* args = nullptr;
};

}