#pragma once

#include <stdexcept>

namespace we::world {

/// @brief Exception thrown when loading a world fails unexpectedly.
class load_failure : public std::runtime_error {
   using std::runtime_error::runtime_error;
};

}