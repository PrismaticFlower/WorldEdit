#pragma once

#include "../tool_context.hpp"

#include <string_view>
#include <vector>

namespace we::assets {
struct option;
}

namespace we::munge {

struct execute_builtin_context {
   /// @brief Input extension (without the ".").
   std::string_view input_extension;

   /// @brief Output extension (without the ".").
   std::string_view output_extension;

   /// @brief Tool name to use in error messages.
   std::string_view tool_name;

   /// @brief Function to call to munge a file.
   void (&execute_munge)(const io::path& input_file_path,
                         const std::vector<assets::option>& directory_options,
                         const tool_context& context);
};

void execute_builtin_munge(const execute_builtin_context& context,
                           const tool_context& tool_context) noexcept;

}
