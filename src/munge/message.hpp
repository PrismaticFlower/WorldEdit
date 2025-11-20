#pragma once

#include "io/path.hpp"

#include <string>

namespace we::munge {

struct message {
   io::path file;
   std::string tool;
   std::string message;
};

}