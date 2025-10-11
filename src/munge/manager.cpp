#include "manager.hpp"
#include "output.hpp"
#include "project.hpp"

#include "async/thread_pool.hpp"

#include "io/error.hpp"
#include "io/output_file.hpp"
#include "io/read_file.hpp"

#include "os/process.hpp"

#include "utility/stopwatch.hpp"
#include "utility/string_icompare.hpp"
#include "utility/string_ops.hpp"

#include <fmt/format.h>

namespace we::munge {

namespace {

constexpr os::process_priority munge_process_priority =
   os::process_priority::below_normal;

struct munge_feedback {
   munge_feedback(output& standard_output, output& standard_error)
      : _standard_output{standard_output}, _standard_error{standard_error}
   {
   }

   void print_output(std::string output)
   {
      _standard_output.write(std::move(output));
   }

   void print_errors(std::string output)
   {
      _standard_error.write(std::move(output));
   }

   void parse_error_line(const std::string_view error_output,
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

   auto parse_sound_munge_error_line(const std::string_view error_output) noexcept
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

   /// @brief Parse a munge tool error string.
   /// @param error_output The standard error output from the munge tool
   /// @param source_directory The directory to prepend to files in error messages.
   void parse_error_string(const std::string_view error_output,
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

   /// @brief Parse a script munge error string.
   /// @param error_output The standard error output from the munge tool
   /// @param source_directory The directory to prepend to files in error messages.
   void parse_script_munge_error_string(const std::string_view error_output,
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

   /// @brief Parse a SoundFLMunge error string.
   /// @param error_output The standard error output from the munge tool
   /// @param source_directory The directory to prepend to files in error messages.
   void parse_sound_munge_error_string(const std::string_view error_output) noexcept
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

   /// @brief Parse a MovieMunge error string.
   /// @param error_output The standard error output from the munge tool
   /// @param file The file to use in error messages.
   void parse_movie_munge_error_string(const std::string_view error_output,
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
            errors.push_back(
               {.file = file,
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

   /// @brief Parse a MovieMunge error string.
   /// @param error_output The standard error output from the munge tool
   /// @param file The file to use in error messages.
   void parse_shader_munge_error_string(const std::string_view error_output,
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
            auto [file, message] =
               string::split_first_of_exclusive(line, " : ");

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

   void add_warning(message message) noexcept
   {
      warnings.push_back(std::move(message));
   }

   void add_error(message message) noexcept
   {
      errors.push_back(std::move(message));
   }

   std::vector<message> warnings;
   std::vector<message> errors;

private:
   output& _standard_output;
   output& _standard_error;
};

struct munge_config {
   /// @brief Name of the executable.
   std::string_view tool_name;
   /// @brief Input extension.
   std::string_view input;
};

struct config_munge_config {
   /// @brief Input extension.
   std::string_view input;
   /// @brief Output extension.
   std::string_view output;
   /// @brief Chunk ID.
   std::string_view chunk;
   /// @brief Hash strings in output.
   bool hash_string = false;
};

struct sound_directory_munge {
   bool create_common_bank = false;
   std::span<project_sound_localization> localizations;
};

struct sound_directory_pack {
   std::string_view directory_name;
   std::span<project_sound_localization> localizations;
   std::span<io::path> input_directories;
};

struct tool_context {
   io::path toolsfl_bin_path;
   io::path project_path;
   io::path source_path;
   io::path output_path;
   io::path lvl_output_path;
   std::string_view platform;
   std::vector<io::path> common_files = {};
   std::vector<std::string> sound_languages;
   munge_feedback& feedback;
};

struct munge_context {
   std::string_view platform;

   project project;

   munge_feedback feedback;
};

void execute_munge(const munge_config& config, const tool_context& context)
{
   os::process process = os::process_create_desc{
      .executable_path =
         io::compose_path(context.toolsfl_bin_path, config.tool_name, ".exe"),
      .command_line =
         fmt::format("-inputfile $*.{} -checkdate -platform {} -sourcedir {} "
                     "-outputdir {}",
                     config.input, context.platform, context.source_path.string_view(),
                     context.output_path.string_view()),
      .working_directory = context.toolsfl_bin_path,
      .capture_stdout = true,
      .capture_stderr = true,
      .priority = munge_process_priority,
   };

   std::string standard_error = process.get_standard_error();

   context.feedback.print_output(process.get_standard_output());
   context.feedback.parse_error_string(standard_error, context.source_path);
   context.feedback.print_errors(std::move(standard_error));

   process.wait_for_exit();
}

void execute_config_munge(const config_munge_config& config, const tool_context& context)
{
   os::process process = os::process_create_desc{
      .executable_path =
         io::compose_path(context.toolsfl_bin_path, "ConfigMunge.exe"),
      .command_line =
         fmt::format("-inputfile $*.{} -checkdate -platform {} -sourcedir {} "
                     "-outputdir {} -chunkid {} -ext {}{}",
                     config.input, context.platform, context.source_path.string_view(),
                     context.output_path.string_view(), config.chunk,
                     config.output, config.hash_string ? " -hashstrings" : ""),
      .working_directory = context.toolsfl_bin_path,
      .capture_stdout = true,
      .capture_stderr = true,
      .priority = munge_process_priority,
   };

   std::string standard_error = process.get_standard_error();

   context.feedback.print_output(process.get_standard_output());
   context.feedback.parse_error_string(standard_error, context.source_path);
   context.feedback.print_errors(std::move(standard_error));

   process.wait_for_exit();
}

void execute_movie_munge(const tool_context& context)
{
   const io::path movies_out_directory =
      io::compose_path(context.lvl_output_path, "Movies");

   if (not io::exists(movies_out_directory) and
       not io::create_directory(movies_out_directory)) {
      context.feedback.add_error(
         {.file = movies_out_directory,
          .tool = "MovieMunge",
          .message = "Failed to create directory for output."});

      return;
   }

   for (const io::directory_entry& entry :
        io::directory_iterator{context.source_path, false}) {
      if (not entry.is_file) continue;
      if (not string::iequals(entry.path.extension(), ".mlst")) continue;

      const io::path output_path =
         io::compose_path(movies_out_directory, entry.path.stem(), ".mvs");

      os::process process = os::process_create_desc{
         .executable_path =
            io::compose_path(context.toolsfl_bin_path, "MovieMunge.exe"),
         .command_line =
            fmt::format("-input {} -output {} -checkdate",
                        entry.path.string_view(), output_path.string_view()),
         .working_directory = context.source_path.parent_path(),
         .capture_stdout = true,
         .capture_stderr = true,
         .priority = munge_process_priority,
      };

      std::string standard_error = process.get_standard_error();

      context.feedback.print_output(process.get_standard_output());
      context.feedback.parse_movie_munge_error_string(process.get_standard_error(),
                                                      entry.path);
      context.feedback.print_errors(std::move(standard_error));

      process.wait_for_exit();
   }
}

void execute_localize_munge(const tool_context& context)
{
   const io::path platform_source_path =
      io::compose_path(context.source_path, context.platform);
   const io::path merge_source_path =
      io::compose_path(context.output_path, "TempLocalize");

   bool munge = false;

   std::vector<io::path> temp_files;

   for (const io::directory_entry& entry :
        io::directory_iterator{context.source_path, false}) {
      if (not entry.is_file) continue;
      if (not string::iequals(entry.path.extension(), ".cfg")) continue;
      if (string::iequals(entry.path.stem(), "Comments")) continue;

      const io::path entry_platform_path =
         io::compose_path(platform_source_path, entry.path.filename());

      if (not munge) {
         const io::path entry_output_path =
            io::compose_path(context.output_path, entry.path.stem(), ".loc");

         if (io::get_last_write_time(entry_output_path) <
             std::max(entry.last_write_time,
                      io::get_last_write_time(entry_platform_path))) {
            munge = true;
         }
      }

      if (munge) {
         if (temp_files.empty() and not io::exists(merge_source_path)) {
            if (not io::create_directory(merge_source_path)) {
               context.feedback.add_error(
                  {.file = merge_source_path, .tool = "LocalizeMunge", .message = "Failed to create temporary directory for merging localization."});

               munge = false;

               break;
            }
         }

         const io::path entry_merge_path =
            io::compose_path(merge_source_path, entry.path.filename());

         try {
            io::output_file merge_out{entry_merge_path};

            temp_files.push_back(entry_merge_path);

            try {
               merge_out.write(io::read_file_to_string(entry.path));
            }
            catch (io::open_error& error) {
               context.feedback.add_error(
                  {.file = entry.path,
                   .tool = "LocalizeMunge",
                   .message = fmt::format(
                      "Failed to open platform localization file for "
                      "merging.\n   Reason: {}",
                      error.what())});

               munge = false;

               continue;
            }

            try {
               merge_out.write(io::read_file_to_string(entry_platform_path));
            }
            catch (io::open_error& error) {
               context.feedback.add_error(
                  {.file = entry_platform_path,
                   .tool = "LocalizeMunge",
                   .message = fmt::format(
                      "Failed to open platform localization file for "
                      "merging.\n   Reason: {}",
                      error.what())});

               munge = false;

               continue;
            }
         }
         catch (io::open_error& error) {
            context.feedback.add_error(
               {.file = entry_merge_path,
                .tool = "LocalizeMunge",
                .message =
                   fmt::format("Failed to open temporary localization file for "
                               "merging.\n   Reason: {}",
                               error.what())});

            munge = false;

            continue;
         }
      }
   }

   if (munge) {
      os::process process = os::process_create_desc{
         .executable_path =
            io::compose_path(context.toolsfl_bin_path, "LocalizeMunge.exe"),
         .command_line =
            fmt::format("-inputfile *.cfg -platform {} -sourcedir {} "
                        "-outputdir {}",
                        context.platform, merge_source_path.string_view(),
                        context.output_path.string_view()),
         .working_directory = context.toolsfl_bin_path,
         .capture_stdout = true,
         .capture_stderr = true,
         .priority = munge_process_priority,
      };

      std::string standard_error = process.get_standard_error();

      context.feedback.print_output(process.get_standard_output());
      context.feedback.parse_error_string(standard_error, merge_source_path);
      context.feedback.print_errors(std::move(standard_error));

      process.wait_for_exit();
   }

   for (const io::path& temp_file : temp_files) {
      if (not io::remove(temp_file)) {
         context.feedback.add_warning(
            {.file = temp_file, .tool = "LocalizeMunge", .message = "Failed to cleanup temporary file used for merging localization."});
      }
   }

   if (not io::remove(merge_source_path) and io::exists(merge_source_path)) {
      context.feedback.add_warning(
         {.file = merge_source_path, .tool = "LocalizeMunge", .message = "Failed to cleanup temporary directory used for merging localization."});
   }
}

void execute_level_pack(const level_pack_inputs& level_pack, const tool_context& context)
{
   std::string common_files;

   if (not level_pack.extra_common_files.empty() or not context.common_files.empty()) {
      common_files = "-common";

      for (const io::path& path : context.common_files) {
         common_files += ' ';
         common_files += path.string_view();
      }

      for (const io::path& path : level_pack.extra_common_files) {
         common_files += ' ';
         common_files +=
            io::compose_path(context.output_path, path.string_view()).string_view();
      }
   }

   std::string write_files;

   if (not level_pack.write_files.empty()) {
      write_files = fmt::format("-writefiles {}",
                                io::compose_path(context.output_path,
                                                 level_pack.write_files.string_view())
                                   .string_view());
   }

   std::string input_dir;

   {
      input_dir = fmt::format("-inputdir {}", context.output_path.string_view());

      for (const io::path& path : level_pack.extra_input_directories) {
         input_dir += ' ';
         input_dir += fmt::format(" {}\\{}\\{}", context.project_path.string_view(),
                                  path.string_view(), context.platform);
      }
   }

   const std::string_view input_file = level_pack.mrq_input ? "*.mrq" : "*.req";
   const std::string_view only_files = level_pack.only_files ? "-onlyfiles" : "";
   const io::path source_dir =
      level_pack.source_directory.empty()
         ? context.source_path
         : io::compose_path(context.source_path,
                            level_pack.source_directory.string_view());
   const std::string_view output_dir = level_pack.child_levels
                                          ? context.output_path.string_view()
                                          : context.lvl_output_path.string_view();

   os::process process = os::process_create_desc{
      .executable_path =
         io::compose_path(context.toolsfl_bin_path, "LevelPack.exe"),
      .command_line = fmt::format("-inputfile {} {} {} {} -platform {} "
                                  "-sourcedir {} {} -outputdir {}",
                                  level_pack.input_file.empty()
                                     ? input_file
                                     : level_pack.input_file.string_view(),
                                  common_files, only_files, write_files, context.platform,
                                  source_dir.string_view(), input_dir, output_dir),
      .working_directory = context.toolsfl_bin_path,
      .capture_stdout = true,
      .capture_stderr = true,
      .priority = munge_process_priority,
   };

   std::string standard_error = process.get_standard_error();

   context.feedback.print_output(process.get_standard_output());
   context.feedback.parse_error_string(standard_error, context.source_path);
   context.feedback.print_errors(std::move(standard_error));

   process.wait_for_exit();
}

void execute_load_pack(const tool_context& context)
{
   const io::path temp_path = io::compose_path(context.source_path, "__TEMP__");

   if (not io::create_directory(temp_path) and not io::exists(temp_path)) {
      context.feedback.add_error(
         {.file = temp_path,
          .tool = "LoadPack",
          .message = "Failed to create temporary directory."});

      return;
   }

   const io::path backdrop_source_directory =
      io::compose_path(context.source_path, "backdrops");

   for (const io::directory_entry& entry :
        io::directory_iterator{backdrop_source_directory}) {
      if (not entry.is_file) continue;
      if (not string::iequals(entry.path.extension(), ".tga")) continue;

      try {
         io::output_file{io::compose_path(temp_path, entry.path.stem(), ".req")}.write(
            R"(ucft
{{
   REQN
   {{
      "texture"
      "{}"
   }}
}}
)",
            entry.path.stem());
      }
      catch (io::open_error& error) {
         context.feedback.add_error(
            {.file = entry.path,
             .tool = "LoadPack",
             .message = fmt::format("Failed to create .req file for "
                                    "packing.\n   Reason: {}",
                                    error.what())});

         continue;
      }
   }

   tool_context pack_context = context;

   pack_context.source_path = temp_path;

   execute_level_pack({.child_levels = true}, pack_context);

   for (const io::directory_entry& entry : io::directory_iterator{temp_path, false}) {
      if (not io::remove(entry.path)) {
         context.feedback.add_warning(
            {.file = entry.path,
             .tool = "LoadPack",
             .message = "Failed to remove temporary file."});
      }
   }

   for (const io::directory_entry& backdrop_set_entry :
        io::directory_iterator{backdrop_source_directory, false}) {
      if (not backdrop_set_entry.is_directory) continue;

      try {
         io::output_file out{
            io::compose_path(temp_path, backdrop_set_entry.path.stem(), ".req")};

         out.write_ln("ucft");
         out.write_ln("{");
         out.write_ln("   REQN");
         out.write_ln("   {");
         out.write_ln(R"(      "lvl")");

         for (const io::directory_entry& entry :
              io::directory_iterator{backdrop_set_entry.path, false}) {
            if (not entry.is_file) continue;
            if (not string::iequals(entry.path.extension(), ".tga")) continue;

            out.write_ln(R"(   "{}")", entry.path.stem());
         }

         out.write_ln("   }");
         out.write_ln("}");
      }
      catch (io::open_error& error) {
         context.feedback.add_error(
            {.file = backdrop_set_entry.path,
             .tool = "LoadPack",
             .message = fmt::format("Failed to create .req file for "
                                    "packing.\n   Reason: {}",
                                    error.what())});

         continue;
      }
   }

   execute_level_pack({}, pack_context);

   for (const io::directory_entry& entry : io::directory_iterator{temp_path, false}) {
      if (not io::remove(entry.path)) {
         context.feedback.add_warning(
            {.file = entry.path,
             .tool = "LoadPack",
             .message = "Failed to remove temporary file."});
      }
   }

   if (not io::remove(temp_path) and io::exists(temp_path)) {
      context.feedback.add_warning(
         {.file = temp_path,
          .tool = "LoadPack",
          .message = "Failed to remove temporary directory."});
   }
}

void execute_path_munge(const tool_context& context)
{
   // BF2 paths are a little more complicated to munge than other things. Most
   // files are munged with 1 file as input and 1 file as output. Paths take all
   // the .PTH files for a world as input and output 1 file.

   for (const io::directory_entry& world_directory :
        io::directory_iterator{context.source_path, false}) {
      if (not world_directory.is_directory or
          not string::istarts_with(world_directory.path.stem(), "World")) {
         continue;
      }

      for (const io::directory_entry& entry :
           io::directory_iterator{world_directory.path, false}) {
         if (not entry.is_file or not string::iequals(entry.path.extension(), ".wld")) {
            continue;
         }

         os::process process = os::process_create_desc{
            .executable_path =
               io::compose_path(context.toolsfl_bin_path, "ConfigMunge", ".exe"),
            .command_line = fmt::format(
               "-inputfile ${0}*.pth -checkdate -platform {1} -sourcedir {2} "
               "-outputfile {0} -outputdir {3} -ext path -chunkid path",
               entry.path.stem(), context.platform,
               context.source_path.string_view(), context.output_path.string_view()),
            .working_directory = context.toolsfl_bin_path,
            .capture_stdout = true,
            .capture_stderr = true,
            .priority = munge_process_priority,
         };

         std::string standard_error = process.get_standard_error();

         context.feedback.print_output(process.get_standard_output());
         context.feedback.parse_error_string(standard_error, context.source_path);
         context.feedback.print_errors(std::move(standard_error));

         process.wait_for_exit();
      }
   }
}

void execute_script_munge(const tool_context& context)
{
   os::process process = os::process_create_desc{
      .executable_path =
         io::compose_path(context.toolsfl_bin_path, "ScriptMunge.exe"),
      .command_line =
         fmt::format("-inputfile $*.lua -checkdate -platform {} -sourcedir {} "
                     "-outputdir {}",
                     context.platform, context.source_path.string_view(),
                     context.output_path.string_view()),
      .working_directory = context.toolsfl_bin_path,
      .capture_stdout = true,
      .capture_stderr = true,
      .priority = munge_process_priority,
   };

   std::string standard_error = process.get_standard_error();

   context.feedback.print_output(process.get_standard_output());
   context.feedback.parse_script_munge_error_string(standard_error, context.source_path);
   context.feedback.print_errors(std::move(standard_error));

   process.wait_for_exit();
}

void execute_shader_munge(const munge_config& config, const tool_context& context)
{
   os::process process = os::process_create_desc{
      .executable_path =
         io::compose_path(context.toolsfl_bin_path, config.tool_name, ".exe"),
      .command_line =
         fmt::format("-inputfile $*.{} -checkdate -platform {} -sourcedir {} "
                     "-outputdir {} -I {}",
                     config.input, context.platform, context.source_path.string_view(),
                     context.output_path.string_view(),
                     context.source_path.string_view()),
      .working_directory = context.toolsfl_bin_path,
      .capture_stdout = true,
      .capture_stderr = true,
      .priority = munge_process_priority,
   };

   std::string standard_error = process.get_standard_error();

   context.feedback.print_output(process.get_standard_output());
   context.feedback.parse_shader_munge_error_string(standard_error, context.source_path);
   context.feedback.print_errors(std::move(standard_error));

   process.wait_for_exit();
}

void execute_sound_munge(const sound_munge_inputs& sound_munge, const tool_context& context)
{
   const io::path sound_fl_munge_path =
      io::compose_path(context.toolsfl_bin_path, "SoundFLMunge", ".exe");

   std::string_view platform;

   // clang-format off
   if      (string::iequals(context.platform, "PC"))   platform = "pc";
   else if (string::iequals(context.platform, "PS2"))  platform = "ps2";
   else if (string::iequals(context.platform, "XBOX")) platform = "xbox";
   // clang-format on

   std::string_view input_extension = ".sfx";

   if (sound_munge.additional_bank) input_extension = ".asfx";
   if (sound_munge.stream) input_extension = ".stm";

   const io::path source_path =
      sound_munge.sound_child_directory
         ? io::compose_path(context.source_path, "Sound")
         : context.source_path;

   for (const io::directory_entry& entry : io::directory_iterator{source_path, false}) {
      if (not entry.is_file or
          not string::iequals(entry.path.extension(), input_extension)) {
         continue;
      }

      os::process process = os::process_create_desc{
         .executable_path = sound_fl_munge_path,
         .command_line =
            fmt::format("-platform {} -banklistinput {} -bankoutput {}\\ {} "
                        "-checkdate -checkid -resample",
                        platform, entry.path.string_view(),
                        context.output_path.string_view(),
                        sound_munge.stream ? "-stream" : ""),
         .working_directory = entry.path.parent_path(),
         .capture_stdout = true,
         .capture_stderr = true,
         .priority = munge_process_priority,
      };

      std::string standard_error = process.get_standard_error();

      context.feedback.print_output(process.get_standard_output());
      context.feedback.parse_sound_munge_error_string(standard_error);
      context.feedback.print_errors(std::move(standard_error));

      process.wait_for_exit();
   }
}

void execute_sound_config_munge(std::string input_files, const tool_context& context)
{
   os::process process = os::process_create_desc{
      .executable_path =
         io::compose_path(context.toolsfl_bin_path, "ConfigMunge.exe"),
      .command_line = fmt::format("-inputfile {} -checkdate  -platform {} "
                                  "-sourcedir {} "
                                  "-outputdir {} -hashstrings",
                                  input_files, context.platform,
                                  context.source_path.string_view(),
                                  context.output_path.string_view()),
      .working_directory = context.toolsfl_bin_path,
      .capture_stdout = true,
      .capture_stderr = true,
      .priority = munge_process_priority,
   };

   std::string standard_error = process.get_standard_error();

   context.feedback.print_output(process.get_standard_output());
   context.feedback.parse_error_string(standard_error, context.source_path);
   context.feedback.print_errors(std::move(standard_error));

   process.wait_for_exit();
}

void execute_sound_directory_munge(const sound_directory_munge& sound_munge,
                                   const tool_context& context)
{
   execute_sound_config_munge("*.snd *.mus *.ffx *.tsr", context);

   std::string_view platform;

   // clang-format off
   if      (string::iequals(context.platform, "PC"))   platform = "pc";
   else if (string::iequals(context.platform, "PS2"))  platform = "ps2";
   else if (string::iequals(context.platform, "XBOX")) platform = "xbox";
   // clang-format on

   const io::path sound_fl_munge_path =
      io::compose_path(context.toolsfl_bin_path, "SoundFLMunge", ".exe");

   const std::string_view bank_args = sound_munge.create_common_bank
                                         ? R"(-template -stub C:\Windows\Media\chord.wav)"
                                         : "";

   const bool munge_st4 = not string::iequals(platform, "PS2");

   for (const io::directory_entry& entry :
        io::directory_iterator{context.source_path, false}) {
      if (entry.is_directory) continue;

      if (string::iequals(entry.path.extension(), ".sfx") or
          string::iequals(entry.path.extension(), ".asfx")) {
         if (sound_munge.create_common_bank) continue;

         os::process process = os::process_create_desc{
            .executable_path = sound_fl_munge_path,
            .command_line = fmt::format(
               "-platform {} -banklistinput {} -bankoutput {}\\ "
               "-checkdate -resample -checkid noabort -relativepath {}",
               platform, entry.path.string_view(),
               context.output_path.string_view(), bank_args),
            .working_directory = context.toolsfl_bin_path,
            .capture_stdout = true,
            .capture_stderr = true,
            .priority = munge_process_priority,
         };

         std::string standard_error = process.get_standard_error();

         context.feedback.print_output(process.get_standard_output());
         context.feedback.parse_sound_munge_error_string(standard_error);
         context.feedback.print_errors(std::move(standard_error));

         process.wait_for_exit();
      }
      else if (string::iequals(entry.path.extension(), ".stm")) {
         if (munge_st4 and
             io::exists(io::make_path_with_new_extension(entry.path, ".st4"))) {
            continue;
         }

         os::process process = os::process_create_desc{
            .executable_path = sound_fl_munge_path,
            .command_line = fmt::format(
               "-platform {} -banklistinput {} -bankoutput {}\\ -stream "
               "-checkdate -resample -checkid noabort -relativepath",
               platform, entry.path.string_view(), context.output_path.string_view()),
            .working_directory = context.toolsfl_bin_path,
            .capture_stdout = true,
            .capture_stderr = true,
            .priority = munge_process_priority,
         };

         std::string standard_error = process.get_standard_error();

         context.feedback.print_output(process.get_standard_output());
         context.feedback.parse_sound_munge_error_string(standard_error);
         context.feedback.print_errors(std::move(standard_error));

         process.wait_for_exit();
      }
      else if (munge_st4 and string::iequals(entry.path.extension(), ".st4")) {
         os::process process = os::process_create_desc{
            .executable_path = sound_fl_munge_path,
            .command_line = fmt::format(
               "-platform {} -banklistinput {} -bankoutput {}\\ -stream "
               "-checkdate -resample -checkid noabort -relativepath -substream "
               "2",
               platform, entry.path.string_view(), context.output_path.string_view()),
            .working_directory = context.toolsfl_bin_path,
            .capture_stdout = true,
            .capture_stderr = true,
            .priority = munge_process_priority,
         };

         std::string standard_error = process.get_standard_error();

         context.feedback.print_output(process.get_standard_output());
         context.feedback.parse_sound_munge_error_string(standard_error);
         context.feedback.print_errors(std::move(standard_error));

         process.wait_for_exit();
      }
   }

   for (const project_sound_localization& localization : sound_munge.localizations) {
      tool_context localization_context = context;

      localization_context.output_path =
         io::compose_path(context.output_path, localization.output_directory);
      localization_context.lvl_output_path =
         io::compose_path(context.lvl_output_path, localization.output_directory);

      if (not io::create_directories(localization_context.output_path)) {
         context.feedback.add_error({
            .file = localization_context.output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });

         continue;
      }

      if (not io::create_directories(localization_context.lvl_output_path)) {
         context.feedback.add_error({
            .file = localization_context.lvl_output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });

         continue;
      }

      const std::string stm_extension = fmt::format(".stm_{}", localization.language);
      const std::string st4_extension = fmt::format(".st4_{}", localization.language);

      for (const io::directory_entry& entry :
           io::directory_iterator{localization_context.source_path, false}) {
         if (entry.is_directory) continue;

         if (string::iequals(entry.path.extension(), stm_extension)) {
            if (munge_st4 and
                io::exists(io::make_path_with_new_extension(entry.path, st4_extension))) {
               continue;
            }

            os::process process = os::process_create_desc{
               .executable_path = sound_fl_munge_path,
               .command_line = fmt::format(
                  "-platform {} -banklistinput {} -bankoutput {}\\ -stream "
                  "-checkdate -resample -checkid noabort -relativepath",
                  platform, entry.path.string_view(),
                  localization_context.output_path.string_view()),
               .working_directory = localization_context.toolsfl_bin_path,
               .capture_stdout = true,
               .capture_stderr = true,
               .priority = munge_process_priority,
            };

            std::string standard_error = process.get_standard_error();

            localization_context.feedback.print_output(process.get_standard_output());
            localization_context.feedback.parse_sound_munge_error_string(standard_error);
            localization_context.feedback.print_errors(std::move(standard_error));

            process.wait_for_exit();
         }
         else if (munge_st4 and string::iequals(entry.path.extension(), st4_extension)) {
            os::process process = os::process_create_desc{
               .executable_path = sound_fl_munge_path,
               .command_line = fmt::format(
                  "-platform {} -banklistinput {} -bankoutput {}\\ -stream "
                  "-checkdate -resample -checkid noabort -relativepath "
                  "-substream "
                  "2",
                  platform, entry.path.string_view(),
                  localization_context.output_path.string_view()),
               .working_directory = localization_context.toolsfl_bin_path,
               .capture_stdout = true,
               .capture_stderr = true,
               .priority = munge_process_priority,
            };

            std::string standard_error = process.get_standard_error();

            localization_context.feedback.print_output(process.get_standard_output());
            localization_context.feedback.parse_sound_munge_error_string(standard_error);
            localization_context.feedback.print_errors(std::move(standard_error));

            process.wait_for_exit();
         }
      }
   }
}

void execute_sound_directory_children_pack(const sound_directory_pack& sound_pack,
                                           const tool_context& context)
{
   const io::path level_pack_path =
      io::compose_path(context.toolsfl_bin_path, "LevelPack", ".exe");

   std::string shared_pack_input_directories;

   for (const io::path& input_dir : sound_pack.input_directories) {
      shared_pack_input_directories += input_dir.string_view();
      shared_pack_input_directories += ' ';
   }

   for (const io::directory_entry& entry :
        io::directory_iterator{context.source_path, false}) {
      if (entry.is_directory) continue;
      if (not string::iequals(entry.path.extension(), ".req")) continue;
      if (string::iequals(entry.path.stem(), sound_pack.directory_name)) {
         continue;
      }

      os::process process = os::process_create_desc{
         .executable_path =
            io::compose_path(context.toolsfl_bin_path, "LevelPack.exe"),
         .command_line =
            fmt::format("-inputfile {} -platform {} "
                        "-sourcedir {} -inputdir {} {} -outputdir {}",
                        entry.path.string_view(), context.platform,
                        context.source_path.string_view(),
                        context.output_path.string_view(), shared_pack_input_directories,
                        context.output_path.string_view()),
         .working_directory = context.toolsfl_bin_path,
         .capture_stdout = true,
         .capture_stderr = true,
         .priority = munge_process_priority,
      };

      std::string standard_error = process.get_standard_error();

      context.feedback.print_output(process.get_standard_output());
      context.feedback.parse_error_string(standard_error, context.source_path);
      context.feedback.print_errors(std::move(standard_error));

      process.wait_for_exit();
   }

   for (const project_sound_localization& localization : sound_pack.localizations) {
      tool_context localization_context = context;

      localization_context.output_path =
         io::compose_path(context.output_path, localization.output_directory);
      localization_context.lvl_output_path =
         io::compose_path(context.lvl_output_path, localization.output_directory);

      std::string shared_localization_pack_input_directories;

      for (const io::path& input_dir : sound_pack.input_directories) {
         const io::path localization_input_dir =
            io::compose_path(input_dir, localization.output_directory);

         shared_localization_pack_input_directories +=
            localization_input_dir.string_view();
         shared_localization_pack_input_directories += ' ';
      }

      for (const io::directory_entry& entry :
           io::directory_iterator{localization_context.source_path, false}) {
         if (entry.is_directory) continue;
         if (not string::iequals(entry.path.extension(), ".req")) continue;
         if (string::iequals(entry.path.stem(), sound_pack.directory_name)) {
            continue;
         }

         os::process process = os::process_create_desc{
            .executable_path = io::compose_path(localization_context.toolsfl_bin_path,
                                                "LevelPack.exe"),
            .command_line =
               fmt::format("-inputfile {} -platform {} "
                           "-sourcedir {} -inputdir {} {} {} {} -outputdir {}",
                           entry.path.string_view(), localization_context.platform,
                           localization_context.source_path.string_view(),
                           localization_context.output_path.string_view(),
                           shared_localization_pack_input_directories,
                           context.output_path.string_view(), shared_pack_input_directories,
                           localization_context.output_path.string_view()),
            .working_directory = context.toolsfl_bin_path,
            .capture_stdout = true,
            .capture_stderr = true,
            .priority = munge_process_priority,
         };

         std::string standard_error = process.get_standard_error();

         context.feedback.print_output(process.get_standard_output());
         context.feedback.parse_error_string(standard_error,
                                             localization_context.source_path);
         context.feedback.print_errors(std::move(standard_error));

         process.wait_for_exit();
      }
   }
}

void execute_sound_directory_pack(const sound_directory_pack& sound_pack,
                                  const tool_context& context)
{
   const io::path req_path =
      io::compose_path(context.source_path, sound_pack.directory_name, ".req");

   if (not io::exists(req_path)) return;

   const io::path level_pack_path =
      io::compose_path(context.toolsfl_bin_path, "LevelPack", ".exe");

   std::string shared_pack_input_directories;

   for (const io::path& input_dir : sound_pack.input_directories) {
      shared_pack_input_directories += input_dir.string_view();
      shared_pack_input_directories += ' ';
   }

   {
      os::process process = os::process_create_desc{
         .executable_path =
            io::compose_path(context.toolsfl_bin_path, "LevelPack.exe"),
         .command_line =
            fmt::format("-inputfile {} -platform {} "
                        "-sourcedir {} -inputdir {} {} -outputdir {}",
                        req_path.string_view(), context.platform,
                        context.source_path.string_view(),
                        context.output_path.string_view(), shared_pack_input_directories,
                        context.lvl_output_path.string_view()),
         .working_directory = context.toolsfl_bin_path,
         .capture_stdout = true,
         .capture_stderr = true,
         .priority = munge_process_priority,
      };

      std::string standard_error = process.get_standard_error();

      context.feedback.print_output(process.get_standard_output());
      context.feedback.parse_error_string(standard_error, context.source_path);
      context.feedback.print_errors(std::move(standard_error));

      process.wait_for_exit();
   }

   for (const project_sound_localization& localization : sound_pack.localizations) {
      tool_context localization_context = context;

      localization_context.output_path =
         io::compose_path(context.output_path, localization.output_directory);
      localization_context.lvl_output_path =
         io::compose_path(context.lvl_output_path, localization.output_directory);

      std::string shared_localization_pack_input_directories;

      for (const io::path& input_dir : sound_pack.input_directories) {
         const io::path localization_input_dir =
            io::compose_path(input_dir, localization.output_directory);

         shared_localization_pack_input_directories +=
            localization_input_dir.string_view();
         shared_localization_pack_input_directories += ' ';
      }

      os::process process = os::process_create_desc{
         .executable_path = io::compose_path(localization_context.toolsfl_bin_path,
                                             "LevelPack.exe"),
         .command_line =
            fmt::format("-inputfile {} -platform {} "
                        "-sourcedir {} -inputdir {} {} {} {} -outputdir {}",
                        req_path.string_view(), localization_context.platform,
                        localization_context.source_path.string_view(),
                        localization_context.output_path.string_view(),
                        shared_localization_pack_input_directories,
                        context.output_path.string_view(), shared_pack_input_directories,
                        localization_context.lvl_output_path.string_view()),
         .working_directory = context.toolsfl_bin_path,
         .capture_stdout = true,
         .capture_stderr = true,
         .priority = munge_process_priority,
      };

      std::string standard_error = process.get_standard_error();

      context.feedback.print_output(process.get_standard_output());
      context.feedback.parse_error_string(standard_error,
                                          localization_context.source_path);
      context.feedback.print_errors(std::move(standard_error));

      process.wait_for_exit();
   }
}

void execute_sound_common_bank_munge(const tool_context& context)
{
   std::string bank_list_files;
   bank_list_files.reserve(4096);

   for (const io::directory_entry& entry : io::directory_iterator{context.source_path}) {
      if (entry.is_directory) continue;

      if (string::iequals(entry.path.extension(), ".sfx") or
          string::iequals(entry.path.extension(), ".asfx")) {
         bank_list_files += entry.path.string_view();
         bank_list_files += ' ';
      }
   }

   std::string_view platform;

   // clang-format off
   if      (string::iequals(context.platform, "PC"))   platform = "pc";
   else if (string::iequals(context.platform, "PS2"))  platform = "ps2";
   else if (string::iequals(context.platform, "XBOX")) platform = "xbox";
   // clang-format on

   const io::path sound_fl_munge_path =
      io::compose_path(context.toolsfl_bin_path, "SoundFLMunge", ".exe");
   const io::path output_path =
      io::compose_path(context.lvl_output_path, "common.bnk");

   os::process process = os::process_create_desc{
      .executable_path = sound_fl_munge_path,
      .command_line =
         fmt::format("-platform {} -banklistinput {} -bankoutput {} "
                     "-checkdate -resample -compact nowarning -checkid "
                     "noabort -relativepath",
                     platform, bank_list_files, output_path.string_view()),
      .working_directory = context.toolsfl_bin_path,
      .capture_stdout = true,
      .capture_stderr = true,
      .priority = munge_process_priority,
   };

   std::string standard_error = process.get_standard_error();

   context.feedback.print_output(process.get_standard_output());
   context.feedback.parse_sound_munge_error_string(standard_error);
   context.feedback.print_errors(std::move(standard_error));

   process.wait_for_exit();
}

void execute_tool(const tool& tool, const tool_context& context)
{
   switch (tool.type) {
   case tool_type::animation_munge:
      std::terminate();
   case tool_type::bin_ps2_munge: {
      tool_context child_context = context;

      child_context.source_path =
         io::compose_path(context.source_path, "ps2bin");

      execute_munge({"BinMunge", "ps2bin"}, child_context);
   } break;
   case tool_type::config_boundary_munge: {
      execute_config_munge({"bnd", "boundary", "bnd", true}, context);
   } break;
   case tool_type::config_combo_munge: {
      execute_config_munge({"combo", "config", "comb"}, context);
   } break;
   case tool_type::config_effects_munge: {
      if (tool.config_effects_munge.effects_directory_only) {
         tool_context child_context = context;

         child_context.source_path =
            io::compose_path(context.source_path, "Effects");

         execute_config_munge({"fx", "config", "fx"}, child_context);
      }
      else {
         execute_config_munge({"fx", "config", "fx"}, context);
      }
   } break;
   case tool_type::config_env_effects_munge: {
      execute_config_munge({"fx", "envfx", "fx"}, context);
   } break;
   case tool_type::config_hud_munge: {
      execute_config_munge({"hud", "config", "hud"}, context);
   } break;
   case tool_type::config_light_munge: {
      execute_config_munge({"lgt", "light", "lght"}, context);
   } break;
   case tool_type::config_load_munge: {
      execute_config_munge({"cfg", "config", "load"}, context);
   } break;
   case tool_type::config_movie_munge: {
      execute_config_munge({"mcfg", "config", "mcfg", true}, context);
   } break;
   case tool_type::config_path_munge: {
      execute_config_munge({"pth", "path", "path"}, context);
   } break;
   case tool_type::config_prop_munge: {
      execute_config_munge({"prp", "prop", "prp", true}, context);
   } break;
   case tool_type::config_pvs_munge: {
      execute_config_munge({"pvs", "povs", "PORT"}, context);
   } break;
   case tool_type::config_sky_munge: {
      execute_config_munge({"sky", "config", "sky"}, context);
   } break;
   case tool_type::config_soldier_animation_munge: {
      execute_config_munge({"sanm", "config", "sanm"}, context);
   } break;
   case tool_type::config_sound_munge: {
      execute_sound_config_munge("*.snd *.mus *.ffx *.tsr", context);
   } break;
   case tool_type::config_sound_common_munge: {
      tool_context child_context = context;

      child_context.source_path =
         io::compose_path(context.source_path, "Sound");

      execute_sound_config_munge("*.snd *.mus", context);
   } break;
   case tool_type::config_sound_world_munge: {
      tool_context child_context = context;

      child_context.source_path =
         io::compose_path(context.source_path, "Sound");

      execute_sound_config_munge("*.snd *.mus *.tsr", context);
   } break;
   case tool_type::font_munge: {
      if (tool.font_munge.fonts_directory_only) {
         tool_context child_context = context;

         child_context.source_path =
            io::compose_path(context.source_path, "Fonts");

         execute_munge({"FontMunge", "fff"}, child_context);
      }
      else {
         execute_munge({"FontMunge", "fff"}, context);
      }
   } break;
   case tool_type::level_pack: {
      execute_level_pack(tool.level_pack, context);
   } break;
   case tool_type::load_pack: {
      execute_load_pack(context);
   } break;
   case tool_type::localize_munge: {
      tool_context child_context = context;

      child_context.source_path =
         io::compose_path(context.source_path, "Localize");

      execute_localize_munge(child_context);
   } break;
   case tool_type::model_munge: {
      execute_munge({"pc_ModelMunge", "msh"}, context);
   } break;
   case tool_type::movie_munge: {
      tool_context child_context = context;

      child_context.source_path =
         io::compose_path(context.source_path,
                          fmt::format(R"(movies\{})", child_context.platform));

      execute_movie_munge(child_context);
   } break;
   case tool_type::odf_munge: {
      execute_munge({"OdfMunge", "odf"}, context);
   } break;
   case tool_type::path_munge: {
      execute_path_munge(context);
   } break;
   case tool_type::path_planning_munge: {
      execute_munge({"PathPlanningMunge", "pln"}, context);
   } break;
   case tool_type::script_munge: {
      if (tool.script_munge.scripts_directory_only) {
         tool_context child_context = context;

         child_context.source_path =
            io::compose_path(context.source_path, "Scripts");

         execute_script_munge(child_context);
      }
      else {
         execute_script_munge(context);
      }
   } break;
   case tool_type::shader_munge: {
      if (string::iequals(context.platform, "PC")) {
         tool_context child_context = context;

         child_context.source_path =
            io::compose_path(context.source_path, R"(shaders\PC)");

         execute_shader_munge({"pc_ShaderMunge", "xml"}, child_context);
      }
      else if (string::iequals(context.platform, "XBOX")) {
         context.feedback.add_warning(
            {.tool = "xbox_ShaderMunge", .message = "Munging Xbox shaders is currently unsupported through WorldEdit."});
      }
   } break;
   case tool_type::sound_munge: {
      execute_sound_munge(tool.sound_munge, context);
   } break;
   case tool_type::terrain_munge: {
      execute_munge({"TerrainMunge", "ter"}, context);
   } break;
   case tool_type::texture_munge: {
      execute_munge({"pc_TextureMunge", "tga"}, context);
   } break;
   case tool_type::world_munge: {
      execute_munge({"WorldMunge", tool.world_munge.layers ? "lyr" : "wld"}, context);
   } break;
   case tool_type::copy_premunged: {
      for (const io::directory_entry& entry :
           io::directory_iterator{io::compose_path(context.source_path,
                                                   "munged"),
                                  false}) {
         if (entry.is_directory) continue;

         const io::path destination_file =
            io::compose_path(context.output_path, entry.path.filename());

         if (io::get_last_write_time(destination_file) !=
             io::get_last_write_time(entry.path)) {
            if (not io::copy_file(entry.path, destination_file)) {
               context.feedback.add_error({
                  .file = entry.path,
                  .tool = "Copy File",
                  .message = fmt::format("Failed to copy file to {}.",
                                         destination_file.string_view()),
               });
            }
         }
      }
   } break;
   }
}

void clean_directory(const std::string_view relative_directory,
                     const io::path& project_directory, munge_feedback& feedback) noexcept
{
   const io::path path = io::compose_path(project_directory, relative_directory);

   feedback.print_output(fmt::format("Cleaning {}", relative_directory));

   for (const io::directory_entry& entry : io::directory_iterator{path}) {
      if (not entry.is_file) continue;

      if (not io::remove(entry.path) and io::exists(entry.path)) {
         feedback.warnings.push_back({
            .file = entry.path,
            .tool = "Clean",
            .message = "Failed to delete file.",
         });
      }
   }
}

auto run_munge(munge_context context) -> report
{
   utility::stopwatch timer;

   tool_context root_context = {
      .toolsfl_bin_path = context.project.config.toolsfl_bin_path,
      .project_path = context.project.directory,
      .platform = context.platform,
      .feedback = context.feedback,
   };

   const io::path common_output_path =
      io::compose_path(root_context.project_path,
                       fmt::format(R"(_BUILD\Common\MUNGED\{})", root_context.platform));

   for (const io::path& file : context.project.config.common_files) {
      root_context.common_files.push_back(
         io::compose_path(common_output_path, file.string_view()));
   }

   root_context.common_files.reserve(context.project.config.common_files.size());

   if (context.project.addme_active) {
      tool_context addme_context = root_context;

      addme_context.source_path =
         io::compose_path(addme_context.project_path, "addme");
      addme_context.output_path =
         io::compose_path(addme_context.project_path, R"(addme\munged)");

      if (not io::create_directories(addme_context.output_path)) {
         context.feedback.add_error({
            .file = addme_context.output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });
      }
      else {
         for (const tool& tool : context.project.config.addme) {
            execute_tool(tool, addme_context);
         }
      }
   }

   if (context.project.common_active) {
      tool_context common_context = root_context;

      common_context.source_path =
         io::compose_path(common_context.project_path, "Common");
      common_context.output_path =
         io::compose_path(common_context.project_path,
                          fmt::format(R"(_BUILD\Common\MUNGED\{})",
                                      common_context.platform));
      common_context.lvl_output_path =
         io::compose_path(common_context.project_path,
                          fmt::format("_LVL_{}", common_context.platform));
      common_context.common_files = {};

      if (not io::create_directories(common_context.output_path)) {
         context.feedback.add_error({
            .file = common_context.output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });
      }
      else if (not io::create_directories(common_context.lvl_output_path)) {
         context.feedback.add_error({
            .file = common_context.lvl_output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });
      }
      else {
         for (const tool& tool : context.project.config.common) {
            execute_tool(tool, common_context);
         }

         for (const tool& tool : context.project.config.common_pack) {
            execute_tool(tool, common_context);
         }

         tool_context post_pack_context = common_context;
         post_pack_context.common_files = root_context.common_files;

         for (const tool& tool : context.project.config.common_mission_child_pack) {
            execute_tool(tool, post_pack_context);
         }

         for (const tool& tool : context.project.config.common_mission_pack) {
            execute_tool(tool, common_context);
         }

         tool_context fpm_context = post_pack_context;

         fpm_context.lvl_output_path =
            io::compose_path(post_pack_context.lvl_output_path, R"(FPM\COM)");

         if (not io::create_directories(fpm_context.output_path)) {
            context.feedback.add_error({
               .file = fpm_context.output_path,
               .tool = "Create Directory",
               .message = "Failed to create directory for use as output path.",
            });
         }
         else if (not io::create_directories(fpm_context.lvl_output_path)) {
            context.feedback.add_error({
               .file = fpm_context.lvl_output_path,
               .tool = "Create Directory",
               .message = "Failed to create directory for use as output path.",
            });
         }
         else {
            for (const tool& tool : context.project.config.common_fpm_pack) {
               execute_tool(tool, fpm_context);
            }
         }
      }
   }

   if (context.project.load_active) {
      tool_context load_context = root_context;

      load_context.source_path =
         io::compose_path(load_context.project_path, "Load");
      load_context.output_path =
         io::compose_path(load_context.project_path,
                          fmt::format(R"(_BUILD\Load\MUNGED\{})", load_context.platform));
      load_context.lvl_output_path =
         io::compose_path(load_context.project_path,
                          fmt::format(R"(_LVL_{}\load)", load_context.platform));

      std::erase_if(load_context.common_files, [](const io::path& path) {
         return not string::iequals("core.files", path.filename());
      });

      if (not io::create_directories(load_context.output_path)) {
         context.feedback.add_error({
            .file = load_context.output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });
      }
      else if (not io::create_directories(load_context.lvl_output_path)) {
         context.feedback.add_error({
            .file = load_context.lvl_output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });
      }
      else {
         for (const tool& tool : context.project.config.load) {
            execute_tool(tool, load_context);
         }

         for (const tool& tool : context.project.config.load_pack) {
            execute_tool(tool, load_context);
         }
      }
   }

   if (context.project.shell_active) {
      tool_context shell_context = root_context;

      shell_context.source_path =
         io::compose_path(shell_context.project_path, "Shell");
      shell_context.output_path =
         io::compose_path(shell_context.project_path,
                          fmt::format(R"(_BUILD\Shell\MUNGED\{})",
                                      shell_context.platform));
      shell_context.lvl_output_path =
         io::compose_path(shell_context.project_path,
                          fmt::format("_LVL_{}", shell_context.platform));

      std::erase_if(shell_context.common_files, [](const io::path& path) {
         return string::iequals("ingame.files", path.filename());
      });

      if (not io::create_directories(shell_context.output_path)) {
         context.feedback.add_error({
            .file = shell_context.output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });
      }
      else if (not io::create_directories(shell_context.lvl_output_path)) {
         context.feedback.add_error({
            .file = shell_context.lvl_output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });
      }
      else {
         for (const tool& tool : context.project.config.shell) {
            execute_tool(tool, shell_context);
         }

         for (const tool& tool : context.project.config.shell_pack) {
            execute_tool(tool, shell_context);
         }

         if (string::iequals(shell_context.platform, "PS2")) {
            for (const tool& tool : context.project.config.shell_ps2_pack) {
               execute_tool(tool, shell_context);
            }
         }
      }
   }

   for (const project_child& side : context.project.sides) {
      if (not side.active) continue;

      tool_context side_context = root_context;

      side_context.source_path =
         io::compose_path(side_context.project_path,
                          fmt::format(R"(Sides\{})", side.name));
      side_context.output_path =
         io::compose_path(side_context.project_path,
                          fmt::format(R"(_BUILD\Sides\{}\MUNGED\{})", side.name,
                                      side_context.platform));
      side_context.lvl_output_path =
         io::compose_path(side_context.project_path,
                          fmt::format(R"(_LVL_{}\side)", side_context.platform));

      if (not io::create_directories(side_context.output_path)) {
         context.feedback.add_error({
            .file = side_context.output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });

         continue;
      }

      if (not io::create_directories(side_context.lvl_output_path)) {
         context.feedback.add_error({
            .file = side_context.lvl_output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });

         continue;
      }

      for (const tool& tool : context.project.config.side) {
         execute_tool(tool, side_context);
      }

      for (const tool& tool : context.project.config.side_child_pack) {
         execute_tool(tool, side_context);
      }

      if (string::iequals(side.name, "Common")) continue;

      for (const tool& tool : context.project.config.side_pack) {
         execute_tool(tool, side_context);
      }

      tool_context fpm_context = side_context;

      fpm_context.lvl_output_path =
         io::compose_path(fpm_context.project_path,
                          fmt::format(R"(_LVL_{}\FPM\{})", fpm_context.platform,
                                      side.name));

      if (not io::create_directories(fpm_context.lvl_output_path)) {
         context.feedback.add_error({
            .file = fpm_context.lvl_output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });

         continue;
      }

      for (const tool& tool : context.project.config.side_fpm_pack) {
         execute_tool(tool, fpm_context);
      }
   }

   for (const project_child& world : context.project.worlds) {
      if (not world.active) continue;

      tool_context world_context = root_context;

      world_context.source_path =
         io::compose_path(world_context.project_path,
                          fmt::format(R"(Worlds\{})", world.name));
      world_context.output_path =
         io::compose_path(world_context.project_path,
                          fmt::format(R"(_BUILD\Worlds\{}\MUNGED\{})",
                                      world.name, world_context.platform));
      world_context.lvl_output_path =
         io::compose_path(world_context.project_path,
                          fmt::format(R"(_LVL_{}\{})", world_context.platform,
                                      world.name));

      if (not io::create_directories(world_context.output_path)) {
         context.feedback.add_error({
            .file = world_context.output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });

         continue;
      }

      if (not io::create_directories(world_context.lvl_output_path)) {
         context.feedback.add_error({
            .file = world_context.lvl_output_path,
            .tool = "Create Directory",
            .message = "Failed to create directory for use as output path.",
         });

         continue;
      }

      for (const tool& tool : context.project.config.world) {
         execute_tool(tool, world_context);
      }

      if (string::iequals(world.name, "Common")) continue;

      for (const io::directory_entry& entry :
           io::directory_iterator{world_context.source_path, false}) {
         if (not entry.is_directory) continue;
         if (not string::istarts_with(entry.path.stem(), "World")) continue;

         tool_context child_context = world_context;

         child_context.source_path = entry.path;

         for (const tool& tool : context.project.config.world_pack) {
            execute_tool(tool, child_context);
         }
      }
   }

   if (context.project.sound_active) {
      std::vector<io::path> shared_pack_input_directories;
      shared_pack_input_directories.reserve(context.project.sound_shared.size());

      const bool create_common_bank =
         string::iequals(context.platform, "PC") and context.project.sound_common_bank;

      for (const project_child_sound_shared& shared_sound : context.project.sound_shared) {
         tool_context sound_context = root_context;

         sound_context.source_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(Sound\{})", shared_sound.name));
         sound_context.output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_BUILD\Sound\{}\MUNGED\{})",
                                         shared_sound.name, sound_context.platform));
         sound_context.lvl_output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_LVL_{}\sound)", sound_context.platform));

         if (not io::create_directories(sound_context.output_path)) {
            context.feedback.add_error({
               .file = sound_context.output_path,
               .tool = "Create Directory",
               .message = "Failed to create directory for use as output path.",
            });

            continue;
         }

         if (not io::create_directories(sound_context.lvl_output_path)) {
            context.feedback.add_error({
               .file = sound_context.lvl_output_path,
               .tool = "Create Directory",
               .message = "Failed to create directory for use as output path.",
            });

            continue;
         }

         execute_sound_directory_munge({.create_common_bank = create_common_bank,
                                        .localizations =
                                           shared_sound.localized
                                              ? context.project.sound_localizations
                                              : std::span<project_sound_localization>{}},
                                       sound_context);

         shared_pack_input_directories.push_back(sound_context.output_path);
      }

      for (const project_child_sound_world& world : context.project.sound_worlds) {
         if (not world.active) continue;

         tool_context sound_context = root_context;

         sound_context.source_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(Sound\Worlds\{})", world.name));
         sound_context.output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_BUILD\Sound\worlds\{}\MUNGED\{})",
                                         world.name, sound_context.platform));
         sound_context.lvl_output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_LVL_{}\sound)", sound_context.platform));

