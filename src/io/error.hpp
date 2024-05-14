#pragma once

#include <stdexcept>

namespace we::io {

/// @brief IO open error codes.
enum class open_error_code { generic, sharing_violation };

/// @brief Base class for IO errors.
class error : public std::runtime_error {
protected:
   using std::runtime_error::runtime_error;
};

/// @brief Indicates an error opening a file.
class open_error : public error {
public:
   /// @brief Construct an open_error from a message and error code.
   /// @param str The message.
   /// @param code The error code.
   open_error(std::string str, open_error_code code) noexcept
      : error{std::move(str)}, _code{code} {};

   /// @brief Gets the error code for the open error.
   /// @return The error code.
   auto code() const noexcept -> open_error_code
   {
      return _code;
   }

   using error::error;

private:
   open_error_code _code = open_error_code::generic;
};

/// @brief Indicates an error reading from a file.
class read_error : public error {
public:
   using error::error;
};

}
