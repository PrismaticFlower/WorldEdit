#include "executor.hpp"

#include "model_munge/error.hpp"
#include "odf_munge/error.hpp"
#include "texture_munge/error.hpp"

#include "assets/option_file.hpp"

#include "io/read_file.hpp"

#include "utility/string_icompare.hpp"

#include <fmt/format.h>

#include <forward_list>

using we::string::iequals;

namespace we::munge {

namespace {

struct queued_munge {
   io::path path;
   async::task<void> task;
};

}

void execute_builtin_munge(const execute_builtin_context& context,
                           const tool_context& tool_context) noexcept
{
   std::forward_list<std::vector<assets::option>> directory_options;
   std::vector<queued_munge> munge_tasks;

   const std::string input_extension = fmt::format(".{}", context.input_extension);
   const std::string output_extension = fmt::format(".{}", context.output_extension);
   const std::string directory_option_extension =
      fmt::format("{}.option", context.input_extension);
   const std::string option_extension =
      fmt::format(".{}.option", context.input_extension);

   try {
      std::vector<assets::option>* folder_options = nullptr;

      {
         const io::path root_options_path =
            io::compose_path(tool_context.source_path, directory_option_extension);

         try {
            folder_options = &directory_options.emplace_front(
               assets::parse_options(io::read_file_to_string(root_options_path)));
         }
         catch (std::exception&) {
            folder_options = nullptr;
         }
      }

      for (io::directory_iterator it = io::directory_iterator{tool_context.source_path};
           it != it.end(); ++it) {
         const io::directory_entry& entry = *it;

         if (entry.is_file and iequals(entry.path.extension(), input_extension)) {
            const io::path platform_file_path{
               fmt::format("{}\\{}\\{}", entry.path.parent_path(),
                           tool_context.platform, entry.path.filename())};

            if (io::exists(platform_file_path)) continue;

            const uint64 output_last_write_time = io::get_last_write_time(
               io::compose_path(tool_context.output_path, entry.path.stem(),
                                output_extension));
            const uint64 option_file_last_write_time = io::get_last_write_time(
               io::make_path_with_new_extension(entry.path, option_extension));

            if (entry.last_write_time < output_last_write_time and
                option_file_last_write_time < output_last_write_time) {
               continue;
            }

            munge_tasks.emplace_back(
               entry.path,
               tool_context.thread_pool
                  .exec(async::task_priority::low, [input_file_path = entry.path,
                                                    folder_options = folder_options,
                                                    &context, &tool_context] {
                     context.execute_munge(input_file_path,
                                           folder_options
                                              ? *folder_options
                                              : std::vector<assets::option>{},
                                           tool_context);
                  }));
         }
         else if (entry.is_directory) {
            if (iequals(entry.path.stem(), "PC") or iequals(entry.path.stem(), "PS2") or
                iequals(entry.path.stem(), "XBOX")) {
               if (not iequals(entry.path.stem(), tool_context.platform)) {
                  it.skip_directory();
               }
            }

            {
               const io::path options_path =
                  io::compose_path(tool_context.source_path, directory_option_extension);

               try {
                  folder_options = &directory_options.emplace_front(
                     assets::parse_options(io::read_file_to_string(options_path)));
               }
               catch (std::exception&) {
                  folder_options = nullptr;
               }
            }
         }
      }
   }
   catch (std::exception& e) {
      tool_context.feedback.add_error(
         {.tool = std::string{context.tool_name},
          .message = fmt::format("Unknown error occured while enumerating "
                                 "models. Unhelpful Message: {}",
                                 e.what())});
   }

   for (std::ptrdiff_t i = std::ssize(munge_tasks) - 1; i >= 0; --i) {
      try {
         munge_tasks[i].task.get();
      }
      catch (model_error& e) {
         tool_context.feedback.add_error({.file = munge_tasks[i].path,
                                          .tool = std::string{context.tool_name},
                                          .message = get_descriptive_message(e)});
      }
      catch (odf_error& e) {
         tool_context.feedback.add_error({.file = munge_tasks[i].path,
                                          .tool = std::string{context.tool_name},
                                          .message = get_descriptive_message(e)});
      }
      catch (texture_error& e) {
         tool_context.feedback.add_error({.file = munge_tasks[i].path,
                                          .tool = std::string{context.tool_name},
                                          .message = get_descriptive_message(e)});
      }
      catch (std::exception& e) {
         tool_context.feedback.add_error(
            {.file = munge_tasks[i].path,
             .tool = std::string{context.tool_name},
             .message = fmt::format("Unknown error occured while munging "
                                    "model. Unhelpful Message: {}",
                                    e.what())});
      }
   }
}

}