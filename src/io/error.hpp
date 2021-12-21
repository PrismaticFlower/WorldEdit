#pragma once

#include <stdexcept>

namespace we::io {

/// @brief Base class for IO errors.
class error : public std::runtime_error {
protected:
   using std::runtime_error::runtime_error;
};

/// @brief Indicates an error opening a file.
class open_error : public error {
public:
   using error::error;
};

/// @brief Indicates an error reading from a file.
class read_error : public error {
public:
   using error::error;
};

}