         if (not io::create_directories(sound_context.output_path)) {
            context.feedback.add_error({
               .file = sound_context.output_path,
               .tool = "Create Directory",
               .message = "Failed to create directory for use as output path.",
            });

            continue;
         }

         if (not io::create_directories(sound_context.lvl_output_path)) {
            context.feedback.add_error({
               .file = sound_context.lvl_output_path,
               .tool = "Create Directory",
               .message = "Failed to create directory for use as output path.",
            });

            continue;
         }

         execute_sound_directory_munge({.create_common_bank = create_common_bank,
                                        .localizations =
                                           world.localized
                                              ? context.project.sound_localizations
                                              : std::span<project_sound_localization>{}},
                                       sound_context);
      }

      for (const project_child_sound_shared& shared_sound : context.project.sound_shared) {
         tool_context sound_context = root_context;

         sound_context.source_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(Sound\{})", shared_sound.name));
         sound_context.output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_BUILD\Sound\{}\MUNGED\{})",
                                         shared_sound.name, sound_context.platform));
         sound_context.lvl_output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_LVL_{}\sound)", sound_context.platform));

         execute_sound_directory_children_pack(
            {.directory_name = shared_sound.name,
             .localizations = shared_sound.localized
                                 ? context.project.sound_localizations
                                 : std::span<project_sound_localization>{},
             .input_directories = shared_pack_input_directories},
            sound_context);
      }

      for (const project_child_sound_shared& shared_sound : context.project.sound_shared) {
         tool_context sound_context = root_context;

         sound_context.source_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(Sound\{})", shared_sound.name));
         sound_context.output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_BUILD\Sound\{}\MUNGED\{})",
                                         shared_sound.name, sound_context.platform));
         sound_context.lvl_output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_LVL_{}\sound)", sound_context.platform));

         execute_sound_directory_pack({.directory_name = shared_sound.name,
                                       .localizations =
                                          shared_sound.localized
                                             ? context.project.sound_localizations
                                             : std::span<project_sound_localization>{},
                                       .input_directories = shared_pack_input_directories},
                                      sound_context);
      }

      for (const project_child_sound_world& world : context.project.sound_worlds) {
         if (not world.active) continue;

         tool_context sound_context = root_context;

         sound_context.source_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(Sound\Worlds\{})", world.name));
         sound_context.output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_BUILD\Sound\worlds\{}\MUNGED\{})",
                                         world.name, sound_context.platform));
         sound_context.lvl_output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_LVL_{}\sound)", sound_context.platform));

         execute_sound_directory_children_pack({.directory_name = world.name,
                                                .localizations = context.project.sound_localizations,
                                                .input_directories =
                                                   shared_pack_input_directories},
                                               sound_context);
         execute_sound_directory_pack({.directory_name = world.name,
                                       .localizations =
                                          world.localized
                                             ? context.project.sound_localizations
                                             : std::span<project_sound_localization>{},
                                       .input_directories = shared_pack_input_directories},
                                      sound_context);
      }

      if (create_common_bank) {
         tool_context sound_context = root_context;

         sound_context.source_path =
            io::compose_path(sound_context.project_path, "Sound");
         sound_context.lvl_output_path =
            io::compose_path(sound_context.project_path,
                             fmt::format(R"(_LVL_{}\sound)", sound_context.platform));

         execute_sound_common_bank_munge(sound_context);
      }
   }

   context.feedback.print_output("Munge Finished");
   context.feedback.print_output(fmt::format("Time Taken: {:.3f}s", timer.elapsed()));

   return {
      .warnings = std::move(context.feedback.warnings),
      .errors = std::move(context.feedback.errors),
   };
}

