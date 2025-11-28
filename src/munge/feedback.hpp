#pragma once

#include "message.hpp"
#include "output.hpp"

#include "utility/implementation_storage.hpp"

#include "io/path.hpp"

#include <string>
#include <vector>

namespace we::munge {

struct munge_feedback {
   munge_feedback(output& standard_output, output& standard_error);

   munge_feedback(const munge_feedback&) = delete;

   auto operator=(const munge_feedback&) -> munge_feedback& = delete;

   ~munge_feedback();

   void print_output(std::string output);

   void print_errors(std::string output);

   /// @brief Parse a munge tool error string.
   /// @param error_output The standard error output from the munge tool
   /// @param source_directory The directory to prepend to files in error messages.
   void parse_error_string(const std::string_view error_output,
                           const io::path& source_directory) noexcept;

   /// @brief Parse a script munge error string.
   /// @param error_output The standard error output from the munge tool
   /// @param source_directory The directory to prepend to files in error messages.
   void parse_script_munge_error_string(const std::string_view error_output,
                                        const io::path& source_directory) noexcept;

   /// @brief Parse a SoundFLMunge error string.
   /// @param error_output The standard error output from the munge tool
   /// @param source_directory The directory to prepend to files in error messages.
   void parse_sound_munge_error_string(const std::string_view error_output) noexcept;

   /// @brief Parse a MovieMunge error string.
   /// @param error_output The standard error output from the munge tool
   /// @param file The file to use in error messages.
   void parse_movie_munge_error_string(const std::string_view error_output,
                                       const io::path& file) noexcept;

   /// @brief Parse a MovieMunge error string.
   /// @param error_output The standard error output from the munge tool
   /// @param file The file to use in error messages.
   void parse_shader_munge_error_string(const std::string_view error_output,
                                        const io::path& source_directory) noexcept;

   void add_warning(message message) noexcept;

   void add_error(message message) noexcept;

   auto take_warnings() noexcept -> std::vector<message>;

   auto take_errors() noexcept -> std::vector<message>;

private:
   struct impl;

   implementation_storage<impl, 88> impl;
};

}