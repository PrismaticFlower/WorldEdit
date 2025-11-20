#pragma once

#include "message.hpp"
#include "output.hpp"

#include "io/path.hpp"

#include <string>
#include <vector>

namespace we::munge {

struct munge_feedback {
   munge_feedback(output& standard_output, output& standard_error);

   void print_output(std::string output);

   void print_errors(std::string output);

   void parse_error_line(const std::string_view error_output,
                         const io::path& source_directory,
                         std::vector<message>& messages_out) noexcept;

   auto parse_sound_munge_error_line(const std::string_view error_output) noexcept
      -> message&;

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

   std::vector<message> warnings;
   std::vector<message> errors;

private:
   output& _standard_output;
   output& _standard_error;
};

}