auto run_clean(munge_context context) noexcept -> report
{
   utility::stopwatch timer;

   const std::string_view platform = context.platform;
   const io::path& project_directory = context.project.directory;

   if (context.project.addme_active) {
      clean_directory(R"(addme\munged)", project_directory, context.feedback);
   }

   if (context.project.common_active) {
      clean_directory(fmt::format(R"(_BUILD\Common\MUNGED\{})", platform),
                      project_directory, context.feedback);
      clean_directory(fmt::format(R"(_LVL_{}\FPM\COM)", platform),
                      project_directory, context.feedback);

      const io::path lvl_output_directory =
         io::compose_path(project_directory, fmt::format("_LVL_{}", platform));

      for (const tool& tool : context.project.config.common_pack) {
         if (tool.type != tool_type::level_pack) continue;
         if (tool.level_pack.input_file.empty() or
             not string::iequals(tool.level_pack.input_file.extension(),
                                 ".req")) {
            continue;
         }

         context.feedback.print_output(
            fmt::format("Cleaning _LVL_{}\\{}.lvl", platform,
                        tool.level_pack.input_file.stem()));

         const io::path lvl_path =
            io::compose_path(lvl_output_directory,
                             tool.level_pack.input_file.stem(), ".lvl");

         if (not io::remove(lvl_path) and io::exists(lvl_path)) {
            context.feedback.warnings.push_back({
               .file = lvl_path,
               .tool = "Clean",
               .message = "Failed to delete file.",
            });
         }
      }
   }

   if (context.project.load_active) {
      clean_directory(fmt::format(R"(_BUILD\Load\MUNGED\{})", platform),
                      project_directory, context.feedback);
      clean_directory(fmt::format(R"(_LVL_{}\load)", platform),
                      project_directory, context.feedback);
   }

   if (context.project.shell_active) {
      clean_directory(fmt::format(R"(_BUILD\Shell\MUNGED\{})", platform),
                      project_directory, context.feedback);
      clean_directory(fmt::format(R"(_LVL_{}\Movies)", platform),
                      project_directory, context.feedback);

      const io::path lvl_output_directory =
         io::compose_path(project_directory, fmt::format("_LVL_{}", platform));

      for (const tool& tool : context.project.config.shell_pack) {
         if (tool.type != tool_type::level_pack) continue;
         if (tool.level_pack.input_file.empty() or
             not string::iequals(tool.level_pack.input_file.extension(),
                                 ".req")) {
            continue;
         }

         context.feedback.print_output(
            fmt::format("Cleaning _LVL_{}\\{}.lvl", platform,
                        tool.level_pack.input_file.stem()));

         const io::path lvl_path =
            io::compose_path(lvl_output_directory,
                             tool.level_pack.input_file.stem(), ".lvl");

         if (not io::remove(lvl_path) and io::exists(lvl_path)) {
            context.feedback.warnings.push_back({
               .file = lvl_path,
               .tool = "Clean",
               .message = "Failed to delete file.",
            });
         }
      }
   }

   for (const project_child& world : context.project.worlds) {
      if (not world.active) continue;

      clean_directory(fmt::format(R"(_BUILD\Worlds\{}\MUNGED\{})", world.name, platform),
                      project_directory, context.feedback);

      clean_directory(fmt::format(R"(_LVL_{}\{})", platform, world.name),
                      project_directory, context.feedback);
   }

   for (const project_child& side : context.project.sides) {
      if (not side.active) continue;
      clean_directory(fmt::format(R"(_BUILD\Sides\{}\MUNGED\{})", side.name, platform),
                      project_directory, context.feedback);
      clean_directory(fmt::format(R"(_LVL_{}\FPM\{})", platform, side.name),
                      project_directory, context.feedback);

      for (const io::directory_entry& entry :
           io::directory_iterator{io::compose_path(context.project.directory,
                                                   fmt::format(R"(Sides\{})",
                                                               side.name)),
                                  false}) {
         if (not entry.is_file) continue;
         if (not string::iequals(entry.path.extension(), ".req")) continue;

         context.feedback.print_output(fmt::format(R"(Cleaning _LVL_{}\side\{}.lvl)",
                                                   platform, entry.path.stem()));

         const io::path lvl_path =
            io::compose_path(context.project.directory,
                             fmt::format(R"(_LVL_{}\side\{}.lvl)", platform,
                                         entry.path.stem()));

         if (not io::remove(lvl_path) and io::exists(lvl_path)) {
            context.feedback.warnings.push_back({
               .file = lvl_path,
               .tool = "Clean",
               .message = "Failed to delete file.",
            });
         }
      }
   }

   if (context.project.sound_active) {
      for (const project_child_sound_shared& shared_sound : context.project.sound_shared) {
         clean_directory(fmt::format(R"(_BUILD\Sound\{}\MUNGED\{})",
                                     shared_sound.name, platform),
                         project_directory, context.feedback);

         {
            const io::path lvl_path =
               io::compose_path(context.project.directory,
                                fmt::format(R"(_LVL_{}\sound\{}.lvl)", platform,
                                            shared_sound.name));

            if (not io::remove(lvl_path) and io::exists(lvl_path)) {
               context.feedback.warnings.push_back({
                  .file = lvl_path,
                  .tool = "Clean",
                  .message = "Failed to delete file.",
               });
            }
         }

         if (shared_sound.localized) {
            for (const project_sound_localization& localization :
                 context.project.sound_localizations) {
               const io::path lvl_path =
                  io::compose_path(context.project.directory,
                                   fmt::format(R"(_LVL_{}\sound\{}\{}.lvl)", platform,
                                               localization.output_directory,
                                               shared_sound.name));

               if (not io::remove(lvl_path) and io::exists(lvl_path)) {
                  context.feedback.warnings.push_back({
                     .file = lvl_path,
                     .tool = "Clean",
                     .message = "Failed to delete file.",
                  });
               }
            }
         }
      }

      for (const project_child_sound_world& world : context.project.sound_worlds) {
         if (not world.active) continue;

         clean_directory(fmt::format(R"(_BUILD\Sound\worlds\{}\MUNGED\{})",
                                     world.name, platform),
                         project_directory, context.feedback);

         {
            const io::path lvl_path =
               io::compose_path(context.project.directory,
                                fmt::format(R"(_LVL_{}\sound\{}.lvl)", platform,
                                            world.name));

            if (not io::remove(lvl_path) and io::exists(lvl_path)) {
               context.feedback.warnings.push_back({
                  .file = lvl_path,
                  .tool = "Clean",
                  .message = "Failed to delete file.",
               });
            }
         }

         if (world.localized) {
            for (const project_sound_localization& localization :
                 context.project.sound_localizations) {
               const io::path lvl_path =
                  io::compose_path(context.project.directory,
                                   fmt::format(R"(_LVL_{}\sound\{}\{}.lvl)", platform,
                                               localization.output_directory,
                                               world.name));

               if (not io::remove(lvl_path) and io::exists(lvl_path)) {
                  context.feedback.warnings.push_back({
                     .file = lvl_path,
                     .tool = "Clean",
                     .message = "Failed to delete file.",
                  });
               }
            }
         }
      }

      if (context.project.sound_common_bank and
          string::iequals(context.platform, "PC")) {
         const io::path lvl_path =
            io::compose_path(context.project.directory,
                             fmt::format(R"(_LVL_{}\sound\common.bnk)", platform));

         if (not io::remove(lvl_path) and io::exists(lvl_path)) {
            context.feedback.warnings.push_back({
               .file = lvl_path,
               .tool = "Clean",
               .message = "Failed to delete file.",
            });
         }
      }
   }

   context.feedback.print_output("Clean Finished");
   context.feedback.print_output(fmt::format("Time Taken: {:.3f}s", timer.elapsed()));

   return {
      .warnings = std::move(context.feedback.warnings),
      .errors = std::move(context.feedback.errors),
   };
}

}

