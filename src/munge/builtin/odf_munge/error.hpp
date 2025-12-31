#pragma once

#include <stdexcept>

namespace we::munge {

enum class odf_ec {
   odf_load_io_error,
   odf_load_parse_error,

   req_write_io_open_error,
   req_write_io_generic_error,

   write_io_open_error,
   write_io_generic_error,
};

/// @brief Indicates an error munging a odf.
class odf_error : public std::runtime_error {
public:
   /// @brief Construct an texture_error from a message and error code.
   /// @param message The message.
   /// @param code The error code.
   odf_error(const std::string& message, odf_ec ec) noexcept
      : std::runtime_error{message}, _code{ec} {};

   /// @brief Construct an texture_error from a message and error code.
   /// @param message The message.
   /// @param code The error code.
   odf_error(const char* message, odf_ec ec) noexcept
      : std::runtime_error{message}, _code{ec} {};

   /// @brief Gets the error code for the error.
   /// @return The error code.
   auto code() const noexcept -> odf_ec
   {
      return _code;
   }

private:
   odf_ec _code;
};

auto get_descriptive_message(const odf_error& e) noexcept -> std::string;

}