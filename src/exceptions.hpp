#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace we {

struct file_load_failure : std::runtime_error {
   file_load_failure(const char* what, std::string filepath, std::string description)
      : std::runtime_error{what}, filepath{std::move(filepath)}, description{std::move(description)}
   {
   }

   std::string filepath;
   std::string description;
};

}