struct manager::impl {
   explicit impl(async::thread_pool& thread_pool) : _thread_pool{thread_pool} {}

   void open_project(const io::path& project_directory) noexcept
   {
      _project = {.directory = project_directory};

      for (const io::directory_entry& entry :
           io::directory_iterator{io::compose_path(project_directory, "Worlds"), false}) {
         if (not entry.is_directory) continue;

         if (string::iequals(entry.path.stem(), "Common")) {
            _project.worlds.insert(_project.worlds.begin(),
                                   {.name = std::string{entry.path.stem()}});
         }
         else {
            _project.worlds.push_back({.name = std::string{entry.path.stem()}});
         }
      }

      for (const io::directory_entry& entry :
           io::directory_iterator{io::compose_path(project_directory, "Sides"), false}) {
         if (not entry.is_directory) continue;

         if (string::iequals(entry.path.stem(), "Common")) {
            _project.sides.insert(_project.sides.begin(),
                                  {.name = std::string{entry.path.stem()}});
         }
         else {
            _project.sides.push_back({.name = std::string{entry.path.stem()}});
         }
      }

      for (const io::directory_entry& entry :
           io::directory_iterator{io::compose_path(project_directory, "Sound"), false}) {
         if (not entry.is_directory) continue;

         if (not string::iequals(entry.path.stem(), "worlds") and
             not string::iequals(entry.path.stem(), "International")) {
            _project.sound_shared.push_back({.name = std::string{entry.path.stem()}});
         }
      }

      for (const io::directory_entry& entry :
           io::directory_iterator{io::compose_path(project_directory, R"(Sound\worlds)"),
                                  false}) {
         if (not entry.is_directory) continue;

         _project.sound_worlds.push_back({.name = std::string{entry.path.stem()}});
      }
   }

