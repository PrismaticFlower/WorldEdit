#include "feedback.hpp"

#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

namespace we::munge {

munge_feedback::munge_feedback(output& standard_output, output& standard_error)
   : _standard_output{standard_output}, _standard_error{standard_error}
{
}

void munge_feedback::print_output(std::string output)
{
   _standard_output.write(std::move(output));
}

void munge_feedback::print_errors(std::string output)
{
   _standard_error.write(std::move(output));
}

void munge_feedback::parse_error_line(const std::string_view error_output,
                                      const io::path& source_directory,
                                      std::vector<message>& messages_out) noexcept
{
   const auto [tool_file, message] =
      string::split_first_of_exclusive(error_output, ":");
   auto [tool, file] = string::split_first_of_exclusive(tool_file, " ");

   tool.remove_prefix(1); // Remove [
   file.remove_suffix(1); // Remove ]

   messages_out.push_back({
      .file = io::compose_path(source_directory, file),
      .tool = std::string{tool},
      .message = std::string{message},
   });
}

auto munge_feedback::parse_sound_munge_error_line(const std::string_view error_output) noexcept
   -> message&
{
   const auto [tool, category_message] =
      string::split_first_of_exclusive(error_output, " : ");
   const auto [category, message] =
      string::split_first_of_exclusive(category_message, " : ");
   const std::string_view file =
      string::split_first_of_exclusive(message, "while munging ")[1];

   std::vector<munge::message>& messages = category == "Error" ? errors : warnings;

   messages.push_back({
      .file = file,
      .tool = std::string{tool == "soundflmunge.exe" ? "SoundFLMunge" : tool},
      .message = std::string{message},
   });

   return messages.back();
}

void munge_feedback::parse_error_string(const std::string_view error_output,
                                        const io::path& source_directory) noexcept
{
   if (error_output.empty()) return;

   using namespace std::literals;

   message* last_message = nullptr;

   for (const auto [_, line] : string::lines_iterator{error_output}) {
      if (line.empty()) continue;

      if (line.starts_with("WARNING")) {
         parse_error_line(line.substr("WARNING"sv.size()), source_directory, warnings);

         last_message = &warnings.back();
      }
      else if (line.starts_with("ERROR")) {
         parse_error_line(line.substr("ERROR"sv.size()), source_directory, errors);

         last_message = &errors.back();
      }
      else if (line.starts_with("   ") and line.ends_with("Warnings")) {
         break;
      }
      else if (last_message) {
         last_message->message += '\n';
         last_message->message += line;
      }
   }
}

void munge_feedback::parse_script_munge_error_string(const std::string_view error_output,
                                                     const io::path& source_directory) noexcept
{
   if (error_output.empty()) return;

   using namespace std::literals;

   message* last_message = nullptr;

   for (const auto [_, line] : string::lines_iterator{error_output}) {
      if (line.empty()) continue;

      if (line.starts_with("WARNING")) {
         parse_error_line(line.substr("WARNING"sv.size()), source_directory, warnings);

         last_message = &warnings.back();
      }
      else if (line.starts_with("ERROR")) {
         parse_error_line(line.substr("ERROR"sv.size()), source_directory, errors);

         last_message = &errors.back();
      }
      else if (line.contains("luac.exe: ")) {
         const std::string_view message =
            string::split_first_of_exclusive(line, "luac.exe: ")[1];
         std::string_view file =
            string::split_first_of_inclusive(message, ".lua:")[0];

         file.remove_suffix(1);

         errors.push_back({
            .file = file,
            .tool = "ScriptMunge",
            .message = std::string{message},
         });

         last_message = &errors.back();
      }
      else if (line.starts_with("   ") and line.ends_with("Warnings")) {
         break;
      }
      else if (last_message) {
         last_message->message += '\n';
         last_message->message += line;
      }
   }
}

void munge_feedback::parse_sound_munge_error_string(const std::string_view error_output) noexcept
{
   if (error_output.empty()) return;

   using namespace std::literals;

   message* last_message = nullptr;

   for (const auto [_, line] : string::lines_iterator{error_output}) {
      if (line.empty()) continue;

      if (string::istarts_with(line, "soundflmunge.exe")) {
         last_message = &parse_sound_munge_error_line(line);
      }
      else if (last_message) {
         last_message->message += '\n';
         last_message->message += line;
      }
   }
}

void munge_feedback::parse_movie_munge_error_string(const std::string_view error_output,
                                                    const io::path& file) noexcept
{
   if (error_output.empty()) return;

   using namespace std::literals;

   message* last_message = nullptr;

   for (const auto [_, line] : string::lines_iterator{error_output}) {
      if (line.empty()) continue;

      if (line.starts_with("Warning : ")) {
         warnings.push_back(
            {.file = file,
             .tool = "MovieMunge",
             .message = std::string{line.substr("Warning : "sv.size())}});

         last_message = &warnings.back();
      }
      else if (line.starts_with("Error : ")) {
         errors.push_back({.file = file,
                           .tool = "MovieMunge",
                           .message = std::string{line.substr("Error : "sv.size())}});

         last_message = &errors.back();
      }
      else if (last_message) {
         last_message->message += '\n';
         last_message->message += line;
      }
   }
}

void munge_feedback::parse_shader_munge_error_string(const std::string_view error_output,
                                                     const io::path& source_directory) noexcept
{
   if (error_output.empty()) return;

   using namespace std::literals;

   message* last_message = nullptr;

   for (const auto [_, line] : string::lines_iterator{error_output}) {
      if (line.empty()) continue;

      if (line.starts_with("WARNING")) {
         parse_error_line(line.substr("WARNING"sv.size()), source_directory, warnings);

         last_message = &warnings.back();
      }
      else if (line.starts_with("ERROR")) {
         parse_error_line(line.substr("ERROR"sv.size()), source_directory, errors);

         last_message = &errors.back();
      }
      else if (line.contains(" : ")) {
         auto [file, message] = string::split_first_of_exclusive(line, " : ");

         file = file.substr(0, file.find_last_of('('));

         std::vector<munge::message>& messages =
            string::istarts_with(message, "error") ? errors : warnings;

         messages.push_back(
            {.file = file, .tool = "ShaderMunge", .message = std::string{line}});

         last_message = &messages.back();
      }
      else if (line.starts_with("   ") and line.ends_with("Warnings")) {
         continue;
      }
      else if (last_message) {
         last_message->message += '\n';
         last_message->message += line;
      }
   }
}

void munge_feedback::add_warning(message message) noexcept
{
   warnings.push_back(std::move(message));
}

void munge_feedback::add_error(message message) noexcept
{
   errors.push_back(std::move(message));
}

}