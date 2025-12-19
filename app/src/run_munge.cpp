#include "run_munge.hpp"

#include "async/thread_pool.hpp"

#include "munge/manager.hpp"
#include "munge/message.hpp"
#include "munge/project.hpp"

#include <fmt/format.h>

#include <stdlib.h>

namespace we {

namespace {

constexpr std::string_view usage_instructions = R"(WorldEdit Munge Runner Usage Instructions

-------------|-------------------------------------------------------------------------------------------------
-munge       | Munge the project.
-clean       | Clean the project.
-------------|-------------------------------------------------------------------------------------------------
-project     | <path> | Set the path to the project. Without this the current directory will be used.
-toolsfl_bin | <path> | Set the path to ToolsFL\bin. This is required if missing from the saved project config.
-------------|-------------------------------------------------------------------------------------------------
-deploy      | <path> | Set the path to deploy the munged project to. This is optional.
-------------|-------------------------------------------------------------------------------------------------
-all         | Process everything.
-none        | Process nothing, turn individual items on using flags.
-config      | Process using the saved project config from WorldEdit.
-------------|-------------------------------------------------------------------------------------------------
-addme       | Process addme.
-common      | Process common.
-load        | Process load.
-shell       | Process shell.
-sides       | Process sides.
-worlds      | Process worlds.
-sounds      | Process sounds.
-------------|-------------------------------------------------------------------------------------------------
)";

enum class run_type { munge, clean };

template<typename T>
void activate(std::vector<T>& children) noexcept
{
   for (T& child : children) child.active = true;
}

template<typename T>
void deactivate(std::vector<T>& children) noexcept
{
   for (T& child : children) child.active = false;
}

template<typename T>
bool any_active(const std::vector<T>& children) noexcept
{
   for (const T& child : children) {
      if (child.active) return true;
   }

   return false;
}

bool open_project(utility::command_line command_line, munge::manager& manager) noexcept
{
   const io::path current_directory = io::current_directory();

   if (std::string_view project =
          command_line.get_or("-project", current_directory.string_view());
       not project.empty()) {
      manager.open_project(project);

      fmt::println("Opened Project: {}", project);
   }
   else {
      fmt::println(
         "Missing Project path and unable to use Current Directory "
         "as Project. Use -project to manually set the project path.");

      return false;
   }

   if (std::string_view toolsfl_bin = command_line.get_or("-toolsfl_bin", "");
       not toolsfl_bin.empty()) {
      manager.get_project().config.toolsfl_bin_path = toolsfl_bin;

      fmt::println("Set ToolsFL\\bin Path: {}", toolsfl_bin);
   }

   if (manager.get_project().config.toolsfl_bin_path.empty()) {
      fmt::println("ToolsFL\\bin Path is not set! Use -toolsfl_bin to set it.");

      return false;
   }

   fmt::println("ToolsFL\\bin Path: {}",
                manager.get_project().config.toolsfl_bin_path.c_str());

   if (not io::exists(manager.get_project().config.toolsfl_bin_path)) {
      fmt::println("ToolsFL\\bin Path does not exist.");

      return false;
   }

   munge::project& project = manager.get_project();

   if (command_line.get_flag("-all")) {
      project.addme_active = true;
      project.common_active = true;
      project.load_active = true;
      project.shell_active = true;
      project.sound_active = true;

      activate(project.sides);
      activate(project.worlds);
      activate(project.sound_worlds);
   }
   else if (command_line.get_flag("-none")) {
      project.addme_active = false;
      project.common_active = false;
      project.load_active = false;
      project.shell_active = false;
      project.sound_active = false;

      deactivate(project.sides);
      deactivate(project.worlds);
      deactivate(project.sound_worlds);
   }
   else if (command_line.get_flag("-config")) {
      // Use loaded config as is.
   }
   else {
      project.addme_active = true;
      project.common_active = true;
      project.load_active = false;
      project.shell_active = false;
      project.sound_active = true;

      activate(project.sides);
      activate(project.worlds);
      activate(project.sound_worlds);
   }

   project.deploy = false;

   if (command_line.get_flag("-addme")) project.addme_active = true;
   if (command_line.get_flag("-common")) project.common_active = true;
   if (command_line.get_flag("-load")) project.load_active = true;
   if (command_line.get_flag("-shell")) project.shell_active = true;

   if (command_line.get_flag("-sides")) activate(project.sides);
   if (command_line.get_flag("-worlds")) activate(project.worlds);

   if (command_line.get_flag("-sounds")) {
      project.sound_active = true;

      activate(project.sound_worlds);
   }

   fmt::println("Loaded Project");

   fmt::println("Deploy: {:s}", project.deploy);
   fmt::println("Addme: {:s}", project.addme_active);
   fmt::println("Common: {:s}", project.common_active);
   fmt::println("Load: {:s}", project.load_active);
   fmt::println("Shell: {:s}", project.shell_active);

   const bool sides_active = any_active(project.sides);

   fmt::println("Sides: {:s}", sides_active);

   if (sides_active) {
      for (const munge::project_child& side : project.sides) {
         fmt::println("\t{}: {:s}", side.name, side.active);
      }
   }

   const bool worlds_active = any_active(project.worlds);

   fmt::println("Worlds: {:s}", worlds_active);

   if (worlds_active) {
      for (const munge::project_child& world : project.worlds) {
         fmt::println("\t{}: {:s}", world.name, world.active);
      }
   }

   fmt::println("Sound: {:s}", project.sound_active);

   if (project.sound_active) {
      for (const munge::project_child_sound_world& world : project.sound_worlds) {
         fmt::println("\t{}: {:s}", world.name, world.active);
      }
   }

   return true;
}

int run_generic(utility::command_line command_line, run_type run_type) noexcept
{
   std::shared_ptr<async::thread_pool> thread_pool = async::thread_pool::make();
   munge::manager manager{*thread_pool};

   if (command_line.get_flag("-?") or command_line.get_flag("/?")) {
      fmt::println("{}", usage_instructions);

      return EXIT_SUCCESS;
   }

   if (not open_project(command_line, manager)) {
      return EXIT_FAILURE;
   }

   if (run_type == run_type::munge) {
      fmt::println("Starting munge...");

      manager.start_munge(command_line.get_or("-deploy", ""));
   }
   else if (run_type == run_type::clean) {
      fmt::println("Starting clean...");

      manager.start_clean();
   }

   manager.wait_for_idle();

   const munge::report& report = manager.get_munge_report();

   fmt::println("Printing Standard Output");
   fmt::println("------------------------");

   for (std::string_view line : manager.view_standard_output_lines()) {
      fmt::println("{}", line);
   }

   fmt::println("Printing Standard Error");
   fmt::println("-----------------------");

   for (std::string_view line : manager.view_standard_error_lines()) {
      fmt::println(stderr, "{}", line);
   }

   fmt::println("Printing structured Warnings and Errors");
   fmt::println("---------------------------------------");

   for (const munge::message& message : report.warnings) {
      fmt::println("WARNINGS[{} {}]:{}", message.tool,
                   message.file.string_view(), message.message);
   }

   for (const munge::message& message : report.errors) {
      fmt::println(stderr, "ERROR[{} {}]:{}", message.tool,
                   message.file.string_view(), message.message);
   }

   return report.errors.empty() ? EXIT_SUCCESS : EXIT_FAILURE;
}

}

int run_munge(utility::command_line command_line) noexcept
{
   return run_generic(command_line, run_type::munge);
}

int run_clean(utility::command_line command_line) noexcept
{
   return run_generic(command_line, run_type::clean);
}

}