   void start_munge() noexcept
   {
      if (_munge_task.valid() and not _munge_task.ready()) {
         return;
      }

      _standard_output.clear();
      _standard_error.clear();
      _report = {};
      _munge_task = _thread_pool.exec(
         async::task_priority::low,
         [this, context = munge_context{
                   .platform = "PC",
                   .project = _project,
                   .feedback = munge_feedback{_standard_output, _standard_error},
                }] {
            try {
               return run_munge(context);
            }
            catch (os::process_launch_error& e) {
               std::string message =
                  fmt::format("Failed to launch process!\n{}\nEnsure your "
                              "ToolsFL\\bin directory is configured correctly.",
                              e.what());

               _standard_output.write(message);

               return report{
                  .errors =
                     {
                        {.file = "", .tool = "Munge", .message = std::move(message)},
                     },
               };
            }
            catch (std::exception& e) {
               std::string message =
                  fmt::format("Unexpected error occured while munging!\n{}", e.what());

               _standard_output.write(message);

               return report{
                  .errors =
                     {
                        {.file = "", .tool = "Munge", .message = std::move(message)},
                     },
               };
            }
         });
   }

   void start_clean() noexcept
   {
      if (_munge_task.valid() and not _munge_task.ready()) {
         return;
      }

      _standard_output.clear();
      _standard_error.clear();
      _report = {};
      _munge_task =
         _thread_pool.exec(async::task_priority::low,
                           [this, context = munge_context{
                                     .platform = "PC",
                                     .project = _project,
                                     .feedback = munge_feedback{_standard_output, _standard_error},
                                  }] { return run_clean(context); });
   }

   auto get_munge_report() noexcept -> const report&
   {
      if (_munge_task.valid() and _munge_task.ready()) {
         _report = _munge_task.get();
         _munge_task = {};
      }

      return _report;
   }

   bool is_busy() const noexcept
   {
      return _munge_task.valid() and not _munge_task.ready();
   }

   auto view_standard_output_lines() noexcept -> std::span<const std::string_view>
   {
      return _standard_output.view_lines();
   }

   auto view_standard_error_lines() noexcept -> std::span<const std::string_view>
   {
      return _standard_error.view_lines();
   }

   auto get_project() noexcept -> project&
   {
      return _project;
   }

private:
   project _project;

   report _report;
   output _standard_output;
   output _standard_error;

   async::task<report> _munge_task;

   async::thread_pool& _thread_pool;
};

manager::manager(async::thread_pool& thread_pool) : impl{thread_pool} {}

manager::~manager() = default;

void manager::open_project(const io::path& project_directory) noexcept
{
   return impl->open_project(project_directory);
}

void manager::start_munge() noexcept
{
   impl->start_munge();
}

void manager::start_clean() noexcept
{
   impl->start_clean();
}

auto manager::get_munge_report() noexcept -> const report&
{
   return impl->get_munge_report();
}

bool manager::is_busy() const noexcept
{
   return impl->is_busy();
}

auto manager::view_standard_output_lines() noexcept -> std::span<const std::string_view>
{
   return impl->view_standard_output_lines();
}

auto manager::view_standard_error_lines() noexcept -> std::span<const std::string_view>
{
   return impl->view_standard_error_lines();
}

auto manager::get_project() noexcept -> project&
{
   return impl->get_project();
}

}